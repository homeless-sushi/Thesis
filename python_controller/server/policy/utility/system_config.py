from typing import Union, List, Dict


import model.system
import model.knowledge
import model.prediction
from model.knowledge import Column
from model.app import App, Location
from model.system_state import SystemState

import pandas as pd

class AppResources :

    def __init__(self, pid : int, location : Location, row : pd.Series) :
        self.pid : int = pid
        self.location : Location = location

        self.approximate : bool = (Column.PRECISION in row)
        self.row : pd.Series = row

    def __getitem__(self, name):
        return self.row[name]
    
class AppGpuColocationResources :
    
    def __init__(self, pid_A : int, pid_B : int, row : pd.Series) :
        self.pid_A : int = pid_A
        self.pid_B : int = pid_B

        self.row : pd.Series = row
        self.approximate_A : bool = ((Column.PRECISION + Column.A) in row)
        self.approximate_B : bool = ((Column.PRECISION + Column.B) in row)
    
    def __getitem__(self, name):
        return self.row[name]

class SystemConfig :

    def __init__(self, cpu_freq : int = model.system.cpu_freqs[0], gpu_freq : int = model.system.gpu_freqs[0]):
        self.cpu_freq : int = cpu_freq
        self.gpu_freq : int = gpu_freq
        self.cpu_apps_resources : Dict[int, AppResources] = {}
        self.gpu_apps_resources : Union[None, AppResources, AppGpuColocationResources] = None

    def compute_total_power(self) -> float :

        total_power = model.system.cpu_idle_power[self.cpu_freq]
        for cpu_app_resources in self.cpu_apps_resources.values() :
            total_power += model.prediction.cpu_app_power(
                self.cpu_freq,
                cpu_app_resources[Column.KERNEL_FRACTION], 
                cpu_app_resources[Column.ACTIVE_FRACTION], 
                int(cpu_app_resources[Column.CPU_N_CORES])
            )

        if type(self.gpu_apps_resources) is AppResources :
            
            total_power -= model.system.cpu_idle_power[self.cpu_freq]
            total_power += model.prediction.gpu_app_power(
                self.cpu_freq,
                self.gpu_apps_resources[Column.KERNEL_FRACTION], 
                self.gpu_apps_resources[Column.ACTIVE_FRACTION],
                self.gpu_apps_resources[Column.GPU_W]
            )
        
        elif type(self.gpu_apps_resources) is AppGpuColocationResources :

            total_power -= model.system.cpu_idle_power[self.cpu_freq]
            total_power += model.prediction.gpu_apps_power(
                self.gpu_apps_resources[Column.F_CPU], 
                self.gpu_apps_resources[Column.KERNEL_FRACTION + Column.A], self.gpu_apps_resources[Column.ACTIVE_FRACTION + Column.A],
                self.gpu_apps_resources[Column.KERNEL_FRACTION + Column.B], self.gpu_apps_resources[Column.ACTIVE_FRACTION + Column.B],
                self.gpu_apps_resources[Column.GPU_W_INTERFERENCE]
            )

        return total_power 

class NoViableConfigException(Exception) :
    def __init__(self, message="No viable configuration"):
        self.message = message
        super().__init__(self.message)

def update_system_state(system_config : SystemConfig, system_state: SystemState) -> None:

    system_state.cpu_freq = system_config.cpu_freq
    system_state.gpu_freq = system_config.gpu_freq
    system_state.power = system_config.compute_total_power()

    cpu_config_pids : List[int] = [pid for pid in system_config.cpu_apps_resources.keys()]
    for pid in cpu_config_pids :
        system_state.current_apps[pid].location = Location.CPU
        system_state.current_apps[pid].cpu_cores = system_config.cpu_apps_resources[pid][Column.CPU_N_CORES]
        system_state.current_apps[pid].curr_thr = system_config.cpu_apps_resources[pid][Column.THR]

    gpu_config_pids : List[int] = []
    if type(system_config.gpu_apps_resources) is AppResources :

        pid = system_config.gpu_apps_resources.pid
        system_state.current_apps[pid].location = Location.GPU
        system_state.current_apps[pid].cpu_cores = 1
        system_state.current_apps[pid].curr_thr = system_config.gpu_apps_resources[Column.THR]
        gpu_config_pids.append(pid)

    elif type(system_config.gpu_apps_resources) is AppGpuColocationResources :

        pid_A = system_config.gpu_apps_resources.pid_A
        system_state.current_apps[pid_A].location = Location.GPU
        system_state.current_apps[pid_A].cpu_cores = 1
        system_state.current_apps[pid_A].curr_thr = system_config.gpu_apps_resources[Column.THR_INTERFERENCE + Column.A]
        gpu_config_pids.append(pid_A)

        pid_B = system_config.gpu_apps_resources.pid_B
        system_state.current_apps[pid_B].location = Location.GPU
        system_state.current_apps[pid_B].cpu_cores = 1
        system_state.current_apps[pid_B].curr_thr = system_config.gpu_apps_resources[Column.THR_INTERFERENCE + Column.B]
        gpu_config_pids.append(pid_B) 
    
    config_pids = cpu_config_pids + gpu_config_pids
    apps_to_remove = [pid for pid in system_state.current_apps.keys() if pid not in config_pids]
    for pid in apps_to_remove :
        system_state.current_apps.pop(pid)

def system_state_2_system_config(system_state : SystemState) -> SystemConfig :
    raise NotImplementedError

def system_config_2_system_state(system_config : SystemConfig) -> SystemState :
    raise NotImplementedError


