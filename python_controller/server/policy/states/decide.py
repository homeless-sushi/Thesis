import itertools
from typing import Tuple, List, Dict

from . import state
from . import wait_decide

import policy.utility.system_state_utility as system_state_utility
import policy.utility.system_config as system_config
import policy.utility.system_power as system_power
import policy.utility.app_throughput as app_throughput
from policy.utility.app_knowledge import AppKnowledge

import model.system
from model.system_state import SystemState
import model.knowledge 
from model.knowledge import Column
import model.prediction as prediction
from model.app import App, Location

class Decide(state.State) :

    def __init__(self, prev_prescribed: SystemState, curr_measured: SystemState, reset_power : bool = False):
        super().__init__(prev_prescribed, curr_measured)

        if reset_power :
            system_power.reset_coefficient()

        self.apps_knowledge : Dict[int, AppKnowledge] = {}

    def next(self, new_measured: SystemState) -> state.State :

        # Applications enter or exit the system
        new_pids, deleted_pids = (
            system_state_utility.compare_system_states(self.curr_prescribed, new_measured)
        )
        if len(new_pids) > 0 or len(deleted_pids) > 0 :
            return Decide(self.curr_prescribed, new_measured, reset_power=True)
        
        # Update power coefficient
        system_power.update_coefficient(self.curr_prescribed.power, new_measured.power)
        # Update throughput coeffiecients
        app_throughput.update_coefficients(self.curr_prescribed, new_measured)

        # Wait 
        return wait_decide.WaitDecide(self.curr_prescribed, new_measured)
    
    def handle(self):

        # update apps_throughput
        app_throughput.update_apps(self.prev_prescribed, self.curr_measured)

        # fill apps_knowledge
        for pid, throughput in app_throughput.apps_throughput.items() :
            self.apps_knowledge[pid] = AppKnowledge(throughput)

        # new apps
        new_pids, deleted_pids = (
            system_state_utility.compare_system_states(self.prev_prescribed, self.curr_measured)
        )

        # if all new apps were UNVIABLE return the old configuration
        unviable_new_apps = [pid for pid in new_pids if self.apps_knowledge[pid].suggested == Location.UNVIABLE]
        if len(unviable_new_apps) == len(new_pids) and len(deleted_pids) == 0 :
            self.curr_prescribed = system_state_utility.copy_system_state(self.prev_prescribed)
            return
        
        viable_pids = list(self.curr_measured.current_apps.keys())
        for pid in unviable_new_apps :
            viable_pids.remove[pid]
        
        # generate all possible CPU/GPU assignements
        cpu_gpu_assignments : List[Tuple[List[int], List[int]]] = generate_assignments(viable_pids)
        # filter impossible CPU/GPU assignments
        cpu_gpu_assignments = list(filter(lambda config: is_possible(self.apps_knowledge, config), cpu_gpu_assignments))
        
        # find the minimum power configuration
        min_power : float = float("inf")
        min_pow_configuration : system_config.SystemConfig = system_config.SystemConfig()
        for cpu_gpu_assignment in cpu_gpu_assignments:
            try:
                # given the configuration, select the minimum resources
                candidate : system_config.SystemConfig = check_assignment(
                    self.apps_knowledge, cpu_gpu_assignment[0], cpu_gpu_assignment[1]
                )
            except system_config.NoViableConfigException as e:
                continue
            
            candidate_power : float = candidate.compute_total_power()
            if candidate_power < min_power :
                min_power = candidate_power
                min_pow_configuration = candidate

        # if the minimum power violates the power constraint, return at the previous state
        if system_power.check_violation(min_power) :
            self.curr_prescribed = SystemState()
            return
        else :
            new_state = system_state_utility.copy_system_state(self.curr_measured)
            system_config.update_system_state(min_pow_configuration, new_state)
            self.curr_prescribed = new_state
            return
    
