#ifndef SQLITE_COMMON_H
#define SQLITE_COMMON_H

#include <QtSql/QSqlDatabase>
#include <QFile>
#include "def.h"

class SqliteCom : public QObject
{
    Q_OBJECT

public:
    explicit SqliteCom(QObject *parent = nullptr);
    ~SqliteCom();

    bool openFile(const QString& path);

    // 存储抓包数据到数据库
    bool storePacket(const figkey::PacketInfo &packet);

    void closeFile();

signals:
    void transactionCommitted(const QList<figkey::PacketInfo>& packetList);

protected:
    void timerEvent(QTimerEvent *event);

private:

    // 创建数据库表格
    bool createTableIfNotExists();

    int timerId{ 0 };  // 添加一个成员变量来保存定时器ID
    QSqlDatabase db;
};

#endif // SQLITE_COMMON_H
