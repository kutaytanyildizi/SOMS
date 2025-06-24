#include "TCPServer.h"

#pragma comment(lib, "Ws2_32.lib")

#include <functional>
#include <condition_variable>
#include <chrono>

#define STATIC

static constexpr int ACTIVATE_SO_REUSEADDR = 1;

static constexpr int SOCKET_TIMEOUT_SECONDS = 0;
static constexpr long SOCKET_TIMEOUT_MICROSECONDS = 200'000;

static constexpr size_t DEFAULT_BUFLEN = 512;

TCPServer::TCPServer(const int port = 8080) : serverPort(port){}

void TCPServer::SetupServerSocket()
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

    if(int opt = ACTIVATE_SO_REUSEADDR; setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt)) == SOCKET_ERROR)
    {
        closesocket(m_socket);
        WSACleanup();
        int error = WSAGetLastError();
        throw TCPServerException("Setting SO_REUSEADDR failed\n");
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
    try
    {
        WSAStartup(MAKEWORD(2, 2), &socketInfo);
        SetupServerSocket();
    }
    catch(const std::exception& ex)
    {
        std::cout << ex.what() << "\n";

        throw TCPServerException("Start Server failed when setting up server socket\n");
    }
    
    isServerStopped = false;
    mThread = std::thread(std::bind(AcceptConnections, this));
}

void TCPServer::StopServer()
{
    isServerStopped = true;

    ClearClients();

    while(!mThread.joinable()){}

    mThread.join();

    shutdown(m_socket, SD_BOTH);
    closesocket(m_socket);

    FD_CLR(m_socket, &readFDS);
    
    WSACleanup();

    std::cout << "Socket Closed\n";
}

CONNECTION_STATUS TCPServer::IsIncomingConnectionAvailable(int timeoutSeconds, long timeoutMicroseconds)
{
    FD_ZERO(&readFDS);
    FD_SET(m_socket, &readFDS);

    timeout.tv_sec = timeoutSeconds;
    timeout.tv_usec = timeoutMicroseconds;

    int result = select(0, &readFDS, nullptr, nullptr, &timeout);

    if(result > 0 && FD_ISSET(m_socket, &readFDS))
    {
        return CONNECTION_STATUS::NEW_CONNECTION;
    }
    else if(result == 0)
    {
        return CONNECTION_STATUS::NO_CONNECTION;
    }
    else
    {
        return CONNECTION_STATUS::CONNECTION_ERROR;
    }
}

void TCPServer::AcceptConnections()
{
    if(listen(m_socket, SOMAXCONN) == SOCKET_ERROR) 
    {
        StopServer();
        throw TCPServerException("Server cannot start listening\n");
    }

    std::cout << "Listening server port : " << serverPort << std::endl;

    while (!isServerStopped)
    {
        switch(IsIncomingConnectionAvailable(SOCKET_TIMEOUT_SECONDS, SOCKET_TIMEOUT_MICROSECONDS))
        {
            case CONNECTION_STATUS::NEW_CONNECTION:
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
    
                break;
            }
    
            case CONNECTION_STATUS::NO_CONNECTION:
            {
                // std::cout << "Accept timeout server stopped accepting connections" << "\n";
                // Do nothing :)
                break;
            }
            
            case CONNECTION_STATUS::CONNECTION_ERROR:
            {
                std::cout << "Select error: " << WSAGetLastError() << "\n";
                StopServer();
                throw TCPServerException("Connection Error occured\n");
    
                break;
            }
            
            default:
            {
                StopServer();
                throw TCPServerException("Unknown Error occured\n");
                break;
            }
        }
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