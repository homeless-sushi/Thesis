#ifndef POLICIES_SET_CONFIGURATION_POLICY_H
#define POLICIES_SET_CONFIGURATION_POLICY_H

#include <fstream>
#include <map>
#include <set>
#include <string>

#include <sys/types.h>

#include "Utils/ConfigInfo.h"

#include "AppRegisterServer/Policy.h"
#include "AppRegisterServer/Sensors.h"
#include "AppRegisterServer/Utilization.h"
#include "AppRegisterServer/Frequency.h"

namespace Policy
{
    class SetConfigurationPolicy : public Policy
    {
        private:
            std::fstream controllerLogFile;
            std::fstream sensorLogFile;

            Utilization::Utilization utilization;
            Sensors::Sensors sensors;

            ConfigInfo::ConfigInfo config;

        public:
            SetConfigurationPolicy(
                unsigned nCores,
                std::string controllerLogUrl,
                std::string sensorsLogUrl,
                std::string configFileUrl
            );

            void run(int cycle) override;
    };
}

#endif //POLICIES_SET_CONFIGURATION_POLICY_H