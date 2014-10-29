#ifndef BSSIDNODE_H_
#define BSSIDNODE_H_

enum BSSID_NODE_OPS
{
    // Ansynchronous Bssid update
    BSSID_ADD,
    BSSID_REMOVE,
    BSSID_CHECK,
    // Asynchronous warp protocol
    SEND_MAC_ADDR_CNTRL,
    SEND_TRANSMISSION_CNTRL,
    // Synchronous Warp protocol
    MAC_ADD,
    TRANSMISSION_CNTRL
};

class BssidNode
{
    public:
        virtual ~BssidNode() {};
        virtual int sync(int operation_code, void* data) { return 0; };
        virtual int timed_sync(int operation_code, int timeout) { return 0; };
};

#endif