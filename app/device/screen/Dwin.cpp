#include "Dwin.h"
#include <iostream>
#include <sstream>
#include "Base.h"
#include "Utility.h"
#include "unistd.h"
#include "Device.h"
#include <vector>
#include "DataFormater.h"
#include "PortInfo.h"
#include <algorithm>
#include "GlobalFlag.h"
#include <vector>
#include <string>
#include <dirent.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>

#define MAX_TRY_CONNECT_TIME 3
#define DEFAULT_DWIN_TIMEOUT 10000
#define DEFAULT_DWIN_OTA_SMALL_BLOCK_SIZE 240
#define DEFAULT_DWIN_OTA_BIG_BLOCK_SIZE 32768

unsigned char DWIN_CMD_GET_VERSION[] = {0x5a, 0xa5, 0x04, 0x83, 0x000, 0x0f, 0x01};
unsigned char DWIN_CMD_GET_DWIN_VERSION[] = {0x5a, 0xa5, 0x04, 0x83, 0x000, 0x0f, 0x01};

// bool Dwin::initialized = false;

Dwin::Dwin() : initialized(false)
{
    // init();
}

void Dwin::init()
{
    tryCount = 0;
    if (initialized)
        return;
    while (!connected)
    {
        // connectSerialPort(RS232_1_PORT, DEFAULT_BAUDRATE, DEFAULT_DATABIT, DEFAULT_STOPBIT, DEFAULT_PARITY);
        connectSerialPort();
        if (++tryCount >= MAX_TRY_CONNECT_TIME)
            break;
        sleep(1);
    }
    if (connected)
        initialized = true;
}

Dwin::Dwin(int portNumber) : initialized(false)
{
    ; // connectSerialPort(RS232_1_PORT, 115200, DEFAULT_DATABIT, DEFAULT_STOPBIT, DEFAULT_PARITY);
}

void Dwin::connectSerialPort(int portNumber, int baudrate, int databit, int stopbit, int parity)
{
    Rs485::connectSerialPort(portNumber, baudrate, databit, stopbit, parity);
}

void Dwin::connectSerialPort()
{
    Rs485::connectSerialPort(SCREEN_PORT, 9600, DEFAULT_DATABIT, DEFAULT_STOPBIT, DEFAULT_PARITY);
}

Dwin::~Dwin()
{
    close();
}

bool Dwin::open()
{
    return isConnected();
}

void Dwin::close()
{
    closeSerialPort();
}

bool Dwin::getVersion()
{
    if (!isConnected())
    {
        COUT << "DWIN RS232 not connected!!!" << endl;
        return false;
    }
    bool bSuccess = false;
    strSN = "";
    if (GlobalFlag::getInstance().bScreenDataLogOn)
    {
        cout << "[SendData size=" << sizeof(DWIN_CMD_GET_VERSION) << "]: ";
        for (int i = 0; i < sizeof(DWIN_CMD_GET_VERSION); i++)
        {
            // fflush(stdout);
            cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(DWIN_CMD_GET_VERSION[i]) << " ";
        }
        cout << "" << endl;
    }
    int sendLen = writeDataToPort((char *)DWIN_CMD_GET_VERSION, sizeof(DWIN_CMD_GET_VERSION));
    if (sendLen <= 0)
    {
        COUT << "Failed to send data to DWIN!" << endl;
        return false;
    }
    int timeout = DEFAULT_DWIN_TIMEOUT;
    auto start = std::chrono::steady_clock::now(); // 获取当前时间
    while (true)
    {
        auto end = std::chrono::steady_clock::now();                                                // 获取当前时间
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count(); // 计算时间间隔
        if (duration >= timeout)
        {
            COUT << "----readSensorSN----timeout" << endl;
            break;
        }
        unsigned char rxData[DEFAULT_DWIN_DATA_BUFFER_SIZE] = {0};
        int rxLen = readDataFromPort((char *)rxData, sizeof(rxData));
        if (rxLen > 0)
        {
            if (GlobalFlag::getInstance().bScreenDataLogOn)
            {
                cout << "[RecvData size=" << rxLen << "]: ";
                for (int i = 0; i < rxLen; i++)
                {
                    // fflush(stdout);
                    cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(rxData[i]) << " ";
                }
                cout << "" << endl;
            }
            bSuccess = true;
            break;
        }
        usleep(10000); // 等待10毫秒
    }
    return bSuccess;
}

int Dwin::readData(uint8_t *rxData, int len)
{
    int rxLen = -1;
    if (!isConnected())
    {
        COUT << "DWIN Rs232 serial not connected!!!" << endl;
        return -1;
    }

    rxLen = readDataFromPort((char *)rxData, len);
    if (rxLen > 0)
    {
        if (GlobalFlag::getInstance().bScreenDataLogOn)
        {
            cout << "[RecvData size=" << rxLen << "]: ";
            for (int i = 0; i < rxLen; i++)
            {
                // fflush(stdout);
                cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(rxData[i]) << " ";
            }
            cout << "" << endl;
        }
    }
    else
    {
        ; // COUT << "[Receive ERROR]" << std::endl;
    }
    return rxLen;
}

