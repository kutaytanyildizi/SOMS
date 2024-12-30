#include "../inc/TCPServer.h"

#include <functional>

#define forever for(;;)

#define DEFAULT_BUFLEN 512

TCPServer::TCPServer(int port = 8080) : serverPort(port)
{
    struct addrinfo* addressInfo = NULL, hints;
   
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

void TCPServer::SetupSocketAddress(struct addrinfo** addressInfo, struct addrinfo* hints)
{
    if(getaddrinfo(nullptr, std::to_string(serverPort).c_str() , hints, addressInfo))
    {
        WSACleanup();
        throw TCPServerException("Error while getting address information\n");
    }

    std::cout << "Succesfully got adress information\n";
}

void TCPServer::CreateSocket(int family, int sockType, int protocol)
{
    m_socket = socket(family, sockType, protocol);

    if(m_socket == INVALID_SOCKET)
    {
        WSACleanup();
        throw TCPServerException("Creating socket failed\n");
    }

    std::cout << "Socket succesfully created\n";
}

void TCPServer::BindSocket(const sockaddr *name, int namelen)
{
    if(bind(m_socket, name, namelen) == SOCKET_ERROR)
    {
        closesocket(m_socket);
        WSACleanup();
        throw TCPServerException("Binding failed\n");
    }

    std::cout << "Binding succesfull\n";
}

void TCPServer::StartServer()
{
    int result;

    result = listen(m_socket, SOMAXCONN);
    if(result == SOCKET_ERROR) 
    {
        closesocket(m_socket);
        WSACleanup();
        throw TCPServerException("Server cannot start listening\n");
    }

    std::cout << "Listening server port : " << serverPort << std::endl;
}

void TCPServer::AcceptConnections()
{
    forever
    {
        if(SOCKET clientSocket = accept(m_socket, nullptr, nullptr); clientSocket != INVALID_SOCKET) 
        {
            std::cout << "New client connected - Socket ID : " << clientSocket << "\n";

            if(auto clientEntry = clients.find(clientSocket); clientEntry != clients.end())
            {
                if(clientEntry->second.joinable())
                {
                    clientEntry->second.join();
                }

                clients.erase(clientSocket);
            }

            std::thread thread(std::bind(HandleClient, this, clientSocket));

            clients.emplace(std::make_pair(clientSocket, std::move(thread)));
        }
        else
        {
            std::cout << "Connection error : " << WSAGetLastError() << "\n";
            closesocket(m_socket);
            WSACleanup();
            return;
        }
    }
}

void TCPServer::HandleClient(SOCKET clientSocket)
{
    char recvbuf[DEFAULT_BUFLEN];
    int recvSize = 1;

    while(recvSize > 0)
    {
        recvSize = recv(clientSocket, recvbuf, DEFAULT_BUFLEN, 0);

        if(recvSize > 0)
            std::cout << "Client " << clientSocket << " : " << std::string(recvbuf, recvSize) << std::endl;
    }

    std::cout << "Client disconnected - Socket ID : " << clientSocket << "\n";
    closesocket(clientSocket);
}