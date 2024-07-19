#include "AppRegisterServer/CGroupUtils.h"

#include <iostream>
#include <string>
#include <vector>

#include <stdio.h>
#include <signal.h>
#include <sys/types.h>

namespace AppUtils
{    
    bool isAppRunning(pid_t pid) 
    {
        std::string cmdname = std::string("/proc/") + std::to_string(pid) + std::string("/status");
        FILE *fp;
        fp = fopen(cmdname.c_str(), "r");
        if (fp){
            fclose(fp);
            return true;
        }
        return false;
    }

    std::vector<pid_t> getAppPids(pid_t pid) 
    {
        std::vector<pid_t> pids;
        std::string pipe = std::string("ps -e -T | grep ")+ std::to_string(pid) + std::string(" | awk '{print $2}'");
        FILE *fp;
        fp = popen(pipe.c_str(), "r");
        if (fp == NULL){
            std::cerr << "ERROR: While reading app PIDs" << std::endl;
            exit (EXIT_FAILURE);
        }
        pid_t tmp_pid;
        fscanf(fp, "%d", &tmp_pid);
        while (!feof(fp)) {
            pids.push_back(tmp_pid);
            fscanf(fp, "%d", &tmp_pid);
        }
        pclose(fp);
        return pids;
    }

    void killApp(pid_t pid)
    {
        //Remove CGroup settings
        int error = CGroupUtils::Remove(pid);
        if (error==-1){
          std::cerr << "ERROR: While removing CGroups" << std::endl;
          exit(EXIT_FAILURE);
        }
        std::vector<pid_t> pids = getAppPids(pid);
        for (auto p : pids) {
            int error=CGroupUtils::Remove(p);
            if (error==-1){
                std::cerr << "ERROR: While removing CGroups" << std::endl;
                exit(EXIT_FAILURE);
            }      
        }
        //Kill process
        kill(pid, SIGINT);
    }

    void UpdateCpuSet(pid_t pid, std::vector<int> cores) 
    {
        int error=CGroupUtils::UpdateCpuSet(pid, cores);
        if (error==-1){
            std::cerr << "ERR0R: While updating CGroups" << std::endl;
            exit(EXIT_FAILURE);
        }   
    }

    void UpdateCpuQuota(pid_t pid, float quota) 
    {
        int error = CGroupUtils::UpdateCpuQuota(pid, quota);  
        if(error==-1){
            std::cerr << "ERR0R: While updating CGroups" << std::endl;
            exit(EXIT_FAILURE);
        }
    }
}