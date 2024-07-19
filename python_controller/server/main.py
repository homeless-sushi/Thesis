import os

import model.system_state
import model.knowledge

from policy.states.state import State
from policy.states.idle import Idle
import policy.utility.system_state_utility as system_state_utility

import pandas as pd

import zmq

# directory where to place all of a runs data
SERVER_DIR = os.path.dirname(os.path.realpath(__file__))

def server_init():

    context = zmq.Context()
    socket = context.socket(zmq.REP)
    socket.bind("tcp://*:5555")

    return (context,socket)

def knowledge_init():
    
    KNOWLEDGE_DIR = os.path.join(os.path.dirname(SERVER_DIR), "knowledge")
    benchmark_names = [d for d in os.listdir(KNOWLEDGE_DIR) if os.path.isdir(os.path.join(KNOWLEDGE_DIR, d))]
    for benchmark_name in benchmark_names:
        model.knowledge.knowledge_dfs[benchmark_name] = {}
        benchmark_dir = os.path.join(KNOWLEDGE_DIR, benchmark_name)
        sizes = [d for d in os.listdir(benchmark_dir) if os.path.isdir(os.path.join(benchmark_dir, d))]
        for size in sizes: 
            model.knowledge.knowledge_dfs[benchmark_name][int(size)] = {}
            size_dir = os.path.join(benchmark_dir, size)
            cpu_file = os.path.join(size_dir, "cpu.csv")
            gpu_file = os.path.join(size_dir, "gpu.csv")
            model.knowledge.knowledge_dfs[benchmark_name][int(size)]["CPU"] = pd.read_csv(cpu_file)
            model.knowledge.knowledge_dfs[benchmark_name][int(size)]["GPU"] = pd.read_csv(gpu_file)

def main():
    
    knowledge_init()
    context, socket = server_init()

    controller_state : State = Idle(model.system_state.SystemState(), model.system_state.SystemState())
    controller_state.handle()

    while True:
        
        measured_state : str = socket.recv().decode('utf-8')
        new_system_state = system_state_utility.read_system_state(measured_state)
        controller_state = controller_state.next(new_system_state)
        controller_state.handle()
        prescribed_state : str = system_state_utility.write_system_state(controller_state.curr_prescribed)
        socket.send(prescribed_state.encode('utf8'))

if __name__ == "__main__" :
    main()