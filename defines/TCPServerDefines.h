#pragma once

#include <exception>

#include <winsock2.h>

enum CONNECTION_STATUS
{
    NEW_CONNECTION,
    NO_CONNECTION,
    CONNECTION_ERROR
};

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