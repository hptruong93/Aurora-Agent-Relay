/**
* mon_to_warp_agent.cpp
* Date: 2014/09/17
* Author: Alan Yang
*/

using namespace std;

MonToWarpAgent::MonToWarpAgent(WARP_ProtocolSender* init_protocol_sender) : RelayAgent(WARP_ProtocolSender* init_protocol_sender)
{

}

bool MonToWarpAgent::process(PDU &pkt) override
{
    if (pkt.pdu_type() == pkt.RADIOTAP) {
    
        //Start processing of RadioTap Packets
        PDU *innerPkt = pkt.inner_pdu();
        if (MonToWarpAgent::Is_Management_Frame(innerPkt->pdu_type())) {
            Dot11ManagementFrame &management_frame = innerPkt->rfind_pdu<Dot11ManagementFrame>();
            Dot11::address_type source = management_frame.addr2();
            Dot11::address_type dest = management_frame.addr1();

            if (source == HOSTAPD || source == BROADCAST) {
                //Add an ethernet frame and send over iface
                //-----------------> Create WARP transmit info
                WARP_protocol::WARP_transmit_struct* transmit_info = WARP_protocol::get_default_transmit_struct();
                transmit_info->retry = (dest == BROADCAST) ? 0 : MAX_RETRY;
                transmit_info->payload_size = (uint16_t) management_frame.size();

                //-----------------> Using fragment sender to send
                this->protocol_sender->send(management_frame, TYPE_TRANSMIT, SUBTYPE_MANGEMENT_TRANSMIT, transmit_info);

                //-----------------> Clean up
                free(transmit_info);
                cout << "Sent 1 management packet" << endl;
            }
        } else if (innerPkt->pdu_type() == pkt.DOT11_DATA) {
            Dot11Data &dataPkt = innerPkt->rfind_pdu<Dot11Data>();
            Dot11::address_type source = dataPkt.addr2();
            if(source == HOSTAPD || source == BROADCAST) {
                //Add an ethernet frame and send over iface to warp
                //-----------------> Create WARP transmit info
                WARP_protocol::WARP_transmit_struct* transmit_info = WARP_protocol::get_default_transmit_struct();
                transmit_info->retry = MAX_RETRY;
                transmit_info->payload_size = (uint16_t) dataPkt.size();

                //-----------------> Using fragment sender to send
                this->protocol_sender->send(dataPkt, TYPE_TRANSMIT, SUBTYPE_DATA_TRANSMIT, transmit_info);

                //-----------------> Clean up
                free(transmit_info);
                cout << "Sent 1 data packet" << endl;
            }
        } else {//802.11 Control packets
            cout << "Control packet: Inner Layer is " << PDUTypeToString(pkt.inner_pdu()->pdu_type()) << endl;

            Dot11Control &controlPacket = innerPkt->rfind_pdu<Dot11Control>();
            Dot11::address_type dest = controlPacket.addr1();

            if (dest != HOSTAPD) {
                EthernetII to_send = EthernetII(WARP, PC_ENGINE);
                to_send.payload_type(WARP_PROTOCOL_TYPE);

                //-----------------> Create WARP transmit info
                WARP_protocol::WARP_transmit_struct* transmit_info = WARP_protocol::get_default_transmit_struct();
                transmit_info->retry = MAX_RETRY;
                transmit_info->payload_size = (uint16_t) controlPacket.size();

                //-----------------> Create WARP layer and append at the end
                this->protocol_sender->send(controlPacket, TYPE_TRANSMIT, SUBTYPE_DATA_TRANSMIT, transmit_info);

                //-----------------> Clean up
                free(transmit_info);
                cout << "Sent 1 control packet" << endl;
            } else {
                cerr << "Inner Layer is " << pkt.inner_pdu()->pdu_type() << "! Skipping..." << endl;
            }
        }
    } else {
        cerr << "Error: Non RadioTap Packet Detected!" << endl;
    }
    return true;
}

void run(int argc, char *argv[]) override
{
    if (argc >= 3) {
        this->set_in_interface(argv[1]);
        this->set_out_interface(argv[2]);
        cout << "Init pc to warp from " << argv[1] << " to " << argv[2] << endl;

        if (argc == 4) {
            Config::HOSTAPD = Tins::HWAddress<6>(argv[3]);
            cout << "hostapd mac is " << argv[3] << endl;
        }
    } else {
        this->set_in_interface("hwsim0");
        this->set_out_interface("eth0");
        cout << "Init pc to warp from hwsim0 to eth0" << endl;
    }
    
    this->sniff(in_interface);
}

// Static methods

std::string MonToWarpAgent::PDU_Type_To_String(int type)
{
    if(PDUTypeFlag == 1000)
        return std::string("USER_DEFINED_PDU");
    else
        return MonToWarpAgent::_DEFINITIONS[PDUTypeFlag];
}

bool MonToWarpAgent::Is_Management_Frame(int type)
{
    string converted = MonToWarpAgent::PDUTypeToString(type);
    if (converted == "DOT11" ||
        converted == "DOT11_BEACON" ||
        converted == "DOT11_PROBE_REQ" ||
        converted == "DOT11_PROBE_RESP" ||
        converted == "DOT11_AUTH" ||
        converted == "DOT11_DEAUTH" ||
        converted == "DOT11_ASSOC_REQ" ||
        converted == "DOT11_ASSOC_RESP" ||
        converted == "DOT11_REASSOC_REQ" ||
        converted == "DOT11_REASSOC_RESP" ||
        converted == "DOT11_DIASSOC") {
        return true;
    } else {
        return false;
    }
}