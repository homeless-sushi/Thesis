#ifndef APP_REGISTER_SERVER_APP_UTILS
#define APP_REGISTER_SERVER_APP_UTILS

#include <vector>

#include <sys/types.h>

namespace AppUtils
{
    //Check if the given app is running
    bool isAppRunning(pid_t pid);
    //Get thread ids of the given app
    std::vector<pid_t> getAppPids(pid_t pid);
    //Kill the given app
    void killApp(pid_t pid);
    //Set CPU cores for the given app
    void UpdateCpuSet(pid_t pid, std::vector<int> cores);
    //Set CPU quotas for the given app
    void UpdateCpuQuota(pid_t pid, float quota);
}

#endif //APP_REGISTER_CONTROLLER_APP_UTILS