bool Dwin::readFloatValue(uint16_t addr, float &result)
{
    if (!isConnected())
    {
        COUT << "DWIN Rs232 not connected!!!" << endl;
        return false;
    }
    bool bSuccess = false;
    uint8_t cmdBuf[] = {0x5a,
                        0xa5,
                        0x04,
                        UI_CMD_CODE_READ,
                        (uint8_t)((addr >> 8) & 0xFF),
                        (uint8_t)(addr & 0xFF),
                        UI_DATA_FLOAT_SIZE / 2};
    if (GlobalFlag::getInstance().bScreenDataLogOn)
    {
        cout << "[SendData size=" << sizeof(cmdBuf) << "]: ";
        for (int i = 0; i < sizeof(cmdBuf); i++)
        {
            // fflush(stdout);
            cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(cmdBuf[i]) << " ";
        }
        cout << "" << endl;
    }
    int sendLen = writeDataToPort((char *)cmdBuf, sizeof(cmdBuf));
    if (sendLen <= 0)
    {
        COUT << "Failed to send data to DWIN!" << endl;
        return false;
    }
    int timeout = DEFAULT_DWIN_TIMEOUT;
    auto start = std::chrono::steady_clock::now(); // 获取当前时间
    while (true)
    {
        auto end = std::chrono::steady_clock::now();                                                // 获取当前时间
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count(); // 计算时间间隔
        if (duration >= timeout)
        {
            COUT << "----read UI data----timeout" << endl;
            break;
        }
        unsigned char rxData[DEFAULT_DWIN_DATA_BUFFER_SIZE] = {0};
        int rxLen = readDataFromPort((char *)rxData, sizeof(rxData));
        if (rxLen > 0)
        {
            if (GlobalFlag::getInstance().bScreenDataLogOn)
            {
                cout << "[RecvData size=" << rxLen << "]: ";
                for (int i = 0; i < rxLen; i++)
                {
                    // fflush(stdout);
                    cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(rxData[i]) << " ";
                }
                cout << "" << endl;
            }
            // uint8_t dataBuf[UI_DATA_FLOAT_SIZE] = {0};
            if (rxLen == 11 && rxData[6] == 0x02)
            {
                // printf("---copy--\n");
                uint8_t dataBuf[UI_DATA_FLOAT_SIZE] = {0};
                Utility::convert_endian(rxData + 7, UI_DATA_FLOAT_SIZE);
                memcpy(&result, rxData + 7, UI_DATA_FLOAT_SIZE);
            }
            bSuccess = true;
            break;
        }
        usleep(10000); // 等待10毫秒
    }
    return bSuccess;
}

bool Dwin::writeFloatValue(uint16_t addr, float data)
{
    if (!isConnected())
    {
        COUT << "DWIN Rs232 not connected!!!" << endl;
        return false;
    }
    bool bSuccess = false;
    uint8_t cmdBuf[] = {0x5a,
                        0xa5,
                        0x07,
                        UI_CMD_CODE_WRITE,
                        (uint8_t)((addr >> 8) & 0xFF),
                        (uint8_t)(addr & 0xFF),
                        0x00,
                        0x00,
                        0x00,
                        0x00};

    uint8_t *dataPtr = (uint8_t *)&data;
    memcpy(cmdBuf + 6, dataPtr, UI_DATA_FLOAT_SIZE);
    Utility::convert_endian(cmdBuf + 6, UI_DATA_FLOAT_SIZE);

    if (GlobalFlag::getInstance().bScreenDataLogOn)
    {
        cout << "[SendData size=" << sizeof(cmdBuf) << "]: ";
        for (int i = 0; i < sizeof(cmdBuf); i++)
        {
            // fflush(stdout);
            cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(cmdBuf[i]) << " ";
        }
        cout << "" << endl;
    }
    int sendLen = writeDataToPort((char *)cmdBuf, sizeof(cmdBuf));
    if (sendLen <= 0)
    {
        COUT << "Failed to send data to DWIN!" << endl;
        return false;
    }
    int timeout = DEFAULT_DWIN_TIMEOUT;
    auto start = std::chrono::steady_clock::now(); // 获取当前时间
    while (true)
    {
        auto end = std::chrono::steady_clock::now();                                                // 获取当前时间
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count(); // 计算时间间隔
        if (duration >= timeout)
        {
            COUT << "----read UI data----timeout" << endl;
            break;
        }
        unsigned char rxData[DEFAULT_DWIN_DATA_BUFFER_SIZE] = {0};
        int rxLen = readDataFromPort((char *)rxData, sizeof(rxData));
        if (rxLen > 0)
        {
            if (GlobalFlag::getInstance().bScreenDataLogOn)
            {
                cout << "[RecvData size=" << rxLen << "]: ";
                for (int i = 0; i < rxLen; i++)
                {
                    // fflush(stdout);
                    cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(rxData[i]) << " ";
                }
                cout << "" << endl;
            }
            // uint8_t dataBuf[UI_DATA_FLOAT_SIZE] = {0};

            bSuccess = true;
            break;
        }
        usleep(10000); // 等待10毫秒
    }
    return bSuccess;
}

