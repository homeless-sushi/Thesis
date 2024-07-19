#include "Policies/MargotOnlyPolicy.h"

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
    MargotOnlyPolicy::MargotOnlyPolicy(
        unsigned nCores,
        std::string controllerLogUrl,
        std::string sensorsLogUrl
    ) :
        Policy(nCores),
        controllerLogFile(controllerLogUrl, controllerLogFile.out),
        utilization(nCores),
        sensorLogFile(sensorsLogUrl, sensorLogFile.out)
    {
        controllerLogFile << "CYCLE,PID,NAME,APP,INPUT_SIZE,TARGET_THR,CURRENT_THR,MIN_PRECISION,CURR_PRECISION" << std::endl;

        sensorLogFile << "CYCLE,";
        for(unsigned i = 0; i < nCores; ++i)
            sensorLogFile << "UTILIZATION_" << i << ",";
        sensorLogFile << "CPUFREQ,GPUFREQ,SOCW,CPUW,GPUW" << std::endl;
    };

    void MargotOnlyPolicy::run(int cycle)
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
        registerNewApps();

        //set new apps registered and set current cpu and gpu frequency
        for(auto tuple = registeredApps.begin(); tuple != registeredApps.end(); ++tuple) {
            pid_t pid = tuple->first;
            registeredApps[pid]->lock();
            AppData::setRegistered(registeredApps[pid]->data, true);
            AppData::setCpuFreq(registeredApps[pid]->data, currCpuFreq);
            AppData::setGpuFreq(registeredApps[pid]->data, currGpuFreq);
            registeredApps[pid]->unlock();
        }

        unlock();
    }
}