#ifndef __CANFDSOCK_H__
#define __CANFDSOCK_H__

#include "CanSock.h"

class CanFDSock : CanSock
{
private:
    long dbaudrate;

public:
    CanFDSock(int index, long baudrate, long dbaudrate);
    ~CanFDSock();

    // 重载CAN方法
    int config(long baudrate, long dbaudrate);
    int openCan();
    ssize_t sendDataFrame(char* data, int length, uint32_t id, bool isExtend);
    ssize_t recvData(char *data, uint32_t* pid);
};

#endif