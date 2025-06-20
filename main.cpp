/* #include <QApplication>
#include <QPushButton> */

#include "TCPServer.h"

#include <chrono>

#define forever for(;;)

int main(int argc, char *argv[]) 
{
   /* QApplication a(argc, argv);
    QPushButton button("Hello world!", nullptr);
    button.resize(200, 100);
    button.show();

    return QApplication::exec(); */

    TCPServer TCPServer(8080);

    try
    {   
        forever
        {
            TCPServer.StartServer();
            std::this_thread::sleep_for(std::chrono::seconds(20));
            TCPServer.StopServer();
        }
    }
    catch(const TCPServerException& ex)
    {
        std::cout << ex.what() << "\n";
    }
    
}