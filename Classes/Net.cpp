#include "Net.h"
#ifdef WIN32
#else
#define INVALID_SOCKET -1
#define closesocket close
#endif 
SOCKET Net::_server = INVALID_SOCKET;
SOCKET Net::_connet = INVALID_SOCKET;
bool Net::_isConnected = false;
bool Net::_isRecvComplete = false;
char * Net::_recvData = NULL;


bool Net::Listen(short port)
{
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET)
		return false;
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
#ifdef WIN32
	addr.sin_addr.S_un.S_addr = INADDR_ANY;
#else
	addr.sin_addr.s_addr = INADDR_ANY;
#endif
	int ret = bind(sock, (struct sockaddr *)&addr, sizeof(addr));
	if (ret != 0)
	{
		closesocket(sock);
		return false;
	}
	listen(sock, 10);

	// 阻塞
	//SOCKET conn = accept(sock, NULL, NULL);
	_server = sock;
	_isConnected = false;
#ifdef WIN32
	HANDLE hThread = CreateThread(NULL, 0, AcceptThreadFunc, NULL, 0, NULL);
	CloseHandle(hThread);
#else
	pthread_t tid;
	pthread_create(&tid, NULL, AcceptThreadFunc, NULL);
#endif
	return true;
}
#ifdef WIN32
DWORD Net::AcceptThreadFunc(void *arg)
#else
void * Net::AcceptThreadFunc(void *)
#endif
{
	_connet = accept(_server, NULL, NULL);
	_isConnected = true;
#ifdef WIN32
	return 0;
#else
	return NULL;
#endif 
}
bool Net::Connect(const char * ip, short port)
{
	_connet = socket(AF_INET, SOCK_STREAM, 0);
	if (_connet == INVALID_SOCKET)
		return false;
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
#ifdef WIN32
	addr.sin_addr.S_un.S_addr = inet_addr(ip);
#else
	addr.sin_addr.s_addr = inet_addr(ip);
#endif
	int ret = connect(_connet, (struct sockaddr *) &addr, sizeof(addr));
	if (ret != 0)
	{
		closesocket(_connet);
		return false;
	}
	return true;
}
bool Net::RecvStart()
{
	_isRecvComplete = false;
#ifdef WIN32
	HANDLE hThread = CreateThread(NULL, 0, RecvThreadFunc, NULL, 0, NULL);
	CloseHandle(hThread);
#else
	pthread_t tid;
	pthread_create(&tid, NULL, RecvThreadFunc, NULL);
#endif
	return true;
}
#ifdef WIN32
DWORD Net::RecvThreadFunc(void *arg)
#else
void * Net::RecvThreadFunc(void *arg)
#endif
{
	static char buf[16] = { 0 };
	recv(_connet, buf, 1, 0);
	// 选中
	if (buf[0] == 1)
	{
		recv(_connet, &buf[1], 1, 0);
	}
	// 走棋
	else if (buf[0] == 2)
	{
		for (int i = 1; i <= 3; i++)
			recv(_connet, &buf[i], 1, 0);
	}
	// 接收结束
	_recvData = buf;
	_isRecvComplete = true;
#ifdef WIN32
	return 0;
#else
	return NULL;
#endif
}
int  Net::Send(const char * buffer, int len)
{
	return send(_connet, buffer, len, 0);
}


bool Net::isRecvComplete()
{
	return _isRecvComplete;
}
char * Net::RecvData(int& len)
{
	len = 0; // len 目前没用
	return _recvData;
}
bool Net::isConnected()
{
	_isRecvComplete = false; // 报文被取走
	return _isConnected;
}