bool Dwin::writeStringValue(uint16_t addr, std::string &data)
{
    if (!isConnected())
    {
        COUT << "DWIN Rs232 not connected!!!" << endl;
        return false;
    }
    bool bSuccess = false;
    std::string strGB2312 = Utility::convertToGB2312(data);
    uint8_t *pStrData = (uint8_t *)strGB2312.c_str();
    int dataLen = strGB2312.size();
    // Utility::convert_endian(pStrData, dataLen);
    int cmdLen = dataLen + 6;
    uint8_t *cmdBuf = new uint8_t[6 + dataLen];
    memset(cmdBuf, 0, 6 + dataLen);
    cmdBuf[0] = 0x5a;
    cmdBuf[1] = 0xa5;
    cmdBuf[2] = dataLen + 3;
    cmdBuf[3] = UI_CMD_CODE_WRITE;
    cmdBuf[4] = (uint8_t)((addr >> 8) & 0xFF);
    cmdBuf[5] = (uint8_t)(addr & 0xFF);
    memcpy(cmdBuf + 6, pStrData, dataLen);

    if (GlobalFlag::getInstance().bScreenDataLogOn)
    {
        cout << "[SendData size=" << cmdLen << "]: ";
        for (int i = 0; i < cmdLen; i++)
        {
            // fflush(stdout);
            cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(cmdBuf[i]) << " ";
        }
        cout << "" << endl;
    }
    int sendLen = writeDataToPort((char *)cmdBuf, cmdLen);
    if (sendLen <= 0)
    {
        COUT << "Failed to send data to DWIN!" << endl;
        return false;
    }
    if (cmdBuf != nullptr)
        delete cmdBuf;
    int timeout = DEFAULT_DWIN_TIMEOUT;
    auto start = std::chrono::steady_clock::now(); // 获取当前时间
    while (true)
    {
        auto end = std::chrono::steady_clock::now();                                                // 获取当前时间
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count(); // 计算时间间隔
        if (duration >= timeout)
        {
            COUT << "----read UI data----timeout" << endl;
            break;
        }
        unsigned char rxData[DEFAULT_DWIN_DATA_BUFFER_SIZE] = {0};
        int rxLen = readDataFromPort((char *)rxData, sizeof(rxData));
        if (rxLen > 0)
        {
            if (GlobalFlag::getInstance().bScreenDataLogOn)
            {
                cout << "[RecvData size=" << rxLen << "]: ";
                for (int i = 0; i < rxLen; i++)
                {
                    // fflush(stdout);
                    cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(rxData[i]) << " ";
                }
                cout << "" << endl;
            }
            // uint8_t dataBuf[UI_DATA_FLOAT_SIZE] = {0};
            bSuccess = true;
            break;
        }
        usleep(10000); // 等待10毫秒
    }
    return bSuccess;
}

bool Dwin::readStringValue(uint16_t addr, std::string &data, int len)
{
    if (!isConnected())
    {
        COUT << "DWIN Rs232 not connected!!!" << endl;
        return false;
    }
    bool bSuccess = false;
    // std::string strGB2312 = Utility::convertToGB2312(data);
    // uint8_t *pStrData = (uint8_t *)strGB2312.c_str();
    // int dataLen = strGB2312.size();
    // Utility::convert_endian(pStrData, dataLen);
    int cmdLen = 7;
    uint8_t *cmdBuf = new uint8_t[7];
    memset(cmdBuf, 0, 7);
    cmdBuf[0] = 0x5a;
    cmdBuf[1] = 0xa5;
    cmdBuf[2] = 0x04;
    cmdBuf[3] = UI_CMD_CODE_READ;
    cmdBuf[4] = (uint8_t)((addr >> 8) & 0xFF);
    cmdBuf[5] = (uint8_t)(addr & 0xFF);
    cmdBuf[6] = len / 2;

    if (GlobalFlag::getInstance().bScreenDataLogOn)
    {
        cout << "[SendData size=" << cmdLen << "]: ";
        for (int i = 0; i < cmdLen; i++)
        {
            // fflush(stdout);
            cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(cmdBuf[i]) << " ";
        }
        cout << "" << endl;
    }
    int sendLen = writeDataToPort((char *)cmdBuf, cmdLen);
    if (sendLen <= 0)
    {
        COUT << "Failed to send data to DWIN!" << endl;
        return false;
    }
    if (cmdBuf != nullptr)
        delete cmdBuf;
    int timeout = DEFAULT_DWIN_TIMEOUT;
    auto start = std::chrono::steady_clock::now(); // 获取当前时间
    while (true)
    {
        auto end = std::chrono::steady_clock::now();                                                // 获取当前时间
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count(); // 计算时间间隔
        if (duration >= timeout)
        {
            COUT << "----read UI data----timeout" << endl;
            break;
        }
        unsigned char rxData[DEFAULT_DWIN_DATA_BUFFER_SIZE] = {0};
        int rxLen = readDataFromPort((char *)rxData, sizeof(rxData));
        if (rxLen > 0)
        {
            if (GlobalFlag::getInstance().bScreenDataLogOn)
            {
                cout << "[RecvData size=" << rxLen << "]: ";
                for (int i = 0; i < rxLen; i++)
                {
                    // fflush(stdout);
                    cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(rxData[i]) << " ";
                }
                cout << "" << endl;
            }
            // uint8_t dataBuf[UI_DATA_FLOAT_SIZE] = {0};
            if (rxLen == (len + 7))
            {
                uint8_t *pBuf = (rxData + 7);
                string strData(reinterpret_cast<char *>(pBuf), len);
                data = strData;
            }
            bSuccess = true;
            break;
        }
        usleep(10000); // 等待10毫秒
    }
    return bSuccess;
}

