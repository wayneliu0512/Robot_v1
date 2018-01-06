#include "mainwindow.h"
#include <QApplication>
#include <QtDebug>
#include <QFile>
#include <QTextStream>
#include <QTime>
#include <QDir>
#include <QMessageBox>
void myMessageHandler(QtMsgType type, const QMessageLogContext &, const QString & msg)
{
    QString txt;
    switch (type) {
    case QtDebugMsg:
        txt = QString("Debug: %1").arg(msg);
        break;
    case QtWarningMsg:
        txt = QString("Warning: %1").arg(msg);
    break;
    case QtCriticalMsg:
        txt = QString("Critical: %1").arg(msg);
    break;
    case QtFatalMsg:
        txt = QString("Fatal: %1").arg(msg);
    break;
    }
    QFile outFile("DebugLog//" + QDate::currentDate().toString(Qt::ISODate) + "_qDebugLog");
    if(outFile.exists())
    {
        outFile.open(QIODevice::WriteOnly | QIODevice::Append);
    }else{
        outFile.open(QIODevice::WriteOnly);
    }
    QTextStream ts(&outFile);
    ts << "Time: " << QTime::currentTime().toString() << "\t" << txt << endl;
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

#if defined(Q_OS_MACOS)
    QDir dir("/Users/wayneliu/Documents/Qt/build-Robot_v1-Desktop_Qt_5_8_0_clang_64bit-Release/DebugLog");
#elif defined(Q_OS_WIN)
    QDir dir("DebugLog");
#endif

    if(!dir.exists())
    {
        QMessageBox::critical(0, "Error", "DebugLog folder doesn't exist.");
        return 1;
    }
//    安裝MessageHandler可以輸出QtDebug訊息
    qInstallMessageHandler(myMessageHandler);

    MainWindow w;
    w.show();

    return a.exec();
}
