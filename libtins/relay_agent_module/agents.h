#ifndef AGENTS_H_
#define AGENTS_H_

#include <vector>
#include <string>

namespace RelayAgents {
    enum class ParseFunctionCode {
        NEW_THREAD,
        HELP,
        ERROR
    };
}

RelayAgents::ParseFunctionCode parse_input(string input, vector<string>& tokens);

#endif