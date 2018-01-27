#include <QCoreApplication>
#include <client.h>
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    Client socket;

    return a.exec();
}
