#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include<QWidget>

class QTcpSocket;
class QMessageBox;
class QXmlStreamAttributes;

class Communication : public QWidget
{
    Q_OBJECT
public:
    enum Protocol_ACK{ ACK, NO_ACK };
    enum Protocol_HostClient{ CONNECT_TO_HOST, CONNECT_TO_CLIENT };
    enum Protocol_Format{ XML, JSON, NONE};
    enum State_Connection{ ONLINE, OFFLINE, ERROR };

    explicit Communication(QWidget *parent = 0, Communication::Protocol_ACK _protocal_ACK = ACK,
                           Communication::Protocol_HostClient _protocol_HostClient = CONNECT_TO_HOST,
                           Communication::Protocol_Format _protocol_Format = NONE);

    ~Communication();

    void setPortIP(int _port, const QString &_IP);
    void connectToHost();
    bool setSocket(QTcpSocket *_socket);
    void sendXML(const QString &_ID, const QString &_startElement1, const QString &_startElement2,
                 const QXmlStreamAttributes &_attributes, const QString &_character);
    void sendJSON(const QString &_ID, const QString &_CMD);
    void send(const QString &_message, const int &_deviceNumber);

    State_Connection state_Connection = OFFLINE;
signals:
    void online(const QString &_str);
    void offline();
    void error(const QString &_str);

    void receiveACK(const QString &_ID);
    void receiveDONE(const QString &_ID);
    void receiveErrorDONE(const QString &_ID);
    void receiveSN(const QString &_SN);
    void receiveMAC(const QString &_MAC);
    void receiveMessage(const QString &_message);

private slots:
    void socketError();
    void readyRead();
    void reSendTimeout();

private:
    void ACK_XML_ReadSocket();
    void ACK_JSON_ReadSocket();
    void NO_ACK_ReadSocket();

    void prependNextTask(const QString _ID);

    Protocol_ACK protocol_ACK;
    Protocol_HostClient protocol_HostClient;
    Protocol_Format protocol_Format;

    QTcpSocket *socket;
    int port;
    QString IP;

    QString ACK_ID;
    QStringList *ID_List;
    QString reSendText;

    QMessageBox *msgBox;
    QTimer *reSendTimer;
};

#endif // COMMUNICATION_H
