#pragma once
#include <string>
#include <unordered_map>
#include <vector>


class Arg_Parser {
private:
    std::unordered_map<std::string, std::string> options; // Stores key-value pairs
    std::vector<std::string> positional_args;            // Stores positional arguments
    std::vector<std::string> switches;                  // Stores switches (flags without values)

public:
    void parse(int argc, char* argv[]);

    std::string get_option(const std::string& key, const std::string& default_value = "") const;

    bool has_option(const std::string& key) const;

    const std::vector<std::string>& get_positional_args() const;


};

void print_help();