def generate_assignments(pids: List[int]) -> List[Tuple[List[int], List[int]]]:

    # Generate combinations with up to 2 PIDs on GPU
    possbile_gpu_pids : List[List[int]] = []
    for n_gpu_apps in range(min(model.system.max_gpu_apps, len(pids)) + 1):  
        gpu_pids = map(list, itertools.combinations(pids, n_gpu_apps))
        possbile_gpu_pids.extend(gpu_pids)

    # The remaining PIDs are placed on the CPU
    possbile_cpu_pids : List[List[int]] = []
    for gpu_pids in possbile_gpu_pids:
        possbile_cpu_pids.append([pid for pid in pids if pid not in gpu_pids])

    return list(zip(possbile_cpu_pids, possbile_gpu_pids))

def is_possible (
    apps_knowledge : Dict[int, AppKnowledge],
    configuration : Tuple[List[int], List[int]]
) -> bool:

    cpu_pids : List[int] = configuration[0]
    for cpu_pid in cpu_pids :
        if apps_knowledge[cpu_pid].suggested == Location.GPU :
            return False
    
    gpu_pids : List[int] = configuration[1]
    for gpu_pid in gpu_pids :
        if apps_knowledge[gpu_pid].suggested == Location.CPU :
            return False
    
    return True

def check_assignment(
    apps_knowledge : Dict[int, AppKnowledge],
    cpu_pids : List[int], gpu_pids : List[int]
) -> system_config.SystemConfig:

    # find a gpu configuration for every cpu frequency
    try:
        gpu_configurations : List[system_config.SystemConfig] = check_gpu(apps_knowledge, gpu_pids)
    except system_config.NoViableConfigException as e:
        raise system_config.NoViableConfigException
    
    # find a cpu configuration with the lowest possible frequency.
    # The minimum frequency is the minimum possible cpu frequency 
    # found for the gpu configuration
    min_cpu_freq : int = gpu_configurations[0].cpu_freq
    occupied_cores : int = len(gpu_pids)
    try:
        cpu_configuration : system_config.SystemConfig = check_cpu(
            apps_knowledge, cpu_pids, min_cpu_freq, occupied_cores
        )
    except system_config.NoViableConfigException as e:
        raise system_config.NoViableConfigException

    # find the corresponding gpu configuration
    gpu_configuration : system_config.SystemConfig = None
    for conf in gpu_configurations:
        if conf.cpu_freq == cpu_configuration.cpu_freq:
            gpu_configuration = conf
            break
    
    # merge the two configurations
    result_configuration : system_config.SystemConfig = system_config.SystemConfig(
        gpu_configuration.cpu_freq, 
        gpu_configuration.gpu_freq
    )
    result_configuration.cpu_apps_resources = cpu_configuration.cpu_apps_resources
    result_configuration.gpu_apps_resources = gpu_configuration.gpu_apps_resources

    return result_configuration

