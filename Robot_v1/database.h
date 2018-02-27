#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QSqlDatabase>

class Database : public QObject
{
    Q_OBJECT
public:
    explicit Database(QObject *parent = nullptr, QString connectionName = "default", QString url = "172.16.4.92");

    void insert(const QString &toolingSN, const int toolingNumber, const QString &mo, const QString &pn,
                        const QString &sn, const QString &mac, const QString &testName, const int testVersion);
    void update(const QString &result, const int testTime, const QString &sn, const QString &testName, const int testVersion,
                const int reTest);
    int getMaxVersion(const QString &sn);
    int getReTest(const QString &sn, const QString &testName, const int testVersion);
private:
    QVariant execDatabase(const QString &command);

    QSqlDatabase database;
};

#endif // DATABASE_H
