#pragma once

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "esp_log.h"
#include "esp_base.h"
#include "endpoint.h"


class Socket : public IStream
{
	int handle = -1;

public:
	~Socket();
	///0	Further receives are disallowed
	///1	Further sends are disallowed
	///2	Further sends and receives are disallowed(like close())
	///		https://beej.us/guide/bgnet/html/#close-and-shutdownget-outta-my-face
	void Shutdown(int how);
	void Close();
	bool Init(int domain, int type, int protocol);	//	AF_INET, SOCK_STREAM, IPPROTO_IP
	bool InitTCP() { return Init(AF_INET, SOCK_STREAM, IPPROTO_IP); };
	bool InitUDP() { return Init(AF_INET, SOCK_DGRAM, IPPROTO_IP); };
	void SetKeepAlive(int enable, int idle, int interval, int count);
	void SetTimeout(TimeSpan timespan);
	int Receive(void* buffer, size_t size, int flags = 0);
	int Send(const void* buffer, size_t size, int flags = 0);
	int SendTo(Endpoint* endpoint, const void* buffer, size_t size, int flags = 0);
	bool Connect(Endpoint* endpoint);
	bool Bind(Endpoint* endpoint);
	bool Accept(Socket* client);
	bool Listen(int backlog);
	size_t Read(void* data, size_t size) override;
	size_t Write(const void* data, size_t size) override;
};
	
