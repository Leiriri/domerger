/*
 * Manages basegame data, loading them from tsv files into maps for easy access
 * Utilizes tsv files from the dom6inspector https://github.com/larzm42/dom6inspector
 */

#include "gamedata_manager.h"
#include "utils.h"
#include "logging.h"
#include <cstdint>
#include <cstring>
#include <fstream>
#include <variant>
#include <vector>



//                  id,    name,     school, researchlev, path1, pathl1, path2, pathl2, eff	effc	prec	fat	gemcost	next	damage
DualMap<std::tuple<int, std::string, int8_t, uint8_t, int8_t, int8_t, int8_t, int8_t, int, int, int, int, uint8_t, int, long>> spells_map;
DualMap<std::tuple<int, std::string>> basegame_id_name_map[ID_TYPE_COUNT];

// LOWTODO: if spells are replaced without remapping them, new spells that reference them will still count as summon/polymorph (should be very rare though)
std::unordered_set<std::variant<int, std::string>> special_spell_ids[2]; // 0 enchantment, 1 monster summon/polymorph

/*
 * Takes two parameters to identify the old and new ids/names
 * Then updates the basegame id/name map to reflect the remapping
 */
void insert_into_remap(std::variant<int, std::string> old_id, std::variant<int, std::string> new_id, uint8_t type) {
  int old_id_int = old_id.index() == 0 ? std::get<int>(old_id) : INT32_MAX;
  int new_id_int = new_id.index() == 0 ? std::get<int>(new_id) : INT32_MAX;
  std::string old_name = old_id.index() == 1 ? std::get<std::string>(old_id) : "";
  std::string new_name = new_id.index() == 1 ? std::get<std::string>(new_id) : "";

  // if id/name has already been remapped, we simply skip inserting that same value again because we already map to a location that is guaranteed to contain the original spell data
  if (old_id_int == INT32_MAX) {
    debug_logger << "Looking up old name: " << old_name << "\n";
    old_id_int = *basegame_id_name_map[type].find_id_by_name(old_name);
  }
  if (old_name == "") {
    debug_logger << "Looking up old id: " << old_id_int << "\n";
    old_name = *basegame_id_name_map[type].find_name_by_id(old_id_int);
  }
  std::transform(old_name.begin(), old_name.end(), old_name.begin(), ::tolower); // case insensitive check
  if (new_id_int == INT32_MAX) {
    std::transform(new_name.begin(), new_name.end(), new_name.begin(), ::tolower); // case insensitive check
    debug_logger << "Looking up new name: " << old_name << "\n";
    new_id_int = std::get<0>(*basegame_id_name_map[type].find_by_name(old_name));
    // remove old_name from enchantment set and insert new_name
    for (int i = 0; i < SPECIAL_SPELL_TYPE_COUNT; i++) {
      if (special_spell_ids[i].find(old_name) != special_spell_ids[i].end()) {
        special_spell_ids[i].erase(old_name);
        special_spell_ids[i].insert(new_name);
      }
    }
  }
  if (new_name == "") {
    debug_logger << "Looking up new id: " << old_id_int << "\n";
    new_name = std::get<1>(*basegame_id_name_map[type].find_by_id(old_id_int));
    std::transform(new_name.begin(), new_name.end(), new_name.begin(), ::tolower); // case insensitive check
    // remove old_name from enchantment set and insert new_name
    for (int i = 0; i < SPECIAL_SPELL_TYPE_COUNT; i++) {
      if (special_spell_ids[i].find(old_name) != special_spell_ids[i].end()) {
        special_spell_ids[i].erase(old_name);
        special_spell_ids[i].insert(new_name);
      }
    }
  }
  basegame_id_name_map[type].update_item(old_id_int, std::make_tuple(new_id_int, new_name));
  // remapped_basegame_id_name_map[type].insert(old_id_int, old_name, std::make_tuple(new_id_int, new_name));
}

