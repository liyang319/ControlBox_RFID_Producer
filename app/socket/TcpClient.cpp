#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "TcpClient.h"
#include "Base.h"

using namespace std;


TcpClient::TcpClient(/* args */)
{
    COUT << "TcpClient init!" << endl;
}

TcpClient::~TcpClient()
{
    COUT << "TcpClient destroy!" << endl;
}

int TcpClient::makeSocketNonBlocking(int sock) 
{
    int curFlags = fcntl(sock, F_GETFL, 0);
	return fcntl(sock, F_SETFL, curFlags|O_NONBLOCK) >= 0;
}

int TcpClient::makeSocketBlocking(int sock) 
{
	int curFlags = fcntl(sock, F_GETFL, 0);
	return fcntl(sock, F_SETFL, curFlags&(~O_NONBLOCK)) >= 0;
}

/**
 * 连接VLPR，车牌识别设备
 */
int TcpClient::connetServer(string ip, int port, int block)
{
	if (connected)
	{
		COUT << "TCP connected already!" << endl;
		return clientSocket;
	}

	struct sockaddr_in addr;
	bzero(&addr, sizeof(addr));

	unsigned long ip_addr = inet_addr(ip.data());
	if (INADDR_NONE == ip_addr)
	{
		COUT << "input right ip, eg:192.168.1.22" << endl;
		return INVALID_SOCKET;
	}

	clientSocket = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if (INVALID_SOCKET == clientSocket)
	{
		COUT << "create socket error" << endl;
		return INVALID_SOCKET;
	}

	if (block == TCP_UNBLOCK)
	{
		makeSocketNonBlocking(clientSocket);
	}
	else
	{
		makeSocketBlocking(clientSocket);
	}

	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = ip_addr;

	if (connect(clientSocket,(struct sockaddr *)&addr,sizeof(addr))!=0)
	{
		COUT << "socket connect error" << endl;
		disconnect();
		return INVALID_SOCKET;
	}

    connected = true;
	return clientSocket;
}

void TcpClient::disconnect()
{
    close(clientSocket);
    connected = false;
}