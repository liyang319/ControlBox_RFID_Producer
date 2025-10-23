#include <string>
#include <iostream>
#include <termios.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

#include "SerialPort.h"
#include "PortInfo.h"
#include "Base.h"
#include "Device.h"

using namespace std;

SerialPort::SerialPort(int portNumber)
{
    COUT << "SerialPort <" << portNumber << "> init!" << endl;
    this->portNumber = portNumber;
    this->opened = false;
    setPortParameter(DEFAULT_BAUDRATE, DEFAULT_DATABIT, DEFAULT_STOPBIT, DEFAULT_PARITY);
}

SerialPort::SerialPort(int portNumber, int baudrate, int databit, int stopbit, int parity)
{
    COUT << "SerialPort <" << portNumber << "> init!" << endl;
    this->portNumber = portNumber;
    this->opened = false;
    setPortParameter(baudrate, databit, stopbit, parity);
}

SerialPort::~SerialPort()
{
    COUT << "SerialPort destroy!" << endl;
    portClose();
}

/*******************************************
 *  波特率转换函数（请确认是否正确）
 ********************************************/
int SerialPort::convbaud(unsigned long int baudrate)
{
    switch (baudrate)
    {
    case 2400:
        return B2400;
    case 4800:
        return B4800;
    case 9600:
        return B9600;
    case 19200:
        return B19200;
    case 38400:
        return B38400;
    case 57600:
        return B57600;
    case 115200:
        return B115200;
    default:
        return B9600;
    }
}

/*******************************************
 *  GT6658串口定义
 ********************************************/
string SerialPort::convcom(unsigned short port)
{
#ifdef DEVICE_RK2611
    switch (port)
    {
    case SENSOR_PORT:
        return "/dev/ttyUSB0"; // GC31
    case GPS_PORT:
        return "/dev/ttyUSB3"; // GPS
    case SCREEN_PORT:
        return "/dev/ttyUSB0"; // DWIN
    default:
        return "";
    }
#else
    switch (port)
    {
    case SENSOR_PORT:
        return "/dev/ttyS0"; // GC31
    case GPS_PORT:
        return "/dev/ttysWK1"; // GPS
    case SCREEN_PORT:
        return "/dev/ttyS1"; // DWIN
    default:
        return "";
    }
#endif
}

/*******************************************
 *  设置参数
 ********************************************/
void SerialPort::setPortParameter(int baudrate, int databit, int stopbit, int parity)
{
    this->baudrate = baudrate;
    this->databit = databit;
    this->stopbit = stopbit;
    this->parity = parity;
}

/*******************************************
 *  Setup comm attr
 *  fdcom: 串口文件描述符，pportinfo: 待设置的端口信息（请确认）
 *
 ********************************************/
int SerialPort::portSet(const pportinfo_t pportinfo)
{
    struct termios termios_old, termios_new;
    int baudrate, tmp;
    char databit, stopbit, parity, fctl;

    bzero(&termios_old, sizeof(termios_old));
    bzero(&termios_new, sizeof(termios_new));
    cfmakeraw(&termios_new);
    tcgetattr(fdCom, &termios_old); // get the serial port attributions

    /*------------设置端口属性-----------------*/
    // baudrates
    baudrate = convbaud(pportinfo->baudrate);

    cfsetispeed(&termios_new, baudrate); // 填入串口输入端的波特率
    cfsetospeed(&termios_new, baudrate); // 填入串口输出端的波特率

    termios_new.c_cflag |= CLOCAL; // 控制模式，保证程序不会成为端口的占有者
    termios_new.c_cflag |= CREAD;  // 控制模式，使能端口读取输入的数据

    // 控制模式，flow control
    fctl = pportinfo->fctl;
    switch (fctl)
    {
    case '0':
        termios_new.c_cflag &= ~CRTSCTS; // no flow control
        break;
    case '1':
        termios_new.c_cflag |= CRTSCTS; // hardware flow control
        break;
    case '2':
        termios_new.c_iflag |= IXON | IXOFF | IXANY; // software flow control
        break;
    default:
        break;
    }

    // 数据位，data bits
    termios_new.c_cflag &= ~CSIZE; //
    databit = pportinfo->databit;

    switch (databit)
    {
    case '5':
        termios_new.c_cflag |= CS5;
        break;
    case '6':
        termios_new.c_cflag |= CS6;
        break;
    case '7':
        termios_new.c_cflag |= CS7;
        break;
    default:
        termios_new.c_cflag |= CS8;
        break;
    }

    // 奇偶校验 parity check
    parity = pportinfo->parity;
    switch (parity)
    {
    case '0':
        termios_new.c_cflag &= ~PARENB; // no parity check
        termios_new.c_iflag &= ~INPCK;
        break;
    case '1':
        termios_new.c_cflag |= (PARODD | PARENB); // odd check
        termios_new.c_iflag |= INPCK;
        break;
    case '2':
        termios_new.c_cflag |= PARENB; // even check
        termios_new.c_cflag &= ~PARODD;
        termios_new.c_iflag |= INPCK;
        break;
    case '3':
        termios_new.c_cflag |= (PARODD | PARENB); // mark check
        termios_new.c_cflag |= CMSPAR;
        break;
    case '4':
        termios_new.c_cflag |= PARENB; // space check
        termios_new.c_cflag &= ~PARODD;
        termios_new.c_cflag |= CMSPAR;
        break;
    }

    // 停止位，stop bits
    stopbit = pportinfo->stopbit;
    if (stopbit == '2')
    {
        termios_new.c_cflag |= CSTOPB; // 2 stop bits
    }
    else
    {
        termios_new.c_cflag &= ~CSTOPB; // 1 stop bits
    }

    // other attributions default
    termios_new.c_oflag &= ~OPOST; // 输出模式，原始数据输出
    termios_new.c_cc[VMIN] = 1;    // 控制字符, 所要读取字符的最小数量
    termios_new.c_cc[VTIME] = 1;   // 控制字符, 读取第一个字符的等待时间    unit: (1/10)second

    tcflush(fdCom, TCIFLUSH);                      // 溢出的数据可以接收，但不读
    tmp = tcsetattr(fdCom, TCSANOW, &termios_new); // 设置新属性，TCSANOW：所有改变立即生效    tcgetattr(fdcom, &termios_old);
    return (tmp);
}

