#ifndef APP_REGISTER_SERVER_FREQUENCY
#define APP_REGISTER_SERVER_FREQUENCY

#include <cstdlib>

#include <sstream>

namespace Frequency
{
    enum CPU_FRQ
    {
        FRQ_102000KHz = 102000,
        FRQ_204000KHz = 204000,
        FRQ_307200KHz = 307200,
        FRQ_403200KHz = 403200,
        FRQ_518400KHz = 518400,
        FRQ_614400KHz = 614400,
        FRQ_710400KHz = 710400,
        FRQ_825600KHz = 825600, 
        FRQ_921600KHz = 921600, 
        FRQ_1036800KHz = 1036800, 
        FRQ_1132800KHz = 1132800, 
        FRQ_1224000KHz = 1224000, 
        FRQ_1326000KHz = 1326000, 
        FRQ_1428000KHz = 1428000, 
        FRQ_1479000KHz = 1479000,
        N_CPU_FREQS = 15
    };

    enum GPU_FRQ
    {  
        FRQ_76800000Hz =  76800000,
        FRQ_153600000Hz = 153600000,
        FRQ_230400000Hz = 230400000,
        FRQ_307200000Hz = 307200000,
        FRQ_384000000Hz = 384000000,
        FRQ_460800000Hz = 460800000,
        FRQ_537600000Hz = 537600000,
        FRQ_614400000Hz = 614400000,
        FRQ_691200000Hz = 691200000,
        FRQ_768000000Hz = 768000000,
        FRQ_844800000Hz = 844800000,
        FRQ_921600000Hz = 921600000,
        N_GPU_FREQS = 12
    };

    inline CPU_FRQ getMinCpuFreq(){ return FRQ_102000KHz; }
    inline CPU_FRQ getMaxCpuFreq(){ return FRQ_1479000KHz; }
    CPU_FRQ getCurrCpuFreq();
    CPU_FRQ getNextCpuFreq(CPU_FRQ currFreq);
    CPU_FRQ getNextCpuFreq(unsigned int currFreq);
    int SetCpuFreq(CPU_FRQ freq);

    inline GPU_FRQ getMinGpuFreq(){ return FRQ_76800000Hz; }
    inline GPU_FRQ getMaxGpuFreq(){ return FRQ_921600000Hz; }
    GPU_FRQ getCurrGpuFreq();
    GPU_FRQ getNextGpuFreq(GPU_FRQ currFreq);
    GPU_FRQ getNextGpuFreq(unsigned int currFreq);
    int SetGpuFreq(GPU_FRQ freq);
}

#endif //APP_REGISTER_SERVER_FREQUENCY
