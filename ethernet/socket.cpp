#include "socket.h"

#define TAG "Socket"

Socket::~Socket()
{
	if (handle < 0) return;
	Close();
}


void Socket::Shutdown(int how)
{
	if (handle < 0) return;
	shutdown(handle, how);
}


void Socket::Close()
{
	if (handle < 0) return;
	close(handle);
	handle = -1;
}




void Socket::SetKeepAlive(int enable, int idle, int interval, int count)
{
	if (handle < 0) return;
	//https://stackoverflow.com/questions/74053375/should-the-values-set-with-setsockopt-stay-valid-during-lifetime-of-the-socket
	// Set tcp keepalive option
	setsockopt(handle, SOL_SOCKET, SO_KEEPALIVE, &enable, sizeof(int));
	setsockopt(handle, IPPROTO_TCP, TCP_KEEPIDLE, &idle, sizeof(int));
	setsockopt(handle, IPPROTO_TCP, TCP_KEEPINTVL, &interval, sizeof(int));
	setsockopt(handle, IPPROTO_TCP, TCP_KEEPCNT, &count, sizeof(int));
}

int Socket::Receive(void* buffer, size_t size, int flags /* = 0 */)
{
	if (handle < 0) return 0;
	int result =  recv(handle, buffer, size, flags);
	if (result <= 0)
	{
		//ESP_LOGE(TAG, "Error occurred during receiving: errno %d, %s", errno, strerror(errno));
		Shutdown(0);
		Close();
	}
	return result;
}

int Socket::Send(const void* buffer, size_t size, int flags /* = 0 */)
{
	if (handle < 0) return 0;
	size_t toWrite = size;
	while (toWrite > 0) 
	{
		int written = send(handle, buffer, size, flags);
		if (written < 0) 
		{
			ESP_LOGE(TAG, "Error occurred during sending: %d, %s", errno, strerror(errno));
			Shutdown(0);
			Close();
			return written;
		}
		toWrite -= written;
	}
	return size;
}

int Socket::SendTo(Endpoint* endpoint, const void* buffer, size_t size, int flags /* = 0 */)
{
	if (handle < 0) return 0;
	int send = sendto(handle, buffer, size, flags, endpoint->GetSockAddr(), endpoint->Size());
	if (send < 0) {
		ESP_LOGE(TAG, "Error occurred during sending: errno %d, %s", errno, strerror(errno));
		return send;
	}
	return send;
}



bool Socket::Connect(Endpoint* endpoint)
{
	int err = connect(handle, endpoint->GetSockAddr(), endpoint->Size());
	if (err != 0) {
		ESP_LOGE(TAG, "Socket unable to connect: errno %d, %s", errno, strerror(errno));
		return false;
	}
	return true;
}


bool Socket::Init(int domain, int type, int protocol)
{
	handle = socket(domain, type, protocol);
	if (handle < 0) 
	{
		ESP_LOGE(TAG, "Unable to create socket: errno %d, %s", errno, strerror(errno));
		return false;
	}
	//int opt = 1;
	//setsockopt(handle, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	return true;
}


bool Socket::Bind(Endpoint* endpoint)
{
	int err = bind(handle, endpoint->GetSockAddr(), endpoint->Size());
	if (err != 0) {
		ESP_LOGE(TAG, "Socket unable to bind: errno %d, %s", errno, strerror(errno));
		Close();
		return false;
	}
	return true;
}




bool Socket::Accept(Socket* client)
{
	if (handle < 0) return false;
	struct sockaddr_storage source_addr; // Large enough for both IPv4 or IPv6
	socklen_t addr_len = sizeof(source_addr);
	int sock = accept(handle, (struct sockaddr *)&source_addr, &addr_len);
	if (sock < 0) {
		ESP_LOGE(TAG, "Unable to accept connection: errno %d, %s", errno, strerror(errno));
		return false;
	}
	client->handle = sock;
	//
	//if (source_addr.ss_family == PF_INET) {
	//	inet_ntoa_r(((struct sockaddr_in *)&source_addr)->sin_addr, addr_str, sizeof(addr_str) - 1);
	//}
	//else if (source_addr.ss_family == PF_INET6) {
	//	inet6_ntoa_r(((struct sockaddr_in6 *)&source_addr)->sin6_addr, addr_str, sizeof(addr_str) - 1);
	//}
	
	
	
	
	return true;
}

bool Socket::Listen(int backlog)
{
	int err = listen(handle, backlog);
	if (err != 0) {
		ESP_LOGE(TAG, "Error occurred during listen: errno %d, %s", errno, strerror(errno));
		Close();
		return false;
	}	
	return true;
}




void Socket::SetTimeout(TimeSpan timespan)
{
	struct timeval timeout;
	timeout.tv_sec = timespan.GetSeconds();
	timeout.tv_usec = (timespan.GetMiliSeconds() % 1000) * 1000;
	setsockopt(handle, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout);
	
}


size_t Socket::Read(uint8_t* data, size_t size)
{
	int result = Receive(data, size);
	if (result < 0)
		result = 0;
	return result;
}


size_t Socket::Write(const uint8_t* data, size_t size)
{
	int result = Send(data, size);
	if (result < 0)
		result = 0;
	return result;
}