bool Dwin::clearStringValue(uint16_t addr, int len)
{
    if (!isConnected())
    {
        COUT << "DWIN Rs232 not connected!!!" << endl;
        return false;
    }
    bool bSuccess = false;
    int cmdLen = 6 + len;
    uint8_t *cmdBuf = new uint8_t[cmdLen];
    memset(cmdBuf, 0, cmdLen);
    cmdBuf[0] = 0x5a;
    cmdBuf[1] = 0xa5;
    cmdBuf[2] = len + 3;
    cmdBuf[3] = UI_CMD_CODE_WRITE;
    cmdBuf[4] = (uint8_t)((addr >> 8) & 0xFF);
    cmdBuf[5] = (uint8_t)(addr & 0xFF);
    memset(cmdBuf + 6, 0xff, len);

    if (GlobalFlag::getInstance().bScreenDataLogOn)
    {
        cout << "[SendData size=" << cmdLen << "]: ";
        for (int i = 0; i < cmdLen; i++)
        {
            // fflush(stdout);
            cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(cmdBuf[i]) << " ";
        }
        cout << "" << endl;
    }

    int sendLen = writeDataToPort((char *)cmdBuf, cmdLen);
    if (sendLen <= 0)
    {
        COUT << "Failed to send data to DWIN!" << endl;
        return false;
    }
    if (cmdBuf != nullptr)
        delete cmdBuf;
    int timeout = DEFAULT_DWIN_TIMEOUT;
    auto start = std::chrono::steady_clock::now(); // 获取当前时间
    while (true)
    {
        auto end = std::chrono::steady_clock::now();                                                // 获取当前时间
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count(); // 计算时间间隔
        if (duration >= timeout)
        {
            COUT << "----read UI data----timeout" << endl;
            break;
        }
        unsigned char rxData[DEFAULT_DWIN_DATA_BUFFER_SIZE] = {0};
        int rxLen = readDataFromPort((char *)rxData, sizeof(rxData));
        if (rxLen > 0)
        {
            if (GlobalFlag::getInstance().bScreenDataLogOn)
            {
                cout << "[RecvData size=" << rxLen << "]: ";
                for (int i = 0; i < rxLen; i++)
                {
                    // fflush(stdout);
                    cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(rxData[i]) << " ";
                }
                cout << "" << endl;
            }
            // uint8_t dataBuf[UI_DATA_FLOAT_SIZE] = {0};
            bSuccess = true;
            break;
        }
        usleep(10000); // 等待10毫秒
    }
    return bSuccess;
}

