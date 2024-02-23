#include "TCPClient.h"


Result TCPClient::DeviceSetConfig(IDeviceConfig &config)
{
    ContextLock lock(mutex);
	DeviceSetStatus(DeviceStatus::Dependencies);
	return Result::Ok;
}

Result TCPClient::DeviceLoadDependencies(std::shared_ptr<DeviceManager> deviceManager)
{
    //TODO: We have some dependency on the initialisation of the network stuff. 
    //Figure out how this works in IDF, and add the proper dependencies. (Perhaps fetch device by interface and check if its ready.)
    ContextLock lock(mutex);
	DeviceSetStatus(DeviceStatus::Initializing);
	return Result::Ok;
}

Result TCPClient::DeviceInit()
{
    ContextLock lock(mutex);
    RETURN_ON_ERR(socket.Close());      // Ensure any open connections are closed if initialisation is called again.
    RETURN_ON_ERR(socket.InitTCP());    
	DeviceSetStatus(DeviceStatus::Ready);
	return Result::Ok;
}

Result TCPClient::Connect(Endpoint& endpoint)
{
    ContextLock lock(mutex);
    RETURN_ON_ERR(socket.Connect(endpoint));    
	return Result::Ok;
}

Result TCPClient::StreamWrite(const uint8_t *data, size_t length, size_t *written, TickType_t timeout)
{
    // ContextLock lock(mutex); 
    // TODO: If we lock, we cant read and write at the same time. But if we don't, we don't protect the state of the socket.
    // Perhaps use 2 mutexes, a read and write mutex. Take them both in all public functions and take the respective in the read / write functions.
    return socket.Send(data, length, written);   
}

Result TCPClient::StreamRead(uint8_t *data, size_t length, size_t *read, TickType_t timeout)
{
    // ContextLock lock(mutex); 
    // TODO: If we lock, we cant read and write at the same time. But if we don't, we don't protect the state of the socket.
    // Perhaps use 2 mutexes, a read and write mutex. Take them both in all public functions and take the respective in the read / write functions.
    return socket.Receive(data, length, read);   
}
