/* #include <QApplication>
#include <QPushButton> */

#include "inc/TCPServer.h"

int main(int argc, char *argv[]) 
{
   /* QApplication a(argc, argv);
    QPushButton button("Hello world!", nullptr);
    button.resize(200, 100);
    button.show();

    return QApplication::exec(); */
    try
    {
        TCPServer server(8080);
        server.StartServer();
        server.AcceptConnections();
    }
    catch(const TCPServerException& ex)
    {
        std::cout << ex.what() << "\n";
    }
    
}