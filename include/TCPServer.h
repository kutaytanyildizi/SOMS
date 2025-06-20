#pragma once

#include <WS2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

#include <iostream>
#include <unordered_map>
#include <thread>

#include "TCPServerDefines.h"

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
    std::thread mThread;

    WSAInitializer wsaData;
    SOCKET m_socket{};

    int serverPort;

    std::unordered_map<SOCKET, std::thread> clients;

    bool isServerStopped = false;
};