bool Dwin::switchPage(int pageIndex)
{
    if (!isConnected())
    {
        COUT << "DWIN Rs232 not connected!!!" << endl;
        return false;
    }
    bool bSuccess = false;
    uint8_t cmdBuf[] = {0x5a, 0xa5, 0x07, UI_CMD_CODE_WRITE, 0x00, 0x84, 0x5a, 0x01, 0x00, 0x00};

    cmdBuf[8] = (uint8_t)((pageIndex >> 8) & 0xFF);
    cmdBuf[9] = (uint8_t)(pageIndex & 0xFF);

    if (GlobalFlag::getInstance().bScreenDataLogOn)
    {
        cout << "[SendData size=" << sizeof(cmdBuf) << "]: ";
        for (int i = 0; i < sizeof(cmdBuf); i++)
        {
            // fflush(stdout);
            cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(cmdBuf[i]) << " ";
        }
        cout << "" << endl;
    }

    int sendLen = writeDataToPort((char *)cmdBuf, sizeof(cmdBuf));
    if (sendLen <= 0)
    {
        COUT << "Failed to send data to GC31!" << endl;
        return false;
    }
    int timeout = DEFAULT_DWIN_TIMEOUT;
    auto start = std::chrono::steady_clock::now(); // 获取当前时间
    while (true)
    {
        auto end = std::chrono::steady_clock::now();                                                // 获取当前时间
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count(); // 计算时间间隔
        if (duration >= timeout)
        {
            COUT << "----read UI data----timeout" << endl;
            break;
        }
        unsigned char rxData[DEFAULT_DWIN_DATA_BUFFER_SIZE] = {0};
        int rxLen = readDataFromPort((char *)rxData, sizeof(rxData));
        if (rxLen > 0)
        {
            if (GlobalFlag::getInstance().bScreenDataLogOn)
            {
                cout << "[RecvData size=" << rxLen << "]: ";
                for (int i = 0; i < rxLen; i++)
                {
                    // fflush(stdout);
                    cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(rxData[i]) << " ";
                }
                cout << "" << endl;
            }
            bSuccess = true;
            break;
        }
        usleep(10000); // 等待10毫秒
    }
    return bSuccess;
}

bool Dwin::getDwinVersion(string &strVersion)
{

    bool bSuccess = false;
    bSuccess = readStringValue(UI_ADDR_DWIN_VERSION, strVersion, 16);
    return bSuccess;
}

bool Dwin::syncSystemTime()
{
    if (!isConnected())
    {
        COUT << "DWIN Rs232 not connected!!!" << endl;
        return false;
    }
    bool bSuccess = false;
    uint8_t cmdBuf[] = {0x5a, 0xa5, 0x0b, UI_CMD_CODE_WRITE, 0x00, 0x9c, 0x5a, 0xa5, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    time_t t = time(nullptr);
    struct tm *now = localtime(&t);
    cmdBuf[8] = now->tm_year + 1900 - 2000; // 年份减去2000
    cmdBuf[9] = now->tm_mon + 1;            // 月份从0开始，加1
    cmdBuf[10] = now->tm_mday;
    cmdBuf[11] = now->tm_hour;
    cmdBuf[12] = now->tm_min;
    cmdBuf[13] = now->tm_sec;

    if (GlobalFlag::getInstance().bScreenDataLogOn)
    {
        cout << "[SendData size=" << sizeof(cmdBuf) << "]: ";
        for (int i = 0; i < sizeof(cmdBuf); i++)
        {
            // fflush(stdout);
            cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(cmdBuf[i]) << " ";
        }
        cout << "" << endl;
    }

    int sendLen = writeDataToPort((char *)cmdBuf, sizeof(cmdBuf));
    if (sendLen <= 0)
    {
        COUT << "Failed to send data to GC31!" << endl;
        return false;
    }
    int timeout = DEFAULT_DWIN_TIMEOUT;
    auto start = std::chrono::steady_clock::now(); // 获取当前时间
    while (true)
    {
        auto end = std::chrono::steady_clock::now();                                                // 获取当前时间
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count(); // 计算时间间隔
        if (duration >= timeout)
        {
            COUT << "----read UI data----timeout" << endl;
            break;
        }
        unsigned char rxData[DEFAULT_DWIN_DATA_BUFFER_SIZE] = {0};
        int rxLen = readDataFromPort((char *)rxData, sizeof(rxData));
        if (rxLen > 0)
        {
            if (GlobalFlag::getInstance().bScreenDataLogOn)
            {
                cout << "[RecvData size=" << rxLen << "]: ";
                for (int i = 0; i < rxLen; i++)
                {
                    // fflush(stdout);
                    cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(rxData[i]) << " ";
                }
                cout << "" << endl;
            }
            bSuccess = true;
            break;
        }
        usleep(10000); // 等待10毫秒
    }
    return bSuccess;
}

bool Dwin::setIcon(uint16_t addr, uint16_t index)
{
    if (!isConnected())
    {
        COUT << "DWIN Rs232 not connected!!!" << endl;
        return false;
    }
    bool bSuccess = false;
    // Utility::convert_endian(pStrData, dataLen);
    uint8_t cmdBuf[8] = {0};
    cmdBuf[0] = 0x5a;
    cmdBuf[1] = 0xa5;
    cmdBuf[2] = 0x05;
    cmdBuf[3] = UI_CMD_CODE_WRITE;
    cmdBuf[4] = (uint8_t)((addr >> 8) & 0xFF);
    cmdBuf[5] = (uint8_t)(addr & 0xFF);
    cmdBuf[6] = (uint8_t)((index >> 8) & 0xFF);
    cmdBuf[7] = (uint8_t)(index & 0xFF);

    if (GlobalFlag::getInstance().bScreenDataLogOn)
    {
        cout << "[SendData size=" << sizeof(cmdBuf) << "]: ";
        for (int i = 0; i < sizeof(cmdBuf); i++)
        {
            // fflush(stdout);
            cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(cmdBuf[i]) << " ";
        }
        cout << "" << endl;
    }
    int sendLen = writeDataToPort((char *)cmdBuf, sizeof(cmdBuf));
    if (sendLen <= 0)
    {
        COUT << "Failed to send data to DWIN!" << endl;
        return false;
    }
    int timeout = DEFAULT_DWIN_TIMEOUT;
    auto start = std::chrono::steady_clock::now(); // 获取当前时间
    while (true)
    {
        auto end = std::chrono::steady_clock::now();                                                // 获取当前时间
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count(); // 计算时间间隔
        if (duration >= timeout)
        {
            COUT << "----read UI data----timeout" << endl;
            break;
        }
        unsigned char rxData[DEFAULT_DWIN_DATA_BUFFER_SIZE] = {0};
        int rxLen = readDataFromPort((char *)rxData, sizeof(rxData));
        if (rxLen > 0)
        {
            if (GlobalFlag::getInstance().bScreenDataLogOn)
            {
                cout << "[RecvData size=" << rxLen << "]: ";
                for (int i = 0; i < rxLen; i++)
                {
                    // fflush(stdout);
                    cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(rxData[i]) << " ";
                }
                cout << "" << endl;
            }
            // uint8_t dataBuf[UI_DATA_FLOAT_SIZE] = {0};
            bSuccess = true;
            break;
        }
        usleep(10000); // 等待10毫秒
    }
    return bSuccess;
}

bool getFileContent(std::string filePath, int offset, uint8_t *buf, int &len)
{
    // 打开文件
    std::ifstream file(filePath, std::ios::binary);

    // 检查文件是否成功打开
    if (!file.is_open())
    {
        std::cerr << "Error opening file: " << filePath << std::endl;
        return false;
    }

    // 移动到指定的偏移量
    file.seekg(offset, std::ios::beg);
    if (file.fail())
    {
        std::cerr << "Error seeking to offset: " << offset << std::endl;
        return false;
    }

    // 读取数据
    file.read(reinterpret_cast<char *>(buf), len);

    // 更新实际读取的长度
    len = file.gcount();

    // 关闭文件
    file.close();

    // 返回是否成功
    return true;
}

int getFileSize(std::string filePath)
{
    // 打开文件
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open())
    {
        std::cerr << "无法打开文件: " << filePath << std::endl;
        return -1; // 返回 -1 表示错误
    }

    // 移动到文件末尾
    file.seekg(0, std::ios::end);
    if (!file.good())
    {
        std::cerr << "无法定位到文件末尾: " << filePath << std::endl;
        file.close();
        return -1; // 返回 -1 表示错误
    }

    // 获取文件大小
    std::streampos fileSize = file.tellg();
    file.close(); // 关闭文件

    return static_cast<int>(fileSize); // 返回文件大小
}

