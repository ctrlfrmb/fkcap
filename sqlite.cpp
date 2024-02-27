#include "sqlite.h"
#include <QtSql/QSqlQuery>
#include <QDebug>
#include <QtSql/QSqlError>

SqliteCom::SqliteCom(QObject *parent, int time)
    : QObject(parent), isOpen(false), submitTime(time)
{
}

SqliteCom::~SqliteCom()
{
    closeDataBase();
}

void SqliteCom::closeDataBase()
{
    isOpen = false;
    if (db.isOpen())
        db.close();
}

bool SqliteCom::loadFile(const QString& path)
{
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(path);

    if (db.open()) {
        if (!createTableIfNotExists())
            return false;
    } else {
        qDebug() << "Error opening database: " << db.lastError();
        return false;
    }

    this->startTimer(submitTime); // 启动定时器，默认每隔1秒提交一次
    isOpen = true;
    return true;
}

void SqliteCom::storePacket(const figkey::PacketInfo &packet)
{
    if (!isOpen)
        return;

    QSqlQuery query(db);
    query.prepare("INSERT INTO Packets (index, timestamp, srcIP, destIP, "
                  "srcMAC, destMAC, srcPort, destPort, protocolType, "
                  "info) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
    query.addBindValue(QVariant::fromValue(packet.index));
    query.addBindValue(QString::fromStdString(packet.timestamp));
    query.addBindValue(QString::fromStdString(packet.srcIP));
    query.addBindValue(QString::fromStdString(packet.destIP));
    query.addBindValue(QString::fromStdString(packet.srcMAC));
    query.addBindValue(QString::fromStdString(packet.destMAC));
    query.addBindValue(QVariant::fromValue(packet.srcPort));
    query.addBindValue(QVariant::fromValue(packet.destPort));
    query.addBindValue(QVariant::fromValue(packet.protocolType));
    query.addBindValue(QString::fromStdString(packet.info));

    if (!query.exec()) {
        qDebug() << "Error inserting into the table: " << query.lastError();
        // Handle error...
    }
}

bool SqliteCom::createTableIfNotExists()
{
    QSqlQuery query(db);
    if (!query.exec("CREATE TABLE IF NOT EXISTS Packets "
                    "(index INTEGER PRIMARY KEY, timestamp TEXT, srcIP TEXT, "
                    "destIP TEXT, srcMAC TEXT, destMAC TEXT, srcPort INTEGER, "
                    "destPort INTEGER, protocolType INTEGER, info TEXT)")) {
        qDebug() << "Error creating table: " << query.lastError();
        return false;
    }

    return true;
}

void SqliteCom::timerEvent(QTimerEvent * /* event */)
{
    if (!isOpen)
        return;

    if (db.commit()) {
        // 如果提交成功，发送transactionCommitted信号
        //emit transactionCommitted(...); // 你需要替换...为实际的缓存数据
    } else {
        qDebug() << "Error commiting transaction: " << db.lastError();
        // Handle error...
    }
}
