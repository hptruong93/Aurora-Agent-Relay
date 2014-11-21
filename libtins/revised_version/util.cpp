#include "tins/tins.h"
#include <cstdlib>
#include <cstring>

#include "util.h"

using namespace std;
using namespace Tins;

bool compare_mac(uint8_t* mac1, uint8_t* mac2) {
	uint8_t i = 0;
	for (; i < 6; i++) {
		if (mac1[i] != mac2[i]) {
			return false;
		}
	}
	return true;
}

void convert_mac(uint8_t* result_mac, Tins::HWAddress<6> mac) {
	auto it = mac.begin();
	uint8_t i = 0;
	for (; it != mac.end(); it++) {
		result_mac[i] = *it;
		i++;
	}
}

void print_packet(uint8_t* packet, uint32_t length) {
	uint32_t i = 0;
	printf("Length is %d\n", length);

	if (length > 500) {
		length = 100;
	}
	
	for (i = 0; i < length; i++) {
		printf("%d-", packet[i]);
	}
	printf("\n");
}

char* get_interface(Tins::Dot11::address_type addr) {
    string address = addr.to_string();

    if (address == "ff:ff:ff:ff:ff:ff")
    {
    	FILE *fp;
    	char *interfaces = (char*)malloc(1024);
    	bool should_take = true;
    	size_t interfaces_index = 0;
    	char c;

    	std::string command = std::string(GREP_FROM_IFCONFIG) + std::string(MON_INTERFACE_KEYWORD);
    	fp = popen(command.c_str(), "r");

    	while ((c = fgetc(fp)) != EOF)
    	{
    		if (c == '\n')
    		{
    			should_take = true;
    		}
    		else if (c == ' ')
    		{
    			should_take = false;
    		}

    		if (should_take)
    		{
    			interfaces[interfaces_index++] = c;
    		}
    	}

    	interfaces[interfaces_index] = '\0';

    	cout << "Interfaces are: " << interfaces << endl;

    	return interfaces;
    }

    cout << "Address is " << address << endl;

    FILE *fp;
    char *interface_name = (char*)malloc(64);
    size_t interface_name_len = 0;
    int c;
    std::string command = std::string(GREP_FROM_IFCONFIG) + std::string("'") +  std::string(HW_ADDR_KEYWORD) + address + std::string("'");
    fp = popen(command.c_str(), "r");

    strcpy(interface_name, "mon.");
    interface_name_len += strlen("mon.");

    while ((c = fgetc(fp)) != EOF)
    {
        if ((char) c == ' ')
        {
            break;
        }
        interface_name[interface_name_len++] = (char)c;
    }

    interface_name[interface_name_len] = '\0';

    fclose(fp);

    return interface_name;
}

char* get_interface_name(const std::string& addr)
{
    FILE *fp;
    char *interface_name = (char*)malloc(64);
    size_t interface_name_len = 0;
    int c;
    std::string command = std::string(GREP_FROM_IFCONFIG) + std::string("'") +  std::string(HW_ADDR_KEYWORD) + addr + std::string("'");
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

    fclose(fp);

    return interface_name;
}

char* get_command_output(const std::string& command)
{
    FILE *fp;
    char *output = (char*)malloc(1024);
    size_t index = 0;

    fp = popen(command.c_str(), "r");
    char c;
    while((c = fgetc(fp)) != EOF)
    {
        output[index++] = c;
    }

    output[index] = 0;

    fclose(fp);

    return output;
}

void split_string(std::string& original, const std::string& delimiter, std::vector<std::string> *splits)
{
	int pos;
	while ((pos = original.find(delimiter)) != std::string::npos)
	{
		splits->push_back(original.substr(0, pos));
		original.erase(0, pos + delimiter.length());
	}

	if (original.length() > 0 && original != " ")
	{
		splits->push_back(original);
	}
}

void split_string(std::string& original, const std::string& delimiter, std::set<std::string> *splits)
{
    int pos;
    while ((pos = original.find(delimiter)) != std::string::npos)
    {
        splits->insert(original.substr(0, pos));
        original.erase(0, pos + delimiter.length());
    }

    if (original.length() > 0 && original != " ")
    {
        splits->insert(original);
    }
}

void get_pid(std::string& command_line_regex, std::vector<int> *pids, int limit) {
    char* output = get_command_output(std::string("ps -ef | grep \"" + command_line_regex + "\" | awk '{print $2;}'"));

    std::vector<std::string> pid_string;
    std::string output_string(output);
    split_string(output_string, "\n", &pid_string);

    int count = 0;
    for (std::vector<std::string>::iterator it = pid_string.begin(); it != pid_string.end(); it++) {
        int found_pid = std::atoi((*it).c_str());
        pids->push_back(found_pid);
        count++;
        if (count >= limit) {
            break;
        }
    }

    free(output);
}