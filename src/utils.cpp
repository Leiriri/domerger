
#include "utils.h"
#include "logging.h"
#include "constants.h"


std::tuple<int, std::size_t, std::size_t> get_id_from_line(const std::string &line) {
  // Since commands can contain digits, we need to find the first digit after the command
  std::size_t const after_command = line.find_first_of(" \t");
  std::size_t start = line.find_first_of("0123456789", after_command);
  if (start == std::string::npos) {
    // return INT32_MAX to indicate no id found as that is not a valid id
    return std::make_tuple(INT32_MAX, std::string::npos, std::string::npos);
  }
  std::size_t end = line.find_first_not_of("0123456789", start);
  // if no non-digit is found, set end to line length to not break difference calculation
  if (end == std::string::npos) {
    end = line.length();
  }

  // check if resulting value is too large (largest viable id is 100000 for montag)
  debug_logger << "Extracted positions for id: start=" << start << ", end=" << end << " difference=" << end - start << "\n";
  if ((end - start) > 6) {
    // return INT32_MAX to indicate no id found as that is not a valid id
    return std::make_tuple(INT32_MAX, std::string::npos, std::string::npos);
  }
  // Make sure the sign is part of the substring
  if (line[start - 1] == '-') {
    start--;
  }
  std::string id_str = line.substr(start, end - start);
  int id = std::stoi(id_str);
  return std::make_tuple(id, start, end);
}


std::tuple<std::string, std::size_t, std::size_t> get_string_from_line(const std::string &line) {
  // Since commands can contain digits, we need to find the first digit after the command
  std::size_t const after_command = line.find_first_of(" \t");
  std::size_t start = line.find_first_of("\"", after_command);
  if (start == std::string::npos) {
    // return empty string to indicate no string found
    return std::make_tuple("", std::string::npos, std::string::npos);
  }
  start++; // move to first character of string rather than the quote
  std::size_t end = line.find_first_of("\"", start);

  std::string str = line.substr(start, end - start);
  return std::make_tuple(str, start, end);
}


std::tuple<std::string, std::string> preprocess_line(const std::string &line) {
  std::string trimmed_line = line;
  // remove UTF-8 BOM if present, mods can rarely have this and it will cause issues with parsing if not removed
  if (!trimmed_line.empty() && trimmed_line[0] == '\xEF' && trimmed_line[1] == '\xBB' && trimmed_line[2] == '\xBF') {
      trimmed_line.erase(0, 3); // Remove the BOM
  }
  trimmed_line.erase(0, trimmed_line.find_first_not_of(" \t"));
  // remove carriage returns
  if (!trimmed_line.empty() && trimmed_line.back() == '\r') {
    trimmed_line.pop_back();
  }
  // remove comments for easier parsing and insert after processing
  // comments start with --
  std::size_t comment_pos = trimmed_line.find("--");
  std::string comment;
  if (comment_pos != std::string::npos) {
    comment = trimmed_line.substr(comment_pos);
    trimmed_line = trimmed_line.substr(0, comment_pos);
  }
  // remove brackets from anywhere in the trimmed_line as they can cause some weird problems when interpreted by the game (saw one nation get some units without any stats added to their roster)
  trimmed_line.erase(std::remove(trimmed_line.begin(), trimmed_line.end(), '['), trimmed_line.end());
  trimmed_line.erase(std::remove(trimmed_line.begin(), trimmed_line.end(), ']'), trimmed_line.end());
  return std::make_tuple(trimmed_line, comment);
}


const int max_allowed_id[ID_TYPE_COUNT] = {
    248, // Sound
    3999, // Weapons
    1999, // Armor
    19999, // Monsters
    399,  // Nametypes
    7999, // Spells
    9999,  // Enchantments
    1999,  // Items
    3999, // Magic Sites
    499,  // Nations
    249,  // Poptypes
    99999,  // Montag
    4999   // Event codes
};

void check_max_id_violation(int current_id_table[]) {
  for (uint8_t type = 0; type < ID_TYPE_COUNT; type++) {
    if (current_id_table[type] > max_allowed_id[type]) {
      error_logger << "ID limit exceeded for type " << type_to_string.at(type) << ": assigned ID " << current_id_table[type] - 1 << " exceeds maximum of " << max_allowed_id[type] - 1 << ". The game will crash when loading the mod. You need to remove mods until this message disappears." << "\n";
    }
  }
}
