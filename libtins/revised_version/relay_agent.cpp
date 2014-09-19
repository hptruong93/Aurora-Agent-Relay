#include "relay_agent.h"

RelayAgent::RelayAgent()
{
    this->in_interface = std::unique_ptr<std::string>(new std::string(""));
    this->out_interface = std::unique_ptr<std::string>(new std::string(""));
}

RelayAgent::RelayAgent(const WARP_ProtocolSender* init_protocol_sender) : RelayAgent()
{
    this->protocol_sender = std::unique_ptr<WARP_ProtocolSender>(init_protocol_sender);
    this->packet_sender = nullptr;
}

RelayAgent::RelayAgent(const PacketSender* init_packer_sender) : RelayAgent()
{
    this->protocol_sender = nullptr;
    this->packet_sender = std::unique_ptr<PacketSender>(init_packer_sender);
}

RelayAgent::~RelayAgent()
{
    this->protocol_sender.reset();
    this->packet_sender.reset();
    this->in_interface.reset();
    this->out_interface.reset();
}

void RelayAgent::sniff()
{
    Sniffer sniffer(*(this->in_interface).c_str(), Sniffer::PROMISC);
    sniffer.sniff_loop(this->process);
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
    if(PDUTypeFlag == 1000)
        return std::string("USER_DEFINED_PDU");
    else
        return RelayAgent::_DEFINITIONS[PDUTypeFlag];
}