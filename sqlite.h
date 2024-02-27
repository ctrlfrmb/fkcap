#ifndef SQLITE_COMMON_H
#define SQLITE_COMMON_H

#include <QtSql/QSqlDatabase>
#include <QFile>
#include "def.h"

class SqliteCom : public QObject
{
    Q_OBJECT

public:
    explicit SqliteCom(QObject *parent = nullptr, int time = 1000);
    ~SqliteCom();

    bool loadFile(const QString& path);

    // 存储抓包数据到数据库
    void storePacket(const figkey::PacketInfo &packet);

signals:
    void transactionCommitted(const QList<figkey::PacketInfo>& packetList);

protected:
    void timerEvent(QTimerEvent *event);

private:
    void closeDataBase();

    // 创建数据库表格
    bool createTableIfNotExists();

    bool isOpen;
    int submitTime;
    QSqlDatabase db;
};

#endif // SQLITE_COMMON_H
