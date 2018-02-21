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
//    設定通訊協定
    enum Protocol_ACK{ ACK, NO_ACK };// 有 or 沒有Ack(回傳確認機制)
    enum Protocol_HostClient{ CONNECT_TO_HOST, CONNECT_TO_CLIENT }; //連結到Server or Client
    enum Protocol_Format{ XML, JSON, NONE};// 傳輸格式
//    連線狀態
    enum State_Connection{ ONLINE, OFFLINE, ERROR };

    explicit Communication(QWidget *parent = 0, Communication::Protocol_ACK _protocal_ACK = ACK,
                           Communication::Protocol_HostClient _protocol_HostClient = CONNECT_TO_HOST,
                           Communication::Protocol_Format _protocol_Format = NONE);

    ~Communication();

    void setPortIP(int _port, const QString &_IP);
    bool setSocket(QTcpSocket *_socket);
    void closeSocket();
//    連線到Server
    void connectToHost();
//    傳送介面
    void sendXML(const QString &_ID, const QString &_startElement1, const QString &_startElement2,
                 const QXmlStreamAttributes &_attributes, const QString &_character);
    void sendJSON(const QString &_ID, const QString &_CMD);
    void send(const QString &_message, const int &_deviceNumber);

    State_Connection state_Connection = OFFLINE;
signals:
    void online(const QString &_str);
    void offline();
    void error(const QString &_str);

//    接收時觸發
    void receiveACK(const QString &_ID);
    void receiveDONE(const QString &_ID);
    void receiveErrorDONE(const QString &_ID);
    void receiveSN(const QString &_SN);
    void receiveMAC(const QString &_MAC);
    void receiveMessage(const QString &_testName, const int &_testStage, const int &_testResult);
private slots:
    void socketError();
    void readyRead();
//    重送機制的timeout
    void reSendTimeout();

private:
    void ACK_XML_ReadSocket();
    void ACK_JSON_ReadSocket();
    void NO_ACK_Json_ReadSocket();
//    檢查完成的任務後面還有沒有連結的任務, 有的話prepend到任務等待列表
    void prependNextTask(const QString _ID);
//    各種協定參數
    Protocol_ACK protocol_ACK;
    Protocol_HostClient protocol_HostClient;
    Protocol_Format protocol_Format;

    QTcpSocket *socket;
    QDataStream in;
    int port;
    QString IP;
//    紀錄ID用的
    QString ACK_ID;
    QStringList *ID_List;
    QString reSendText;

    QMessageBox *msgBox;
    QTimer *reSendTimer;
};

#endif // COMMUNICATION_H
