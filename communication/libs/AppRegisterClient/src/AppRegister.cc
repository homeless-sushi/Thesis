#include "AppRegisterClient/AppRegister.h"

#include <iostream>

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "AppRegisterCommon/AppRegister.h"
#include "AppRegisterCommon/Semaphore.h"

struct app_data* registerAttach(
    const char* app_name,
    const char* app_type,
    unsigned int input_size,
    long double requested_throughput,
    int max_threads,
    bool gpu_implementation,
    bool approximate_application,
    unsigned int minimum_precision) 
{
    pid_t controllerPid = getControllerPid(DEFAULT_CONTROLLER_NAME);

    /*--- begin app_data initialization ---*/
    // create semaphore associated with struct app_data
    int dataSemId = semget(getpid(), 1, IPC_CREAT | IPC_EXCL | 0666);
    union semun argument;
    unsigned short values[1] = {1};
    argument.array = values;
    int semval = semctl(dataSemId, 0, SETALL, argument);

    // create struct app_data in shared memory
    int dataSegmentId = shmget(getpid(), sizeof(struct app_data), IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
    struct app_data* data = (struct app_data*) shmat(dataSegmentId, 0, 0);
    if (dataSegmentId == -1 || data == (void*)-1){
      std::cerr << "ERROR: While initilizing shared memory" << std::endl;
      exit(EXIT_FAILURE); 
    }

    //initialize struct app_data
    data->segment_id = dataSegmentId;
    data->registered = false;
    data->use_gpu = 0;
    data->u_sleep_time = 0;
    data->n_cpu_cores = 1;
    data->requested_throughput = requested_throughput;
    data->minimum_precision = minimum_precision;
    data->curr_precision = 100;

    //initialize ticks
    struct timeval tv;
    struct timezone tz;
    gettimeofday(&tv, &tz);
    long double now = tv.tv_sec + 0.000001*tv.tv_usec;
    data->from_start.start = now;
    data->from_start.end = now;
    data->curr_period.start = now;
    data->curr_period.end = now;
    data->lastTimeSample = now;
    /*--- end data initialization ---*/

    /*--- begin app_register initialization ---*/
    //lock
    int registerSemId = semget (controllerPid, 1, 0);
    binarySemaphoreWait(registerSemId);

    //attach to app_register in shared memory
    int registerSegmentId = shmget(controllerPid, sizeof(struct app_register), 0);
    struct app_register* appRegister = (struct app_register*) shmat (registerSegmentId, 0, 0);
    if (registerSegmentId == -1 || registerSemId == -1 || appRegister == (void*)-1){
        std::cerr << "ERROR: While attaching to app_register in shared memory" << std::endl;
        exit(EXIT_FAILURE);
    }
    
    //save application data in the app_register
    if(appRegister->n_new == MAX_NUM_OF_APPS){
      std::cerr << "ERROR: Maximum number of new applications reached" << std::endl;
      exit (EXIT_FAILURE);
    }

    appRegister->new_apps[appRegister->n_new].pid         = getpid();
    strcpy(appRegister->new_apps[appRegister->n_new].name, app_name);
    strcpy(appRegister->new_apps[appRegister->n_new].app_type, app_type);
    appRegister->new_apps[appRegister->n_new].input_size = input_size;
    appRegister->new_apps[appRegister->n_new].segment_id  = dataSegmentId;
    appRegister->new_apps[appRegister->n_new].gpu_implementation = gpu_implementation;
    appRegister->new_apps[appRegister->n_new].max_threads  = max_threads;
    appRegister->new_apps[appRegister->n_new].approximate_application = approximate_application;
    appRegister->n_new++;

    //unlock
    binarySemaphorePost(registerSemId);
    /*--- end app_register initialization ---*/

    return data;
}

int registerDetach(struct app_data* data) 
{ 
    pid_t controllerPid = getControllerPid(DEFAULT_CONTROLLER_NAME);
    //lock  
    int registerSemId = semget (controllerPid, 1, 0);
    binarySemaphoreWait(registerSemId);
    int registerSegmentId = shmget(controllerPid, sizeof(struct app_register), 0);
    struct app_register* appRegister = (struct app_register*) shmat (registerSegmentId, 0, 0);
    if (registerSegmentId == -1 || registerSemId == -1 || appRegister == (void*)-1) {
        std::cerr << "Something wrong happened during controller shared memory attaching" << std::endl;
        exit(EXIT_FAILURE);
    }
    if(appRegister->n_detached == MAX_NUM_OF_APPS) {
        std::cerr << "ERROR: Maximum number of detached applications reached" << std::endl;
        exit (EXIT_FAILURE);
    }
    appRegister->detached_apps[appRegister->n_detached] = getpid();
    appRegister->n_detached++;
    //unlock
    binarySemaphorePost(registerSemId);

    //delete shared memory for the application app_data 
    int segmentId = data->segment_id;
    int errorDt = shmdt(data);
    if(errorDt == -1){
        std::cerr << "ERROR: While detaching from shared memory" << std::endl;
        exit (EXIT_FAILURE);
    }

    //delete semaphore for the application app_data
    int dataSemId = semget(getpid(), 1, 0);
    union semun ignored_argument;
    int err = semctl(dataSemId, 1, IPC_RMID, ignored_argument);
    if(dataSemId == -1 || err == -1){
        std::cerr << "ERROR: While destroying app_data's semaphore" << std::endl;
        exit(EXIT_FAILURE);
    }

    return 0;
}

void setTickStartTime(struct app_data* data)
{
    struct timeval tv;
    struct timezone tz;
    gettimeofday(&tv, &tz);
    long double now = (tv.tv_sec + 0.000001*tv.tv_usec);
    data->from_start.start = now;
    data->from_start.end = now;
    data->curr_period.start = now;
    data->curr_period.end = now;
}

void addTick(struct app_data* data, int n_ticks)
{
    struct timeval tv;
    struct timezone tz;
    gettimeofday(&tv, &tz);
    data->curr_period.value+=n_ticks;
    data->curr_period.end = (tv.tv_sec + 0.000001*tv.tv_usec);
}

pid_t getControllerPid(const char* controller_name) 
{
    pid_t controller_pid;
    std::string pipe = std::string("pgrep -i ") + controller_name;
    FILE *fp;
    fp = popen(pipe.c_str(), "r");
    if (fp == NULL){
        std::cerr << "ERROR: While reading controller pid" << std::endl;
        exit (EXIT_FAILURE); 
    }
    int ret = fscanf(fp, "%d", &controller_pid);
    if (feof(fp)){
        std::cerr << "ERROR: While reading controller pid" << std::endl;
        exit (EXIT_FAILURE);  
    }
    pclose(fp);
    return controller_pid;
}

void setCurrPrecision(struct app_data* data, unsigned int curr_precision){
    data->curr_precision = curr_precision;
}

void setRequestedThroughput(struct app_data* data, long double requested_throughput)
{
    data->requested_throughput = requested_throughput;
}

void setMinimumPrecision(struct app_data* data, unsigned int minimum_precision)
{
    data->minimum_precision = minimum_precision;
}

bool isRegistered(struct app_data *data) 
{
    return data->registered;
}

bool getUseGpu(struct app_data *data) 
{
    return data->use_gpu;
}

int getUSleepTime(struct app_data* data) 
{
    return data->u_sleep_time;
}

int getNCpuCores(struct app_data *data) 
{
    return data->n_cpu_cores;
}

unsigned int getCpuFreq(struct app_data* data)
{
    return data->cpu_freq;
}

unsigned int getGpuFreq(struct app_data* data)
{
    return data->gpu_freq;
}

void autosleep(struct app_data* data, long double ref_thr) {
    struct timeval tv;
    struct timezone tz;
    gettimeofday(&tv, &tz);
    long double curr_time = tv.tv_sec + 0.000001*tv.tv_usec;
    long double last_tick_time = data->curr_period.end;
    long double u_sleep_time = (1.0/ref_thr - (curr_time - last_tick_time))*1000000;

    if(u_sleep_time>0)
        usleep(u_sleep_time);
}
