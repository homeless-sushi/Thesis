#ifndef UTILS_CONFIG_INFO_H
#define UTILS_CONFIG_INFO_H

#include <map>
#include <vector>

#include "AppRegisterServer/Frequency.h"

namespace ConfigInfo
{
    struct AppConfigInfo
    {
        bool gpu;
        std::vector<int> cores;

        AppConfigInfo() : 
            gpu{false},
            cores{0}
        {};

        AppConfigInfo(
            bool gpu,
            std::vector<int> cores
        ) : 
            gpu{gpu},
            cores{cores}
        {};
    };

    class ConfigInfo 
    {
        private:
            Frequency::CPU_FRQ cpuFrq;
            Frequency::GPU_FRQ gpuFrq;

            std::map<std::string, AppConfigInfo> appConfigs;
        
        public:
            ConfigInfo(
                Frequency::CPU_FRQ cpuFrq,
                Frequency::GPU_FRQ gpuFrq,
                std::map<std::string, AppConfigInfo> appConfigs
            ) :
                gpuFrq{gpuFrq},
                cpuFrq{cpuFrq},
                appConfigs{appConfigs}
            {};
            
            Frequency::CPU_FRQ getCpuFrq() { return cpuFrq; }
            Frequency::GPU_FRQ getGpuFrq() { return gpuFrq; }
            AppConfigInfo& getAppConfigInfo(std::string name) { return appConfigs[name]; }
    };

    ConfigInfo readConfigInfo(std::string configFileUrl);
}

#endif //UTILS_CONFIG_INFO_H