#ifndef POLICIES_PYTHONPOLICY_H
#define POLICIES_PYTHONPOLICY_H

#include <fstream>
#include <set>
#include <string>

#include <sys/types.h>

#include "AppRegisterServer/Policy.h"
#include "AppRegisterServer/Sensors.h"
#include "AppRegisterServer/Utilization.h"

#include <zmqpp/zmqpp.hpp>

namespace Policy
{
    class PythonPolicy : public Policy
    {
        private:            
            std::fstream controllerLogFile;
            std::fstream sensorLogFile;

            Utilization::Utilization utilization;
            Sensors::Sensors sensors;

            // ZMQ
            zmqpp::context context;
            zmqpp::socket socket;
            
        public:
            PythonPolicy(
                unsigned nCores,
                std::string controllerLogUrl,
                std::string sensorsLogUrl,
                std::string serverEndpoint);

            ~PythonPolicy() override = default;

            void run(int cycle) override;
    };
}

#endif //POLICIES_PYTHONPOLICY_H