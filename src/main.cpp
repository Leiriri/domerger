#include "arg_parser.h"
#include "logging.h"
#include "merge_rules.h"
#include <filesystem>
#include <vector>

std::vector<std::string> get_workshop_mod_dirs(const std::string &dom_path) {
  // add all directories in dom_path to a vector and return it
  std::vector<std::string> mod_dirs;
  for (const auto &entry : std::filesystem::directory_iterator(dom_path)) {
    if (entry.is_directory()) {
      mod_dirs.push_back(entry.path().string());
    }
  }

  std::sort(mod_dirs.begin(), mod_dirs.end());

  return mod_dirs;
}

int main(int argc, char *argv[]) {
  Arg_Parser parser;
  parser.parse(argc, argv);

  std::string mods_path = parser.get_option("mods-path", "./mods");
  std::string target_path = parser.get_option("target-path", "./target");
  bool persistent_log = parser.has_option("persistent-log") || parser.has_option("p");
  std::string log_level_str = parser.get_option("log-level", "INFO");
  log_level log_level_option = log_level_str == "ERROR"     ? log_level::ERROR
                               : log_level_str == "WARNING" ? log_level::WARNING
                               : log_level_str == "INFO"    ? log_level::INFO
                               : log_level_str == "DEBUG"   ? log_level::DEBUG
                                                            : log_level::ERROR;
  bool show_help = parser.has_option("help") || parser.has_option("h");
  if (show_help) {
    print_help();
    exit(0);
  }

  always_logger << "Mods path: " << mods_path << "\n";
  always_logger << "Target path: " << target_path << "\n";
  always_logger << "Log level: " << log_level_str << "\n";
  always_logger << "Persistent log: " << (persistent_log ? "Enabled" : "Disabled") << "\n";

  // Set log level to DEBUG for detailed output
  Logger::set_log_level(log_level_option);
  Logger::set_persistent_log(persistent_log);

  std::vector<std::string> mod_dirs = get_workshop_mod_dirs(mods_path);
  always_logger << "Found " << mod_dirs.size() << " mod directories in workshop content path." << "\n";
  debug_logger << mod_dirs[0] << "\n"; // Print the first mod directory for verification

  std::vector<std::string> dmfile_process_order;
  copy_all_mods(parser, mod_dirs, target_path, dmfile_process_order);

  // print the mod directories found
  always_logger << "Mods should be activated in the following order, otherwise some names might not be referenced correctly and cause the game to crash:" << "\n";
  for (const auto &mod : dmfile_process_order) {
    always_logger << mod << "\n";
  }

  return 0;
}
