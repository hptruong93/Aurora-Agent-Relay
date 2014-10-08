#ifndef AGENTS_H_
#define AGENTS_H_

// Agent types
#define WLAN_TO_WARP                    "wlan_to_warp"
#define MON_TO_WARP                     "mon_to_warp"
#define WARP_TO_WLAN                    "warp_to_wlan"

namespace RelayAgents {
    
    // Receives command and initialize different agents
    class AgentFactory {
        public:
            AgentFactory();
            void spawn_agent_thread(const char* agent_type, int argc, char *argv[]);
    };
}

#endif