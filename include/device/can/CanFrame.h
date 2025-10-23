#ifndef __CANFRAME_H__
#define __CANFRAME_H__

#include <iostream>
#include <cstdint>

#define CAN_TYPE_CAN_20             0   // CAN2.0
#define CAN_TYPE_CAN_FD             1   // CANFD

#define SIGNAL_BYTE_ORDER_INTEL     0
#define SIGNAL_BYTE_ORDER_MOTOROLA  1

#define FRAME_ZERO_DLC              0
#define FRAME_MAX_DLC               8
#define FRAME_MAX_FD_DLC            64
#define SIGNAL_ZERO_LENGTH          0
#define SIGNAL_MAX_LENGTH           64
#define SIGNAL_MAX_FD_LENGTH        512
#define SIGNAL_INIT_VALUE           0
#define SIGNAL_INIT_MASK            0

#define CAN_SIGNAL_NORMAL           0
#define CAN_SIGNAL_NO_VALUE         1
#define CAN_FRAME_DLC_OUT_OF_RANGE  2
#define INVALID_INPUT_PARAMETERS    3

using namespace std;

// CAN信号结构
struct CanSignal 
{
    uint32_t id;        // 信号ID
    uint8_t state;      // 信号状态
    uint64_t value;     // 信号值
    uint8_t startBit;   // 信号起始位
    uint8_t bitSize;    // 信号长度
    uint8_t byteOrder;  // 信号字节序
    float factor;       // 缩放因子
    float offset;       // 偏移量
};

class CanFrame
{
    int canType;        // 0: can2.0; 1: canfd

public:
    uint32_t id;        // 帧ID
    bool isRemote;      // 是否是远程帧
    bool isExtend;      // 是否扩展帧
    uint8_t length;     // 数据长度

private:
    uint8_t data[8];    // 数据 
    uint8_t datafd[64]; // 数据 
    uint8_t realLength; // 实际数据字节数

public:
    CanFrame();
    CanFrame(int canType);
    ~CanFrame();

    bool insertSignal(const CanSignal &signal);
    CanSignal* getSignal(uint8_t startbit, uint8_t bitsize, uint8_t byteorder);
    uint8_t getRealFrameLength();
    uint8_t* getData();
    void setData(uint8_t* buffer, size_t size);
    int getType();
};

#endif