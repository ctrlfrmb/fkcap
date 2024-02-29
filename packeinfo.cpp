//PacketInfoModel.cpp
#include "packeinfo.h"

PacketInfoModel::PacketInfoModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

int PacketInfoModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_data.size();
}

int PacketInfoModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 12;
}

QVariant PacketInfoModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role == Qt::DisplayRole) {
        const auto& packet = m_data.at(index.row());
        switch (index.column()) {
        case 0: return QVariant::fromValue<uint64_t>(packet.index);
        case 1: return QString::fromStdString(packet.timestamp);
        case 2: return packet.err;
        case 3: return QString::fromStdString(packet.srcIP);
        case 4: return QString::fromStdString(packet.destIP);
        case 5: return QString::fromStdString(packet.srcMAC);
        case 6: return QString::fromStdString(packet.destMAC);
        case 7: return packet.srcPort;
        case 8: return packet.destPort;
        case 9: return packet.payloadLength;
        case 10: return packet.protocolType;
        case 11: return QString::fromStdString(packet.data);
        default: return QVariant();
        }
    }
    else
        return QVariant();
}

QVariant PacketInfoModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        switch (section) {
        case 0: return tr("Index");
        case 1: return tr("Timestamp");
        case 2: return tr("Error Code");
        case 3: return tr("Source IP");
        case 4: return tr("Destination IP");
        case 5: return tr("Source MAC");
        case 6: return tr("Destination MAC");
        case 7: return tr("Source Port");
        case 8: return tr("Destination Port");
        case 9: return tr("Payload Length");
        case 10: return tr("Protocol Type");
        case 11: return tr("Data");
        default: return QVariant();
        }
    }
    return QVariant();
}

void PacketInfoModel::addPacket(figkey::PacketInfo &&packet)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    m_data.append(packet);
    endInsertRows();

    if (m_data.size() > 100)
    {
        beginRemoveRows(QModelIndex(), 0, 0);
        m_data.removeFirst();
        endRemoveRows();
    }
}