std::tuple<int, std::string> *get_current_of_type(std::variant<int, std::string> old_id, uint8_t type) {
  int old_id_int = old_id.index() == 0 ? std::get<int>(old_id) : INT32_MAX;
  std::string old_name = old_id.index() == 1 ? std::get<std::string>(old_id) : "";

  std::tuple<int, std::string> *remapped_tuple;
  if (old_id_int == INT32_MAX) {
    remapped_tuple = basegame_id_name_map[type].find_by_name(old_name);
  } else {
    remapped_tuple = basegame_id_name_map[type].find_by_id(old_id_int);
  }
  return remapped_tuple;
}

/*
 * Checks if the given spell id/name corresponds to a basegame enchantment spell
 */
bool is_special_spell(std::variant<int, std::string> id, uint8_t special_type) {
  int spell_id = id.index() == 0 ? std::get<int>(id) : INT32_MAX;
  if (spell_id == INT32_MAX) {
    std::string spell_name = std::get<std::string>(id);
    std::transform(spell_name.begin(), spell_name.end(), spell_name.begin(), ::tolower); // case insensitive check
    return special_spell_ids[special_type].find(spell_name) != special_spell_ids[special_type].end();
  }
  return special_spell_ids[special_type].find(spell_id) != special_spell_ids[special_type].end();
}

/*
 * Takes spell id and name and inserts them into the enchantment spell set
 */
void insert_special_spell(int spell_id, std::string spell_name, uint8_t special_type) {
  // case insensitive insert for names
  std::transform(spell_name.begin(), spell_name.end(), spell_name.begin(), ::tolower);
  special_spell_ids[special_type].insert(spell_id);
  special_spell_ids[special_type].insert(spell_name);
}

void load_spells() {
  std::string data_path = std::string("./gamedata/") + type_to_string.find(SPELLS)->second;
  std::ifstream data_file(data_path + ".tsv");

  if (!data_file.is_open()) {
    error_logger << "Failed to open basegame data file: " << data_path << ".tsv " << strerror(errno) << "\n";
    return;
  }

  std::string line;
  // skip header line
  std::getline(data_file, line);
  while (std::getline(data_file, line)) {
    std::vector<std::string> tokens;
    std::size_t start = 0;
    std::size_t end = line.find("\t");
    while (end != std::string::npos) {
      tokens.push_back(line.substr(start, end - start));
      start = end + 1;
      end = line.find("\t", start);
    }
    tokens.push_back(line.substr(start));

    if (tokens.size() < 14) {
      error_logger << "Invalid line in basegame data file: " << line << "\n";
      continue;
    }

    int id = std::stoi(tokens[0]);
    std::string name = tokens[1];
    std::int8_t school = static_cast<std::int8_t>(std::stoi(tokens[2]));
    uint8_t researchlevel = static_cast<uint8_t>(std::stoi(tokens[3]));
    int8_t path1 = static_cast<int8_t>(std::stoi(tokens[4]));
    int8_t pathlevel1 = static_cast<int8_t>(std::stoi(tokens[5]));
    int8_t path2 = static_cast<int8_t>(std::stoi(tokens[6]));
    int8_t pathlevel2 = static_cast<int8_t>(std::stoi(tokens[7]));
    int effect_record_id = std::stoi(tokens[8]);
    int effects_count = std::stoi(tokens[9]);
    int precision = std::stoi(tokens[10]);
    int fatiguecost = std::stoi(tokens[11]);
    uint8_t gemcost = static_cast<uint8_t>(std::stoi(tokens[12]));
    int next_spell = std::stoi(tokens[13]);
    long damage = stol(tokens[14]);

    spells_map.insert(id, name,
                      std::make_tuple(id, name, school, researchlevel, path1, pathlevel1, path2, pathlevel2, effect_record_id, effects_count, precision, fatiguecost, gemcost, next_spell, damage));
  }
}

