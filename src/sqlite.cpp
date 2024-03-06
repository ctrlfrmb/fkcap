#include "sqlite.h"
#include "config.h"
#include <QtSql/QSqlQuery>
#include <QDebug>
#include <QtSql/QSqlError>
#include <QFileInfo>
#include <QDir>

SqliteCom::SqliteCom(QObject *parent)
    : QObject(parent)
{
}

SqliteCom::~SqliteCom()
{
    closeFile();
}

void SqliteCom::closeFile()
{
    // 停止定时器
    if (0 != timerId) {
        this->killTimer(timerId);
        timerId = 0;
    }

    if (db.isOpen()) {
        if (db.transaction()) {
            db.commit();
        }
        db.close();
    }
}

bool SqliteCom::openFile(const QString& path)
{
    if (db.isOpen())
        return true;

    // 提取数据库文件的目录
    QFileInfo fileInfo(path);
    QString dirPath = fileInfo.absolutePath();

    // 创建数据库目录
    QDir dir;
    if (!dir.mkpath(dirPath)) {
        qDebug() << "Failed to create directory" << dirPath;
        return false;
    }

    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(path);

    if (db.open()) {
        if (!createTableIfNotExists())
            return false;
    } else {
        qDebug() << "Error opening database "<< path <<", "<< db.lastError();
        return false;
    }

    // 启动定时器，保存定时器ID
    timerId = this->startTimer(figkey::CaptureConfig::Instance().getConfigInfo().timeSqlTransaction);

    return true;
}

bool SqliteCom::storePacket(const figkey::PacketInfo &packet)
{
    if (!db.isOpen())
        return false;

    if (!db.transaction()) {
        db.transaction();  // 如果没有正在进行的事务，则开始一个新的事务
    }

    QSqlQuery query(db);
    query.prepare("INSERT INTO Packets (id, timestamp, error, srcIP, destIP, "
                  "srcMAC, destMAC, srcPort, destPort, protocol,"
                  "length, data) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
    query.addBindValue(QVariant::fromValue(packet.index));
    query.addBindValue(QString::fromStdString(packet.timestamp));
    query.addBindValue(QVariant::fromValue(packet.err));
    query.addBindValue(QString::fromStdString(packet.srcIP));
    query.addBindValue(QString::fromStdString(packet.destIP));
    query.addBindValue(QString::fromStdString(packet.srcMAC));
    query.addBindValue(QString::fromStdString(packet.destMAC));
    query.addBindValue(QVariant::fromValue(packet.srcPort));
    query.addBindValue(QVariant::fromValue(packet.destPort));
    query.addBindValue(QVariant::fromValue(packet.protocolType));
    query.addBindValue(QVariant::fromValue(packet.payloadLength));
    query.addBindValue(QString::fromStdString(packet.data));

    if (!query.exec()) {
        db.rollback();  // 如果数据插入失败，则回滚事务
        qDebug() << "Error inserting into the table: " << query.lastError();
        return false;
    }

    return true;  // 仅插入数据，但不提交事务
}

bool SqliteCom::createTableIfNotExists()
{
    QSqlQuery query(db);
    if (!query.exec("CREATE TABLE IF NOT EXISTS Packets "
                    "(id INTEGER PRIMARY KEY, timestamp TEXT, error INTEGER, "
                    "srcIP TEXT, destIP TEXT, srcMAC TEXT, destMAC TEXT, "
                    "srcPort INTEGER, destPort INTEGER, protocol INTEGER, "
                    "length INTEGER, data TEXT, remark TEXT DEFAULT '')")) {
        qDebug() << "Error creating table: " << query.lastError();
        return false;
    }

    return true;
}

void SqliteCom::timerEvent(QTimerEvent * /* event */)
{
    if (!db.isOpen())
       return;

    if (db.transaction()) {
        // 提交事务
        if (db.commit()) {
            db.transaction();  // 开始下一个新的事务
            //emit transactionCommitted(...); // 你需要替换...为实际的缓存数据
        } else {
            qDebug() << "Error committing transaction: " << db.lastError();
        }
    }
}
