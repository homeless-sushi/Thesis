#include "Policies/PythonPolicy.h"

#include <iostream>
#include <iterator>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <vector>

#include "AppRegisterServer/App.h"
#include "AppRegisterServer/AppData.h"
#include "AppRegisterServer/AppUtils.h"
#include "AppRegisterServer/Policy.h"
#include "AppRegisterServer/Sensors.h"
#include "AppRegisterServer/Utilization.h"
#include "AppRegisterServer/Frequency.h"
#include "AppRegisterServer/CGroupUtils.h"

#include <zmqpp/zmqpp.hpp>

namespace Policy
{
    PythonPolicy::PythonPolicy(
        unsigned nCores,
        std::string controllerLogUrl,
        std::string sensorsLogUrl,
        std::string serverEndpoint
    ) :
        Policy(nCores),
        controllerLogFile(controllerLogUrl, controllerLogFile.out),
        utilization(nCores),
        sensorLogFile(sensorsLogUrl, sensorLogFile.out),
        context(),
        socket(context, zmqpp::socket_type::req)
    {
        //Frequency::SetCpuFreq(Frequency::getMinCpuFreq());
        //Frequency::SetGpuFreq(Frequency::getMinGpuFreq());

        socket.connect(serverEndpoint);

        controllerLogFile << "CYCLE,PID,NAME,APP,INPUT_SIZE,TARGET_THR,CURRENT_THR,MIN_PRECISION,CURR_PRECISION" << std::endl;

        sensorLogFile << "CYCLE,";
        for(unsigned i = 0; i < nCores; ++i)
            sensorLogFile << "UTILIZATION_" << i << ",";
        sensorLogFile << "CPUFREQ,GPUFREQ,SOCW,CPUW,GPUW" << std::endl;
    };

    void PythonPolicy::run(int cycle)
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
            pairPidApp.second->currentPrecision = pairPidApp.second->data->curr_precision;
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
                << pairPidApp.second->currentPrecision << "," 
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

        //write outgoing message
        std::ostringstream messageToSend;
        messageToSend << "SYSTEM:" 
            << static_cast<int>(currCpuFreq) << "," 
            << static_cast<int>(currGpuFreq) << ","
            << cpuW + gpuW << "\n";

        messageToSend << "APPS:";
        for (const auto& pairPidApp : registeredApps) {

            messageToSend 
                << pairPidApp.first << ","
                << pairPidApp.second->descriptor.app_type << ","
                << pairPidApp.second->descriptor.input_size << ","
                << pairPidApp.second->descriptor.max_threads << ","
                << pairPidApp.second->descriptor.gpu_implementation << ","
                << pairPidApp.second->descriptor.approximate_application << ","
                << static_cast<double>(pairPidApp.second->data->requested_throughput) << ","
                << pairPidApp.second->data->minimum_precision << ","
                << static_cast<double>(pairPidApp.second->currentThroughput) << ","
                << pairPidApp.second->currentPrecision << ","
                << pairPidApp.second->data->use_gpu << ","
                << pairPidApp.second->data->n_cpu_cores  << ";";
        }
        socket.send(messageToSend.str());

