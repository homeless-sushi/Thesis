#ifndef POLICIES_MARGOT_ONLY_POLICY_H
#define POLICIES_MARGOT_ONLY_POLICY_H

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
    class MargotOnlyPolicy : public Policy
    {
        private:
            std::fstream controllerLogFile;
            std::fstream sensorLogFile;

            Utilization::Utilization utilization;
            Sensors::Sensors sensors;

        public:
            MargotOnlyPolicy(
                unsigned nCores,
                std::string controllerLogUrl,
                std::string sensorsLogUrl
            );

            void run(int cycle) override;
    };
}

#endif //POLICIES_MARGOT_ONLY_POLICY_H