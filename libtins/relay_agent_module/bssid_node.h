#ifndef BSSIDNODE_H_
#define BSSIDNODE_H_

enum BSSID_OPS
{
    BSSID_ADD,
    BSSID_REMOVE,
    BSSID_CHECK
};

enum SYNC_OPS
{
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