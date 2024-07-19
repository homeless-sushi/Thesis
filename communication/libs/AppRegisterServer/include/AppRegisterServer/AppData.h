#ifndef APP_REGISTER_SERVER_APP_DATA
#define APP_REGISTER_SERVER_APP_DATA

#include "AppRegisterCommon/AppRegister.h"

#define DEFAULT_TIME_WINDOW 1

namespace AppData
{
    //Get pointer to app_data
    struct app_data* getAppData(int segment_id);
    //Get global throughput
    struct ticks getPrevTicks(struct app_data* data);
    //Get current throughput 
    struct ticks getCurrTicks(struct app_data* data);
    //Get current input size
    unsigned int getCurrPrecision(struct app_data* data);
    //Get requested throughput
    double getRequestedThroughput(struct app_data* data);
    //Get requested precision
    double getMinimumPrecision(struct app_data* data);
    //Set is_registered
    void setRegistered(struct app_data* data, bool value);
    //Set use_gpu
    void setUseGpu(struct app_data* data, bool value);
    //Set u_sleep_time
    void setUSleepTime(struct app_data* data, int value);
    //Set n_cpu_cores
    void setNCpuCores(struct app_data* data, int value);
    //Set cpu_freq
    void setCpuFreq(struct app_data* data, int value);
    //Set gpu_freq
    void setGpuFreq(struct app_data* data, int value);
}

#endif //APP_REGISTER_CONTROLLER_APP_DATA
