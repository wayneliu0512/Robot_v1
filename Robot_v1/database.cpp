#include "database.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>

Database::Database(QObject *parent, QString connectionName, QString url) : QObject(parent)
{
    QString dsn = QString("DRIVER={SQL Server};SERVER=%1;UID=sa;PWD=a123456;DATABASE=SmartFactory").arg(url);

    database = QSqlDatabase::addDatabase("QODBC", connectionName);
    database.setDatabaseName(dsn);
}

void Database::insert(const QString &toolingSN, const int toolingNumber, const QString &mo, const QString &pn,
                              const QString &sn, const QString &mac, const QString &testName, const int testVersion)
{
    QString cmdInsert = QString("insert into RobotDashboardDemo (ToolNo, MLocation, MO, PN, SN, MAC, TestingItem, CreateOn, Version) "
                                "values ('%1', %2, '%3', '%4', '%5', '%6', '%7', convert(varchar, getdate(), 120), '%8')")
                                .arg(toolingSN).arg(toolingNumber).arg(mo).arg(pn).arg(sn).arg(mac).arg(testName).arg(testVersion);

    execDatabase(cmdInsert);
}

void Database::update(const QString &result, const int testTime, const QString &sn, const QString &testName,
                      const int testVersion, const int reTest)
{
    QString cmdUpdate = QString("update RobotDashboardDemo set Result = '%1', CycleTime = %2, ReTest = %3 where SN = '%4' and "
                                "TestingItem = '%5' and Version = %6")
                                .arg(result).arg(testTime).arg(reTest).arg(sn).arg(testName).arg(testVersion);

    execDatabase(cmdUpdate);
}

int Database::getMaxVersion(const QString &sn)
{
    QString cmdSelect = QString("select MAX(Version) from RobotDashboardDemo where SN = '%1'").arg(sn);
    int testVersion = execDatabase(cmdSelect).toInt();
    qDebug() << "GetMaxVersion: " << testVersion;
    return testVersion;
}

int Database::getReTest(const QString &sn, const QString &testName, const int testVersion)
{
    QString cmdSelect = QString("select ReTest from RobotDashboardDemo where SN = '%1' and TestingItem = '%2' and Version = %3")
                                .arg(sn).arg(testName).arg(testVersion);
    int reTestNum = execDatabase(cmdSelect).toInt();
    qDebug() << "GetReTest: " << reTestNum;
    return reTestNum;
}

QVariant Database::execDatabase(const QString &command)
{
    if (!database.open())
        qCritical() << "Db open fail: " << database.lastError().text();

    QSqlQuery sq(database);

    if (!sq.exec(command))
        qCritical() << "Db update error.";

    QVariant answer = 0;

    if (sq.next()) {
        answer = sq.value(0);
    }
    database.close();
    return answer;
}
