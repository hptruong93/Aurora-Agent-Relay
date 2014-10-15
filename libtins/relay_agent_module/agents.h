#ifndef AGENTS_H_
#define AGENTS_H_

#include <vector>
#include <string>

// Commands
#define HELP_COMMAND                    "help"
#define KILL_COMMAND					"kill"

namespace RelayAgents {
    enum class ParseFunctionCode {
        NEW_THREAD,
        HELP,
        KILL,
        ERROR
    };
}

RelayAgents::ParseFunctionCode parse_input(string input, vector<string>& tokens);

#endif