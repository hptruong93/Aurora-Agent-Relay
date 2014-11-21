#ifndef BSSIDNODE_H_
#define BSSIDNODE_H_

enum BSSID_NODE_OPS
{
    // Ansynchronous Bssid update
    BSSID_ADD,
    BSSID_REMOVE,
    BSSID_CHECK,
    BSSID_MAC_ASSOCIATE,
    BSSID_MAC_DISASSOCIATE,
    // Asynchronous warp protocol
    SEND_MAC_ADDR_CNTRL,
    SEND_TRANSMISSION_CNTRL,
    SEND_BSSID_CNTRL,
    // Synchronous Warp protocol
    MAC_ADD,
    MAC_REMOVE,
    TRANSMISSION_CNTRL,
    BSSID_CNTRL,
    // Comms agent associate/disassociate mac addr
    COMMAND_ADD
};

class BssidNode
{
    public:
        virtual ~BssidNode() {};
        virtual int sync(int operation_code, void* data) { return 0; };
        virtual int timed_sync(int operation_code, void* data, int timeout) { return 0; };
};

#endif