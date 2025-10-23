#ifndef TDA04D_H
#define TDA04D_H

#include "Rs485.h"
#include "Device.h"
#include <unistd.h>
#include <iostream>
#include "DataDef.h"

#define DEFAULT_TDA04D_TIMEOUT 3000

#define CODE_TDA04D_CMD_READATA 0x01

#define DEFAULT_TDA04D_ADDR 0x01

// typedef struct _SensorDataTimerUnit
// {
//     int sensorID;
//     std::chrono::steady_clock::time_point lastTime;
// } SensorDataTimerUnit;

class TDA04d : public Rs485
{
public:
    static TDA04d &getInstance()
    {
        static TDA04d instance;
        return instance;
    }

    std::map<uint8_t, SensorDataTimerUnit> sensorMap;

    void init();

    void connectSerialPort();
    uint16_t getCrc(unsigned char *buf, int len);

    void initTimer(map<uint8_t, SensorDataUnit> sensorDataMap);
    bool updateSensorTimer(uint8_t devAddr);
    bool checkTimeout(std::chrono::steady_clock::time_point currentTime, uint8_t &exception);
    int getSensorIDByAddr(uint8_t devAddr);

    bool readMeasureData(unsigned char devAddr);
    bool processGetMeasureData(uint8_t *rxData, int rxLen, uint16_t *chlVal);

    int readData(uint8_t *rxData, int len);
    int sendData(uint8_t *sendData, int len);

private:
    TDA04d();  // 构造函数
    ~TDA04d(); // 析构函数
    TDA04d(const TDA04d &) = delete;
    void operator=(const TDA04d &) = delete;
    int tryCount;
    bool initialized;
};

#endif // TDA04D_H
