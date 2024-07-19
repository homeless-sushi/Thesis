from typing import List, Dict

import model.system as system
from model.app import App, Location

class SystemState:
    def __init__(self):
        self.cpu_freq : int = system.cpu_freqs[0]
        self.gpu_freq : int = system.gpu_freqs[0]
        self.power : float = 0
        self.current_apps : Dict[int, App] = {}

    def get_cpu_pids(self) -> List[int] :

        return [app.pid for app in self.current_apps.values() if app.location == Location.CPU]

    def get_gpu_pids(self) -> List[int] :

        return [app.pid for app in self.current_apps.values() if app.location == Location.GPU]

