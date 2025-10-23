/**
 * @Class SerialPort
 * @Brief 串口通信基类，串口通信参数配置，打开、关闭、数据收发等 */

#pragma once
#include <string.h>
#include "PortInfo.h"

using namespace std;

class SerialPort
{
private:
    int baudrate;
    int databit;
    int stopbit;
    int parity;
    int fdCom; // 端口句柄

    int portNumber; // 端口号
    bool opened;    // COM端口是否已经打开

public:
    SerialPort(int portNumber);
    SerialPort(int portNumber, int baudrate, int databit, int stopbit, int parity);
    ~SerialPort();

    int portOpen();
    int portClose();
    int portSend(char *data, int datalen);
    int portRecv(char *data, int datalen);
    void setPortParameter(int baudrate, int databit, int stopbit, int parity);

private:
    int convbaud(unsigned long int baudrate);
    string convcom(unsigned short port);
    int portSet(const pportinfo_t pportinfo);
};
