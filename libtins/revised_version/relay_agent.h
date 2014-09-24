/**
* relay_agent.h
* Date: 2014/09/17
* Author: Alan Yang
*/
#ifndef RELAYAGENT_H_
#define RELAYAGENT_H_

#include <stdio.h>
#include <string>
#include <exception>

#include "config.h"
#include "util.h"
#include <tins/tins.h>
#include "../send_receive_module/warp_protocol_sender.h"
#include "../send_receive_module/fragment_receiver.h"
#include "../warp_protocol/warp_protocol.h"

using namespace Tins;
using namespace Config;
using namespace std;

namespace RelayAgents {
    static  std::string _DEFINITIONS[] = {
                "RAW", "ETHERNET_II", "IEEE802_3", "RADIOTAP",
                "DOT11", "DOT11_ACK", "DOT11_ASSOC_REQ", "DOT11_ASSOC_RESP",
                "DOT11_AUTH", "DOT11_BEACON", "DOT11_BLOCK_ACK", "DOT11_BLOCK_ACK_REQ",
                "DOT11_CF_END", "DOT11_DATA", "DOT11_CONTROL", "DOT11_DEAUTH",
                "DOT11_DIASSOC", "DOT11_END_CF_ACK", "DOT11_MANAGEMENT", "DOT11_PROBE_REQ",
                "DOT11_PROBE_RESP", "DOT11_PS_POLL", "DOT11_REASSOC_REQ", "DOT11_REASSOC_RESP",
                "DOT11_RTS", "DOT11_QOS_DATA", "LLC", "SNAP",
                "IP", "ARP", "TCP", "UDP",
                "ICMP", "BOOTP", "DHCP", "EAPOL",
                "RC4EAPOL", "RSNEAPOL", "DNS", "LOOPBACK",
                "IPv6", "ICMPv6", "SLL", "DHCPv6",
                "DOT1Q", "PPPOE", "STP", "PPI",
                "IPSEC_AH", "IPSEC_ESP"
            };

    class RelayAgent {
        public:
            WARP_ProtocolSender* getSender() const;
            virtual void sniff();
            virtual void set_in_interface(const char* set_in_interface);
            virtual void set_out_interface(const char* out_interface);
            virtual bool process(PDU &pkt);
            virtual void run(int argc, char *argv[]);
            // static
            static std::string PDU_Type_To_String(int PDUTypeFlag);
        protected:
            RelayAgent();
            RelayAgent(WARP_ProtocolSender* init_protocol_sender);
            RelayAgent(PacketSender* init_packet_sender);
            ~RelayAgent();
            std::unique_ptr<WARP_ProtocolSender> protocol_sender;
            std::unique_ptr<PacketSender> packet_sender;
            std::unique_ptr<std::string> in_interface;
            std::unique_ptr<std::string> out_interface;
        private:
            void init(WARP_ProtocolSender* init_protocol_sender, PacketSender* init_packet_sender);
    };
}

#endif