#include "communication.h"
#include "mainwindow.h"
#include "task.h"
#include <QTcpSocket>
#include <QMessageBox>
#include <QJsonObject>
#include <QJsonDocument>
#include <QTimer>
#include <QString>
#include <QXmlStreamWriter>

Communication::Communication(QWidget *parent, Communication::Protocol_ACK _protocol_ACK,
                             Communication::Protocol_HostClient _protocol_HostClient,
                             Communication::Protocol_Format _protocol_Format):
    QWidget(parent), protocol_ACK(_protocol_ACK), protocol_HostClient(_protocol_HostClient),
    protocol_Format(_protocol_Format)
{

    if(protocol_HostClient == Communication::CONNECT_TO_HOST)
    {
        socket = new QTcpSocket(this);
        connect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()));
        connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(socketError()));
    }

    if(protocol_ACK == Communication::ACK)
    {
        ID_List = new QStringList;
        reSendTimer = new QTimer(this);
        reSendTimer->setInterval(2000);
        connect(reSendTimer, SIGNAL(timeout()), this, SLOT(reSendTimeout()));
    }

    msgBox = new QMessageBox(this);
    msgBox->setStandardButtons(QMessageBox::Cancel);
    msgBox->setDefaultButton(QMessageBox::Cancel);
    msgBox->setIcon(QMessageBox::Critical);
    msgBox->setWindowTitle("Error");

}

Communication::~Communication()
{
    if(protocol_ACK == ACK)
        delete ID_List;
}

void Communication::setPortIP(int _port, const QString &_IP)
{
    port = _port;
    IP = _IP;
}

void Communication::connectToHost()
{
    if(protocol_HostClient != Communication::CONNECT_TO_HOST)
    {
        qCritical() << "Error: Communication protocol error but try to connect to host";
        return;
    }

    socket->connectToHost(IP, port);

    if (socket->waitForConnected(1000))
    {
        qDebug("Connected to Host!");
        state_Connection = ONLINE;
        emit online("Online");
    }else
    {
        QMessageBox::critical(this, "Error", "Connect to host fail");
        qDebug() << "Connect to host fail";
        state_Connection = OFFLINE;
        emit offline();
    }
}

bool Communication::setSocket(QTcpSocket *_socket)
{
    if(protocol_HostClient != Communication::CONNECT_TO_CLIENT)
    {
        qCritical() << "Error: Communication protocol error but try to set socket";
        return false;
    }

    if(!_socket->isValid())
        return false;

    socket = _socket;
    socket->setParent(this);
    connect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()));
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(socketError()));
    state_Connection = ONLINE;
    emit online("Online");
    return true;
}

void Communication::readyRead()
{
    if(protocol_ACK == ACK)
    {
        switch (protocol_Format) {
        case XML:
            ACK_XML_ReadSocket();
            break;
        case JSON:
            ACK_JSON_ReadSocket();
            break;
        case NONE:
            qCritical() << "Communication::readyRead()/ACK/NONE exception error.";
            QMessageBox::critical(this, "Error", "Communication::readyRead()/ACK/NONE\nStill on development.");
            break;
        default:
            qCritical() << "Communication::readyRead()/ACK/NONE Unknown protocol_Format.";
            QMessageBox::critical(this, "Error", "Communication::readyRead()/ACK\nUnknown protocol_Format");
            break;
        }

    }else
    {
        switch (protocol_Format) {
        case XML:
            qCritical() << "Communication::readyRead()/NO_ACK/XML exception error";
            QMessageBox::critical(this, "Error", "Communication::readyRead()/NO_ACK/XML\nStill on development.");
            break;
        case JSON:
            qCritical() << "Communication::readyRead()/NO_ACK/JSON exception error";
            QMessageBox::critical(this, "Error", "Communication::readyRead()/NO_ACK/JSON\nStill on development.");
            break;
        case NONE:
            NO_ACK_Json_ReadSocket();
            break;
        default:
            qCritical() << "Communication::readyRead()/NO_ACK/ Unknown protocol_Format";
            QMessageBox::critical(this, "Error", "Communication::readyRead()/NO_ACK\nUnknown protocol_Format");
            break;
        }
    }
}

void Communication::ACK_XML_ReadSocket()
{
    QString str = socket->readAll();

    QString ID = str.split(",").at(0);

    //比對ID是否正確
    if(!ID_List->contains(ID))
    {
        if(str.contains("error", Qt::CaseInsensitive))
        {
            msgBox->setText(str);
            msgBox->show();
        }else
        {
            qDebug() << "Robot Comunication Exception: " + str;
        }
        return;
    }

    if(str.contains("ACK"))
    {
        if(ACK_ID == ID)
        {
            reSendTimer->stop();
            qDebug() << "ACK Back << " + str;
            ACK_ID.clear();
            emit receiveACK(ID);
        }
    }else if(str.contains("DONE"))
    {
        qDebug() << "DONE Back << " + str;
        ID_List->removeOne(ID);
        prependNextTask(ID);
        emit receiveDONE(ID);
    }
}

