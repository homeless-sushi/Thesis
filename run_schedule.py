import argparse
import os
import subprocess
import sys
import time

import numpy
import pandas as pd

# directory where to place all of a runs data
PROJECT_DIR = os.path.dirname(os.path.realpath(__file__)) + "/"

SCHEDULE_URL = ""
SCHEDULE_DIR = ""

# csv field and keywords
BENCHMARK = 'BENCHMARK'
TYPE = "TYPE"
INSTANCE_NAME = 'INSTANCE_NAME'
START_TIME = 'START_TIME'
INPUT = 'INPUT'
OUTPUT = 'OUTPUT'
THROUGHPUT = 'THROUGHPUT'
APPEND = 'APPEND'

PYTHON_CONTROLLER_NAME = "PYTHON_CONTROLLER"
CPP_CONTROLLER_NAME = "CPP_CONTROLLER"
TERMINATE = "TERMINATE"

def setup_arg_parse() :

    parser = argparse.ArgumentParser(
        prog='Benchmark Scheduler',
        description=
            'This program reads a schedule of benchmarks'
            'and runs them according to it;\n'
            'The schedule is a csv, with the following format:\n\n'
            'BENCHMARK,INSTANCE_NAME,START_TIME,INPUT,OUTPUT,THROUGHPUT,APPEND',
        usage=f'{sys.argv[0]} schedule_url'
    )

    parser.add_argument(
        "schedule_url",
        type=str,
        help="the schedule's url"
    )

    return parser

def run_benchmark(name, type, instance_name, in_file, out_file, target_throughput, append) :
    
    # path names
    BENCHMARK_PATH = f"benchmarks/programs/{name}/{type}/build/{name}/{name}"
    BENCHMARK_INPUT_DIR = f"benchmarks/programs/{name}/data/in/"
    BENCHMARK_OUTPUT_DIR = f"benchmarks/programs/{name}/data/out/"
    
    # command options
    INSTANCE_NAME_OPTION = "--instance-name"
    INPUT_OPTION = "--input-file"
    OUTPUT_OPTION = "--output-file"
    THROUGHTPUT_OPTION = "--target-throughput"

    INSTANCE_LOG_URL = os.path.join(SCHEDULE_DIR, instance_name + ".csv")
    with open(INSTANCE_LOG_URL, "w") as instance_log_file :
        command = [
            PROJECT_DIR + BENCHMARK_PATH,
            INPUT_OPTION, PROJECT_DIR + BENCHMARK_INPUT_DIR + in_file, 
            OUTPUT_OPTION, PROJECT_DIR + BENCHMARK_OUTPUT_DIR + out_file,
            INSTANCE_NAME_OPTION, instance_name,
            THROUGHTPUT_OPTION, str(target_throughput)
        ]
    
        if not (isinstance(append, float) and numpy.isnan(append)) :
            command = command + str(append).split(" ")

        process = subprocess.Popen(
            command,
            stdout=instance_log_file
        )
 
    return process

def run_python_controller(append) :
    
    # python controller
    PYTHON_CONTROLLER_PATH = PROJECT_DIR + f"python_controller/server/main.py"

    if not isinstance(append, str):
        append = str(append)

    command = ["python3.8", PYTHON_CONTROLLER_PATH] + append.split(" ")

    process = subprocess.Popen(
        command,
        stdin=sys.stdin,
        stdout=sys.stdout,
        stderr=sys.stderr
    )

    return process

def run_cpp_controller(controller_type, append) :

    # controller
    CPP_CONTROLLER_PATH = PROJECT_DIR + f"cpp_controller/build/controllers/{controller_type}/RtrmController"

    if not isinstance(append, str):
        append = str(append)
    command = [CPP_CONTROLLER_PATH] + append.split(" ")
    process = subprocess.Popen(
        command,
        stdin=sys.stdin,
        stdout=sys.stdout,
        stderr=sys.stderr
    )

    return process

def main() :

    # Setup argument parser and parse 
    parser = setup_arg_parse()
    args = parser.parse_args()

    # Set the directory for the results
    global SCHEDULE_URL
    global SCHEDULE_DIR
    SCHEDULE_URL = os.path.abspath(args.schedule_url)
    SCHEDULE_DIR = os.path.dirname(SCHEDULE_URL)

    # Read schedule
    schedule_df = pd.read_csv(SCHEDULE_URL)
    schedule_df = schedule_df.sort_values("START_TIME")

    # Convert start times to delays from previous events
    DELAY = "DELAY"
    schedule_df[DELAY] = schedule_df[START_TIME] - schedule_df[START_TIME].shift(1, fill_value=0)  
    schedule_df = schedule_df.drop(START_TIME, axis=1)

    # Start controller first
    processes = {}
    try: 
        # Start or terminate benchmarks accourding to the schedule
        for _, row in schedule_df.iterrows() :

            # Sleep according to the schedule
            time.sleep(row[DELAY])

            # if BENCHMARK is python controller
            if row[BENCHMARK] == PYTHON_CONTROLLER_NAME :
                processes[PYTHON_CONTROLLER_NAME] = run_python_controller(
                    row[APPEND]
                )
            # if BENCHMARK is cpp controller
            elif row[BENCHMARK] == CPP_CONTROLLER_NAME :
                processes[CPP_CONTROLLER_NAME] = run_cpp_controller(
                    row[TYPE],
                    row[APPEND]
                )
            # if BENCHMARK is terminate
            elif row[BENCHMARK] == TERMINATE :
                processes[row[INSTANCE_NAME]].terminate()
            # else run benchmark
            else : 
                processes[row[INSTANCE_NAME]] = run_benchmark(
                    row[BENCHMARK],
                    row[TYPE],
                    row[INSTANCE_NAME],
                    row[INPUT],
                    row[OUTPUT],
                    row[THROUGHPUT],
                    row[APPEND]
                )
    # If an exception is raised, terminate the controller
    # Otherwise the app register will persist in shared memory
    except Exception as e:
        print(e)
        if CPP_CONTROLLER_NAME in processes and processes[CPP_CONTROLLER_NAME].poll() is None:
            processes[CPP_CONTROLLER_NAME].terminate()

    # Wait for the controller to terminate
    processes[CPP_CONTROLLER_NAME].wait()

    # If any other process is open, terminate it
    for _, process in processes.items():
        process.terminate()

    time.sleep(0.5)

    # If any process won't terminate, kill it
    for _, process in processes.items():  
        if process.poll() is None:
            process.kill()

    return 0

if __name__ == "__main__" :
    main()