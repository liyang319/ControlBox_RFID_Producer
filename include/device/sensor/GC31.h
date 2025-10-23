#ifndef GC31_H
#define GC31_H

#include "Rs485.h"
#include "Device.h"
#include <unistd.h>
#include <iostream>
#include "DataDef.h"

#define DEFAULT_GC31_TIMEOUT 3000

#define CODE_CMD_GET_SN 0xa0
#define CODE_CMD_READ_DATA 0xa1
#define CODE_CMD_SET_FILTER 0Xa2
#define CODE_CMD_SET_EMPTYLOAD 0xa3
#define CODE_CMD_SET_GAIN 0Xa4
#define CODE_CMD_SET_REVERSE 0xa5
#define CODE_CMD_GET_SETTING 0xa6
#define CODE_CMD_RESET_DEVICE 0xa7
#define CODE_CMD_GET_VERSION 0xa8
#define CODE_CMD_SET_AUTOCALIBRATION 0xa9

#define CODE_RESPONSE_GET_SN 0xb0
#define CODE_RESPONSE_READ_DATA 0xb1
#define CODE_RESPONSE_SET_FILTER 0Xb2
#define CODE_RESPONSE_SET_EMPTYLOAD 0xb3
#define CODE_RESPONSE_SET_GAIN 0Xb4
#define CODE_RESPONSE_SET_REVERSE 0xb5
#define CODE_RESPONSE_GET_SETTING 0xb6
#define CODE_RESPONSE_RESET_DEVICE 0xb7
#define CODE_RESPONSE_GET_VERSION 0xb8
#define CODE_RESPONSE_SET_AUTOCALIBRATION 0xb9

class GC31 : public Rs485
{
public:
    static GC31 &getInstance()
    {
        static GC31 instance;
        return instance;
    }

    std::map<uint8_t, SensorDataTimerUnit> sensorMap;

    void init();

    void connectSerialPort();
    unsigned char getCrc(unsigned char *dataBuf, int dataLen);

    // bool getDeviceSN(unsigned char devAddr, std::string &strSN);                                                                                            // 读取设备SN
    // bool readMeasureData(unsigned char devAddr, unsigned char *dataBuf, int &dataLen);                                                                      // 读取测量数据
    // bool readMeasureData(unsigned char devAddr, uint16_t *chlVal, double *tempVal);                                                                         // 读取测量数据
    // bool setFilterParam(unsigned char devAddr, unsigned char filterType);                                                                                   // 设置滤波器
    // bool setEmptyLoadValue(unsigned char devAddr, uint16_t value);                                                                                          // 设置空载值
    // bool setGainParam(unsigned char devAddr, unsigned char *gain);                                                                                          // 设置增益
    // bool setReverseParam(unsigned char devAddr, unsigned char reverseVal);                                                                                  // 设置反向
    // bool getDeviceSetting(unsigned char devAddr, unsigned char *gain, unsigned char *reverseVal, unsigned char *filterType, unsigned char *autoClibration); // 获取配置
    // bool resetDevice(unsigned char devAddr);
    // bool getVersion(unsigned char devAddr, unsigned char &ver);
    // bool setAutoCalibration(unsigned char devAddr, unsigned char switchVal);
    void initTimer(map<uint8_t, SensorDataUnit> sensorDataMap);
    bool updateSensorTimer(uint8_t devAddr);
    bool checkTimeout(std::chrono::steady_clock::time_point currentTime, uint8_t &exception);
    int getSensorIDByAddr(uint8_t addr);
    // 发起指令函数，放入队列
    bool getDeviceSN(unsigned char devAddr);                               // 读取测量数据
    bool readMeasureData(unsigned char devAddr);                           // 读取测量数据
    bool setFilterParam(unsigned char devAddr, unsigned char filterType);  // 设置滤波器
    bool setEmptyLoadValue(unsigned char devAddr, uint16_t value);         // 设置空载值
    bool setGainParam(unsigned char devAddr, uint16_t *gain);         // 设置增益
    bool setReverseParam(unsigned char devAddr, unsigned char reverseVal); // 设置反向
    bool getDeviceSetting(unsigned char devAddr);                          // 获取配置
    bool resetDevice(unsigned char devAddr);
    bool getVersion(unsigned char devAddr);
    bool setAutoCalibration(unsigned char devAddr, unsigned char switchVal);
    // 处理接收指令函数，校验结果
    bool processGetDeviceSN(uint8_t *rxData, int rxLen, uint8_t &devAddr, std::string &strSN);                                                                    // 读取测量数据
    bool processGetMeasureData(uint8_t *rxData, int rxLen, uint8_t &devAddr, uint16_t *chlVal, double *tempVal);                                                  // 读取测量数据
    bool processSetFilterParam(uint8_t *rxData, int rxLen, uint8_t &devAddr);                                                                                     // 设置滤波器
    bool processSetEmptyLoadValue(uint8_t *rxData, int rxLen, uint8_t &devAddr);                                                                                  // 设置空载值
    bool processSetGainParam(uint8_t *rxData, int rxLen, uint8_t &devAddr);                                                                                       // 设置增益
    bool processSetReverseParam(uint8_t *rxData, int rxLen, uint8_t &devAddr);                                                                                    // 设置反向
    bool processGetDeviceSetting(uint8_t *rxData, int rxLen, uint8_t &devAddr, uint16_t *gain, uint16_t *reverseVal, uint16_t *filterType, uint16_t *autoClibration); // 获取配置
    bool processResetDevice(uint8_t *rxData, int rxLen, uint8_t &devAddr);
    bool processGetVersion(uint8_t *rxData, int rxLen, uint8_t &devAddr, uint8_t &ver);
    bool processSetAutoCalibration(uint8_t *rxData, int rxLen, uint8_t &devAddr);

    int readData(uint8_t *rxData, int len);
    int sendData(uint8_t *sendData, int len);

private:
    GC31();  // 构造函数
    ~GC31(); // 析构函数
    GC31(const GC31 &) = delete;
    void operator=(const GC31 &) = delete;
    int tryCount;
    bool initialized;
};

#endif
