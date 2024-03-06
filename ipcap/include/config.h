/**
 * @file    config.h
 * @ingroup figkey
 * @brief   
 * @author  leiwei
 * @date    2023.12.21
 * Copyright (c) figkey 2023-2033
 */

#pragma once

#ifndef FIGKEY_PCAP_CONFIG_HPP
#define FIGKEY_PCAP_CONFIG_HPP

#include <string>
#include "def.h"

namespace figkey {

    // 抓包配置类
    class CaptureConfig {
    private:
        CaptureConfigInfo configInfo;

        // Capture config constructor
        CaptureConfig()=default;

        // Capture config destructor
        ~CaptureConfig()=default;

    public:
        // Delete copy constructor and assignment operator to ensure uniqueness of the capture config
        CaptureConfig(const CaptureConfig&) = delete;
        CaptureConfig(CaptureConfig&&) = delete;
        CaptureConfig& operator=(const CaptureConfig&) = delete;
        CaptureConfig& operator=(CaptureConfig&&) = delete;

        // Retrieve an instance of the capture config(singleton pattern)
        static CaptureConfig& Instance() {
            static CaptureConfig obj;
            return obj;
        }

        bool loadConfigFile(const std::string& path);

        const CaptureConfigInfo& getConfigInfo() const ;

        void setNetwork(const NetworkInfo& network);

        void setFilter(const FilterInfo& filter);
    };

}  // namespace figkey

#endif // !FIGKEY_PCAP_CONFIG_HPP
