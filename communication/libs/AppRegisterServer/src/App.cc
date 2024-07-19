#include "AppRegisterServer/App.h"
#include "AppRegisterServer/AppData.h"
#include "AppRegisterServer/AppUtils.h"
#include "AppRegisterServer/CGroupUtils.h"

#include <algorithm>
#include <iostream>
#include <stdexcept>

#include <sys/shm.h>
#include <sys/types.h>

#include "AppRegisterCommon/AppRegister.h"
#include "AppRegisterCommon/Semaphore.h"

namespace App 
{
    App::App(
        app_descriptor descriptor,
        unsigned int ticksSWMaxSize
    ) :
        descriptor(descriptor),
        ticksSlidingWindow(),
        ticksSWMaxSize(ticksSWMaxSize)
    {
        data = (struct app_data*) shmat(descriptor.segment_id, 0, 0);
        if(data == (void*) -1){
            std::cerr << "ERROR: While attaching app_data in shared memory!" << std::endl;
        }

        int error = CGroupUtils::Initialize(descriptor.pid);
        if(error==-1){
            std::cerr << "ERROR: While initializing CGroups" << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    App::~App()
    {   
        nAssignedCores = 0;

        if (AppUtils::isAppRunning(descriptor.pid))
            AppUtils::killApp(descriptor.pid);

        int errCtl = shmctl(descriptor.segment_id, IPC_RMID, 0);
        int errDt = shmdt(data); 
        if(errCtl == -1 || errDt == -1){
            std::cerr << "ERROR: While detaching app_data from shared memory" << std::endl;
        }

        int error=CGroupUtils::Remove(descriptor.pid);
        if (error==-1){
            std::cerr << "ERROR: While removing CGroups" << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    void App::lock()
    {
        int semId = semget(descriptor.pid, 1, 0);
        binarySemaphorePost(semId);
    }

    void App::unlock()
    {
        int semId = semget(descriptor.pid, 1, 0);
        binarySemaphorePost(semId);
    }

    void App::readTicks()
    {
        if(ticksSlidingWindow.size() == ticksSWMaxSize){
            ticksSlidingWindow.pop_front();
        }
        ticksSlidingWindow.push_back(AppData::getCurrTicks(data));
    }

    struct ticks App::getWindowTicks(unsigned int sampleWindow)
    {
        if(sampleWindow <= 0 || sampleWindow > ticksSWMaxSize){
            throw std::range_error("Invalid sample window");
        }

        struct ticks ticksInWindow = {0, 0, 0};

        unsigned int currentSize = ticksSlidingWindow.size();
        unsigned int actualSampleWindow = std::min(sampleWindow, currentSize);

        for(unsigned int i = 0; i < actualSampleWindow; i++){
            ticksInWindow.value+=ticksSlidingWindow[currentSize-1 - i].value;
        } 

        ticksInWindow.start = ticksSlidingWindow[currentSize - actualSampleWindow].start;
        ticksInWindow.end = ticksSlidingWindow.back().end;
        
        return ticksInWindow;
    }
}
