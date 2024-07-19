# README

## Structure

This project contains:
- the benchmarking applications `benchmarks`
- the cpp controller `cpp_controller`
- the python controller `python_controller`
- the controller-application interface `communication`

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

To run an experiment, create a directory in the `data` folder containing a `.csv` schedule file. Two example schedules are provided.









