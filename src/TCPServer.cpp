#include "TCPServer.h"

#include <functional>
#include <condition_variable>
#include <chrono>

#define STATIC

#define forever for(;;)

#define DEFAULT_BUFLEN 512

TCPServer::TCPServer(const int port = 8080) : serverPort(port)
{
    struct addrinfo* addressInfo = nullptr, hints{};

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;       // IPv4
    hints.ai_socktype = SOCK_STREAM; // TCP
    hints.ai_protocol = IPPROTO_TCP; // TCP Protocol
    hints.ai_flags = AI_PASSIVE;     // Used as Server

    try
    {
        SetupSocketAddress(&addressInfo, &hints);
        CreateSocket(addressInfo->ai_family, addressInfo->ai_socktype, addressInfo->ai_protocol);
        BindSocket(addressInfo->ai_addr, static_cast<int>(addressInfo->ai_addrlen));
    }
    catch(const TCPServerException& ex)
    {
        freeaddrinfo(addressInfo);
        throw;
    }

    freeaddrinfo(addressInfo);
}

void TCPServer::SetupSocketAddress(struct addrinfo** addressInfo, const struct addrinfo* hints) const
{
    if(getaddrinfo(nullptr, std::to_string(serverPort).c_str() , hints, addressInfo))
    {
        WSACleanup();
        throw TCPServerException("Error while getting address information\n");
    }

    std::cout << "Successfully got address information\n";
}

void TCPServer::CreateSocket(const int family, const int sockType, const int protocol)
{
    m_socket = socket(family, sockType, protocol);

    if(m_socket == INVALID_SOCKET)
    {
        WSACleanup();
        throw TCPServerException("Creating socket failed\n");
    }

    std::cout << "Socket successfully created\n";
}

void TCPServer::BindSocket(const sockaddr* name, const int nameLen) const
{
    if(bind(m_socket, name, nameLen) == SOCKET_ERROR)
    {
        closesocket(m_socket);
        WSACleanup();
        throw TCPServerException("Binding failed\n");
    }

    std::cout << "Binding successful\n";
}

void TCPServer::StartServer()
{
    isServerStopped = false;
    mThread = std::thread(std::bind(AcceptConnections, this));
}

void TCPServer::StopServer()
{
    ClearClients();

    isServerStopped = true;

    while(!mThread.joinable()){}

    mThread.join();

    std::cout << "Socket Closed\n";
}

void TCPServer::AcceptConnections()
{
    if(listen(m_socket, SOMAXCONN) == SOCKET_ERROR) 
    {
        closesocket(m_socket);
        WSACleanup();
        throw TCPServerException("Server cannot start listening\n");
    }

    std::cout << "Listening server port : " << serverPort << std::endl;

    while (!isServerStopped)
    {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(m_socket, &readfds);

        timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 200000;

        int selectResult = select(0, &readfds, nullptr, nullptr, &timeout);

        if (selectResult > 0 && FD_ISSET(m_socket, &readfds))
        {
            std::cout << "Starting to accept connections from clients" << "\n";

            SOCKET clientSocket = accept(m_socket, nullptr, nullptr);
            
            if (clientSocket != INVALID_SOCKET)
            {
                std::cout << "New client connected - Socket ID : " << clientSocket << "\n";

                if (auto clientEntry = clients.find(clientSocket); clientEntry != clients.end())
                {
                    if (clientEntry->second.joinable())
                    {
                        clientEntry->second.join();
                    }

                    clients.erase(clientSocket);
                }

                std::thread thread([clientSocket] { HandleClient(clientSocket); });

                clients.emplace(clientSocket, std::move(thread));
            }
            else
            {
                std::cout << "Accept error: " << WSAGetLastError() << "\n";
            }
        }
        else if (selectResult == 0)
        {
            std::cout << "Accept timeout server stopped accepting connections" << "\n";
        }
        else
        {
            std::cout << "Select error: " << WSAGetLastError() << "\n";
            closesocket(m_socket);
            WSACleanup();
            return;
        }

        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
}

void TCPServer::ClearClients()
{
    for(auto& [socket, thread] : clients)
    {
        closesocket(socket);
        
        if(thread.joinable())
        {
            thread.join();
        }
    }

    clients.clear();
}

STATIC void TCPServer::HandleClient(const SOCKET clientSocket)
{
    int recvSize = 1;

    while(recvSize > 0)
    {
        char buf[DEFAULT_BUFLEN];
        recvSize = recv(clientSocket, buf, DEFAULT_BUFLEN, 0);

        if(recvSize > 0)
        {
            std::cout << "Client " << clientSocket << " : " << std::string(buf, recvSize) << std::endl;
        }
    }

    std::cout << "Client disconnected - Socket ID : " << clientSocket << "\n";
    closesocket(clientSocket);
}