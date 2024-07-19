import itertools
from typing import Union, Optional, Tuple, List, Dict

from . import state
from . import decide
from . import wait_refine

import policy.utility.system_state_utility as system_state_utility
import policy.utility.system_config as system_config
import policy.utility.system_power as system_power
import policy.utility.app_throughput as app_throughput
from policy.utility.app_knowledge import AppKnowledge

import model.system
import model.knowledge
from model.knowledge import Column
from model.app import App, Location
from model.system_state import SystemState

class ConfigBuilder :
    """A class for generating possible BaseConfig.
    
    Attributes:
        base_cpu_config (BaseConfig) : the configuration containing the 
            cpu_freq and the AppResource for the not approximate applications
        possible_cpu_configs (Dict[int, List[AppResources]]) : a dictionary 
            that maps the approximate cpu applications' pids to possible 
            AppResources
        possible_gpu_configs (Dict[int, List[AppResources]]) : a dictionary 
            that maps the approximate gpu applications' pids to possible 
            AppResources
    """
    def __init__(self, base_config : system_config.SystemConfig) -> None :
        self.base_config : system_config.SystemConfig = base_config
        self.possibile_cpu_configs : Dict[int, List[system_config.AppResources]] = {}
        self.possibile_gpu_configs : Union[None, List[system_config.AppResources], List[system_config.AppGpuColocationResources]] = None

    class ConfigBuilderIterator:

        def __init__(self, 
            base_config : system_config.SystemConfig, 
            possibile_cpu_configs : Dict[int, List[system_config.AppResources]],
            possibile_gpu_configs : Union[None, List[system_config.AppResources], List[system_config.AppGpuColocationResources]]
        ) :
            self.base_config : system_config.SystemConfig 
            self.possibile_cpu_configs : Dict[int, List[system_config.AppResources]]
            self.possibile_gpu_configs : Union[None, List[system_config.AppResources], List[system_config.AppGpuColocationResources]]

            self.base_config = base_config
            self.possibile_cpu_configs = possibile_cpu_configs
            self.possibile_gpu_configs = possibile_gpu_configs

            self.additional_app_resources = None
            if self.possibile_gpu_configs is None :
                self.additional_app_resources = itertools.product(
                    *self.possibile_cpu_configs.values()
                )
            else :
                self.additional_app_resources = itertools.product(
                    *self.possibile_cpu_configs.values(),
                    self.possibile_gpu_configs
                )

        def __iter__(self) :
            return self

        def __next__(self) :
            additional_apps_resources = next(self.additional_app_resources)

            if self.possibile_gpu_configs is None :
                additional_cpu_apps_resources = additional_apps_resources
                additional_gpu_apps_resources = None
            else :
                additional_cpu_apps_resources = additional_apps_resources[:-1]
                additional_gpu_apps_resources = additional_apps_resources[-1]

            config : system_config.SystemConfig = system_config.SystemConfig(
                self.base_config.cpu_freq,
                self.base_config.gpu_freq
            )
            config.cpu_apps_resources = {pid: app_resource for pid, app_resource in self.base_config.cpu_apps_resources.items()}
            for additional_cpu_app_resources in additional_cpu_apps_resources:
                config.cpu_apps_resources[additional_cpu_app_resources.pid] = additional_cpu_app_resources
            config.gpu_apps_resources = additional_gpu_apps_resources

            return config

    def __iter__(self) -> ConfigBuilderIterator :
        return ConfigBuilder.ConfigBuilderIterator(
            self.base_config,
            self.possibile_cpu_configs,
            self.possibile_gpu_configs
        )

