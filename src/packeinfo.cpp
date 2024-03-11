//PacketInfoModel.cpp
#include "packeinfo.h"
#include "config.h"
#include <QDebug>

PacketInfoModel::PacketInfoModel(QObject *parent)
    : QAbstractTableModel(parent), m_rows(figkey::CaptureConfig::Instance().getConfigInfo().displayRows)
{
}

PacketInfoModel::~PacketInfoModel() {
    clearPacket();
}

int PacketInfoModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    //QMutexLocker locker(&m_mutex);  // 使用 QMutexLocker，它会在析构函数中自动解锁
    return m_data.size();
}

int PacketInfoModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 7;
}

QString PacketInfoModel::getProtocolName(uint8_t protocolType) const
{
    switch (protocolType) {
    case figkey::PROTOCOL_TYPE_TCP:
        return "TCP";
    case figkey::PROTOCOL_TYPE_UDP:
        return "UDP";
    case figkey::PROTOCOL_TYPE_DOIP:
        return "DOIP";
    case figkey::PROTOCOL_TYPE_UDS:
        return "UDS";
    default:
        break;
    }

    return "UNKNOWN";
}

QVariant PacketInfoModel::data(const QModelIndex &index, int role) const
{
    QMutexLocker locker(&m_mutex);  // 需要对 m_data 的访问进行加锁

    if (role != Qt::DisplayRole || !index.isValid())
        return QVariant();

    if (index.row() >= m_data.size()) {
         qDebug()<<"row: "<<index.row()<<" PacketInfoModel out_of_range data size: "<<m_data.size();
         return QVariant();
    }

    const auto& packet = m_data.at(index.row());
    switch (index.column()) {
        case 0: return QVariant::fromValue<uint64_t>(packet.index);
        case 1: return QString::fromStdString(packet.timestamp);
        case 2: return QString::fromStdString(packet.srcIP);
        case 3: return QString::fromStdString(packet.destIP);
        case 4: return getProtocolName(packet.protocolType);
        case 5: return packet.payloadLength;
        case 6: return QString::fromStdString(packet.data);
        default: break;
    }

    return QVariant();
}

QVariant PacketInfoModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        switch (section) {
        case 0: return tr("Index");
        case 1: return tr("Timestamp");
        case 2: return tr("Source IP");
        case 3: return tr("Destination IP");
        case 4: return tr("Protocol");
        case 5: return tr("Length");
        case 6: return tr("Information");
        default: return QVariant();
        }
    }
    return QVariant();
}

void PacketInfoModel::loadPackect(const std::vector<figkey::PacketInfo>& packets) {
    clearPacket();

    QMutexLocker locker(&m_mutex);
    if (!packets.empty()) {
        // 添加新数据包
        beginInsertRows(QModelIndex(), 0, packets.size() - 1);
        for (auto&& packet : packets) {
            m_data.append(std::move(packet));
        }
        endInsertRows();
    }
}

void PacketInfoModel::addPacket(const figkey::PacketInfo &packet)
{
    QMutexLocker locker(&m_mutex);

    if (m_data.size() >= m_rows)
    {
        beginRemoveRows(QModelIndex(), 0, 0);
        m_data.removeFirst();
        endRemoveRows();
    }

    beginInsertRows(QModelIndex(), m_data.size(), m_data.size());
    m_data.append(packet);
    endInsertRows();
}

void PacketInfoModel::clearPacket() {
    QMutexLocker locker(&m_mutex);

    // 移除旧的数据
    if (!m_data.isEmpty()) {
        beginRemoveRows(QModelIndex(), 0, m_data.size() - 1);
        m_data.clear();
        endRemoveRows();
    }
}

figkey::PacketInfo PacketInfoModel::getPacketByIndex(int index) {
    QMutexLocker locker(&m_mutex); // 加锁
    if (m_data.isEmpty())
        return figkey::PacketInfo(); // 或者返回一个默认的PacketInfo

    if (index < 0 )
        return m_data.first();

    if (index >= m_data.size())
        return m_data.last();

    return m_data[index];
}