bool enumFiles(const std::string &path, std::vector<std::string> &files)
{
    DIR *dir = opendir(path.c_str());
    if (dir == nullptr)
    {
        std::cerr << "Error opening directory: " << path << std::endl;
        return false; // 无法打开目录
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != nullptr)
    {
        // 忽略 "." 和 ".." 目录
        if (entry->d_name[0] == '.')
        {
            continue;
        }
        files.push_back(entry->d_name); // 添加文件名到vector中
    }

    closedir(dir); // 关闭目录
    return true;   // 成功
}

bool getFileIndex(const std::string fileName, int &index)
{
    index = 0; // 初始化index

    // 逐个检查字符，直到遇到非数字字符
    for (size_t i = 0; i < fileName.size(); ++i)
    {
        if (std::isdigit(fileName[i]))
        {
            index = index * 10 + (fileName[i] - '0'); // 计算数字
        }
        else
        {
            // 一旦遇到非数字字符，检查是否已经读取了数字
            return i > 0; // 如果i>0，说明前面有数字
        }
    }

    // 如果循环完成后仍然没有遇到非数字字符，说明全是数字
    return true; // 说明文件名是纯数字
}

bool getFilenameFromPath(const std::string &filePath, std::string &fileName)
{
    // 查找最后一个斜杠的位置
    size_t lastSlashPos = filePath.find_last_of("/\\");

    // 如果找到了斜杠，则提取文件名
    if (lastSlashPos != std::string::npos)
    {
        fileName = filePath.substr(lastSlashPos + 1);
    }
    else
    {
        // 如果没有找到斜杠，说明整个路径就是文件名
        fileName = filePath;
    }

    // 检查文件名是否为空
    return !fileName.empty();
}

