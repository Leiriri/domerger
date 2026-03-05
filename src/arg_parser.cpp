#include <algorithm>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "arg_parser.h"

std::unordered_map<std::string, std::string> options; // Stores key-value pairs
std::vector<std::string> positional_args;             // Stores positional arguments

void Arg_Parser::parse(int argc, char *argv[]) {
  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg.starts_with("--")) { // Long option (e.g., --key=value or --key value)
      auto eq_pos = arg.find('=');
      if (eq_pos != std::string::npos) {
        std::string key = arg.substr(2, eq_pos - 2);
        std::string value = arg.substr(eq_pos + 1);
        options[key] = value;
      } else if (i + 1 < argc && !std::string(argv[i + 1]).starts_with("-")) {
        options[arg.substr(2)] = argv[++i]; // --key value
      } else {
        options[arg.substr(2)] = ""; // --key with no value
      }
    } else if (arg.starts_with("-")) { // Short option (e.g., -k value)
      if (i + 1 < argc && !std::string(argv[i + 1]).starts_with("-")) {
        options[arg.substr(1)] = argv[++i]; // -k value
      } else {
        options[arg.substr(1)] = ""; // -k with no value
      }
    } else {
      positional_args.push_back(arg); // Positional argument
    }
  }
}

// Get the value of an option (returns empty string if not found)
std::string Arg_Parser::get_option(const std::string &key, const std::string &default_value) const {
  auto it = options.find(key);
  return it != options.end() ? it->second : default_value;
}

// Check if an option exists
bool Arg_Parser::has_option(const std::string &key) const { return options.find(key) != options.end(); }

// Get all positional arguments
const std::vector<std::string> &Arg_Parser::get_positional_args() const { return positional_args; }

void print_help() {
  std::cout << "Usage: merger [options]\n"
               "Options:\n"
               "  --mods-path <path>        Path to the mods directory (default: ~/.local/share/Steam/steamapps/workshop/content/2511500)\n"
               "  --target-path <path>      Path to the target merged mods directory (default: ~/.dominions6/mods/merged_workshop_mods)\n"
               "  --era-merge, -e <mode>    Era merge mode currently only affects Nations. (default: -1, no merging), 0: Disable all, 1: EA, 2: MA, 3: LA\n"
               "  --log-level <level>       Set the logging level (DEBUG, INFO, WARNING, ERROR) (default: ERROR)\n"
               "  --persistent-log, -p      Enable persistent logging to a file\n"
               "  --help, -h                Show this help message\n";
}
