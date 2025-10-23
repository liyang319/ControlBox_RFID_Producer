#ifndef __CANMANAGER_H__
#define __CANMANAGER_H__

#include <string>
#include <queue>
#include <array>
#include <mutex>
#include <thread>
#include <map>
#include <condition_variable>
#include <vector>
#include "CanSock.h"
#include "CanComThread.h"

using namespace std;

#define DEFAULT_MAX_QUEUE_SIZE 1024
#define DEFAULT_MAX_CAN_COUNT 12

using namespace std;

enum CmdType
{
    READ,
    WRITE,
    OTHER
};

struct CanCmd
{
    uint32_t messageId;
    uint32_t interval;
    int canType;

    CanCmd() : messageId(0), interval(100), canType(0) {}
    CanCmd(uint32_t id, uint32_t interval, int canType) : messageId(id), interval(interval), canType(canType) {}
};

// 对于简单CAN协议（自己组装协议数据，信号为整字节），用如下数据结构。如果是复杂CAN协议（信号跨字节的情况）数据，采用CanFrame
struct GeneralCanData
{
    uint32_t messageId;

    bool isExtend;   // 是否扩展帧
    uint8_t length;  // 数据长度
    uint8_t data[8]; // 数据
};

class CanManager
{
    // CAN Mutex
    mutex canMutex; // CAN 发送队列mutex

    //
    static bool bCANExit;

    // CAN
    CanSock *canSocks[DEFAULT_MAX_CAN_COUNT];

    // interval
    unsigned short recvInterval; // 单位：ms
    bool receiving;              // 接收线程运行中

private:
    CanManager(/* args */);
    ~CanManager();

    CanComThread canComThread; // CAN发送线程

    // 单例模式
    CanManager(const CanManager &) = delete;
    CanManager &operator=(const CanManager &) = delete;

    // 发送函数
    static void *canThreadFunction(CanCmd canCmd);
    static void *canReceiveFunction(CanManager *manager);

    // tools
    uint16_t crc16(uint8_t *data, int length);
    static std::string toHexString(int num);
    static int hexStringToNumber(const std::string &hexString);
    bool parseCanRecvData(CanCmd &canCmd, char *data, size_t length);
    bool parseCanRecvGeneralData(CanCmd &canCmd, char *data, size_t length);

    // can thread: 对于自主发送can数据的，可以采用线程模式；
    int CANSendStart(uint32_t id, int cmdInterval);
    int CANSendStop(uint32_t id);

public:
    static CanManager &getInstance()
    {
        static CanManager instance;
        return instance;
    }

    // 测试用
    void testCan();

    // 提供给外部接口调用，发送一次CAN数据
    bool sendGeneralData(GeneralCanData canData);

    // 接收CAN数据()
    bool startGeneralDataReceive(unsigned short interval);
    bool stopGeneralDataReceive();
    void startLoop();
    void stopLoop();

    void sendZeroCalibResult(uint8_t result, uint8_t sensorid);
    void sendWeight(double totalWeigh);
};

#endif