bool Dwin::resetDwin()
{
    // 重启屏幕
    bool bSuccess = false;
    uint8_t cmdBuf_reset[] = {0x5a, 0xa5, 0x07, 0x82, 0x00, 0x04, 0x55, 0xaa, 0x5a, 0xa5};
    int sendLen2 = writeDataToPort((char *)cmdBuf_reset, sizeof(cmdBuf_reset));
    cout << "[reset SendData size=" << sendLen2 << "]: ";
    for (int j = 0; j < sendLen2; j++)
    {
        cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(cmdBuf_reset[j]) << " ";
    }
    cout << "" << endl;
    int timeout = DEFAULT_DWIN_TIMEOUT;
    auto start = std::chrono::steady_clock::now(); // 获取当前时间
    while (true)
    {
        auto end = std::chrono::steady_clock::now();                                                // 获取当前时间
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count(); // 计算时间间隔
        if (duration >= timeout)
        {
            COUT << "----read UI data----timeout" << endl;
            return false;
        }
        unsigned char rxData[DEFAULT_DWIN_DATA_BUFFER_SIZE] = {0};
        int rxLen = readDataFromPort((char *)rxData, sizeof(rxData));
        if (rxLen > 0)
        {
            cout << "[RecvData size=" << rxLen << "]: ";
            for (int i = 0; i < rxLen; i++)
            {
                // fflush(stdout);
                cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(rxData[i]) << " ";
            }
            cout << "" << endl;
            bSuccess = true;
            break;
        }
        usleep(10000); // 等待10毫秒
    }
    return true;
}

