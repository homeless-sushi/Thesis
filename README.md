# README

## Structure

This project contains:
- the benchmarking applications `benchmarks`
- the cpp controller `cpp_controller`
- the python controller `python_controller`
- the controller-application interface `communication`

### Benchmarks

The current benchmarks are found under `benchmarks\programs\`, in their respective directories.<br>
For each benchmark, multiple versions exist:
- `ALGORITHM` is used to test the algorithm of the benchmark.
- `CONTROLLER` is the controller only implementation. It allows to set applications software knobs. It does not signal the actual precision to the controller.
- `MARGOT` is the margot only implementation. Margot sets the applications knobs.
- `CONTROLLER+MARGOT` is the thesis implementation.

Other benchmarks might be implemented by creating a folder under `benchmarks\programs\`. Other version of existing benchmark might be implemented by creating a folder under `benchmarks\programs\EXAMPLE\`.

### CPP Controller

The current benchmarks are found under `cpp_controller\controllers\`, in their respective directories. Each controller uses a different policy, which can be found under `cpp_controller\common\libs\Policies\`.<br>
The available controllers are:<br>
- `SET_CONFIGURATION` which allows to set fixed CPU/GPU frequencies and set each application's CPU cores and GPU access.
- `MARGOT_ONLY` which simply logs the system and applications' status. It is used when we want to leave control of the applications knob to Margot.
- `PYTHON_POLICY` which connects to the python server and run its policy.

Other benchmarks might be implemented by creating a folder under `cpp_controllers\controller\` and another under `cpp_controller\common\libs\Policies\`, the former containing the main program and the latter containing the policy.

### Python Controller

Currently, a single python controller is supported. It can be found in the `python_controller\`. In this directory, `knowledge\` contains the application knowledge and `server\` the python program.

## Dependencies

Boost C++ Libraries<br>
mARGOt<br>
paho-mqtt<br>
zmq<br>
zmqpp<br>

pandas<br>
Pyzmq<br>

## Build

To build the applications, configure the .env file and export the variable to the ENV using:

```bash
set -a
source .env
set +a
```

then run the `build.sh` scripts in the respective directories.

## Run

To run an experiment, create a directory in the `data` folder containing a `.csv` schedule file. Then, use `run_schedule.py` to launch the selected controllers and applications. Two example schedules are provided.









