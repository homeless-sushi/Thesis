#include "Policies/SetConfigurationPolicy.h"

#include <iostream>
#include <iterator>
#include <memory>
#include <set>
#include <vector>

#include "Utils/ConfigInfo.h"

#include "AppRegisterServer/App.h"
#include "AppRegisterServer/AppData.h"
#include "AppRegisterServer/Policy.h"
#include "AppRegisterServer/Sensors.h"
#include "AppRegisterServer/Utilization.h"
#include "AppRegisterServer/Frequency.h"
#include "AppRegisterServer/CGroupUtils.h"

namespace Policy
{
    SetConfigurationPolicy::SetConfigurationPolicy(
        unsigned nCores,
        std::string controllerLogUrl,
        std::string sensorsLogUrl,
        std::string configFileUrl
    ) :
        Policy(nCores),
        controllerLogFile(controllerLogUrl, controllerLogFile.out),
        utilization(nCores),
        sensorLogFile(sensorsLogUrl, sensorLogFile.out),
        config(ConfigInfo::readConfigInfo(configFileUrl))
    {
        Frequency::SetCpuFreq(config.getCpuFrq());
        Frequency::SetGpuFreq(config.getGpuFrq());

        controllerLogFile << "CYCLE,PID,NAME,APP,INPUT_SIZE,TARGET_THR,CURRENT_THR,MIN_PRECISION,CURR_PRECISION" << std::endl;

        sensorLogFile << "CYCLE,";
        for(unsigned i = 0; i < nCores; ++i)
            sensorLogFile << "UTILIZATION_" << i << ",";
        sensorLogFile << "CPUFREQ,GPUFREQ,SOCW,CPUW,GPUW" << std::endl;
    };

    void SetConfigurationPolicy::run(int cycle)
    {
        lock();

        deregisterDetachedApps();
        deregisterDeadApps();

        // Write controllerLogFile
        for (const auto& pairPidApp : registeredApps) {

            pairPidApp.second->lock();
            pairPidApp.second->readTicks();
            long double requestedThroughput = pairPidApp.second->data->requested_throughput;
            struct ticks ticks = pairPidApp.second->getWindowTicks();
            pairPidApp.second->currentThroughput = getWindowThroughput(ticks);
            unsigned int minimumPrecison = pairPidApp.second->data->minimum_precision;
            unsigned int currPrecision = pairPidApp.second->data->curr_precision;
            unsigned int useGpu = pairPidApp.second->data->use_gpu;
            unsigned int nCores = pairPidApp.second->data->n_cpu_cores;
            pairPidApp.second->unlock();
            
            controllerLogFile << cycle << ","
                << pairPidApp.second->descriptor.pid << ","
                << pairPidApp.second->descriptor.name << ","
                << pairPidApp.second->descriptor.app_type << ","
                << pairPidApp.second->descriptor.input_size << ","
                << requestedThroughput << ","
                << pairPidApp.second->currentThroughput << ","
                << currPrecision << "," 
                << useGpu << ","
                << nCores << std::endl;
        }

        Frequency::CPU_FRQ currCpuFreq = Frequency::getCurrCpuFreq();
        Frequency::GPU_FRQ currGpuFreq = Frequency::getCurrGpuFreq();
        std::vector<int> utilizations = utilization.computeUtilization();
        sensors.readSensors();
        float socW = sensors.getSocW();
        float cpuW = sensors.getCpuW();
        float gpuW = sensors.getGpuW();


        // Write sensorLogFile
        {
            sensorLogFile << cycle << ",";
            for(unsigned i = 0; i < nCores; ++i)
                sensorLogFile << utilizations[i] << ",";
            sensorLogFile 
                << currCpuFreq << ","
                << currGpuFreq << ","
                << socW << ","
                << cpuW << ","
                << gpuW << std::endl;
        }

        //instert new apps
        std::vector<int> newRegisteredApps = registerNewApps();

        for(auto newAppPid : newRegisteredApps){
            registeredApps[newAppPid]->lock();
            AppData::setRegistered(registeredApps[newAppPid]->data, true);
            ConfigInfo::AppConfigInfo& newAppConfig =
                config.getAppConfigInfo(std::string(registeredApps[newAppPid]->descriptor.name));
            AppData::setUseGpu(registeredApps[newAppPid]->data, newAppConfig.gpu);
            CGroupUtils::UpdateCpuSet(newAppPid, newAppConfig.cores);
            AppData::setNCpuCores(registeredApps[newAppPid]->data, newAppConfig.cores.size());
            AppData::setCpuFreq(registeredApps[newAppPid]->data, currCpuFreq);
            AppData::setGpuFreq(registeredApps[newAppPid]->data, currGpuFreq);
            registeredApps[newAppPid]->unlock();
        }

        unlock();
    }
}