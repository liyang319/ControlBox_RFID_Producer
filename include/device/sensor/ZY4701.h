#ifndef ZY4701_H
#define ZY4701_H

#include "Rs485.h"
#include "Device.h"
#include <unistd.h>
#include <iostream>
#include "DataDef.h"

#define DEFAULT_ZY4701_TIMEOUT 8000

#define CODE_ZY4701_CMD_READATA 0x03

#define DEFAULT_ZY4701_ADDR 0x01

class ZY4701 : public Rs485
{
public:
    static ZY4701 &getInstance()
    {
        static ZY4701 instance;
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

    bool getDeviceSN(unsigned char devAddr, string &strSN, uint16_t &ver);                                                                    // 获取设备SN
    bool getDeviceSetting(unsigned char devAddr, int chlID, uint16_t &gainVal, uint16_t &reverseVal, uint16_t &filterVal, uint16_t &zeroVal); // 获取配置
    bool readMeasureData(unsigned char devAddr, double &tempVal, uint16_t *chlVal);
    bool setSingleReg(unsigned char devAddr, uint16_t regAddr, uint16_t regVal);
    bool setGainParam(unsigned char devAddr, int chlID, uint16_t gainVal);      // 设置增益
    bool setFilterParam(unsigned char devAddr, int chlID, uint16_t filterType); // 设置filterType

    bool processGetMeasureData(uint8_t *rxData, int rxLen, uint8_t devAddr, double &tempVal, uint16_t *chlVal);
    bool processGetDeviceSN(uint8_t *rxData, int rxLen, uint8_t devAddr, string &strSN, uint16_t &ver);
    bool processGetDeviceSetting(uint8_t *rxData, int rxLen, uint8_t devAddr, int chlID, uint16_t &gainVal, uint16_t &reverseVal, uint16_t &filterVal, uint16_t &zeroVal);
    bool processSetSingleReg(uint8_t *rxData, int rxLen, uint8_t devAddr, uint16_t regAddr);
    // bool processSetGainParam(uint8_t *rxData, int rxLen, uint8_t devAddr, int chlID);

    int readData(uint8_t *rxData, int len);
    int sendData(uint8_t *sendData, int len);

private:
    ZY4701();  // 构造函数
    ~ZY4701(); // 析构函数
    ZY4701(const ZY4701 &) = delete;
    void operator=(const ZY4701 &) = delete;
    int tryCount;
    bool initialized;
};

#endif // ZY4701_H
