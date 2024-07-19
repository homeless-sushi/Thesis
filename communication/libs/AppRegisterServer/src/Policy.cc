#include "AppRegisterServer/AppRegister.h"
#include "AppRegisterServer/AppUtils.h"
#include "AppRegisterServer/App.h"
#include "AppRegisterServer/Policy.h"
#include "AppRegisterServer/CGroupUtils.h"

#include <iostream>
#include <map>
#include <memory>
#include <vector>

#include <sys/shm.h>
#include <sys/types.h>

#include "AppRegisterCommon/AppRegister.h"
#include "AppRegisterCommon/Semaphore.h"

namespace Policy 
{
    Policy::Policy(unsigned int nCores) :
        nCores{nCores}
    {           
        for(unsigned int i = 0; i < nCores; i++){
            freeCores.push_back(i);
        }

        AppRegister::registerSemaphoreCreate();
        lock();
        appRegister = AppRegister::registerCreate(nCores);
        unlock();

        // Setup CGroups
        int error = CGroupUtils::Setup(nCores);
        if(error==-1){
            std::cerr << "ERROR: While setting up CGroup" << std::endl;
            exit(EXIT_FAILURE);
        }

        // Map the controller on the first CPU cluster
        pid_t controller_pid = getpid();
        error = CGroupUtils::Initialize(controller_pid);
        if(error==-1){
            std::cerr << "ERROR: While initializing CGroups" << std::endl;
            exit(EXIT_FAILURE);
        }
        std::vector<int> cpu0({0});
        error = CGroupUtils::UpdateCpuSet(controller_pid, cpu0);
        if(error==-1){
            std::cerr << "ERROR: While updating CGroups sets" << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    Policy::~Policy()
    {
        lock();
        std::vector<pid_t> newApps = AppRegister::registerDestroy(appRegister);
        unlock();
        AppRegister::registerSemaphoreDestroy();
        
        for(pid_t newApp : newApps)
            AppUtils::killApp(newApp);

        // Remove controller from CGroups
        pid_t controller_pid = getpid();
        int error = CGroupUtils::Remove(controller_pid);
        if(error==-1){
            std::cerr << "ERROR: While removing controller from CGroups" << std::endl;
            exit(EXIT_FAILURE);
        }
        // Destroy CGroups
        error = CGroupUtils::Destroy();
        if(error==-1){
            std::cerr << "ERROR: While destroying CGroups" << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    std::vector<pid_t> Policy::deregisterDeadApps()
    {
        std::vector<pid_t> deadApps;
        for(auto i = registeredApps.cbegin(), next_i = i; i != registeredApps.cend(); i = next_i){
            ++next_i;
            pid_t pid = i->first;
            if(AppUtils::isAppRunning(pid)){
                continue;
            }
            freeCores.insert(freeCores.end(), registeredApps[pid]->currentCores.begin(), registeredApps[pid]->currentCores.end());
            registeredApps.erase(i);
            deadApps.push_back(pid);
        }
        return deadApps;
    }

    std::vector<pid_t> Policy::deregisterDetachedApps()
    {        
        //deregister detached apps
        std::vector<pid_t> deregisteredApps;
        for(int i = 0; i < appRegister->n_detached; i++){
            pid_t detached_pid = appRegister->detached_apps[i];
            freeCores.insert(freeCores.end(), registeredApps[detached_pid]->currentCores.begin(), registeredApps[detached_pid]->currentCores.end());
            registeredApps.erase(detached_pid);
            deregisteredApps.push_back(detached_pid);
        }
        appRegister->n_detached = 0;

        return deregisteredApps;
    }

    std::vector<pid_t> Policy::registerNewApps()
    {
        //register new apps
        std::vector<pid_t> newApps;
        for(int i = 0; i < appRegister->n_new; i++){
            struct app_descriptor descriptor = appRegister->new_apps[i];
            registeredApps[descriptor.pid] = std::unique_ptr<App::App>(new App::App(descriptor));
            newApps.push_back(descriptor.pid);
        }
        appRegister->n_new = 0;

        return newApps;
    }

    void Policy::lock()
    {
        pid_t controllerId = getpid();
        int semId = semget (controllerId, 1, 0);
        binarySemaphoreWait(semId);
    }

    void Policy::unlock()
    {
        pid_t controllerId = getpid();
        int semId = semget (controllerId, 1, 0);
        binarySemaphorePost(semId);
    }

    long double getWindowThroughput(struct ticks windowTicks)
    {
        long double deltaTime = windowTicks.end - windowTicks.start;
        if(deltaTime == 0)
            return 0;
        
        return windowTicks.value/deltaTime;
    }
}
