#include <QDebug>
#include <QtSql/QSqlError>
#include <QFileInfo>
#include <QDir>
#include <QDateTime>
#include <QStandardPaths>
#include <QMessageBox>
#include <QCoreApplication>
#include <QFileDialog>

#include "sqlite.h"
#include "config.h"

#define FKCAP_SQLITE_DATABASE_PATH "/db/figkey.db"

SqliteCom::SqliteCom(QObject *parent)
    : QObject(parent),
      dbFile(QCoreApplication::applicationDirPath()+FKCAP_SQLITE_DATABASE_PATH),
      db(QSqlDatabase::addDatabase("QSQLITE", FKCAP_SQLITE_CONNECT_NAME)),
      query(QSqlQuery(db))
{
}

SqliteCom::~SqliteCom()
{
    closeFile();
    if (dbFile.exists()) {
        dbFile.remove();
    }

}

void SqliteCom::saveFile() {
    if(!dbFile.exists()) {
        QMessageBox::critical(nullptr, "Error",
                              QString("Unable to save, the current system temporary database file is abnormal.")
                              );
        return;
    }

    // 保存数据库到用户指定位置
    QString destFileName = QFileDialog::getSaveFileName(
        nullptr,
        "Save Capture File",
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
        "Sqlite DataBase (*.db);;Text files (*.txt)");

    if (!destFileName.isEmpty()) {
        if (!dbFile.copy(destFileName)) {
            QMessageBox::critical(nullptr, "Error",
                                  QString("Failed to save the file %1 ")
                                  .arg(destFileName));
            return;
        }
    }
}

void SqliteCom::moveFile() {
    if (!dbFile.exists())
        return;

    // 获取现在的时间并格式化为你想要的形式
    QString now = QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz");

    // 构建新的文件名和路径
    QString tempPath = QDir(QStandardPaths::writableLocation(QStandardPaths::TempLocation))
        .filePath(QFileInfo(dbFile).fileName() + "_" + now);

    // 重命名并移动文件
    auto fileName = dbFile.fileName();
    if (!dbFile.rename(tempPath)) {
        QMessageBox::warning(nullptr, "Warning",
                             QString("Failed to move the file %1 to the temp directory: %2")
                             .arg(fileName).arg(tempPath));
        return;
    }

    QMessageBox::information(nullptr, "Information",
                             QString("Old data has been moved to: %1").arg(tempPath));

    dbFile.setFileName(fileName);
}

void SqliteCom::closeDataBase() {
    if (db.isOpen()) {
        db.commit();
        db.close();
    }
}

void SqliteCom::closeFile()
{
    closeDataBase();
}

void SqliteCom::checkFile() {
    if (!dbFile.exists())
        return;

    QMessageBox box;
    box.setWindowTitle("Warning");
    box.setText("Whether the old data captured is saved?");
    box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    box.setDefaultButton(QMessageBox::No);

    if (box.exec() == QMessageBox::Yes) {
        saveFile();
    } else {
        // 用户选择"No"，停止数据捕获并删除临时数据库
        dbFile.remove();
    }
}

bool SqliteCom::loadFile() {
    checkFile();

    QString fileName = QFileDialog::getOpenFileName(
       nullptr,
       "Open Capture File",
       QCoreApplication::applicationDirPath(),
       "Sqlite DataBase (*.db);;Text files (*.txt)");

    if (fileName.isEmpty()) {
       // 用户取消打开文件操作
       return false;
    }

    closeFile();

    db.setDatabaseName(fileName);

    if (!db.open()) {
        QMessageBox::critical(nullptr, "Error",
                              QString("Error opening database: %1, Error: %2")
                              .arg(fileName).arg(db.lastError().text()));
        return false;
    }

    return true;
}

