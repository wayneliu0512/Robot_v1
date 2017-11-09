#include <QCoreApplication>
#include <client.h>

QT_USE_NAMESPACE

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QCoreApplication::setApplicationName("Robot-ClientTester-Program");
    QCoreApplication::setApplicationVersion("1.0");

    QCommandLineParser parser;
    parser.setApplicationDescription("For test robot client program");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("SN", QCoreApplication::translate("main", "Serial number."));
    parser.addPositionalArgument("MAC", QCoreApplication::translate("main", "MAC ID number."));

    // A boolean option with a single name (-p)
    QCommandLineOption startOption(QStringList() << "s" <<"start", QCoreApplication::translate("main", "Start running."));
    parser.addOption(startOption);

    // Process the actual command line arguments given by the user
    parser.process(a);

    const QStringList args = parser.positionalArguments();
    // SN is args.at(0), MAC is args.at(1)

    bool start = parser.isSet(startOption);

    if(!start)
    {
        parser.showHelp(0);
        return 1;
    }

    Client client(args.at(0), args.at(1));

    return a.exec();
}
