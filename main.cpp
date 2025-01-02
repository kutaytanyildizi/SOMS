/* #include <QApplication>
#include <QPushButton> */

#include "inc/TCPServer.h"

#include <chrono>

int main(int argc, char *argv[]) 
{
   /* QApplication a(argc, argv);
    QPushButton button("Hello world!", nullptr);
    button.resize(200, 100);
    button.show();

    return QApplication::exec(); */
    TCPServer server(8080);

    try
    {
        while(1)
        {
            server.StartServer();

            std::this_thread::sleep_for(std::chrono::seconds(10));

            server.StopServer();
        }
    }
    catch(const TCPServerException& ex)
    {
        std::cout << ex.what() << "\n";
    }
    
}