bool Dwin::downloadFile(std::string filePath)
{
    int fileSize = getFileSize(filePath);
    bool bSuccess = false;
    cout << "------file size-------" << std::dec << fileSize << endl;
    int numBigBlock = fileSize / DEFAULT_DWIN_OTA_BIG_BLOCK_SIZE + 1; // 32k的个数
    cout << "----numBigBlock------" << std::dec << numBigBlock << endl;

    string fileName = "";
    getFilenameFromPath(filePath, fileName);
    int fileIndex = 0;
    getFileIndex(fileName, fileIndex);
    int blockStartAddr = fileIndex * 8;
    cout << "------fileIndex-------" << std::hex << blockStartAddr << endl;

    // int blockStartAddr = 0x00b8;                                                               // 0x00b8; //      b8*32/256 = 23
    int numMaxSmall = DEFAULT_DWIN_OTA_BIG_BLOCK_SIZE / DEFAULT_DWIN_OTA_SMALL_BLOCK_SIZE + 1; // 32k中包含240的个数
    for (int i = 0; i < numBigBlock; i++)
    {
        ///////发送32k数据
        uint16_t ramStartAddr = 0x8000;
        int leftSize = fileSize - i * DEFAULT_DWIN_OTA_BIG_BLOCK_SIZE;
        int sectionSize = DEFAULT_DWIN_OTA_BIG_BLOCK_SIZE; // 默认32k
        if (leftSize < DEFAULT_DWIN_OTA_BIG_BLOCK_SIZE)
        {
            sectionSize = leftSize;
        }
        int numSection = sectionSize / DEFAULT_DWIN_OTA_SMALL_BLOCK_SIZE + 1; // 240字节的个数
        int sectionLeft = sectionSize;
        for (int j = 0; j < numSection; j++)
        {
            uint8_t dataBuf[246] = {0};
            int dataLen = DEFAULT_DWIN_OTA_SMALL_BLOCK_SIZE;
            if (sectionLeft < DEFAULT_DWIN_OTA_SMALL_BLOCK_SIZE)
            {
                dataLen = sectionLeft;
            }
            dataBuf[0] = 0x5a;
            dataBuf[1] = 0xa5;
            dataBuf[2] = 0xf3;
            dataBuf[3] = 0x82;
            dataBuf[4] = (ramStartAddr & 0xff00) >> 8;
            dataBuf[5] = ramStartAddr & 0x00ff;
            ramStartAddr += DEFAULT_DWIN_OTA_SMALL_BLOCK_SIZE / 2;

            bool rv = getFileContent(filePath, i * DEFAULT_DWIN_OTA_BIG_BLOCK_SIZE + j * DEFAULT_DWIN_OTA_SMALL_BLOCK_SIZE, dataBuf + 6, dataLen);
            dataBuf[2] = dataLen + 3;
            cout << "-----index-----" << std::dec << j << "----len----" << std::dec << dataLen << endl;
            int sendLen = writeDataToPort((char *)dataBuf, dataLen + 6);
            cout << "[write RAM SendData size=" << sendLen << "]: ";
            for (int k = 0; k < sendLen; k++)
            {
                cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(dataBuf[k]) << " ";
            }
            cout << "" << endl;
            // usleep(500000);
            int timeout = DEFAULT_DWIN_TIMEOUT;
            auto start = std::chrono::steady_clock::now(); // 获取当前时间
            while (true)
            {
                auto end = std::chrono::steady_clock::now();                                                // 获取当前时间
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count(); // 计算时间间隔
                if (duration >= timeout)
                {
                    COUT << "----read UI data----timeout" << endl;
                    return false;
                }
                unsigned char rxData[DEFAULT_DWIN_DATA_BUFFER_SIZE] = {0};
                int rxLen = readDataFromPort((char *)rxData, sizeof(rxData));
                if (rxLen > 0)
                {
                    cout << "[RecvData size=" << rxLen << "]: ";
                    for (int k = 0; k < rxLen; k++)
                    {
                        // fflush(stdout);
                        cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(rxData[k]) << " ";
                    }
                    cout << "" << endl;
                    break;
                }
                usleep(10000); // 等待10毫秒
            }
            sectionLeft -= dataLen;
            // getchar();
            usleep(50000);
        }

        //////写flash
        uint8_t cmdBuf_23_flash[] = {0x5a, 0xa5, 0x0f, 0x82, 0x00, 0xaa, 0x5a, 0x02, 0x00, 0xb8, 0x80, 0x00, 0x7f, 0xe8, 0x00, 0x00, 0x00, 0x00}; // 5A A5 0F 82 00AA 5A02 0068 3000 0014 0000 0000
        cmdBuf_23_flash[8] = (blockStartAddr & 0xff00) >> 8;
        cmdBuf_23_flash[9] = blockStartAddr & 0x00ff;
        for (int k = 0; k < sizeof(cmdBuf_23_flash); k++)
        {
            cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(cmdBuf_23_flash[k]) << " ";
        }
        cout << "" << endl;
        int sendLen1 = writeDataToPort((char *)cmdBuf_23_flash, sizeof(cmdBuf_23_flash));
        cout << "---big block to flash--" << std::dec << sendLen1 << endl;
        // cout << "---big block to flash--" << endl;
        blockStartAddr += 1;
        int timeout = DEFAULT_DWIN_TIMEOUT;
        auto start1 = std::chrono::steady_clock::now(); // 获取当前时间
        while (true)
        {
            auto end = std::chrono::steady_clock::now();                                                 // 获取当前时间
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start1).count(); // 计算时间间隔
            if (duration >= timeout)
            {
                COUT << "----read UI data----timeout" << endl;
                return false;
            }
            unsigned char rxData[DEFAULT_DWIN_DATA_BUFFER_SIZE] = {0};
            int rxLen = readDataFromPort((char *)rxData, sizeof(rxData));
            if (rxLen > 0)
            {
                cout << "[RecvData size=" << rxLen << "]: ";
                for (int i = 0; i < rxLen; i++)
                {
                    // fflush(stdout);
                    cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(rxData[i]) << " ";
                }
                cout << "" << endl;
                break;
            }
            usleep(10000); // 等待10毫秒
        }
        cout << "--------------------BLOCK---------------------------" << std::dec << i << endl;
        usleep(100000);
        // getchar();

        uint8_t cmdBuf_getstatus[] = {0x5a, 0xa5, 0x04, 0x83, 0x00, 0xaa, 0x01}; // 5A A5 04 83 00 AA 01   读状态
        int sendLen5 = writeDataToPort((char *)cmdBuf_getstatus, sizeof(cmdBuf_getstatus));
        cout << "[write status SendData size=" << sendLen1 << "]: ";
        for (int j = 0; j < sendLen5; j++)
        {
            cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(cmdBuf_getstatus[j]) << " ";
        }
        cout << "" << endl;
        auto start2 = std::chrono::steady_clock::now(); // 获取当前时间
        while (true)
        {
            auto end = std::chrono::steady_clock::now();                                                 // 获取当前时间
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start2).count(); // 计算时间间隔
            if (duration >= timeout)
            {
                COUT << "----read UI data----timeout" << endl;
                return false;
            }
            unsigned char rxData[DEFAULT_DWIN_DATA_BUFFER_SIZE] = {0};
            int rxLen = readDataFromPort((char *)rxData, sizeof(rxData));
            if (rxLen > 0)
            {
                cout << "[get status RecvData size=" << rxLen << "]: ";
                for (int i = 0; i < rxLen; i++)
                {
                    // fflush(stdout);
                    cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(rxData[i]) << " ";
                }
                cout << "" << endl;
                break;
            }
            usleep(10000); // 等待10毫秒
        }
    }
    bSuccess = true;
    // getchar();
    usleep(100000);

    return bSuccess;
}

bool Dwin::doDwinOTA(string fileName)
{
    string pararentPath = string(DEFAULT_DWIN_OTA_SAVE_PATH) + "DWIN_SET/";
    // 遍历所有文件;
    std::vector<std::string> files;
    enumFiles(pararentPath, files);
    for (auto &file : files)
    {
        cout << file << endl;
        std::string filePath = pararentPath + file;
        downloadFile(filePath);
    }
    resetDwin();

    return true;
}