#pragma once
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include <string>


class Endpoint
{
	struct sockaddr_storage addr; // Large enough for both IPv4 or IPv6
		
public:
		
	socklen_t Size()
	{
		return sizeof(addr);
	}
		
	struct sockaddr* GetSockAddr()
	{
		return (struct sockaddr *)&addr;
	}
		
	void Set(int port)
	{
		struct sockaddr_in *dest_addr_ip4 = (struct sockaddr_in *)&addr ;
		dest_addr_ip4->sin_addr.s_addr = htonl(INADDR_ANY) ;
		dest_addr_ip4->sin_family = AF_INET ;
		dest_addr_ip4->sin_port = htons(port) ;
	}

	void Set(std::string host, int port)
	{			
		struct sockaddr_in *dest_addr_ip4 = (struct sockaddr_in *)&addr ;
		dest_addr_ip4->sin_addr.s_addr = inet_addr(host.c_str()) ;
		dest_addr_ip4->sin_family = AF_INET ;
		dest_addr_ip4->sin_port = htons(port) ;
	}
};
