#ifndef APP_REGISTER_SERVER_SENSORS
#define APP_REGISTER_SERVER_SENSORS

#include <string>
#include <vector>

#define SENSOR_IN_W  "/sys/bus/i2c/drivers/ina3221x/6-0040/iio:device0/in_power0_input"
#define SENSOR_IN_A  "/sys/bus/i2c/drivers/ina3221x/6-0040/iio:device0/in_current0_input"
#define SENSOR_IN_V  "/sys/bus/i2c/drivers/ina3221x/6-0040/iio:device0/in_voltage0_input"
#define SENSOR_GPU_W  "/sys/bus/i2c/drivers/ina3221x/6-0040/iio:device0/in_power1_input"
#define SENSOR_GPU_A  "/sys/bus/i2c/drivers/ina3221x/6-0040/iio:device0/in_current1_input"
#define SENSOR_GPU_V  "/sys/bus/i2c/drivers/ina3221x/6-0040/iio:device0/in_voltage1_input"
#define SENSOR_CPU_W "/sys/bus/i2c/drivers/ina3221x/6-0040/iio:device0/in_power2_input"
#define SENSOR_CPU_A "/sys/bus/i2c/drivers/ina3221x/6-0040/iio:device0/in_current2_input"
#define SENSOR_CPU_V "/sys/bus/i2c/drivers/ina3221x/6-0040/iio:device0/in_voltage2_input"

namespace Sensors
{
    class Sensors 
    {
        public:
            void readSensors();
            float getSocW(){ return currSocW;};
            float getSocA(){ return currSocA;};
            float getSocV(){ return currSocV;};
            float getCpuW(){ return currCpuW;};
            float getCpuA(){ return currCpuA;};
            float getCpuV(){ return currCpuV;};
            float getGpuW(){ return currGpuW;};
            float getGpuA(){ return currGpuA;};
            float getGpuV(){ return currGpuV;};

            void sampleSensors();
            float getAvgSocW(){ float avgSocW = getAvg(prevSocW); prevSocW.clear(); return avgSocW; };
            float getAvgSocA(){ float avgSocA = getAvg(prevSocA); prevSocA.clear(); return avgSocA; };
            float getAvgSocV(){ float avgSocV = getAvg(prevSocV); prevSocV.clear(); return avgSocV; };
            float getAvgCpuW(){ float avgCpuW = getAvg(prevCpuW); prevCpuW.clear(); return avgCpuW; };
            float getAvgCpuA(){ float avgCpuA = getAvg(prevCpuA); prevCpuA.clear(); return avgCpuA; };
            float getAvgCpuV(){ float avgCpuV = getAvg(prevCpuV); prevCpuV.clear(); return avgCpuV; };
            float getAvgGpuW(){ float avgGpuW = getAvg(prevGpuW); prevGpuW.clear(); return avgGpuW; };
            float getAvgGpuA(){ float avgGpuA = getAvg(prevGpuA); prevGpuA.clear(); return avgGpuA; };
            float getAvgGpuV(){ float avgGpuV = getAvg(prevGpuV); prevGpuV.clear(); return avgGpuV; };
        private:
            //generic read sensor method
            float readSensor(std::string sensorPath);
            float getAvg(std::vector<float>& vec)
            {
                if (vec.size() == 0)
                    return 0.0;

                float sum = 0.0;
                for(float num : vec) {
                    sum += num;
                }

                return sum / vec.size();
            }
            
            float currSocW = 0;
            float currSocA = 0;
            float currSocV = 0;
            float currCpuW = 0;
            float currCpuA = 0;
            float currCpuV = 0;
            float currGpuW = 0;
            float currGpuA = 0;
            float currGpuV = 0;

            std::vector<float> prevSocW;
            std::vector<float> prevSocA;
            std::vector<float> prevSocV;
            std::vector<float> prevCpuW;
            std::vector<float> prevCpuA;
            std::vector<float> prevCpuV;
            std::vector<float> prevGpuW;
            std::vector<float> prevGpuA;
            std::vector<float> prevGpuV;
    };
}

#endif //APP_REGISTER_SERVER_SENSORS
