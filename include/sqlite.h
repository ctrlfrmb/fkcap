#ifndef SQLITE_COMMON_H
#define SQLITE_COMMON_H

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QFile>
#include "def.h"

#define FKCAP_SQLITE_CONNECT_NAME "figkey_connection"

class SqliteCom : public QObject
{
    Q_OBJECT

public:
    explicit SqliteCom(QObject *parent = nullptr);
    ~SqliteCom();

    bool loadFile();
    bool openFile();

    // 存储抓包数据到数据库
    bool storePacket(const figkey::PacketInfo &packet);

    std::vector<figkey::PacketInfo> getPacket(int start, int rows);

    void writeFile();

    void saveFile();

    void closeFile();

    void checkFile();

private:

    void moveFile();

    // 创建数据库表格
    bool createTableIfNotExists();

    void closeDataBase();

    QFile dbFile;

    QSqlDatabase db;
    QSqlQuery query;
};

#endif // SQLITE_COMMON_H
