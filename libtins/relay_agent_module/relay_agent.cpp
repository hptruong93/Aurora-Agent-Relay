#include "relay_agent.h"


#include <stdio.h>
#include <string>
#include <exception>
#include <functional>

#include "../revised_version/config.h"
#include "../revised_version/util.h"
#include <tins/tins.h>
#include "../send_receive_module/warp_protocol_sender.h"
#include "../send_receive_module/fragment_receiver.h"
#include "../warp_protocol/warp_protocol.h"

using namespace Tins;
using namespace Config;
using namespace std;

using namespace RelayAgents;

RelayAgent::RelayAgent()
{
    this->init(NULL, NULL);
}

RelayAgent::RelayAgent(WARP_ProtocolSender* init_protocol_sender)
{
    this->init(init_protocol_sender, NULL);
}

RelayAgent::RelayAgent(PacketSender* init_packet_sender)
{
    this->init(NULL, init_packet_sender);
}

RelayAgent::~RelayAgent()
{
    this->protocol_sender.reset();
    this->packet_sender.reset();
    this->in_interface.reset();
    this->out_interface.reset();
}

void RelayAgent::init(WARP_ProtocolSender* init_protocol_sender, PacketSender* init_packet_sender)
{
    this->packet_sender = nullptr;
    this->protocol_sender = nullptr;
    this->in_interface = std::unique_ptr<std::string>(new std::string(""));
    this->out_interface = std::unique_ptr<std::string>(new std::string(""));

    if (init_protocol_sender != NULL) {
        this->protocol_sender.reset(init_protocol_sender);
    }

    if (init_packet_sender != NULL) {
        this->packet_sender.reset(init_packet_sender);
    }
}

void RelayAgent::sniff()
{
    Sniffer sniffer(*(this->in_interface), Sniffer::PROMISC);
    sniffer.sniff_loop(make_sniffer_handler(this, &RelayAgent::process));
}

void RelayAgent::set_in_interface(const char* in_interface)
{
    this->in_interface.reset(new std::string(in_interface));
}

void RelayAgent::set_out_interface(const char* out_interface)
{
    this->out_interface.reset(new std::string(out_interface));

    this->packet_sender.reset(new PacketSender(out_interface));

    if (this->protocol_sender) {
        this->protocol_sender.reset(new WARP_ProtocolSender(this->packet_sender.release()));
    }
}

// Static

std::string RelayAgent::PDU_Type_To_String(int type)
{
    if(type == 1000)
        return std::string("USER_DEFINED_PDU");
    else
        return RelayAgents::_DEFINITIONS[type];
}

// Empty functions
bool RelayAgent::process(PDU &pkt)
{
    throw bad_function_call();
}

void RelayAgent::run(int argc, char *argv[])
{
    throw bad_function_call();
}