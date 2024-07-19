#include "AppRegisterServer/Sensors.h"

#include <string>

namespace Sensors
{
    void Sensors::readSensors()
    {
        currSocW = readSensor(SENSOR_IN_W);
        currSocA = readSensor(SENSOR_IN_A);
        currSocV = readSensor(SENSOR_IN_V);
        currCpuW = readSensor(SENSOR_CPU_W);
        currCpuA = readSensor(SENSOR_CPU_A);
        currCpuV = readSensor(SENSOR_CPU_V);
        currGpuW = readSensor(SENSOR_GPU_W);
        currGpuA = readSensor(SENSOR_GPU_A);
        currGpuV = readSensor(SENSOR_GPU_V);
    };

    float Sensors::readSensor(std::string sensorPath)
    {
        float value;
        FILE* fp;
        fp = fopen(sensorPath.c_str(), "r");
        if(fp) {
            fscanf(fp, "%f", &value);
            fclose(fp);
        } else {
            value = -1;
        }
        return value;
    };

    void Sensors::sampleSensors(){
        prevSocW.push_back(readSensor(SENSOR_IN_W));
        prevSocA.push_back(readSensor(SENSOR_IN_A));
        prevSocV.push_back(readSensor(SENSOR_IN_V));
        prevCpuW.push_back(readSensor(SENSOR_CPU_W));
        prevCpuA.push_back(readSensor(SENSOR_CPU_A));
        prevCpuV.push_back(readSensor(SENSOR_CPU_V));
        prevGpuW.push_back(readSensor(SENSOR_GPU_W));
        prevGpuA.push_back(readSensor(SENSOR_GPU_A));
        prevGpuV.push_back(readSensor(SENSOR_GPU_V));
    };
}
