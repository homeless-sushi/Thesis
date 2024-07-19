#include "AppRegisterServer/Utilization.h"

#include <iostream>	
#include <string>

#include <cstdio>	
#include <cstdlib> 
#include <cstring>

#include <unistd.h>

namespace Utilization
{
    Utilization::Utilization(int nCores) :
        nCores(nCores),
        prevUserUtilization(nCores, 0),
        prevSystemUtilization(nCores, 0),
        prevIdleUtilization(nCores, 0)
    {}

    int Utilization::computeCoreUtilization(
        int cpu_idx, 
        long int user, 
        long int nice, 
        long int system, 
        long int idle, 
        long int iowait, 
        long int irq, 
        long int softirq, 
        long int steal) 
    {
        long int total = 0;
        int usage = 0;
        long int diff_user, diff_system, diff_idle;
        long int curr_user, curr_system, curr_idle;

        curr_user = (user+nice);
        curr_system = (system+irq+softirq+steal);
        curr_idle = (idle+iowait);

        diff_user = prevUserUtilization[cpu_idx] - curr_user;
        diff_system = prevSystemUtilization[cpu_idx] - curr_system;
        diff_idle = prevIdleUtilization[cpu_idx] - curr_idle;

        total = diff_user + diff_system + diff_idle;
        if (total != 0)
            usage = (diff_user + diff_system) * 100 / total;

        prevUserUtilization[cpu_idx] = curr_user;
        prevSystemUtilization[cpu_idx] = curr_system;
        prevIdleUtilization[cpu_idx] = curr_idle;

        return usage;
    }

    std::vector<int> Utilization::computeUtilization()
    {
        char buf[80] = {'\0',};
        char cpuid[8] = {'\0',};
        long int user, system, nice, idle, iowait, irq, softirq, steal;
        std::vector<int> currUtilizations(nCores, 0);

        FILE *fp;
        fp = fopen("/proc/stat", "r");
        if (fp == NULL)
          return currUtilizations;

        fgets(buf, 80, fp); //discard first line since it contains aggregated values for the CPU
        char* r = fgets(buf, 80, fp);
        for(int cpu_index = 0; r && cpu_index < nCores; ++cpu_index) {
            char temp[5] = "cpu";
            temp[3] = '0' + cpu_index;
            temp[4] = '\0';
            
            if(!strncmp(buf, temp, 4)){
                sscanf(buf, "%s %ld %ld %ld %ld %ld %ld %ld %ld", cpuid, &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal);
                currUtilizations[cpu_index] = computeCoreUtilization(cpu_index, user, nice, system, idle, iowait, irq, softirq, steal);
                r = fgets(buf, 80, fp);
            }
        }
        fclose(fp);

        return currUtilizations;
    }
}