class Refine(state.State) :

    def __init__(self, prev_prescribed : SystemState, curr_measured: SystemState, reset_power : bool = False):
        super().__init__(prev_prescribed, curr_measured)

        if reset_power :
            system_power.reset_coefficient()

        self.apps_knowledge : Dict[int, AppKnowledge] = {}

    def next(self, new_measured : SystemState) -> state.State :
        # Applications enter or exit the system
        new_pids, deleted_pids = (
            system_state_utility.compare_system_states(self.curr_prescribed, new_measured)
        )
        if len(new_pids) > 0 or len(deleted_pids) > 0 :
            return decide.Decide(self.curr_prescribed, new_measured, reset_power=True)
        
        # Update power coefficient
        system_power.update_coefficient(self.curr_prescribed.power, new_measured.power)
        # Update throughput coeffiecients
        app_throughput.update_coefficients(self.curr_prescribed, new_measured)
            
        # Wait
        return wait_refine.WaitRefine(self.curr_prescribed, new_measured)
    
    def handle(self) -> SystemState:

        # fill apps_knowledge
        for pid, throughput in app_throughput.apps_throughput.items() :
            self.apps_knowledge[pid] = AppKnowledge(throughput)
                
        # generate one ConfigBuilder per (cpu_freq, gpu_freq)
        config_builders : Dict[Tuple[int, int], ConfigBuilder] = generate_config_builders(
            self.curr_measured, self.apps_knowledge)
        # find the pareto configs and the associated power
        pareto_configs : List[Tuple[system_config.SystemConfig, float]] = generate_pareto_configs(
            self.curr_measured, self.apps_knowledge, config_builders)
        # sort them according to their power consumption
        pareto_configs = sorted(pareto_configs, key = lambda x: x[1])
        lowest_power_config : system_config.SystemConfig = pareto_configs[0][0]

        new_state : SystemState = system_state_utility.copy_system_state(self.curr_measured)
        system_config.update_system_state(lowest_power_config, new_state)
        self.curr_prescribed = new_state
        return
    