void Communication::ACK_JSON_ReadSocket()
{
    QByteArray readData = socket->readAll();

    QJsonDocument doc = QJsonDocument::fromJson(readData);
    QJsonValue ID_value = doc.object().value("ID");
    QJsonValue type_value = doc.object().value("Type");
    QJsonValue SN_value = doc.object().value("SN");
    QJsonValue MAC_value = doc.object().value("MAC");
    QJsonValue error_value = doc.object().value("Error");

    qDebug() << "ReadFromCCD: " + readData;

    if(!error_value.isUndefined())
    {
        msgBox->setText(error_value.toString());
        msgBox->show();

        emit receiveErrorDONE(ID_value.toString());
    }

    if(type_value.toString() == "ACK")
    {
        if(ID_value.toString() == ACK_ID)
        {
            reSendTimer->stop();
            qDebug() << "ACK Back << " + readData;
            emit receiveACK(ACK_ID);
            ACK_ID.clear();
        }
    }
    else if(type_value.toString() == "DONE")
    {
        if(ID_List->contains(ID_value.toString()))
        {
            qDebug() << "DONE Back << " + readData;

            if(!SN_value.isUndefined())
                emit receiveSN(SN_value.toString());

            if(!MAC_value.isUndefined())
                emit receiveMAC(MAC_value.toString());

            ID_List->removeOne(ID_value.toString());
            prependNextTask(ID_value.toString());
            emit receiveDONE(ID_value.toString());
        }
    }
}

void Communication::prependNextTask(const QString _ID)
{
    for(int i = 0; i < MainWindow::inActionList.length(); i++)
    {
        if(MainWindow::inActionList.at(i)->ID == _ID)
        {
            Task *nextTask = MainWindow::inActionList.at(i)->takeNextTask();

            if(nextTask == nullptr)
                return;

            MainWindow::waitingList.prepend(nextTask);
        }
    }
}

void Communication::NO_ACK_Json_ReadSocket()
{
    QByteArray readData = socket->readAll();
    qDebug() << "ReadFromNoAckJson: " + readData;

    QJsonDocument doc = QJsonDocument::fromJson(readData);
    QJsonValue testName_value = doc.object().value("TestName");
    QJsonValue testStage_value = doc.object().value("TestStage");
    QJsonValue testResult_value = doc.object().value("TestResult");

    emit receiveMessage(testName_value.toString(), testStage_value.toInt(), testResult_value.toInt());
}

void Communication::sendXML(const QString &_ID, const QString &_startElement1, const QString &_startElement2,
                            const QXmlStreamAttributes &_attributes, const QString &_character)
{
    QString sendStr;
    QXmlStreamWriter stream(&sendStr);
    stream.writeStartElement(_startElement1);
    stream.writeStartElement(_startElement2);
    stream.writeAttributes(_attributes);
    stream.writeCharacters(_character);
    stream.writeEndElement();
    stream.writeEndElement();

    ID_List->append(_ID);
    ACK_ID = _ID;

    reSendText = sendStr;

    socket->write(sendStr.toLatin1());
    qDebug() << "sendXML: " << sendStr;

    reSendTimer->start();
}

void Communication::sendJSON(const QString &_ID, const QString &_CMD)
{
    ACK_ID = _ID;
    ID_List->append(_ID);

    QJsonObject ccd_Obj;
    ccd_Obj.insert("ID", _ID);
    ccd_Obj.insert("CMD", _CMD);

    QJsonDocument doc(ccd_Obj);

    QByteArray sendData = doc.toJson();

    reSendText = QString::fromUtf8(sendData);

    //發送資料
    socket->write(sendData);
    qDebug() << "sendJSON: " + sendData;

    //開始定時重送封包
    reSendTimer->start();
}

void Communication::send(const QString &_message, const int &_deviceNumber)
{
    socket->write(_message.toLatin1());
    qDebug() << "Send to Tooling" + QString::number(_deviceNumber) + ": " + _message;
}

void Communication::socketError()
{
    msgBox->setText("Error: Communication::socketError()\n" + socket->errorString());
    msgBox->show();

    state_Connection = ERROR;
    emit error("Socket error");
}

void Communication::reSendTimeout()
{
    if(ACK_ID.isEmpty())
    {
        reSendTimer->stop();
        return;
    }

    socket->write(reSendText.toLatin1());
    qDebug() << "ReSendMessage: " << reSendText << "......";
}

