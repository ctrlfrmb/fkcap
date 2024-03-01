// ipcap.cpp: 定义应用程序的入口点。
//
#include "ipcap.h"
#include "common/thread_pool.hpp"
#include "protocol/ip.h"
#include "config.h"
#include <ws2tcpip.h>
#include <iostream>
#pragma comment(lib, "ws2_32.lib")

namespace figkey {

    NpcapCom::NpcapCom() : handle(NULL) {
        init();
    }

    NpcapCom::~NpcapCom() {
        stopCapture();

        opensource::ctrlfrmb::ThreadPool::Instance().shutdown();
    }

    void NpcapCom::init()
    {
        // Initialize any required fields
        auto pcap_version = pcap_lib_version();
        std::cout << "pcap_version: " << pcap_version << std::endl;
        // Npcap 输出：“Npcap 版本 0.92，基于 libpcap 版本 1.8.1”
        // WinPcap 输出：“WinPcap 版本 4.1.3

        // 获取线程池的实例
        opensource::ctrlfrmb::ThreadPool& pool = opensource::ctrlfrmb::ThreadPool::Instance();
        // 设置线程池参数
        pool.set(8, 2, 600); // 设置最大线程数为8，最小线程数为2，线程超时时间为600秒
    }

    void NpcapCom::pcapHandler(u_char* user, const struct pcap_pkthdr* pkthdr, const u_char* packet) {
        NpcapCom* processor = reinterpret_cast<NpcapCom*>(user);
        if (nullptr == processor) {
            std::cerr << "Fatal error: NpcapCom is null" << std::endl;
            return;
        }

        if (!processor->isRunning)
            return;

        if (pkthdr->caplen > CAPTURE_SNAP_LENGTH)  // 确保caplen小于等于捕获长度
        {
            std::cerr << "Fatal error: Capture length exceeds limit, capture length is "<< pkthdr->caplen << std::endl;
            return;
        }

        IPPacketParse::Instance().parse(pkthdr, packet);
    }

    std::vector<NetworkInfo> NpcapCom::getNetworkList() {
        pcap_if_t* alldevs;
        pcap_if_t* device;
        char errbuf[PCAP_ERRBUF_SIZE];
        std::vector<NetworkInfo> devices;

        if (pcap_findalldevs(&alldevs, errbuf) == -1) {
            std::cerr << "Error in pcap_findalldevs: " << errbuf << std::endl;
            return devices;
        }

        for (device = alldevs; device != NULL; device = device->next) {
            NetworkInfo info;
            info.name = device->name;
            info.description = device->description ? device->description : "No description available";
            /* IP addresses */
            IpAddressInfo addr;
            for (auto addresses = device->addresses; addresses; addresses = addresses->next) {
                switch (addresses->addr->sa_family)
                {
                case AF_INET:
                    if (addresses->addr) {
                        char ipv4[INET_ADDRSTRLEN];
                        inet_ntop(AF_INET, &(((struct sockaddr_in*)addresses->addr)->sin_addr), ipv4, INET_ADDRSTRLEN);
                        addr.ip = ipv4;
                    }
                    if (addresses->netmask) {
                        char netmask[INET_ADDRSTRLEN];
                        auto mask = ntohl(((struct sockaddr_in*)addresses->netmask)->sin_addr.S_un.S_addr);
                        inet_ntop(AF_INET, &mask, netmask, INET_ADDRSTRLEN);
                        addr.netmask = netmask;
                    }
                    break;

                case AF_INET6:
                    if (addresses->addr) {
                        char ipv6[INET6_ADDRSTRLEN];
                        socklen_t sockaddrlen;
#ifdef WIN32
                        sockaddrlen = sizeof(struct sockaddr_in6);
#else
                        sockaddrlen = sizeof(struct sockaddr_storage);
#endif
                        if (getnameinfo(addresses->addr,
                            sockaddrlen,
                            ipv6,
                            INET6_ADDRSTRLEN,
                            NULL,
                            0,
                            NI_NUMERICHOST) == 0) {
                            addr.ip = ipv6;
                        }
                    }
                    break;

                default:
                    std::cerr << "Address Family Name: Unknown" << std::endl;
                    break;
                }
                if (!addr.ip.empty())
                    info.address.emplace_back(addr);
            }
            devices.emplace_back(info);
        }

        pcap_freealldevs(alldevs);
        return devices;
    }

    void NpcapCom::setIsRunning(bool flag) {
        isRunning = flag;
    }

    bool NpcapCom::getIsRunning() const {
        return isRunning;
    }

