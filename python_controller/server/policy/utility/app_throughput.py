from typing import Union, List, Dict, Deque
from collections import deque

from model.system_state import SystemState
from model.knowledge import Column
import model.knowledge as knowledge
from model.app import Location

class AppThroughput :
    TOLERANCE : float = 0.05
    ROLLING_AVG_MEM : int = 5

    def __init__(self, pid, name, size, min_thr, is_approximate = False, min_precision = 100):
        
        #TODO remove duplicate information
        self.pid : int = pid
        self.name : str = name
        self.size : int = size
        self.min_thr : float = min_thr
        self.is_approximate : bool = is_approximate
        self.min_precision : int = min_precision

        self.cpu_rolling_avg : Deque[float] = deque(
            [1]*AppThroughput.ROLLING_AVG_MEM, maxlen=AppThroughput.ROLLING_AVG_MEM
        )

        self.gpu_rolling_avg : Deque[float] = deque(
            [1]*AppThroughput.ROLLING_AVG_MEM, maxlen=AppThroughput.ROLLING_AVG_MEM
        )

        self.gpu_colocation_rolling_avg : Deque[float] = deque(
            [1]*AppThroughput.ROLLING_AVG_MEM, maxlen=AppThroughput.ROLLING_AVG_MEM
        )

    def update_cpu_avg(self, predicted : float, measured : float) -> None :
        self.cpu_rolling_avg.append(measured/predicted)

    def update_gpu_avg(self, predicted : float, measured : float) -> None : 
        self.gpu_rolling_avg.append(measured/predicted)

    def update_gpu_colocation_avg(self, predicted : float, measured : float) -> None : 
        self.gpu_rolling_avg.append(measured/predicted)
    
    def get_cpu_avg(self) -> float :
        return sum(self.cpu_rolling_avg) / len(self.cpu_rolling_avg)

    def get_gpu_avg(self) -> float :
        return sum(self.gpu_rolling_avg) / len(self.gpu_rolling_avg)
    
    def get_gpu_colocation_avg(self) -> float :
        return sum(self.gpu_colocation_rolling_avg) / len(self.gpu_colocation_rolling_avg)


apps_throughput : Dict[int, AppThroughput] = {}

def update_apps(currPrescribed : SystemState, newMeasured : SystemState) -> None :

    # Add new apps
    new_pids : List[int] = []
    new_pids.extend(set(newMeasured.current_apps.keys()) - set(currPrescribed.current_apps.keys()))
    for new_pid in new_pids :
        apps_throughput[new_pid] = AppThroughput(
            newMeasured.current_apps[new_pid].pid,
            newMeasured.current_apps[new_pid].name,
            newMeasured.current_apps[new_pid].size,
            newMeasured.current_apps[new_pid].min_thr,
            is_approximate = newMeasured.current_apps[new_pid].is_approximate,
            min_precision = newMeasured.current_apps[new_pid].min_precision
        )

    # Remove old apps
    old_pids : List[int] = []
    old_pids.extend(set(currPrescribed.current_apps.keys()) - set(newMeasured.current_apps.keys()))
    for old_pid in old_pids :
        apps_throughput.pop(old_pid)

def update_coefficients(currPrescribed : SystemState, newMeasured : SystemState) -> None :

    # Check the same pids are
    set(newMeasured.current_apps.keys()) == set(currPrescribed.current_apps.keys())
    
    n_gpu_apps : int = len(newMeasured.get_gpu_pids())
    for pid in newMeasured.current_apps.keys() :
        
        # If app is running on the CPU
        if newMeasured.current_apps[pid].location == Location.CPU :
            apps_throughput[pid].update_cpu_avg(
                currPrescribed.current_apps[pid].curr_thr, newMeasured.current_apps[pid].curr_thr
            )
        
        elif n_gpu_apps == 1:
            apps_throughput[pid].update_gpu_avg(
                currPrescribed.current_apps[pid].curr_thr, newMeasured.current_apps[pid].curr_thr
            )

        else :
            apps_throughput[pid].update_gpu_colocation_avg(
                currPrescribed.current_apps[pid].curr_thr, newMeasured.current_apps[pid].curr_thr
            )

def check_violation(newMeasured : SystemState) -> bool :

    for app in newMeasured.current_apps.values() :

        if app.curr_thr < (app.min_thr * (1-AppThroughput.TOLERANCE)) :
            return True
        
    return False