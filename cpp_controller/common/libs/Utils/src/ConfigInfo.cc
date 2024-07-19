#include <Utils/ConfigInfo.h>

#include <fstream>
#include <map>
#include <vector>

#include "AppRegisterServer/Frequency.h"

namespace ConfigInfo
{
    ConfigInfo readConfigInfo(std::string configFileUrl)
    {
        std::ifstream configFile(configFileUrl);
    
        std::string lineString;
        getline(configFile, lineString);
        std::istringstream firstLineStream(lineString);
        unsigned int cpuFrq;
        firstLineStream >> cpuFrq;
        unsigned int gpuFrq;
        firstLineStream >> gpuFrq;
        
        std::map<std::string, AppConfigInfo> appConfigs;
        while(configFile.good()){

            getline(configFile, lineString);
            std::istringstream lineStream(lineString);
            std::string name;
            lineStream >> name;
            bool gpu;
            lineStream >> gpu;
            std::vector<int> cores;
            while(lineStream.good()){

                int core;
                lineStream >> core;
                cores.push_back(core);
            }
            appConfigs[name] = AppConfigInfo(gpu, cores);
        }

        configFile.close();

        return ConfigInfo(
            static_cast<Frequency::CPU_FRQ>(cpuFrq), 
            static_cast<Frequency::GPU_FRQ>(gpuFrq), 
            std::move(appConfigs));
    };
}