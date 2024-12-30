#include <iostream>

#include <winsock2.h>
#include <WS2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

#include <map>
#include <thread>

class TCPServerException : public std::exception
{
public:
    TCPServerException(const char* errorMessage) : message(errorMessage) {}

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
    TCPServer(int);
    void StartServer();
    void AcceptConnections();

private:
    void SetupSocketAddress(struct addrinfo**, struct addrinfo*);
    void CreateSocket(int family, int sockType, int protocol);
    void BindSocket(const sockaddr *name, int namelen);

    void HandleClient(SOCKET clientSocket);

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
        WSADATA socketInfo;
    };

    WSAInitializer wsaData;
    SOCKET m_socket;

    int serverPort;

    std::map<SOCKET, std::thread> clients;
};