void load_generic_basegame_data(uint8_t type) {
  std::string data_path = std::string("./gamedata/") + type_to_string.find(type)->second;
  std::ifstream data_file(data_path + ".tsv");

  if (!data_file.is_open()) {
    error_logger << "Failed to open basegame data file: " << data_path << ".tsv " << strerror(errno) << "\n";
    return;
  }

  std::string line;
  // skip header line
  std::getline(data_file, line);
  while (std::getline(data_file, line)) {
    std::vector<std::string> tokens;
    std::size_t start = 0;
    std::size_t end = line.find("\t");
    while (end != std::string::npos) {
      tokens.push_back(line.substr(start, end - start));
      start = end + 1;
      end = line.find("\t", start);
      if (tokens.size() >= 2) {
        break; // only need first two tokens
      }
    }
    tokens.push_back(line.substr(start));

    if (tokens.size() < 2) {
      error_logger << "Invalid line in basegame data file: " << line << "\n";
      continue;
    }

    int id = std::stoi(tokens[0]);
    std::string name = tokens[1];
    std::transform(name.begin(), name.end(), name.begin(), ::tolower); // case insensitive insert for names

    basegame_id_name_map[type].insert(id, name, std::make_tuple(id, name));
  }
}

/*
 * Loads the set of all spell ids and their corresponding names that are enchantments
 */
void load_enchantment_set() {
  std::string data_path = std::string("./gamedata/") + "effects_spells.tsv";
  std::ifstream data_file(data_path);

  if (!data_file.is_open()) {
    error_logger << "Failed to open basegame data file: " << data_path << strerror(errno) << "\n";
    return;
  }

  std::string line;
  // skip header line
  std::getline(data_file, line);
  while (std::getline(data_file, line)) {
    std::vector<std::string> tokens;
    std::size_t start = 0;
    std::size_t end = line.find("\t");
    while (end != std::string::npos) {
      tokens.push_back(line.substr(start, end - start));
      start = end + 1;
      end = line.find("\t", start);
      // we only need the first two tokens
      if (tokens.size() >= 2) {
        break;
      }
    }

    int spell_id = std::stoi(tokens[0]);
    int effect_type = std::stoi(tokens[1]);
    // add spell id to set if effect_type is in effect_enchantment_ids
    uint8_t special_type = UINT8_MAX; // nothing
    if (effect_enchantment_values.find(effect_type) != effect_enchantment_values.end()) {
      special_type = ENCHANTMENT_SPELL;
    }
    if (effect_summon_values.find(effect_type) != effect_summon_values.end()) {
      special_type = SUMMON_SPELL;
    }
    if (special_type == UINT8_MAX) {
      continue;
    }

    special_spell_ids[special_type].insert(spell_id);
    // get corresponding spell name and insert that as well
    std::string *spell_name = basegame_id_name_map[SPELLS].find_name_by_id(spell_id);
    if (spell_name != nullptr) {
      std::transform(spell_name->begin(), spell_name->end(), spell_name->begin(), ::tolower); // case insensitive insert for names
      special_spell_ids[special_type].insert(*spell_name);
    } else {
      error_logger << "Failed to find spell name for special_spell_ids[" << special_type << "] spell id: " << spell_id << " in basegame data, this should never happen." << "\n";
      exit(1);
    }
  }
}

/*
* Loads basegame data from dom6inspector tsv files into maps, should be in a subdirectory "gamedata" which should be at the same level as the executable
*/
void load_basegame_data() {
  // load basegame data from tsv files into maps, should be in a subdirectory "gamedata" which should be at the same level as the executable
  for (uint8_t type = 0; type <= MONTAG; type++) {
    if (type == SOUND || type == ENCHANTMENTS || type == POPTYPES || type == MONTAG) {
      continue; // these types don't have a basegame data file or need special handling
    }
    load_generic_basegame_data(type);
  }
  load_enchantment_set();
}