def check_gpu(
    apps_knowledge : Dict[int, AppKnowledge],
    gpu_pids : List[int]
) -> List[system_config.SystemConfig]:

    configurations_list : List[system_config.SystemConfig] = []

    if len(gpu_pids) == 0:

        for cpu_freq in model.system.cpu_freqs :
            configurations_list.append(system_config.SystemConfig(
                cpu_freq = cpu_freq,
                gpu_freq = model.system.gpu_freqs[0]
            ))
    
    elif len(gpu_pids) == 1:

        pid = gpu_pids[0]
        configs = apps_knowledge[pid].viable_gpu_configs_corrected
        if configs.empty :
            raise system_config.NoViableConfigException
    
        sorted_configs = configs.sort_values(by=[Column.F_CPU, Column.F_GPU, Column.GPU_W])
        min_confings = sorted_configs.groupby(Column.F_CPU).first().reset_index()
        for _, row in min_confings.iterrows() :
            candidate_configuration = system_config.SystemConfig(
                cpu_freq = int(row[Column.F_CPU]),
                gpu_freq = int(row[Column.F_GPU])
            )
            candidate_configuration.gpu_apps_resources = system_config.AppResources(pid, Location.GPU, row)
            configurations_list.append(candidate_configuration)

    elif len(gpu_pids) == 2:

        pid_A = gpu_pids[0]
        configs_A = apps_knowledge[pid_A].viable_gpu_configs_plain
        min_thr_A = app_throughput.apps_throughput[pid_A].min_thr
        colocation_coeff_A = app_throughput.apps_throughput[pid_A].get_gpu_colocation_avg()

        pid_B = gpu_pids[1]
        min_thr_B = app_throughput.apps_throughput[pid_B].min_thr
        configs_B = apps_knowledge[pid_B].viable_gpu_configs_plain
        colocation_coeff_B = app_throughput.apps_throughput[pid_B].get_gpu_colocation_avg()

        configs = model.knowledge.viable_gpu_colocation(
            min_thr_A, configs_A, min_thr_B, configs_B, 
            colocation_coeff_A, colocation_coeff_B
        )
        if configs.empty :
            raise system_config.NoViableConfigException
        
        sorted_configs = configs.sort_values(by=[Column.F_CPU, Column.F_GPU, Column.GPU_W_INTERFERENCE])
        min_confings = sorted_configs.groupby(Column.F_CPU).first().reset_index()
        configurations_list = []
        for _, row in min_confings.iterrows() :
            candidate_configuration = system_config.SystemConfig(
                cpu_freq = int(row[Column.F_CPU]),
                gpu_freq = int(row[Column.F_GPU])
            )
            candidate_configuration.gpu_apps_resources = system_config.AppGpuColocationResources(pid_A, pid_B, row)
            configurations_list.append(candidate_configuration)
        
    return configurations_list

def check_cpu(
    apps_knowledge : Dict[int, AppKnowledge], 
    cpu_pids : List[int], cpu_min_freq : int = model.system.cpu_freqs[0], occupied_cores : int = 0
) -> system_config.SystemConfig :

    if len(cpu_pids) + occupied_cores > model.system.n_cpu_cores :
        raise system_config.NoViableConfigException

    data = {}
    for pid in cpu_pids:
        viable_configs = apps_knowledge[pid].viable_cpu_configs_corrected
        sorted_df = viable_configs.sort_values(by=[Column.F_CPU, Column.CPU_N_CORES, Column.THR])
        min_cores_df = sorted_df.groupby(Column.F_CPU).first().reset_index()

        data[pid] = min_cores_df
    
    freqs = list(cpu_freq for cpu_freq in model.system.cpu_freqs if cpu_freq >= cpu_min_freq)
    curr_freq = freqs[0]
    valid_freq_and_cores = False
    
    # find the minimum frequency that can accomodate all apps
    for freq in freqs:
        required_cores = occupied_cores
        valid_freq = True

        for pid in cpu_pids:
            min_cores_df = data[pid]
            freq_df = min_cores_df[min_cores_df[Column.F_CPU] == freq]
            if freq_df.empty : # if the app cannot run at the frequency
                valid_freq = False
                break
            else : # if the app can run at the frequency, count the cores needed
                required_cores += freq_df.iloc[0][Column.CPU_N_CORES]
    
        if valid_freq and required_cores <= model.system.n_cpu_cores : 
            valid_freq_and_cores = True
            curr_freq = freq
            break

    if not valid_freq_and_cores:
        raise system_config.NoViableConfigException
    
    configuration : system_config.SystemConfig = system_config.SystemConfig(curr_freq) 
    for pid in cpu_pids:
        min_cores_df = data[pid]
        freq_row = min_cores_df[min_cores_df[Column.F_CPU] == curr_freq].iloc[0]
        app_resources : system_config.AppResources = system_config.AppResources(pid, Location.CPU, freq_row)
        configuration.cpu_apps_resources[pid] = app_resources
    
    return configuration