#ifndef AGENTS_H_
#define AGENTS_H_

#include <vector>
#include <string>

// cmmand types
#define WLAN_TO_WARP                    "wlan_to_warp"
#define MON_TO_WARP                     "mon_to_warp"
#define WARP_TO_WLAN                    "warp_to_wlan"
#define HELP_COMMAND                    "help"

namespace RelayAgents {
	enum class ParseFunctionCode {
		NEW_THREAD,
		HELP,
		ERROR
	};
    
    // Receives command and initialize different agents
    class AgentFactory {
        public:
            AgentFactory();
            void spawn_agent_thread(std::vector<std::string>& args);
    };
}

#endif