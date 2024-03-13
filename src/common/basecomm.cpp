/**
 * @file    basecomm.h
 * @ingroup opensource::ctrlfrmb
 * @brief
 * @author  leiwei
 * @date    2024.01.18
 * Copyright (c) opensource::ctrlfrmb 2024-2034
 */
#include "common/basecomm.h"

BaseComm::BaseComm(const QString& clientIp, const QString& serverIp,
             int serverPort, bool isServer, QObject *parent)
    : QObject(parent),
      m_clientIp(clientIp),
      m_serverIp(serverIp),
      m_serverPort(serverPort),
      m_isServer(isServer)
{
}

QString BaseComm::getLastError() const {
    return m_lastError;
}
