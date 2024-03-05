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
            if (configInfo.displayRows < 20 || configInfo.displayRows > 10000)
                configInfo.displayRows = 100;
            std::cout << "Protocol display rows : " << configInfo.displayRows << std::endl;
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

        auto type = config.find(CONFIG_CAPTURE_TYPE_NODE);
        if ((type != config.end()) && !type->second.empty())
        {
            std::cout << "Protocol filtering configuration information : " << type->second << std::endl;
        }

        auto save = config.find(CONFIG_CAPTURE_TYPE_NODE);
        if ((save != config.end()) && !save->second.empty())
        {
            configInfo.save = (save->second == "false")? false :true;
            std::cout << "Specifies whether to save packet configuration information : " << (configInfo.save?"true":"false") << std::endl;
        }

        auto async = config.find(CONFIG_RUN_ASYNC_NODE);
        if ((async != config.end()) && !async->second.empty())
        {
            configInfo.async = (async->second == "true") ? true : false;
            std::cout << "Whether to run configuration information asynchronously : " << (configInfo.async ? "true" : "false") << std::endl;
        }

        auto filterProtocol = config.find(CONFIG_FILTER_PROTOCOL_NODE);
        if ((filterProtocol != config.end()) && !filterProtocol->second.empty())
        {
            configInfo.filter.protocolType = std::stoi(filterProtocol->second);
            if (configInfo.filter.protocolType < PROTOCOL_TYPE_DEFAULT || configInfo.filter.protocolType > PROTOCOL_TYPE_UDS)
                configInfo.filter.protocolType = PROTOCOL_TYPE_DEFAULT;
            std::cout << "Protocol filtering configuration information : " << static_cast<int>(configInfo.filter.protocolType) << std::endl;
        }

        auto mac = config.find(CONFIG_FILTER_MAC_NODE);
        if ((mac != config.end()) && !mac->second.empty())
        {
            filterMac = mac->second;
            std::cout << "MAC filtering configuration information : " << filterMac << std::endl;
        }

        auto ip = config.find(CONFIG_FILTER_IP_NODE);
        if ((ip != config.end()) && !ip->second.empty())
        {
            filterIp = ip->second;
            std::cout << "IP filtering configuration information : " << filterIp << std::endl;
        }

        auto port = config.find(CONFIG_FILTER_PORT_NODE);
        if ((port != config.end()) && !port->second.empty())
        {
            filterPort = port->second;
            std::cout << "Port number filtering configuration information : " << filterPort << std::endl;
        }

        if (!filterMac.empty() || !filterIp.empty() || !filterPort.empty())
            enableFilter = true;
        else
            enableFilter = false;

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

    // 检查地址过滤
    bool CaptureConfig::checkFilterAddress(const std::string& address)
    {
        if (!enableFilter)
            return false;

        if (!filterMac.empty() && (std::string::npos == address.find(filterMac))) {
            std::cout << "filter by mac: " << filterMac << std::endl;
            return true;
        }

        if (!filterIp.empty() && (std::string::npos == address.find(filterIp))) {
            std::cout << "filter by ip: " << filterIp << std::endl;
            return true;
        }

        if (!filterPort.empty() && ((std::string::npos == address.find(filterPort+"->")) ||
            (std::string::npos == address.find("->"+filterPort)))) {
            std::cout << "filter by port: " << filterPort << std::endl;
            return true;
        }

        return false;
    }
}
