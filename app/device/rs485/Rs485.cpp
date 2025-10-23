#include <string.h>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <unistd.h>
#include <mutex>

#include "Base.h"
#include "Rs485.h"
#include "Utility.h"

using namespace std;

Rs485::Rs485()
{
    // COUT << "Rs485 init!" << endl;
}

Rs485::~Rs485()
{
    // COUT << "Rs485 destroy!" << endl;

    closeSerialPort();
    delete serialPort;
}

/**!SECTION
 * 连接继电器: com4
 */
void Rs485::connectSerialPort(int portNumber)
{
    serialPort = new SerialPort(portNumber);
    int ret = serialPort->portOpen();

    if (ret == -1)
    {
        COUT << "open com" << portNumber << " fail" << endl;
        return;
    }

    connected = true;
    COUT << "open com" << portNumber << " OK" << endl;
}

void Rs485::connectSerialPort(int portNumber, int baudrate, int databit, int stopbit, int parity)
{
    serialPort = new SerialPort(portNumber, baudrate, databit, stopbit, parity);
    int ret = serialPort->portOpen();
    if (ret == -1)
    {
        COUT << "open com" << portNumber << " fail" << endl;
        return;
    }

    connected = true;
    COUT << "open com" << portNumber << " OK" << endl;
}

void Rs485::closeSerialPort()
{
    if (connected)
    {
        COUT << "Close com" << portNumber << " OK" << endl;
        serialPort->portClose();
        connected = false;
    }
}

/**!SECTION
 * Modbus 协议数据输出
 */
int Rs485::writeDataToPort(char *data, int length)
{
    std::lock_guard<std::mutex> guard(rwMutex);

    int len = 0;
    if (connected)
    {
        len = serialPort->portSend(data, length);
    }
    else
    {
        COUT << "Please open serial port first!" << endl;
    }

    return len;
}

int Rs485::readDataFromPort(char *data, int length)
{
    std::lock_guard<std::mutex> guard(rwMutex);

    int len = serialPort->portRecv(data, length);
    return len;
}

// int Rs485::readRegister(char *data, char cmd)
// {
//     char rxData[100] = {0};
//     int rxLen = 0;

//     // modbus rtu
//     char buffer[8] = {0xfe, cmd, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00};
//     uint16_t crc1 = crc16((uint8_t *)buffer, 6);
//     buffer[6] = crc1 & 0xff;
//     buffer[7] = (crc1 & 0xff00) >> 8;

//     // 串口发送
//     int wx = writeDataToPort(buffer, 8);
//     if (wx < 0)
//     {
//         COUT << "write error!" << endl;
//     }

//     // 接收寄存器数据
//     rxLen = readDataFromPort(rxData, 100);
//     if (rxLen < 0)
//     {
//         COUT << "read error!" << endl;
//     }

//     //
//     if (rxLen > 0)
//     {
//         std::copy(rxData, rxData + rxLen, data);
//     }

//     return rxLen;
// }

uint16_t Rs485::crc16(uint8_t *data, int length)
{
    uint16_t crc = 0xFFFF;
    for (int i = 0; i < length; i++)
    {
        crc ^= data[i];
        for (int j = 0; j < 8; j++)
        {
            if (crc & 1)
            {
                crc >>= 1;
                crc ^= 0xA001;
            }
            else
            {
                crc >>= 1;
            }
        }
    }
    return crc;
}

bool Rs485::isConnected()
{
    return connected;
}
