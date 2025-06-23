#include "TCPServer.h"

#include <chrono>

#define forever for(;;)

int main(int argc, char *argv[]) 
{
    TCPServer TCPServer(8080);

    try
    {   
        forever
        {
            TCPServer.StartServer();
            std::this_thread::sleep_for(std::chrono::seconds(20));
            TCPServer.StopServer();
            std::this_thread::sleep_for(std::chrono::seconds(10));
        }
    }
    catch(const TCPServerException& ex)
    {
        std::cout << ex.what() << "\n";
    }
    
}