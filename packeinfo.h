﻿// PacketInfoModel.h
#ifndef PACKET_INFO_MODEL_H
#define PACKET_INFO_MODEL_H

#include <QAbstractTableModel>
#include <QVector>
#include "def.h"

class PacketInfoModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit PacketInfoModel(QObject *parent = nullptr);

    void setDisplayRow(uint16_t rows);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void addPacket(figkey::PacketInfo&& packet);
    figkey::PacketInfo getFirstPacket();

private:
    QString getProtocolName(uint8_t protocolType) const;

    QVector<figkey::PacketInfo> m_data;
    mutable QMutex m_mutex;  // 增加一个互斥体成员变量
    uint64_t packetCounter{ 0 };
    uint16_t m_rows;
};

#endif // PACKET_INFO_MODEL_H