#ifndef APP_REGISTER_SERVER_APP_REGISTER
#define APP_REGISTER_SERVER_APP_REGISTER

#include <vector>

#include <sys/types.h>

#include "AppRegisterCommon/AppRegister.h"

#define DEFAULT_TIME_WINDOW 1

namespace AppRegister
{
    //Initialize app_register
    struct app_register* registerCreate(int numOfCores);
    //Destroy app_register
    std::vector<pid_t> registerDestroy(struct app_register* app_register);
    //Initialize register semaphore
    void registerSemaphoreCreate();
    //Destroy register semaphore
    void registerSemaphoreDestroy();
}

#endif //APP_REGISTER_CONTROLLER_APP_REGISTER
