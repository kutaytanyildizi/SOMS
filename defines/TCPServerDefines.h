#pragma once

#include <exception>

#include <winsock2.h>

class TCPServerException final : public std::exception
{
public:
    explicit TCPServerException(const char* errorMessage) : message(errorMessage) {}

    const char* what() const noexcept override
    {
        return message;
    }

private:
    const char* message;
};

class WSAInitializer 
{
public:
    WSAInitializer()
    {
        if(WSAStartup(MAKEWORD(2, 2), &socketInfo))
        {
            throw TCPServerException("WSA Startup Failed");
        }
    }

    ~WSAInitializer()
    {
        WSACleanup();
    }
private:
    WSADATA socketInfo{};
};