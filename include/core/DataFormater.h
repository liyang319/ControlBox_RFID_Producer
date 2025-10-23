#ifndef DATAFORMATER_H
#define DATAFORMATER_H

#include <string>
#include "AppData.h"
#include "DataDef.h"
#include "WeighAlgorithm.h"
#include "DBExceptionData.h"



class DataFormater
{
public:
    static string FormatLocationData(GPSData &data, uint16_t handleflag);
    static std::string FormatSensorData(SensorDataUnit &data, uint16_t handleflag);
    static std::string FormatSensorLocationGroupData_FullData(WeighDataUnit &data, AlgorithmResult caculateResult);
    static std::string FormatSensorLocationGroupData_LocalMode(WeighDataUnit &data, AlgorithmResult caculateResult);
    static std::string FormatSensorLocationGroupData_FullData_Compress(WeighDataUnit &data, AlgorithmResult caculateResult);
    static std::string FormatSensorLocationGroupData_LocalMode_Compress(WeighDataUnit &data, AlgorithmResult caculateResult);
    static string FormatModeData(int localMode, bool bOfflineMode);
    static string FormatModifParamData();
    static string FormatCalibrationParamData(int index);
    static string FormatSensorParamData();
    static string FormatAlgorithmParamData(int index);
    static string FormatDtuParamData();
    static string FormatIniData();
    static string FormatJsonData();
    static string FormatExceptionUIContent(ExceptionDBDataUnit exception);
    static string GetExceptionSensor(uint8_t expSensors);
};

#endif // DATAFORMATER_H
