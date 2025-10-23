/**
 * @Class Rs485
 * @Brief RS485串口通信基类，负责与某个地址的RS485串口进行交互：打开、关闭、收发数据等 */

#pragma once

#include <mutex>
#include "../serial/SerialPort.h"

using namespace std;

class Rs485
{
protected:
    SerialPort *serialPort;
    bool connected;
    int portNumber;
    std::mutex rwMutex;

public:
    Rs485();
    ~Rs485();

protected:
    int writeDataToPort(char *data, int length);
    int readDataFromPort(char *data, int length);
    uint16_t crc16(uint8_t *data, int length);
    // int readRegister(char *data, char cmd);
    void connectSerialPort(int portNumber);
    void connectSerialPort(int portNumber, int baudrate, int databit, int stopbit, int parity);

public:
    virtual void connectSerialPort() = 0;
    void closeSerialPort();
    bool isConnected();
};
