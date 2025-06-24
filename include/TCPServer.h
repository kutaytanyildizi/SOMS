#pragma once

#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <WS2tcpip.h>

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
    void SetupServerSocket();

    void SetupSocketAddress(struct addrinfo**, const struct addrinfo*) const;
    void CreateSocket(const int family, const int sockType, const int protocol);
    void BindSocket(const sockaddr* name, const int nameLen) const;

    CONNECTION_STATUS IsIncomingConnectionAvailable(int timeoutSeconds, long timeoutMicroseconds);

    void AcceptConnections();

    void ClearClients();

    static void HandleClient(const SOCKET clientSocket);

private:
    std::thread mThread;

    WSADATA socketInfo;
    SOCKET m_socket{};
    fd_set readFDS;
    timeval timeout;

    int serverPort;

    std::unordered_map<SOCKET, std::thread> clients;

    bool isServerStopped = false;
};

#endif // TCPSERVER_H