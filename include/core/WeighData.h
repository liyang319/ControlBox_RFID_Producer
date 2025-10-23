#ifndef WEIGHDATA_H
#define WEIGHDATA_H

#include <queue>
#include <array>
#include <mutex>
#include "Base.h"
#include "GC31.h"
#include "ZY4701.h"
#include "DataDef.h"
#include <map>
#include "CycleQueue.h"

class WeighData
{

private:
    WeighData();                                      // private constructor to prevent instantiation
    WeighData(const WeighData &) = delete;            // delete the copy constructor
    WeighData &operator=(const WeighData &) = delete; // delete the assignment operator
    bool initialized;
    std::mutex data_mutex;
    std::string currentReportData;
    // std::mutex weigh_data_record_queue_mutex; // 保存的称重数据队列

public:
    CycleQueue weigh_data_record_queue;
    WeighDataUnit currentWeighData;
    float currentTareWeight;
    float loadCap;
    bool bOverload;

    int sensor_num; // 传感器数量                              // 当前位置信息

    static WeighData &getInstance();

    void init();

    void saveReportData(std::string data);

    std::string getReportData();

    void notifySensorSettingChanged(uint8_t sensorKey, bool bUpdated);
};

#endif // WEIGHDATA_H
