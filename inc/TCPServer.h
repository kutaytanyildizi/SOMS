#pragma once
#include <iostream>

#include <winsock2.h>
#include <WS2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

#include <map>
#include <thread>

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

class TCPServer
{
public:
    explicit TCPServer(const int port);
    void StartServer();
    void StopServer();

private:
    void SetupSocketAddress(struct addrinfo**, const struct addrinfo*) const;
    void CreateSocket(const int family, const int sockType, const int protocol);
    void BindSocket(const sockaddr* name, const int nameLen) const;

    void AcceptConnections();

    void ClearClients();

    static void HandleClient(const SOCKET clientSocket);

private:
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

    std::thread mThread;

    WSAInitializer wsaData;
    SOCKET m_socket{};

    int serverPort;

    std::map<SOCKET, std::thread> clients;

    bool isServerStopped = false;
};