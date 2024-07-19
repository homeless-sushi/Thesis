from typing import Tuple, List

from model.system_state import SystemState
from model.app import App, Location

def read_system_state(to_read : str) -> SystemState:
    """
    Reads a SystemState from a string. The string has the following format:
    SYSTEM:CPU_FREQ,GPU_FREQ,CPU_GPU_POWER\n
    APPS:[PID,NAME,SIZE,MAX_CORES,IS_GPU,IS_APPROXIMATE,MIN_THR,MIN_PRECISION,CURR_THR,CURR_PRECISION,ON_GPU,N_CORES;]*
    """

    system_state: SystemState = SystemState()

    system_line, apps_line = to_read.split("\n")

    # SYSTEM
    _, system_content = system_line.split(":")
    cpu_freq, gpu_freq, total_power = system_content.split(",")
    system_state.cpu_freq = int(cpu_freq)
    system_state.gpu_freq = int(gpu_freq)
    system_state.power = float(total_power)

    # APPS
    _, apps_content = apps_line.split(":")

    if not apps_content :
        return system_state
    
    for app in apps_content.split(";")[:-1] :

        pid, name, size, max_cpu_cores, is_gpu, is_approximate, min_thr, min_precision, curr_thr, curr_precision, on_gpu, cpu_cores = app.split(",")
        pid = int(pid)
        size = int(size)
        max_cpu_cores = int(max_cpu_cores)
        is_gpu = bool(int(is_gpu))
        is_approximate = bool(int(is_approximate))
        min_thr = float(min_thr)
        min_precision = int(min_precision)
        curr_thr = float(curr_thr)
        curr_precision = int(curr_precision)
        location = Location.GPU if int(on_gpu) else Location.CPU
        cpu_cores = int(cpu_cores)

        app = App(
            pid, name, size, max_cpu_cores, is_gpu, is_approximate,
            min_thr, min_precision,
            curr_thr, curr_precision, location, cpu_cores
        )

        system_state.current_apps[pid] = app

    return system_state
    
def write_system_state(to_write : SystemState) -> str:
    """
    Writes a SystemState to a string. It has the following format:
    SYSTEM:CPU_FREQ,GPU_FREQ,CPU_GPU_POWER\n
    APPS:[PID,NAME,SIZE,MAX_CORES,IS_GPU,IS_APPROXIMATE,MIN_THR,MIN_PRECISION,CURR_THR,CURR_PRECISION,ON_GPU,N_CORES;]*
    """

    system_line: str = f"SYSTEM:{to_write.cpu_freq},{to_write.gpu_freq},{to_write.power}"
    
    app_strs: str = ""
    for _, app in to_write.current_apps.items() :

        app_str: str = ",".join([
            str(app.pid), app.name, str(app.size), str(app.max_cpu_cores), str(int(app.is_gpu)), str(int(app.is_approximate)),
            str(app.min_thr), str(int(app.min_precision)),
            str(app.curr_thr), str(int(app.curr_precision)), str(1 if app.location == Location.GPU else 0), str(int(app.cpu_cores))
        ])

        app_strs += app_str + ";"
    app_line : str = "APPS:" + app_strs
    
    return system_line + "\n" + app_line

def compare_system_states(old : SystemState, new : SystemState) -> Tuple[List[int],List[int]] :

    new_pids = []
    new_pids.extend(set(new.current_apps.keys()) - set(old.current_apps.keys()))

    old_pids = []
    old_pids.extend(set(old.current_apps.keys()) - set(new.current_apps.keys()))
  
    return new_pids, old_pids

def copy_system_state(to_copy : SystemState) -> SystemState :

    copy: SystemState = SystemState()

    copy.cpu_freq = to_copy.cpu_freq
    copy.cpu_freq = to_copy.cpu_freq
    copy.power = to_copy.power
    for pid, app in to_copy.current_apps.items() :
        app_copy = App(
            app.pid, app.name, app.size, app.max_cpu_cores, app.is_gpu, app.is_approximate,
            app.min_thr, app.min_precision,
            app.curr_thr, app.curr_precision, app.location, app.cpu_cores
        )
        copy.current_apps[pid] = app_copy

    return copy
