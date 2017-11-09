#include <QCoreApplication>
#include <client.h>
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    Client *socket = new Client;

    return a.exec();
}