        //read incoming system message
        std::string replyBuffer;
        socket.receive(replyBuffer);
        std::istringstream replyStream(replyBuffer);
        std::string systemLine;
        std::getline(replyStream, systemLine, '\n');
        std::string appsLine;
        std::getline(replyStream, appsLine, '\n');
        std::istringstream systemLineStream(systemLine);
        std::string systemPrefix;
        char separator;
        unsigned int newCpuFreq;
        unsigned int newGpuFreq;
        unsigned int newPow;
        std::getline(systemLineStream, systemPrefix, ':') 
            >> newCpuFreq >> separator 
            >> newGpuFreq >> separator
            >> newPow;
        if(currCpuFreq != static_cast<Frequency::CPU_FRQ>(newCpuFreq))
            Frequency::SetCpuFreq(static_cast<Frequency::CPU_FRQ>(newCpuFreq));
        if(currGpuFreq != static_cast<Frequency::GPU_FRQ>(newGpuFreq)){
            Frequency::SetGpuFreq(static_cast<Frequency::GPU_FRQ>(newGpuFreq));
        }
        int currentCpu = 0;
        std::istringstream appsLineStream(appsLine);
        std::string appsPrefix;
        std::getline(appsLineStream, appsPrefix, ':');
        std::string currAppInfo;
        std::set<pid_t> returnedApps; 
        while(std::getline(appsLineStream, currAppInfo, ';')){
            std::istringstream currAppLineStream(currAppInfo);
            pid_t appPid;
            std::string name;
            unsigned int size;
            unsigned int maxCpuCores;
            bool isGpu;
            bool isApproximate;
            float minimumThroughput;
            unsigned int minimumPrecision;
            float currentThroughput;
            unsigned int currentPrecision;
            bool onGpu;
            unsigned int nCpuCores;
            currAppLineStream >> appPid >> separator;
            std::getline(currAppLineStream, name, ',');
            currAppLineStream >> size >> separator
                >> maxCpuCores >> separator
                >> isGpu >> separator
                >> isApproximate >> separator
                >> minimumThroughput >> separator
                >> minimumPrecision >> separator
                >> currentThroughput >> separator
                >> currentPrecision >> separator
                >> onGpu >> separator
                >> nCpuCores;
            returnedApps.insert(appPid);
            registeredApps[appPid]->lock();
            if (!(registeredApps[appPid]->data->registered)) {
                AppData::setRegistered(registeredApps[appPid]->data, true);
            }
            registeredApps[appPid]->nAssignedCores = nCpuCores;
            AppData::setUseGpu(registeredApps[appPid]->data, onGpu);
            AppData::setCpuFreq(registeredApps[appPid]->data, newCpuFreq);
            AppData::setGpuFreq(registeredApps[appPid]->data, newGpuFreq);
            registeredApps[appPid]->unlock();
        }

        for(auto tuple = registeredApps.begin(); tuple != registeredApps.end(); ++tuple) {
            pid_t pid = tuple->first;
            if (std::find(returnedApps.begin(), returnedApps.end(), pid) == returnedApps.end()) {
                freeCores.insert(freeCores.end(), registeredApps[pid]->currentCores.begin(), registeredApps[pid]->currentCores.end());
                registeredApps.erase(pid);
                AppUtils::killApp(pid);
            }
        }

        //Lower the number of cores
        for(auto tuple = registeredApps.begin(); tuple != registeredApps.end(); ++tuple) {
            pid_t pid = tuple->first;
            const unsigned int nAssignedCores = registeredApps[pid]->nAssignedCores;
            const unsigned int nCurrentCores = registeredApps[pid]->currentCores.size();

            if(nAssignedCores >= nCurrentCores){
                continue;
            }

            for(int i = nAssignedCores; i < nCurrentCores; i++) {
                const unsigned int core = registeredApps[pid]->currentCores.back();
                registeredApps[pid]->currentCores.pop_back();
                freeCores.push_back(core);
            }
            registeredApps[pid]->lock();
            CGroupUtils::UpdateCpuSet(pid, registeredApps[pid]->currentCores);
            AppData::setNCpuCores(registeredApps[pid]->data, registeredApps[pid]->currentCores.size());
            registeredApps[pid]->unlock();
        }
        //Raise the number of cores
        for(auto tuple = registeredApps.begin(); tuple != registeredApps.end(); ++tuple) {
            pid_t pid = tuple->first;
            const unsigned int nAssignedCores = registeredApps[pid]->nAssignedCores;
            const unsigned int nCurrentCores = registeredApps[pid]->currentCores.size();

            if(nAssignedCores <= nCurrentCores){
                continue;
            }

            for(int i = nCurrentCores; i < nAssignedCores; i++) {
                const unsigned int core = freeCores.back();
                freeCores.pop_back();
                registeredApps[pid]->currentCores.push_back(core);
            }
            registeredApps[pid]->lock();
            CGroupUtils::UpdateCpuSet(pid, registeredApps[pid]->currentCores);
            AppData::setNCpuCores(registeredApps[pid]->data, registeredApps[pid]->currentCores.size());
            registeredApps[pid]->unlock();
        }

        unlock();
    }
}
