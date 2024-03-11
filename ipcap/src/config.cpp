// ipcap.cpp: 定义应用程序的入口点。
//

#include <map>
#include <iostream>
#include <fstream>
#include "config.h"
#include "def.h"

namespace figkey {
    static bool LoadConfigFile(const std::string& path, std::map<std::string, std::string>& filter) {
        std::string line;
        std::ifstream file(path);

        if (!file.is_open()) {
            std::cerr << "Open config file failed: " << path << std::endl;
            return false;
        }

        std::map<std::string, std::map<std::string, std::string>> config;
        std::string section = "";
        while (std::getline(file, line)) {
            // 过滤掉注释和空行
            if (line.empty() || line.substr(0, 1) == "#" || line.substr(0, 1) == ";") {
                continue;
            }

            // 判断是否是节名
            if (line.substr(0, 1) == "[") {
                section = line.substr(1, line.find_last_of("]") - 1);
            }
            else {
                // 解析键值对
                size_t pos = line.find("=");
                std::string key = line.substr(0, pos);
                std::string value = line.substr(pos + 1);
                config[section][key] = value;
            }
        }
        file.close();

        auto ip_config = config.find(CONFIG_ROOT_NODE_NAME);
        if (ip_config == config.end())
        {
            std::cerr << "ip capture config root node name "<< CONFIG_ROOT_NODE_NAME<<" not found" << std::endl;
            return false;
        }

        filter.swap(ip_config->second);
        return true;
    }

    bool CaptureConfig::loadConfigFile(const std::string& path){
        std::map<std::string, std::string> config;
        if (!LoadConfigFile(path, config))
            return false;

        auto captureFilter = config.find(CONFIG_CAPTURE_PROTOCOL_NODE);
        if ((captureFilter != config.end()) && !captureFilter->second.empty())
        {
            // TODO fixed for this tool
            //configInfo.captureFilter = captureFilter->second;
            std::cout << "Capture protocol : " << configInfo.captureFilter << std::endl;
        }

        auto rows = config.find(CONFIG_DISPLAY_ROWS);
        if ((rows != config.end()) && !rows->second.empty())
        {
            configInfo.displayRows = std::stoi(rows->second);
            if (configInfo.displayRows < 20 || configInfo.displayRows > 100000)
                configInfo.displayRows = 100;
            std::cout << "Capture display rows : " << configInfo.displayRows << std::endl;
        }

        auto doipClientSend = config.find(CONFIG_DOIP_CLIENT_SEND);
        if ((doipClientSend != config.end()) && !doipClientSend->second.empty())
        {
            configInfo.doipClientSend = std::stoi(doipClientSend->second);
            if (configInfo.doipClientSend < 3 || configInfo.doipClientSend > 10)
                configInfo.doipClientSend = 5;
            std::cout << "doip client ui send table row : " << configInfo.doipClientSend << std::endl;
        }

        auto doipClientReceive = config.find(CONFIG_DOIP_CLIENT_RECEIVE);
        if ((doipClientReceive != config.end()) && !doipClientReceive->second.empty())
        {
            configInfo.doipClientReceive = std::stoi(doipClientReceive->second);
            if (configInfo.doipClientReceive < 3 || configInfo.doipClientReceive > 100)
                configInfo.doipClientReceive = 5;
            std::cout << "doip client ui receive table row : " << configInfo.doipClientReceive << std::endl;
        }

        auto updateUI = config.find(CONFIG_TIME_UPDATE_UI);
        if ((updateUI != config.end()) && !updateUI->second.empty())
        {
            configInfo.timeUpdateUI = std::stoi(updateUI->second);
            if (configInfo.timeUpdateUI < 200 || configInfo.timeUpdateUI > 60000)
                configInfo.timeUpdateUI = 1000;
            std::cout << "Protocol time update UI : " << configInfo.timeUpdateUI << std::endl;
        }

        auto sqlTransaction = config.find(CONFIG_TIME_SQL_TRANSACTION);
        if ((sqlTransaction != config.end()) && !sqlTransaction->second.empty())
        {
            configInfo.timeSqlTransaction = std::stoi(sqlTransaction->second);
            if (configInfo.timeSqlTransaction < 5 || configInfo.timeSqlTransaction > 30000)
                configInfo.timeSqlTransaction = 100;
            std::cout << "store packet information time : " << configInfo.timeSqlTransaction << std::endl;
        }

        auto filterProtocol = config.find(CONFIG_FILTER_PROTOCOL_NODE);
        if ((filterProtocol != config.end()) && !filterProtocol->second.empty())
        {
            configInfo.filter.protocolType = std::stoi(filterProtocol->second);
            if (configInfo.filter.protocolType > PROTOCOL_TYPE_UDS)
                configInfo.filter.protocolType = PROTOCOL_TYPE_DEFAULT;
            std::cout << "Protocol filtering configuration information : " << static_cast<int>(configInfo.filter.protocolType) << std::endl;
        }

        return true;
    }

    const CaptureConfigInfo& CaptureConfig::getConfigInfo() const {
        return configInfo;
    }

    void  CaptureConfig::setNetwork(const NetworkInfo& network) {
        configInfo.network = network;
    }

    void CaptureConfig::setFilter(const FilterInfo& filter) {
        configInfo.filter = filter;
    }

}
