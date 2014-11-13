#include "dpm_agent.h"

#include <iostream>

char* get_interface_name(const std::string& addr)
{
    FILE *fp;
    char *interface_name = (char*)malloc(64);
    size_t interface_name_len = 0;
    int c;
    std::string command = std::string("ifconfig | grep -E ") + std::string("'") +  std::string("HWaddr ") + addr + std::string("'");
    fp = popen(command.c_str(), "r");

    while ((c = fgetc(fp)) != EOF)
    {
        if ((char) c == ' ')
        {
            break;
        }
        interface_name[interface_name_len++] = (char)c;
    }

    interface_name[interface_name_len] = '\0';

    return interface_name;
}

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
    char c = 's';

    fp = popen((std::string(BASE_COMMAND_STR) + " " + "init" + " " + ovs_name).c_str(), "r");
    while (true)
    {
        c = fgetc(fp);
        if (c == '\n') {
            break;
        }
        line[index++] = c;
    }
    line[index] = '\0';

    std::string line_str = std::string(line);

    free(line);

    line_str.erase(0, line_str.find('/'));

    socket_path = line_str;
    std::cout << "Found socket path " << socket_path << std::endl;
}

int DPMAgent::add(const std::string& bssid, const std::string& ethernet_interface)
{
    char* interface_name = get_interface_name(bssid);
    std::string virtual_interface = std::string(interface_name);
    free(interface_name);

    std::cout<<"Command: "<<"add " + socket_path + " " + ovs_name + " " + virtual_interface + " " + ethernet_interface<<std::endl;
    return execute_command("add " + socket_path + " " + ovs_name + " " + virtual_interface + " " + ethernet_interface);
}

int DPMAgent::remove(const std::string& bssid, const std::string& ethernet_interface)
{
    char* interface_name = get_interface_name(bssid);
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