bool SqliteCom::openFile()
{
    if (db.isOpen())
        return true;

    // 提取数据库文件的目录
    QString dirPath = QFileInfo(dbFile).absolutePath();

    // 创建数据库目录
    QDir dir;
    if (!dir.mkpath(dirPath)) {
        QMessageBox::warning(nullptr, "Warning",
                             QString("Failed to create directory: %1").arg(dirPath));
        return false;
    }

    moveFile();

    db.setDatabaseName(dbFile.fileName());

    if (db.open()) {
        if (!createTableIfNotExists())
            return false;
    } else {
        QMessageBox::critical(nullptr, "Error",
                              QString("Error opening database: %1, Error: %2")
                              .arg(dbFile.fileName()).arg(db.lastError().text()));
        return false;
    }

    return true;
}

bool SqliteCom::storePacket(const figkey::PacketInfo &packet)
{
    if (!db.isOpen())
        return false;

    if (1 == packet.index) {
        db.transaction();
    }

    query.prepare("INSERT INTO Packets (id, timestamp, error, srcIP, destIP, "
                  "srcMAC, destMAC, srcPort, destPort, protocol,"
                  "length, data) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
    query.addBindValue(QVariant::fromValue(packet.index));
    query.addBindValue(QString::fromStdString(packet.timestamp));
    query.addBindValue(QVariant::fromValue(static_cast<int>(packet.err)));
    query.addBindValue(QString::fromStdString(packet.srcIP));
    query.addBindValue(QString::fromStdString(packet.destIP));
    query.addBindValue(QString::fromStdString(packet.srcMAC));
    query.addBindValue(QString::fromStdString(packet.destMAC));
    query.addBindValue(QVariant::fromValue(packet.srcPort));
    query.addBindValue(QVariant::fromValue(packet.destPort));
    query.addBindValue(QVariant::fromValue(static_cast<int>(packet.protocolType)));
    query.addBindValue(QVariant::fromValue(packet.payloadLength));
    query.addBindValue(QString::fromStdString(packet.data));

    if (!query.exec()) {
        db.rollback();  // 如果数据插入失败，则回滚事务
        qDebug() << "Error inserting into the table: " << query.lastError();
        return false;
    }

    return true;  // 仅插入数据，但不提交事务
}

std::vector<figkey::PacketInfo> SqliteCom::getPacket(int start, int rows) {
    std::vector<figkey::PacketInfo> results;

    if (!db.isOpen())
        return results;

    int offset = start - 1;  // 调整为正确的偏移量
    if (offset < 0)
        offset = 0;  // 确保偏移量不为负

    query.prepare("SELECT * FROM Packets LIMIT ? OFFSET ?");
    query.addBindValue(rows);
    query.addBindValue(offset);

    if (!query.exec()) {
        qDebug() << "Failed to execute SQL query: " << query.lastError();
        return results;
    }

    while (query.next()) {
      figkey::PacketInfo packet;
      packet.index = query.value("id").toUInt();
      packet.timestamp = query.value("timestamp").toString().toStdString();
      packet.err = query.value("error").toUInt();
      packet.srcIP = query.value("srcIP").toString().toStdString();
      packet.destIP = query.value("destIP").toString().toStdString();
      packet.srcMAC = query.value("srcMAC").toString().toStdString();
      packet.destMAC = query.value("destMAC").toString().toStdString();
      packet.srcPort = query.value("srcPort").toUInt();
      packet.destPort = query.value("destPort").toUInt();
      packet.protocolType = query.value("protocol").toUInt();
      packet.payloadLength = query.value("length").toUInt();
      packet.data = query.value("data").toString().toStdString();
      results.push_back(packet);
    }

    return results;
}

std::vector<figkey::PacketInfo> SqliteCom::getPacketByFilter(int start, int rows) {
    std::vector<figkey::PacketInfo> results;
    auto filter = figkey::CaptureConfig::Instance().getConfigInfo().filter;

    if (!db.isOpen())
        return results;

    int offset = start - 1;  // 调整为正确的偏移量
    if (offset < 0)
        offset = 0;  // 确保偏移量不为负

    QString protocolType = QString::number(filter.protocolType);
    QString ip = QString::fromStdString(filter.ip);
    QString srcIP = QString::fromStdString(filter.srcIP);
    QString destIP = QString::fromStdString(filter.destIP);
    QString portStr = QString::number(filter.port);
    QString srcPortStr = QString::number(filter.srcPort);
    QString destPortStr = QString::number(filter.destPort);
    QString srcMAC = QString::fromStdString(filter.srcMAC);
    QString destMAC = QString::fromStdString(filter.destMAC);
    QString minLen = QString::number(filter.minLen);
    QString maxLen = QString::number(filter.maxLen);
    QString rowsStr = QString::number(rows);
    QString offsetStr = QString::number(offset);

    QString queryString;
    if(filter.protocolType == figkey::PROTOCOL_TYPE_DEFAULT){
        queryString += QString("SELECT * FROM Packets WHERE (1 = 1) ");
    }else if(filter.protocolType == figkey::PROTOCOL_TYPE_DOIP){
        queryString += QString("SELECT * FROM Packets WHERE (`protocol` >= %1) ").arg(protocolType);
    }else{
        queryString += QString("SELECT * FROM Packets WHERE (`protocol` = %1) ").arg(protocolType);
    }

    if(filter.ip.empty()){
        queryString += QString("AND ('' = '%1' OR `srcIP` = '%1') AND ('' = '%2' OR `destIP` = '%2') ").arg(srcIP,destIP);
    }else{
        queryString += QString("AND (`srcIP` = '%1' OR `destIP` = '%1') ").arg(ip);
    }

    if(filter.port == 0){
        queryString += QString("AND (0 = %1 OR `srcPort` = %1) AND (0 = %2 OR `destPort` = %2) ").arg(srcPortStr,destPortStr);
    }else{
        queryString += QString("AND (`srcPort` = %1 OR `destPort` = %1) ").arg(portStr);
    }

    if(!filter.srcMAC.empty()){
        queryString += QString("AND `srcMAC` = '%1' ").arg(srcMAC);
    }

    if(!filter.destMAC.empty()){
        queryString += QString("AND `destMAC` = '%1' ").arg(destMAC);
    }

    if(filter.minLen != 0 && filter.maxLen != 0){
        queryString += QString("AND (length >= %1 AND length <= %2) ").arg(minLen, maxLen);
    }else if(filter.minLen != 0){
        queryString += QString("AND length >= %1 ").arg(minLen);
    }else if(filter.maxLen != 0){
        queryString += QString("AND length <= %1 ").arg(maxLen);
    }

    queryString += QString("LIMIT %1 OFFSET %2").arg(rowsStr).arg(offsetStr);

    query.prepare(queryString);
    if (!query.exec()) {
         qDebug() << "Error: " << query.lastError();
    }

    qDebug() << queryString;

    query.prepare(queryString);
    if (!query.exec()) {
        qDebug() << "Failed to execute SQL query: " << query.lastError();
        return results;
    }

    while (query.next()) {
        figkey::PacketInfo packet;
        packet.index = query.value("id").toUInt();
        // 我们获取字符串值的方式用了一种稍微简短一些的写法
        packet.timestamp = query.value("timestamp").toString().toUtf8().constData();
        packet.err = query.value("error").toUInt();
        packet.srcIP = query.value("srcIP").toString().toUtf8().constData();
        packet.destIP = query.value("destIP").toString().toUtf8().constData();
        packet.srcMAC = query.value("srcMAC").toString().toUtf8().constData();
        packet.destMAC = query.value("destMAC").toString().toUtf8().constData();
        packet.srcPort = query.value("srcPort").toUInt();
        packet.destPort = query.value("destPort").toUInt();
        packet.protocolType = query.value("protocol").toUInt();
        packet.payloadLength = query.value("length").toUInt();
        packet.data = query.value("data").toString().toUtf8().constData();
        results.push_back(packet);
    }

    return results;
}

void SqliteCom::writeFile() {
    if (!db.isOpen())
        return;

    // 提交事务
    if (db.commit()) {
        db.transaction();  // 开始下一个新的事务
    }
    else
         qDebug() << "Error commit: " << db.lastError();
}

bool SqliteCom::createTableIfNotExists()
{
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

