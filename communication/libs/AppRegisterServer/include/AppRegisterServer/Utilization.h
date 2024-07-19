#ifndef APP_REGISTER_SERVER_UTILIZATION
#define APP_REGISTER_SERVER_UTILIZATION

#include <vector>

namespace Utilization
{
    class Utilization 
    {
        public:   
            Utilization(int nCores);

            //Compute utilization across all cores
            std::vector<int> computeUtilization();
                
        private:
            //helper method to compute utilization
            int computeCoreUtilization(int cpu_idx, long int user, long int nice, long int system, long int idle, long int iowait, long int irq, long int softirq, long int steal);

            //since core usage is computed between two invocations of the getCPUUitlization() function. 
            //it is necessary to save values collected at the previous invocation to compute the difference with the currently sampled values
            //TODO: only the last value is saved, so no need for a vector
            std::vector<long int> prevUserUtilization;
            std::vector<long int> prevSystemUtilization;
            std::vector<long int> prevIdleUtilization;
            int nCores;
    };
}

#endif //APP_REGISTER_SERVER_UTILIZATION