def generate_config_builders(
    system_state: SystemState, apps_knowledge: Dict[int, AppKnowledge]
) -> Dict[Tuple[int, int], ConfigBuilder] :

    cpu_pids : List[int] = [
        pid 
        for pid in system_state.current_apps.keys()
        if system_state.current_apps[pid].location == Location.CPU
    ]
    cpu_pids_approximate : List[int] = [pid for pid in cpu_pids if system_state.current_apps[pid].is_approximate]
    cpu_pids_no_approximate : List[int] = [pid for pid in cpu_pids if not system_state.current_apps[pid].is_approximate]

    gpu_pids : List[int] = [
        pid 
        for pid in system_state.current_apps.keys()
        if system_state.current_apps[pid].location == Location.GPU
    ]
    approximate_on_gpu : bool = any(system_state.current_apps[pid].is_approximate for pid in gpu_pids) #TODO USELESS

    config_builders : Dict[Tuple[int, int], ConfigBuilder] = {}

    candidate_cpu_freqs : List[int] = reversed([cpu_freq for cpu_freq in model.system.cpu_freqs if cpu_freq >= system_state.cpu_freq])
    if approximate_on_gpu :
        candidate_gpu_freqs : List[int] = reversed([gpu_freq for gpu_freq in model.system.gpu_freqs if gpu_freq >= system_state.gpu_freq])
    else :
        candidate_gpu_freqs : List[int] = [model.system.gpu_freqs]

    freqs_combinations : List[Tuple[int, int]] = itertools.product(candidate_cpu_freqs, candidate_gpu_freqs)
    for cpu_freq, gpu_freq in freqs_combinations:

        # create the base config by reducing the resources of current cpu
        base_config = system_config.SystemConfig(cpu_freq, gpu_freq)
        for pid in cpu_pids_no_approximate :

            viable_configs = apps_knowledge[pid].viable_cpu_configs_corrected
            viable_configs = viable_configs[viable_configs[Column.F_CPU] == cpu_freq]

            if viable_configs.empty : #TODO BUG
                continue

            min_cores_row = viable_configs.sort_values(by=[Column.CPU_N_CORES]).iloc[0]

            app_resource = system_config.AppResources(pid, Location.CPU, row=min_cores_row)
            base_config.cpu_apps_resources[pid] = app_resource

        config_builders[(cpu_freq, gpu_freq)] = ConfigBuilder(base_config)

        # fill the possible_cpu_configs
        free_cores = (model.system.n_cpu_cores -
            (sum([base_config.cpu_apps_resources[pid][Column.CPU_N_CORES] for pid in cpu_pids_no_approximate]) +
            len(cpu_pids_approximate) + len(gpu_pids))
        )
        for pid in cpu_pids_approximate :

            viable_cpu_configs = apps_knowledge[pid].viable_cpu_configs_corrected
            viable_cpu_configs = viable_cpu_configs[
                (viable_cpu_configs[Column.F_CPU] == cpu_freq) &
                (viable_cpu_configs[Column.CPU_N_CORES] <= (free_cores+1))
            ]

            if viable_cpu_configs.empty :
                config_builders.pop((cpu_freq, gpu_freq))
                continue
            
            if system_state.current_apps[pid].is_approximate : #TODO
                viable_cpu_configs = viable_cpu_configs.sort_values(by=Column.PRECISION, ascending=False)

            viable_cpu_resources = []
            for _, row in viable_gpu_configs.iterrows() :
                viable_cpu_resources.append(system_config.AppResources(pid, Location.CPU, row))
            config_builders[(cpu_freq, gpu_freq)].possibile_cpu_configs[pid] = viable_cpu_resources

        # fill the possible_gpu_configs
        if len(gpu_pids) == 1 :
            
            pid = gpu_pids[0]
            viable_gpu_configs = apps_knowledge[pid].viable_gpu_configs_corrected
            viable_gpu_configs = viable_gpu_configs[
                (viable_gpu_configs[Column.F_CPU] == cpu_freq) &
                (viable_gpu_configs[Column.F_GPU] == gpu_freq)
            ]

            if viable_gpu_configs.empty :
                config_builders.pop((cpu_freq, gpu_freq))
                continue
            
            if system_state.current_apps[pid].is_approximate :
                viable_gpu_configs = viable_gpu_configs.sort_values(by=Column.PRECISION, ascending=False)

            viable_gpu_resources = []
            for _, row in viable_gpu_configs.iterrows() :
                viable_gpu_resources.append(system_config.AppResources(pid, Location.GPU, row))
            config_builders[(cpu_freq, gpu_freq)].possibile_gpu_configs = viable_gpu_resources

        elif len(gpu_pids) == 2 :
            pid_A = gpu_pids[0]
            viable_gpu_configs_A = apps_knowledge[pid_A].viable_gpu_configs_plain
            viable_gpu_configs_A = viable_gpu_configs_A[
                (viable_gpu_configs_A[Column.F_CPU] == cpu_freq) &
                (viable_gpu_configs_A[Column.F_GPU] == gpu_freq)
            ]
            if viable_gpu_configs_A.empty :
                config_builders.pop((cpu_freq, gpu_freq))
                continue
            min_thr_A = system_state.current_apps[pid_A].min_thr
            colocation_coeff_A = app_throughput.apps_throughput[pid_A].get_gpu_colocation_avg()


            pid_B = gpu_pids[1]
            viable_gpu_configs_B = apps_knowledge[pid_B].viable_gpu_configs_plain
            viable_gpu_configs_B = viable_gpu_configs_B[
                (viable_gpu_configs_B[Column.F_CPU] == cpu_freq) &
                (viable_gpu_configs_B[Column.F_GPU] == gpu_freq)
            ]
            if viable_gpu_configs_B.empty :
                config_builders.pop((cpu_freq, gpu_freq))
                continue
            min_thr_B = system_state.current_apps[pid_B].min_thr
            colocation_coeff_B = app_throughput.apps_throughput[pid_B].get_gpu_colocation_avg()

            viable_gpu_configs_colocation = model.knowledge.viable_gpu_colocation(
                min_thr_A, viable_gpu_configs_B, min_thr_B, viable_gpu_configs_A,
                colocation_coeff_A, colocation_coeff_B
            )

            if viable_gpu_configs_colocation.empty :
                config_builders.pop((cpu_freq, gpu_freq))
                continue

            viable_gpu_resources = []
            for _, row in viable_gpu_configs_colocation.iterrows() :
                viable_gpu_resources.append(system_config.AppGpuColocationResources(pid_A, pid_B, row))
            config_builders[(cpu_freq, gpu_freq)].possibile_gpu_configs = viable_gpu_resources

    return config_builders

