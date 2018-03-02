#include <QCoreApplication>
#include <client.h>
#include <iostream>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    a.setApplicationName("Client");
    a.setApplicationVersion("1.01.01");

    std::cout << "Application version: " << a.applicationVersion().toStdString() << std::endl;

    Client socket;

    return a.exec();
}
