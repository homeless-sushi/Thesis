import argparse
import os
import sys

import knobs

THIS_DIR = os.path.dirname(os.path.realpath(__file__)) # this script directory
PROGRAM_URL = os.path.abspath(os.path.join(THIS_DIR, "../../../PROFILING/build/CUTCP/CUTCP"))
INPUT_DIR = os.path.abspath(os.path.join(THIS_DIR, "../../../data/in/"))
LOG_DIR = os.path.join(THIS_DIR, "/data/")
LOG_FILE = os.path.join(LOG_DIR, "log.txt")

def setup_args() :

    parser = argparse.ArgumentParser(
        prog='Run Profiling',
        description=
            'This program runs the CUTCP algorithm with various settings'
            'and logs the results.\n\n',
        usage=f'{sys.argv[0]}'
    )

    return parser

def run(gpu, cpu_threads, gpu_block_exp, problem_size, precision) :

    input_file = input_file_from_problem(problem_size)
    output_file = "/dev/null"
    log_file = LOG_FILE

    gpu_option = "--gpu" if gpu else ""
    cpu_threads_option = "--cpu-threads " + str(cpu_threads) if not gpu else ""
    gpu_block_exp_option = "--gpu-block-exp " + str(gpu_block_exp) if gpu else ""
    precision_option = "--precision " + str(precision)
    input_file_option = "--input-file " + input_file
    output_file_option = "--output-file " + output_file
    redirect_std_output = "> " + log_file

    os.system(
        " ".join([
            PROGRAM_URL,
            gpu_option,
            cpu_threads_option,
            gpu_block_exp_option,
            precision_option,
            input_file_option,
            output_file_option,
            redirect_std_output
        ])
    )

def input_file_from_problem(problem_size) :
    return os.path.join(INPUT_DIR, str(problem_size) + ".txt")

def read_log(log_url) :

    log_obj = {}

    with open(log_url, 'r') as log:
        for line in log:
            field, time = line.strip().split(' ')
            time = float(time)
            
            log_obj[field] = time

def main() :

    parser = setup_args()
    args = parser.parse_args()

    # with GPU
    for problem_size in knobs.PROBLEM_SIZE :
        for precision in range(0, 100, 10) :
            for precision in knobs.GPU_BLOCK_EXP :
                

    return

if __name__ == '__main__' :
    main()