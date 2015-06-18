#ifndef __NET_H__
#define __NET_H__

#ifdef WIN32
#include <WinSock2.h>
#else
// in linux android
#define SOCKET int
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h> 
#include <netinet/in.h>
#include <pthread.h>
#include <arpa/inet.h>
#endif

class Net
{
public: 
	static SOCKET _server;
	static SOCKET _connet;

	static bool _isRecvComplete;
	static bool _isConnected;

	static char * _recvData;

	static bool Listen(short port = 9999);
	static bool isConnected();
	static bool Connect(const char * ip, short port = 9999);
	static int Send(const char * buffer, int len);
	// 接收数据的接口
	static bool RecvStart();
	static bool isRecvComplete();
	static char * RecvData(int& len);
#ifdef WIN32
	static DWORD WINAPI AcceptThreadFunc(void *arg);
	static DWORD WINAPI RecvThreadFunc(void *arg);
#else
    static void * AcceptThreadFunc(void *arg);
    static void * RecvThreadFunc(void *arg);
#endif
};
#endif // !__NET_H__



