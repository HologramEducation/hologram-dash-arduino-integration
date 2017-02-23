#pragma once

#include <cstdint>
#include <cstddef>
#include <cstdint>

class NetworkEventHandler {
public:
    virtual void onNetworkEvent(uint32_t id, const void* content)=0;
};

class Network {
public:
    virtual void init(NetworkEventHandler &handler);
    virtual bool connect()=0;
    virtual bool disconnect()=0;
    virtual int getConnectionStatus()=0;
    virtual int getSignalStrength()=0;

    virtual void powerUp()=0;
    virtual void powerDown()=0;

    virtual void pollEvents()=0;

    virtual int open(const char* host, int port)=0;
    virtual int open(const char* host, const char* port)=0;
    virtual bool write(int socket, const char* content);
    virtual bool write(int socket, const uint8_t* content, int length)=0;
    virtual void flush(int socket)=0;
    virtual int read(int socket, int numbytes, uint8_t *buffer)=0;
    virtual int read(int socket, int numbytes, uint8_t *buffer, uint32_t timeout)=0;
    virtual bool close(int socket)=0;
protected:
    NetworkEventHandler *eventHandler;
};
