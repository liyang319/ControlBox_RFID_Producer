#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/can.h>
#include <linux/can/raw.h>

#include "Base.h"
#include "CanSock.h"
#include "CanConst.h"

using namespace std;

CanSock::CanSock(int index, long baudrate)
{
    // 创建CAN套接字
    canSock = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (canSock < 0)
    {
        CERR << "socket CAN create error： " << errno << std::endl;
    }

    canname = "can" + std::to_string(index);
    this->index = index;
    this->baudrate = baudrate;
    this->binded = false;
}

CanSock::~CanSock()
{
    if (canSock)
    {
        close(canSock);
    }
}

unsigned char CanSock::dlc2len(unsigned char can_dlc)
{
    return dlc2lenArr[can_dlc & 0x0F];
}

unsigned char CanSock::len2dlc(unsigned char len)
{
    if (len > 64)
    {
        return 0xF;
    }

    return len2dlcArr[len];
}

/**
 * 设置波特率
 */
int CanSock::config(long baudrate)
{
    // down first, set bitrate, and up again
    string cmd1 = "sudo ifconfig " + canname + " down";
    int result = system(cmd1.c_str());
    if (result != 0)
    {
        COUT << "CAN config system call error: " << result << endl;
    }
    usleep(10000);

    // bitrate
    string cmd2 = "sudo ip link set " + canname + " type can bitrate " + std::to_string(baudrate);
    result = system(cmd2.c_str());
    if (result != 0)
    {
        COUT << "CAN config system call error: " << result << endl;
    }
    usleep(10000);

    // up
    string cmd3 = "sudo ifconfig " + canname + " up";
    result = system(cmd3.c_str());
    if (result != 0)
    {
        COUT << "CAN config system call error: " << result << endl;
    }
    usleep(10000);

    return 0;
}

/**
 * 建立CAN socket，并绑定端口
 */
int CanSock::openCan()
{
    if (canSock < 0)
    {
        CERR << "Open failed!" << endl;
        return -1;
    }

    // 是否开启回环功能  
    int loopback = 0; // 0 = disabled, 1 = enabled (default)
    if (setsockopt(canSock, SOL_CAN_RAW, CAN_RAW_LOOPBACK, &loopback, sizeof(loopback)))
    {
        COUT << "Failed to close loopback!" << std::endl;
        close(canSock);
        return -1;
    }
    
    // 绑定 can 设备与 socket 
    struct ifreq ifr; 
    strcpy(ifr.ifr_name, canname.c_str());
    
    // 获取设备索引
    if (ioctl(canSock, SIOCGIFINDEX, &ifr) == -1) 
    {
        CERR << "Failed to get CAN interface index!" << std::endl;
        close(canSock);
        return -1;
    }
    
    COUT << "ifr.ifr_ifindex = " << ifr.ifr_ifindex << endl;

    struct sockaddr_can addr = {0};
    addr.can_family = AF_CAN;               // 指定协议族
    addr.can_ifindex = ifr.ifr_ifindex;     // 设备索引

    // 指定 can 设备，并获取设备索引
    if (ioctl(canSock, SIOCGIFMTU, &ifr) < 0) 
    {
        perror("SIOCGIFMTU");
        return -1;
    }

    // 将套接字与 can 绑定
    if (bind(canSock, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        CERR << "bind error!" << endl;
        close(canSock);
        return -1;
    }

    // 设置过滤规则？
    
    this->binded = true;
    COUT << "open can succeed!" << endl;
    return 0;
}

/**
 * CAN 2.0, size为8，CAN FD, size为64
 */
ssize_t CanSock::sendDataFrame(char* data, int length, uint32_t id, bool isExtend) 
{
    lock_guard<mutex> lock(canMutex);

    struct can_frame frame;

    if (!this->binded)
    {
        COUT << "Please open socket first!" << endl;
        return 0;
    }

    // 数据
    for (int i=0; i<length; i++)
    {
        frame.data[i] = data[i];
    }

    // 设置 DLC
    frame.can_dlc = len2dlc(length);

    // 设置 ID 号
    if (isExtend)
    {
        frame.can_id = CAN_EFF_FLAG | (CAN_EFF_MASK & id); 
    }
    else
    {
        frame.can_id = CAN_SFF_MASK & id; 
    }
                    
    ssize_t wx = write(canSock, &frame, sizeof(frame));

    if (wx == -1)
    {
        CERR << "Failed to send CAN date frame!" << std::endl;
        return -1;
    }

    return wx;
}

ssize_t CanSock::sendRemoteFrame(uint32_t id, bool isExtend)
{
    lock_guard<mutex> lock(canMutex);

    struct can_frame frame;

    if (!this->binded)
    {
        COUT << "Please open socket first!" << endl;
        return 0;
    }

    // 设置 ID 号
    if (isExtend)
    {
        frame.can_id = CAN_EFF_FLAG | (CAN_RTR_FLAG | (CAN_EFF_MASK & id)); 
    }
    else
    {
        frame.can_id = CAN_RTR_FLAG | (CAN_SFF_MASK & id); 
    }

    ssize_t wx = write(canSock, &frame, sizeof(frame));

    if (wx == -1)
    {
        CERR << "Failed to send CAN remote frame!" << std::endl;
        return -1;
    }

    return wx;
}

ssize_t CanSock::recvData(char *data, uint32_t* pid)
{
    lock_guard<mutex> lock(canMutex);

    if (data == NULL)
    {
        CERR << "The revc buffer is invalid!" << endl;
        return 0;
    }

    if (!this->binded)
    {
        COUT << "Please open socket first!" << endl;
        return 0;
    }

    // select
    fd_set readSet;
    FD_ZERO(&readSet);
    FD_SET(canSock, &readSet);

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 100000;

    // 接收 CAN 消息
    struct can_frame frame;

    int result = select(canSock + 1, &readSet, NULL, NULL, &timeout);
    if (result == -1) 
    {
        COUT << "Error occurred while waiting for CAN message" << std::endl;
        return 0;
    } 
    else if (result == 0) 
    {
        COUT << "Timeout occurred while waiting for CAN message" << std::endl;
        return 0;
    }
    else
    {
        memset(&frame, 0, sizeof(can_frame));

        // 接收数据
        ssize_t rx = read(canSock, &frame, sizeof(frame));

        // 错误判断
        if ((frame.can_id & CAN_ERR_FLAG) != 0)
        {
            COUT << "receive error frame!" << endl;
            return 0;
        }

        if (rx == -1)
        {
            return 0;
        }

        COUT << "rx = " << to_string(rx) << " frame.dlc: " << to_string(frame.can_dlc) << endl;
        for (int i = 0; i < frame.can_dlc; ++i) 
        {
            // cout << hex << setw(2) << setfill('0') << static_cast<int>(frame.data[i]) << " ";
            data[i] = frame.data[i];
        }
        // cout << endl;

        *pid = frame.can_id;
    }

    return frame.can_dlc;
}

int CanSock::closeCan()
{
    close(canSock);
    canSock = -1;
    binded = false;
    
    return 0;
}

bool CanSock::isOpened()
{
    return this->binded;
}