    bool NpcapCom::pcapOpen() {
        if (handle != NULL)
            return true;

        const std::string& networkName = figkey::CaptureConfig::Instance().getConfigInfo().network.name;
        char errbuf[PCAP_ERRBUF_SIZE];
#if 0
        handle = pcap_open_live(networkName.c_str(), 65536, PCAP_OPENFLAG_PROMISCUOUS, 1000, errbuf);
        if (handle == NULL) {
            std::cerr << "Couldn't open device " << networkName << ": " << errbuf << std::endl;
            return false;
        }
#else
        // open a capture from the network
        handle = pcap_create(networkName.c_str(), errbuf);
        if (handle == NULL) {
            fprintf(stderr, "pcap_create error: %s\n", errbuf);
            return false;
        }

        auto res = pcap_set_snaplen(handle, CAPTURE_SNAP_LENGTH);
        if (res < 0) {
            fprintf(stderr, "pcap_set_snaplen error: %s\n", pcap_statustostr(res));
            return false;
        }
        res = pcap_set_promisc(handle, CAPTURE_PROMISC); //设置优先级 1，过滤时需设置优先级为 1 保持一致
        if (res < 0) {
            fprintf(stderr, "pcap_set_promisc error: %s\n", pcap_statustostr(res));
            return false;
        }
        res = pcap_set_timeout(handle, 1000);
        if (res < 0) {
            fprintf(stderr, "pcap_set_timeout error: %s\n", pcap_statustostr(res));
            return false;
        }
        res = pcap_activate(handle);
        if (res < 0) {
            fprintf(stderr, "pcap_activate error: %s\n", pcap_statustostr(res));
            return false;
        }
#endif
        switch (pcap_datalink(handle))
        {
        case DLT_NULL:
            std::cout << "device type DLT_NULL" << std::endl;
            break;
        case DLT_EN10MB:
            /* Already set up */
            std::cout << "device type DLT_EN10MB" << std::endl;
            break;
        default:
            std::cerr << "Couldn't open device " << pcap_datalink(handle) << std::endl;
            break;
        }

        return true;
    }

    bool NpcapCom::pcapFilter(uint32_t netmask)
    {
        // 设置过滤器（例如只捕获TCP数据包）
        struct bpf_program filter;
        if (pcap_compile(handle, &filter, figkey::CaptureConfig::Instance().getConfigInfo().captureFilter.c_str(), CAPTURE_PROMISC, netmask) < 0) {
            std::cerr << "Bad filter - " << pcap_geterr(handle) << std::endl;
            return false;
        }
        if (pcap_setfilter(handle, &filter) < 0) {
            std::cerr << "Error setting filter - " << pcap_geterr(handle) << std::endl;
            return false;
        }

        return true;
    }

    void NpcapCom::startCapture() {
        if (!pcapOpen())
            return;

        if (!pcapFilter())
            return;

        isRunning = true;
#if 0
        static std::function<bool(const struct pcap_pkthdr*, const u_char*)> ipParse = std::bind(&IPPacketParse::parse, &IPPacketParse::Instance(),
            std::placeholders::_1, std::placeholders::_2);

        //start the capture
        int res;
        struct pcap_pkthdr* pkthdr;
        const u_char* pkt_data;
        while ((res = pcap_next_ex(handle, &pkthdr, &pkt_data)) >= 0)
        {
            if (res == 0)
                /* Timeout elapsed */
                continue;

            //save the packet on the dump file
            //pcap_dump((unsigned char*)dumpfile, header, pkt_data);

            if ((nullptr != pkthdr) && (0 == pkthdr->caplen)) {
                std::cerr << "Fatal error: capture length 0 , packet length " << pkthdr->len << std::endl;
                continue;
            }

            // 获取线程池的实例
            opensource::ctrlfrmb::ThreadPool& pool = opensource::ctrlfrmb::ThreadPool::Instance();
            pool.submit(ipParse, pkthdr, pkt_data);

        }

        if (res == -1) {
            printf("Error reading the packets: %s\n", pcap_geterr(handle));
        }
#else
        // pcap_loop 可以一次捕获多包数据，在数据处理遇到瓶颈时 性能更佳
        pcap_loop(handle, 0, &NpcapCom::pcapHandler, reinterpret_cast<u_char*>(this));
#endif
    }

    void NpcapCom::asyncStartCapture()
    {
        std::function<void()> fun_async = std::bind(&NpcapCom::startCapture, this);
        opensource::ctrlfrmb::ThreadPool::Instance().submit(fun_async);
    }

    void NpcapCom::stopCapture()
    {
        if (handle) {
            pcap_close(handle);
            handle = NULL;
        }
    }
}
