#ifndef APP_REGISTER_CLIENT_APP_REGISTER
#define APP_REGISTER_CLIENT_APP_REGISTER

#include "AppRegisterCommon/AppRegister.h"

//The application may be implemented in either C or C++
#ifdef __cplusplus
extern "C"
{
#endif

//Get controller pid
pid_t getControllerPid(const char* controller_name);
//Attach the application to the app_register
struct app_data* registerAttach(
    const char* app_name, 
    const char* app_type,
    unsigned int input_size,
    long double requested_throughput, 
    int max_threads,
    bool gpu_implementation,
    bool approximate_application = false,
    unsigned int minimum_precision = 100
);
//Detach the application from the app_register
int registerDetach(struct app_data* data);

//Set app start time
void setTickStartTime(struct app_data* data);
//Send a tick to the register to which the application is connected
void addTick(struct app_data* data, int n_ticks);
//Set current precision
void setCurrPrecision(struct app_data* data, unsigned int curr_precision);
//Set requested throughput
void setRequestedThroughput(struct app_data* data, long double requested_throughput);
//Set minimum precision
void setMinimumPrecision(struct app_data* data, unsigned int minimum_precision);
//True if application has been registered
bool isRegistered(struct app_data* data);
//Get use_gpu
bool getUseGpu(struct app_data* data);
//Get the u_sleep_time
int getUSleepTime(struct app_data* data);
//Get n_cpu_cores
int getNCpuCores(struct app_data* data);
//Get cpu_freq
unsigned int getCpuFreq(struct app_data* data);
//Get gpu_freq
unsigned int getGpuFreq(struct app_data* data);

//Sleep as to not overshoot the reference throughput
void autosleep(struct app_data*, long double ref_thr);

#ifdef __cplusplus
}
#endif

#endif //APP_REGISTER_CLIENT_APP_REGISTER
