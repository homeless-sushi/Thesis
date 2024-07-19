#include "AppRegisterServer/Frequency.h"

#include <algorithm>
#include <array>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <stdio.h>

namespace Frequency
{
    CPU_FRQ getCurrCpuFreq()
    {
        std::string cmd("sudo cat /sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_cur_freq");

        std::array<char, 128> buffer;
        std::string result;
        std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
        if (!pipe) {
            std::cerr << "ERROR: While calling popen()" << std::endl;
        }
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
            result += buffer.data();
        }

        return CPU_FRQ(std::stoul(result.c_str()));
    };

    CPU_FRQ getNextCpuFreq(CPU_FRQ currFrq)
    {
        CPU_FRQ freqs[] = { 
            FRQ_102000KHz,
            FRQ_204000KHz,
            FRQ_307200KHz,
            FRQ_403200KHz,
            FRQ_518400KHz,
            FRQ_614400KHz,
            FRQ_710400KHz,
            FRQ_825600KHz, 
            FRQ_921600KHz, 
            FRQ_1036800KHz, 
            FRQ_1132800KHz, 
            FRQ_1224000KHz, 
            FRQ_1326000KHz, 
            FRQ_1428000KHz, 
            FRQ_1479000KHz
        };

        unsigned i = 0;
        while(i < CPU_FRQ::N_CPU_FREQS-1 && currFrq >= freqs[i])
            i++;

        if (i < CPU_FRQ::N_CPU_FREQS) 
            return freqs[i];
        else 
            return freqs[CPU_FRQ::N_CPU_FREQS-1];
    };

    CPU_FRQ getNextCpuFreq(unsigned int currFrq)
    {
        CPU_FRQ freqs[] = { 
            FRQ_102000KHz,
            FRQ_204000KHz,
            FRQ_307200KHz,
            FRQ_403200KHz,
            FRQ_518400KHz,
            FRQ_614400KHz,
            FRQ_710400KHz,
            FRQ_825600KHz, 
            FRQ_921600KHz, 
            FRQ_1036800KHz, 
            FRQ_1132800KHz, 
            FRQ_1224000KHz, 
            FRQ_1326000KHz, 
            FRQ_1428000KHz, 
            FRQ_1479000KHz
        };

        unsigned i = 0;
        while(i < CPU_FRQ::N_CPU_FREQS-1 && currFrq >= static_cast<unsigned int>(freqs[i]))
            i++;

        
        if (i < CPU_FRQ::N_CPU_FREQS) 
            return freqs[i];
        else 
            return freqs[CPU_FRQ::N_CPU_FREQS-1];
    };

    int SetCpuFreq(CPU_FRQ freq){
        std::stringstream command;
        command
            << "sudo echo \"userspace\" | sudo tee /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor >> /dev/null && " << 
            "echo " << freq << " | sudo tee /sys/devices/system/cpu/cpu0/cpufreq/scaling_setspeed >> /dev/null && "
            "echo " << freq << " | sudo tee /sys/devices/system/cpu/cpu1/cpufreq/scaling_setspeed >> /dev/null && "
            "echo " << freq << " | sudo tee /sys/devices/system/cpu/cpu2/cpufreq/scaling_setspeed >> /dev/null && "
            "echo " << freq << " | sudo tee /sys/devices/system/cpu/cpu3/cpufreq/scaling_setspeed >> /dev/null" 
            << std::endl;
        return std::system(command.str().c_str());
    };

    GPU_FRQ getCurrGpuFreq()
    {
        std::string cmd("cat /sys/devices/57000000.gpu/devfreq/57000000.gpu/cur_freq");

        std::array<char, 128> buffer;
        std::string result;
        std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
        if (!pipe) {
            std::cerr << "ERROR: While calling popen()" << std::endl;
        }
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
            result += buffer.data();
        }

        return GPU_FRQ(std::stoul(result.c_str()));
    };

    GPU_FRQ getNextGpuFreq(GPU_FRQ currFrq)
    {
        GPU_FRQ freqs[] = { 
            FRQ_76800000Hz,
            FRQ_153600000Hz,
            FRQ_230400000Hz,
            FRQ_307200000Hz,
            FRQ_384000000Hz,
            FRQ_460800000Hz,
            FRQ_537600000Hz,
            FRQ_614400000Hz,
            FRQ_691200000Hz,
            FRQ_768000000Hz,
            FRQ_844800000Hz,
            FRQ_921600000Hz
        };

        unsigned i = 0;
        while(i < GPU_FRQ::N_GPU_FREQS-1 && currFrq >= freqs[i])
            i++;

        if (i < GPU_FRQ::N_GPU_FREQS) 
            return freqs[i];
        else 
            return freqs[GPU_FRQ::N_GPU_FREQS-1];
    };

    GPU_FRQ getNextGpuFreq(unsigned int currFrq)
    {
        GPU_FRQ freqs[] = { 
            FRQ_76800000Hz,
            FRQ_153600000Hz,
            FRQ_230400000Hz,
            FRQ_307200000Hz,
            FRQ_384000000Hz,
            FRQ_460800000Hz,
            FRQ_537600000Hz,
            FRQ_614400000Hz,
            FRQ_691200000Hz,
            FRQ_768000000Hz,
            FRQ_844800000Hz,
            FRQ_921600000Hz
        };

        unsigned i = 0;
        while(i < GPU_FRQ::N_GPU_FREQS-1 && currFrq >= static_cast<unsigned int>(freqs[i]))
            i++;

        
        if (i < GPU_FRQ::N_GPU_FREQS) 
            return freqs[i];
        else 
            return freqs[GPU_FRQ::N_GPU_FREQS-1];
    };

    int SetGpuFreq(GPU_FRQ freq){
        std::stringstream command;
        command
            << "echo 0 | sudo tee /sys/devices/57000000.gpu/devfreq/57000000.gpu/device/railgate_enable >> /dev/null && " <<
            "echo " << getMinGpuFreq() << " | sudo tee /sys/devices/57000000.gpu/devfreq/57000000.gpu/min_freq >> /dev/null && "
            "echo " << freq  << " | sudo tee /sys/devices/57000000.gpu/devfreq/57000000.gpu/max_freq >> /dev/null && "
            "echo " << freq << " | sudo tee /sys/devices/57000000.gpu/devfreq/57000000.gpu/min_freq >> /dev/null" 
            << std::endl;
        return std::system(command.str().c_str());
    };
}
