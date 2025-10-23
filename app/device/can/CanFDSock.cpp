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
#include "CanFDSock.h"
#include "CanConst.h"

using namespace std;

CanFDSock::CanFDSock(int index, long baudrate, long dbaudrate) : CanSock(index, baudrate)
{
    this->dbaudrate = dbaudrate;
}

CanFDSock::~CanFDSock()
{
}

int CanFDSock::config(long baudrate, long dbaudrate)
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
    string cmd2 = "sudo ip link set " + canname + " type can bitrate " \
        + std::to_string(baudrate) + " dbitrate " + std::to_string(dbaudrate) + " fd on";
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

int CanFDSock::openCan()
{
    if (canSock < 0)
    {
        CERR << "Open canfd failed!" << endl;
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
        COUT << "Failed to get CANFD interface index!" << std::endl;
        close(canSock);
        return -1;
    } 
    
    // ifindex
    COUT << "ifr.ifr_ifindex = " << ifr.ifr_ifindex << endl;

    struct sockaddr_can addr = {0};
    addr.can_family = AF_CAN;               // 指定协议族
    addr.can_ifindex = ifr.ifr_ifindex;     // 设备索引

    // 指定 can 设备，并获取设备索引
    if (ioctl(canSock, SIOCGIFMTU, &ifr) < 0) 
    {
        COUT << "CANFD SIOCGIFMTU error!" << endl;
        close(canSock);
        return -1;
    }

    if (ifr.ifr_mtu != CANFD_MTU) 
    {
        COUT << "CAN interface is not CAN FD capable" << endl;
        close(canSock);
        return -1;
    }

    int enable_canfd = 1;
    // 切换到 CAN FD 模式 
    if (setsockopt(canSock, SOL_CAN_RAW, CAN_RAW_FD_FRAMES, &enable_canfd, sizeof(enable_canfd)))
    {  
        COUT << "Error when enabling CAN FD support!" << endl;  
        close(canSock);
        return -1; 
    } 

    // 将套接字与 can 绑定
    if (bind(canSock, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        COUT << "bind CAN FD error!" << endl;
        close(canSock);
        return -1;
    }

    // 设置过滤规则？
    /*
    struct can_filter rfilter[3];  // 定义过滤规则数组
    rfilter[0].can_id = 0x100;
    rfilter[0].can_mask = CAN_SFF_MASK; // 标准帧 (SFF: standard frame format)
        
    rfilter[1].can_id = 0x200;
    rfilter[1].can_mask = CAN_SFF_MASK; // 标准帧
    
    setsockopt(canSock, SOL_CAN_RAW, CAN_RAW_FILTER, &rfilter, sizeof(rfilter));
    */

    this->binded = true;
    COUT << "open canfd succeed!" << endl;
    return 0;
}

ssize_t CanFDSock::sendDataFrame(char* data, int length, uint32_t id, bool isExtend)
{
    struct canfd_frame frame;

    if (!this->binded)
    {
        COUT << "Please open canfd socket first!" << endl;
        return 0;
    }

    // 数据
    for (int i=0; i<length; i++)
    {
        frame.data[i] = data[i];
    }

    // 设置 DLC
    frame.len = len2dlc(length);

    // 设置 ID 号
    if (isExtend)
    {
        frame.can_id = CAN_EFF_FLAG | (CAN_EFF_MASK & id); 
    }
    else
    {
        frame.can_id = (CAN_SFF_MASK & id); 
    }
                    
    ssize_t wx = write(canSock, &frame, sizeof(frame));

    if (wx == -1)
    {
        CERR << "Failed to send CANFD date frame!" << std::endl;
        return -1;
    }

    return wx;
}

ssize_t CanFDSock::recvData(char *data, uint32_t* pid)
{
    if (data == NULL)
    {
        CERR << "The revc buffer is invalid!" << endl;
        return 0;
    }

    if (!this->binded)
    {
        COUT << "Please open canfd socket first!" << endl;
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
    struct canfd_frame frame;

    int result = select(canSock + 1, &readSet, NULL, NULL, &timeout);
    if (result == -1) 
    {
        COUT << "Error occurred while waiting for CANFD message" << std::endl;
        return 0;
    } 
    else if (result == 0) 
    {
        COUT << "Timeout occurred while waiting for CANFD message" << std::endl;
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
            COUT << "receive error canfd frame!" << endl;
            return 0;
        }

        if (rx == -1)
        {
            return 0;
        }

        COUT << "rx = " << to_string(rx) << " frame.len: " << to_string(frame.len) << endl;
        for (int i = 0; i < frame.len; ++i) 
        {
            // cout << hex << setw(2) << setfill('0') << static_cast<int>(frame.data[i]) << " ";
            data[i] = frame.data[i];
        }
        // cout << endl;

        *pid = frame.can_id;
        return frame.len;
    }
}