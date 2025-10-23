/**
* @Class TcpClient
* @Brief TCP socket通信基类，管理TCP连接的连接及断开 */

#pragma once

#include <string>

#define SOCKET_ERROR 		-1
#define INVALID_SOCKET  	-1

using namespace std;

enum
{
	TCP_BLOCK = 0,
	TCP_UNBLOCK
};

class TcpClient
{
protected:
    string ip;
    int port;
    int clientSocket;
    bool connected = false;
    
public:
    TcpClient(/* args */);
    ~TcpClient();

protected:
    int makeSocketNonBlocking(int sock);
    int makeSocketBlocking(int sock);
    int connetServer(string ip, int port, int block);
    void disconnect();
};
