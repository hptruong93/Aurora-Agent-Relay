#include "relay_agent.h"

RelayAgent::RelayAgent(const WARP_ProtocolSender* init_protocol_sender)
{
    this->protocol_sender = std::unique_ptr<WARP_ProtocolSender>(init_protocol_sender);
    this->in_interface = std::unique_ptr<std::string>(new std::string(""));
    this->out_interface = std::unique_ptr<std::string>(new std::string(""));
}

RelayAgent::~RelayAgent()
{
    this->protocol_sender.reset();
    this->in_interface.reset();
    this->out_interface.reset();
}

void RelayAgent::sniff()
{
    Sniffer sniffer(*(this->in_interface).c_str(), Sniffer::PROMISC);
    sniffer.sniff_loop(process);
}

void RelayAgent::set_in_interface(const char* in_interface)
{
    this->in_interface.reset(new std::string(in_interface));
}

void RelayAgent::set_out_interface(const char* out_interface)
{
    this->out_interface.reset(new std::string(out_interface));

    this->protocol_sender.reset(new WARP_ProtocolSender(new PacketSender(out_interface)));
}