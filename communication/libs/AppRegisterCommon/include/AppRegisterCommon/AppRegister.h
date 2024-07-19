#ifndef APP_REGISTER_COMMON_APP_REGISTER
#define APP_REGISTER_COMMON_APP_REGISTER

#include <sched.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

// C code only
// include bool type definition
#ifndef __cplusplus
#include <stdbool.h>
#endif

#define MAX_TICKS_SIZE 300
#define MAX_NUM_OF_APPS 128
#define MAX_NUM_OF_CPU_SETS 32
#define MAX_NAME_SIZE 100
#define DEFAULT_CONTROLLER_NAME "RtrmController"
#define ZERO_APPROX 1e-5

/**
 * A struct used to collect ticks
 */
struct ticks
{
    long long unsigned value;           /**< total accumulated ticks */
    long double start;     /**< timestamp of tick before the first */
    long double end;         /**< timestamp of last tick*/
};

/**
 * A struct used to exchange performance data and actuation knobs
 * between controller and application.
 *
 * Allocated by the application.
 */
struct app_data
{
    int segment_id;         /**< ID of the segment where this is stored */
    bool registered;      /**< the application has been registered */

    struct ticks from_start;    /**< ticks from app's start to the last control cycle  */
    struct ticks curr_period;   /**< ticks since the last control cycle  */

    unsigned int curr_precision; /**< percentage of approximation */

    long double requested_throughput;   /**< requested throughtput by the application */
    unsigned int minimum_precision;   /**< requested minimum precision by the application */

    long double lastTimeSample; /**< last timestamp that has been sampled. it is necessary to implement the autosleep function */
    bool use_gpu;               /**< true if the app must use the GPU */
    int n_cpu_cores;            /**< number of cpu cores available to the application*/
    unsigned int cpu_freq;      /**< system's cpu frq*/
    unsigned int gpu_freq;      /**< system's gpu frq*/
    int u_sleep_time;           /**< in microseconds */
};

/**
 * A struct used to represent the app description.
 *
 * Is set by the application in the app_register upon attaching.
 * May not be modified after initialization.
 */
struct app_descriptor
{
    pid_t pid;                    /**< the application pid */
    char name[MAX_NAME_SIZE + 1]; /**< the name of the application */
    char app_type[MAX_NAME_SIZE + 1];   /**< the application's type */
    unsigned int input_size;      /**< the application's input size */
    int segment_id;               /**< the id of the memory segment containing the app_data */
    bool gpu_implementation;      /**< true if the application has a GPU implementation */
    int max_threads;              /**< maximum CPU threads used by the application */
    bool approximate_application; /**< true if the application allows approxiamte execution */
};

/**
 * A struct represent the status of the system.
 *
 * Is allocated by the controller.
 */
struct app_register
{
    struct app_descriptor new_apps[MAX_NUM_OF_APPS]; /**< newly attached apps adds themselves here */
    int n_new;                                       /**< current number of newly attached apps */

    pid_t detached_apps[MAX_NUM_OF_APPS]; /**< newly detacged apps add themselves here */
    int n_detached;                       /**< current number of newly detached apps */
};

// semaphore management, data structure and wait/post functions.
// The semaphore is used to access the monotor_t data structure in the shared memory since both the applications and the controller may write it
union semun
{
    int val;
    struct semid_ds *buf;
    unsigned short int *array;
    struct seminfo *__buf;
};

#endif //APP_REGISTER_COMMON_APP_REGISTER
