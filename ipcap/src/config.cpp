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

    // 获取协议过滤
    CaptureConfigInfo CaptureConfig::getFilterProtocol(const std::string& path) {
        CaptureConfigInfo info;
        std::map<std::string, std::string> config;
        if (!LoadConfigFile(path, config))
            return info;

        auto type = config.find(CONFIG_CAPTURE_TYPE_NODE);
        if ((type != config.end()) && !type->second.empty())
        {
            if (type->second == "DOIP")
                info.type = ProtocolType::DOIP;
            else if (type->second == "UDS")
                info.type = ProtocolType::UDS;
            else
                info.type = ProtocolType::DEFAULT;
            std::cout << "Protocol filtering configuration information : " << type->second << std::endl;
        }

        auto save = config.find(CONFIG_CAPTURE_TYPE_NODE);
        if ((save != config.end()) && !save->second.empty())
        {
            info.save = (save->second == "false")? false :true;
            std::cout << "Specifies whether to save packet configuration information : " << (info.save?"true":"false") << std::endl;
        }

        auto async = config.find(CONFIG_RUN_ASYNC_NODE);
        if ((async != config.end()) && !async->second.empty())
        {
            info.async = (async->second == "true") ? true : false;
            std::cout << "Whether to run configuration information asynchronously : " << (info.async ? "true" : "false") << std::endl;
        }
        
        auto filter = config.find(CONFIG_FILTER_PROTOCOL_NODE);
        if ((filter != config.end()) && !filter->second.empty())
        {
            info.filter = filter->second;
            std::cout << "Protocol filtering configuration information : " << info.filter << std::endl;
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
        return info;
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
