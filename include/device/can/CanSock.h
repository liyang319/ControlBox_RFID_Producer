#ifndef __CANSOCK_H__
#define __CANSOCK_H__

#include <thread>
#include <queue>
#include <mutex>

class CanSock
{
protected:
    int canSock;            // Socket
    int index;              // 指定 CAN 端口：can0、can1、can2、can3等
    long baudrate;          // 波特率
    bool binded;            // 是否已经打开端口
    std::string canname;    // can0/can1...
    std::mutex canMutex;    // CAN Mutex

public:
    CanSock(int index, long baudrat);
    ~CanSock();

    int config(long baudrate);
    int openCan();
    ssize_t sendDataFrame(char* data, int length, uint32_t id, bool isExtend);
    ssize_t sendRemoteFrame(uint32_t id, bool isExtend);
    ssize_t recvData(char *data, uint32_t* pid);
    int closeCan();
    bool isOpened();

protected:
    unsigned char dlc2len(unsigned char can_dlc);
    unsigned char len2dlc(unsigned char len);
};

#endif