#include "AppRegisterServer/AppData.h"

#include <iostream>

#include <sys/shm.h>
#include <sys/time.h>

#include "AppRegisterCommon/AppRegister.h"

namespace AppData 
{
    struct app_data* getAppData(int segment_id)
    {
        struct app_data* shm = (struct app_data*) shmat(segment_id, 0, 0);
        if(shm == (void*) -1)
            std::cerr << "ERROR: While accessing app_data in shared memory!" << std::endl;
        return shm;
    }

    struct ticks getPrevTicks(struct app_data *data) { return data->from_start; }

    struct ticks getCurrTicks(struct app_data *data) 
    {
        struct ticks curr_ticks = data->curr_period;

        data->from_start.value+=curr_ticks.value;
        data->from_start.end=curr_ticks.end;

        data->curr_period.value=0;
        data->curr_period.start=curr_ticks.end;
        data->curr_period.end=curr_ticks.end;
        
        return curr_ticks;
    }

    unsigned int getCurrPrecision(struct app_data* data){ return data->curr_precision; }
    double getRequestedThroughput(struct app_data* data){ return data->requested_throughput; }
    double getMinimumPrecision(struct app_data* data){ return data->minimum_precision; }
    void setRegistered(struct app_data* data, bool value) { data->registered = value; }
    void setUseGpu(struct app_data* data, bool value) { data->use_gpu = value; }
    void setUSleepTime(struct app_data* data, int value) { data->u_sleep_time = value; }
    void setNCpuCores(struct app_data* data, int value) { data->n_cpu_cores = value; }
    void setCpuFreq(struct app_data* data, int value) { data->cpu_freq = value; }
    void setGpuFreq(struct app_data* data, int value) { data->gpu_freq = value; }
}
