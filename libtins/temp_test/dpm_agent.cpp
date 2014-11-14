#include "dpm_agent.h"
#include "util.h"

DPMAgent::DPMAgent()
{

}

int DPMAgent::init(const std::string& init_ovs_name)
{
    ovs_name = init_ovs_name;
    ovs_thread = std::unique_ptr<std::thread>(new std::thread(&DPMAgent::initialize, this, init_ovs_name));
    ovs_thread.get()->detach();
}

void DPMAgent::initialize(std::string ovs_name)
{
    FILE *fp;
    char *line = (char*)malloc(1024 * sizeof(char));
    int index = 0;
    char c;

    fp = popen((std::string(BASE_COMMAND_STR) + " " + "init" + " " + ovs_name).c_str(), "r");
    while ((c = fgetc(fp)) != '\n')
    {
        line[index++] = c;
    }
    line[index] = '\0';

    std::string line_str = std::string(line);

    free(line);

    line_str.erase(0, line_str.find('/'));

    socket_path = line_str;
}

int DPMAgent::add(const std::string& bssid, const std::string& ethernet_interface)
{
    char* interface_name = getInterfaceName(bssid);
    std::string virtual_interface = std::string(interface_name);
    free(interface_name);

    return execute_command("add " + socket_path + " " + ovs_name + " " + virtual_interface + " " + ethernet_interface);
}

int DPMAgent::remove(const std::string& bssid, const std::string& ethernet_interface)
{
    char* interface_name = getInterfaceName(bssid);
    std::string virtual_interface = std::string(interface_name);
    free(interface_name);

    return execute_command("remove " + socket_path + " " + ovs_name + " " + virtual_interface + " " + ethernet_interface);   
}

int DPMAgent::sync(int operation_code, void* bssid)
{
    BSSID_NODE_OPS op = (BSSID_NODE_OPS)operation_code;
    switch(op)
    {
        case BSSID_NODE_OPS::BSSID_ADD:
            return add(std::string((char*)bssid));
        case BSSID_NODE_OPS::BSSID_REMOVE:
            return remove(std::string((char*)bssid));
    }
}

int DPMAgent::execute_command(std::string command)
{
    return system((std::string(BASE_COMMAND_STR) + " " + command).c_str());
}