from policy.utility.app_throughput import AppThroughput

import model.knowledge as knowledge
from model.app import Location

import pandas as pd 

class AppKnowledge :

    def __init__(self, app_throughput : AppThroughput):

        self.viable_cpu_configs_plain : pd.DataFrame
        self.viable_cpu_configs_corrected : pd.DataFrame

        self.viable_gpu_configs_plain : pd.DataFrame
        self.viable_gpu_configs_corrected : pd.DataFrame

        if app_throughput.is_approximate :
            self.viable_cpu_configs_plain = knowledge.viable_cpu(
                app_throughput.name, app_throughput.size, app_throughput.min_thr, app_throughput.min_precision
            )
            self.viable_cpu_configs_corrected = knowledge.viable_cpu(
                app_throughput.name, app_throughput.size, app_throughput.min_thr, app_throughput.min_precision,
                throughput_coefficient = app_throughput.get_cpu_avg()
            )
            self.viable_gpu_configs_plain = knowledge.viable_gpu(
                app_throughput.name, app_throughput.size, app_throughput.min_thr, app_throughput.min_precision
            )
            self.viable_gpu_configs_corrected = knowledge.viable_gpu(
                app_throughput.name, app_throughput.size, app_throughput.min_thr, app_throughput.min_precision,
                throughput_coefficient = app_throughput.get_gpu_avg()
            )

        else :
            self.viable_cpu_configs_plain = knowledge.viable_cpu(
                app_throughput.name, app_throughput.size, app_throughput.min_thr
            )
            self.viable_cpu_configs_corrected = knowledge.viable_cpu(
                app_throughput.name, app_throughput.size, app_throughput.min_thr,
                throughput_coefficient = app_throughput.get_cpu_avg()
            )
            self.viable_gpu_configs_plain = knowledge.viable_gpu(
                app_throughput.name, app_throughput.size, app_throughput.min_thr
            )
            self.viable_gpu_configs_corrected = knowledge.viable_gpu(
                app_throughput.name, app_throughput.size, app_throughput.min_thr,
                throughput_coefficient = app_throughput.get_gpu_avg()
            )

        self.suggested : Location = Location.NONE
        if self.viable_cpu_configs_corrected.empty and self.viable_gpu_configs_corrected.empty:
            self.suggested = Location.UNVIABLE
        elif self.viable_cpu_configs_corrected.empty:
            self.suggested = Location.GPU
        elif self.viable_gpu_configs_corrected.empty:
            self.suggested = Location.CPU