/********************************************
 *  send data
 *  fdcom: 串口描述符，data: 待发送数据，datalen: 数据长度
 *  返回实际发送长度
 *********************************************/
int SerialPort::portSend(char *data, int datalen)
{
    if (!opened)
    {
        COUT << "Please open serial port first!" << endl;
        return -1;
    }

    int len = 0;

    len = write(fdCom, data, datalen); // 实际写入的长度
    if (len == datalen)
    {
        // COUT << "write serial port data OK" << endl;
        return (len);
    }
    else
    {
        COUT << "write serial port data fail" << endl;
        tcflush(fdCom, TCOFLUSH);
        return -1;
    }
}

/*******************************************
 *  receive data
 *  返回实际读入的字节数
 *
 ********************************************/
int SerialPort::portRecv(char *data, int datalen)
{
    if (!opened)
    {
        COUT << "Please open serial port first!" << endl;
        return -1;
    }

    int readlen, fs_sel;
    fd_set fs_read;
    struct timeval tv_timeout;

    FD_ZERO(&fs_read);
    FD_SET(fdCom, &fs_read);
    tv_timeout.tv_sec = 0;
    tv_timeout.tv_usec = 200000;
    int totalLen = 0;

    while (true)
    {
        fs_sel = select(fdCom + 1, &fs_read, NULL, NULL, &tv_timeout);
        if (fs_sel > 0)
        {
            readlen = read(fdCom, data + totalLen, datalen - totalLen);
            totalLen += readlen;
            if (totalLen >= datalen)
            {
                break;
            }
        }
        else
        {
            if (fs_sel == -1)
            {
                perror("select() failed");
            }
            break;
        }
    }
    return (totalLen);
}

int SerialPort::portOpen()
{
    if (opened)
    {
        COUT << "Serial port opened already!" << endl;
        return 0;
    }

    portinfo_t portinfo =
        {
            '0',           // print prompt after receiving
            baudrate,      // baudrate: 9600
            (char)databit, // databit: 8
            '0',           // debug: off
            '0',           // echo: off
            '0',           // flow control: none
            (char)parity,  // parity: none
            (char)stopbit, // stopbit: 1
            0              // reserved
        };

    string comDev = convcom(portNumber);
    fdCom = open(comDev.data(), O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fdCom < 0)
    {
        COUT << "Open <com " << portNumber << "> failed!" << endl;
        return -1;
    }
    else
    {
        COUT << "Open <com " << portNumber << "> succeed!" << endl;
    }

    portSet(&portinfo);

    //
    this->opened = true;
    this->portNumber = portNumber;

    return 0;
}

int SerialPort::portClose()
{
    if (opened)
    {
        close(fdCom);
    }

    this->opened = false;
    COUT << "Close <com " << portNumber << "> succeed!" << endl;

    return 0;
}