def generate_pareto_configs(system_state : SystemState, config_builders : Dict[Tuple[int, int], ConfigBuilder]) -> List[Tuple[system_config.SystemConfig, float]] :

    max_relative_precision : float = -float("inf")
    pareto_configs : List[Tuple[system_config.SystemConfig, float]] = []
    freqs_combinations : List[Tuple[int, int]] = itertools.product(reversed(model.system.cpu_freqs), reversed(model.system.gpu_freqs))
    for cpu_freq, gpu_freq in freqs_combinations:

        config_builder = config_builders.get((cpu_freq, gpu_freq))
        if config_builder is None :
            continue

        for config in config_builder :
    
            if not viable_config(config) :
                continue

            total_power = config.compute_total_power()
            relative_precision = configuration_relative_precision(system_state, config)

            if system_power.check_violation(total_power) :
                continue

            if relative_precision == max_relative_precision :
                pareto_configs.append((config, total_power))

            elif relative_precision > max_relative_precision :
                max_relative_precision = relative_precision
                pareto_configs.clear()
                pareto_configs.append((config, total_power))
                    
    return pareto_configs

def viable_config(config : system_config.SystemConfig) -> bool :

    required_cores : int = 0

    if type(config.gpu_apps_resources) is system_config.AppResources :
        required_cores += 1
    elif type(config.gpu_apps_resources) is system_config.AppGpuColocationResources :
        required_cores += 2

    for app_resources in config.cpu_apps_resources.values() :
        required_cores += app_resources[Column.CPU_N_CORES]

    return required_cores <= model.system.n_cpu_cores

def configuration_relative_precision(system_state : SystemState, config : system_config.SystemConfig) -> float :
    """Given a configuration, provides the total power and a metric for the 
    total precision of the configuration.
    """

    total_relative_precision : float = 0
    for cpu_pid in config.cpu_apps_resources.keys() :
        if system_state.current_apps[cpu_pid].is_approximate :
            min_precision = system_state.current_apps[cpu_pid].min_precision
            curr_precision = config.cpu_apps_resources[cpu_pid].row[Column.PRECISION]
            total_relative_precision += relative_precision(curr_precision, min_precision)

    if type(config.gpu_apps_resources) is system_config.AppResources :

        min_precision = system_state.current_apps[config.gpu_apps_resources.pid].min_precision
        curr_precision = config.gpu_apps_resources[Column.PRECISION]
        total_relative_precision += relative_precision(curr_precision, min_precision)

    elif type(config.gpu_apps_resources) is system_config.AppGpuColocationResources :
        
        min_precision_A = system_state.current_apps[config.gpu_apps_resources.pid_A].min_precision
        curr_precision_A = config.gpu_apps_resources[Column.PRECISION + Column.A]
        total_relative_precision += relative_precision(curr_precision_A, min_precision_A)

        min_precision_B = system_state.current_apps[config.gpu_apps_resources.pid_B].min_precision
        curr_precision_B = config.gpu_apps_resources[Column.PRECISION + Column.B]
        total_relative_precision += relative_precision(curr_precision_B, min_precision_B)

    n_apps = len(config.cpu_apps_resources)
    if type(config.gpu_apps_resources) is system_config.AppResources :
        n_apps += 1
    elif type(config.gpu_apps_resources) is system_config.AppGpuColocationResources :
        n_apps += 2

    total_relative_precision = total_relative_precision/n_apps

    return total_relative_precision

def relative_precision(curr_p : int , min_p : int, max_p : int = 100) -> float: 
    if max_p == min_p  :
        return 0
    else :
        return (curr_p - min_p) / (max_p - min_p)