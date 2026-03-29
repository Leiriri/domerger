#include "merge_rules.h"
#include "gamedata_manager.h"
#include "utils.h"
#include "logging.h"
#include <algorithm>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <queue>

namespace fs = std::filesystem;

int8_t era_merge_mode = -1; // tracks what era to merge all nations into, -1 = no merging, 0 = disable, 1 = EA, 2 = MA, 3 = LA
uint32_t mod_file_number = 1;   // tracks the current mod number being processed
uint32_t mod_dir_number = 1;    // tracks the current mod directory number being processed, this is to properly name and track image files

bool prereading_block = false; // tracks if we are currently in a block that needs to be read ahead for proper processing

int next_id_for_type_base[ID_TYPE_COUNT] = {
    149,  // sound
    882,  // Weapons, modding manual now lists the minimum as 1000 but the lower range is still sometimes used, current minimum is 882
    296,  // Armor, modding manual now lists the minimum as 400 but the lower range is still sometimes used, current minimum is 296
    4113, // Monsters, modding manual now lists the minimum as 5000 but the lower range is still sometimes used, current minimum is 4113, #newmonster minimum seems to be enforced tough and simply
          // replacing it with #selectmonster messes up sprites
    160,  // Nametypes
    1466, // Spells, modding manual now lists the minimum as 2000 but the lower range is still sometimes used, current minimum is 1466
    199,  // Enchantments
    529,  // Items, modding manual now lists the minimum as 700 but some mods still use lower id ranges, 529 is the current highest item id so this is
    1404, // Magic Sites, modding manual now lists the minimum as 1700 but some mods still use lower id ranges, 1404 is the current highest magic site id so this is fine
    134,  // Nations, modding manual now lists the minimum as 150 but some mods still use lower id ranges, 134 is the current highest nation id so this is fine
    124,  // Poptypes
    999,  // Montag
    299   // Event codes, these are actually negative ids but we just add the sign when adding them in
};

std::queue<int> new_id_queue[ID_TYPE_COUNT]; // tracks new ids assigned during preprocessing, when assigning an id found by preprocessing we pop from this queue, this makes sure ids are assigned in
                                             // contiguous order (e.g.: otherwise remapping a not already assigned #copyspell might break this order)
std::unordered_set<std::variant<int, std::string>> found_ids[ID_TYPE_COUNT]; // tracks found ids/name for all types during preprocessing, this helps prevent ids being assigned multiple times

int next_id_for_type[ID_TYPE_COUNT];
int next_id_for_type_start_of_file[ID_TYPE_COUNT];
std::vector<int> id_found_in_file[ID_TYPE_COUNT];    // tracks ids found in current file
std::vector<int> id_remapped_in_file[ID_TYPE_COUNT]; // tracks target ids corresponding to found ids
int last_claimed_id_for_type[ID_TYPE_COUNT]; // track the last id that was claimed for each type, some commands need the last claimed id to properly do some action (remapping a spell) so we track the last one gotten by claim_new_id
int8_t last_of_item_monster = -1;                    // tracks the last id selection type (ITEMS or MONSTERS)
int copy_base_spell = -3; // tracks if new spell is used to copy basegame spell, -3 indicates not tracking anything, -2 indicates #newspell being found, -1 indicates a name being used instead, any non
                          // negative id indicates the target of #copyspell after #newspell
std::string copy_base_spell_name = "";

// map that keeps track of which base game ids/names have been remapped to which new ids/names. Should take either std::string or int as key and return appropriate type as value
std::unordered_map<int, int> basegame_id_remap[ID_TYPE_COUNT];
std::unordered_map<std::string, std::string> basegame_name_remap[ID_TYPE_COUNT];

int8_t current_command_type = -1; // tracks the type of the current selecting command, -1 after #end
int current_id = INT32_MAX;
std::string current_name = "";

// enchantment/summon tracking
int8_t current_modification_is_special_spell = -1; // tracks if the current modification is for a special spell, -1 = no, 0 = enchantment, 1 = monster summon/polymorph

/*
 * We rename all resource files to include the mod directory, either directly or in the subdirectory. This prevents conflicts between mods
 */
void handle_resource(std::string &line, std::ofstream *output_file) {
  std::tuple<std::string, std::size_t, std::size_t> str_info = get_string_from_line(line);
  std::string name = std::get<0>(str_info);
  const std::size_t start = std::get<1>(str_info);
  if (start == std::string::npos) {
    error_logger << "#icon line is incorrect, exiting: " << line << "\n";
    exit(1);
  }
  // remove ./ at the start if present
  line.erase(start, line.substr(start, 2) == "./" ? 2 : 0);
  // if files are in the same directory as the dm file, we need to change the target to include the mod directory name to prevent conflicts, if it is in a subdirectory we can leave it as is
  std::size_t const end = line.find_first_of("\"", start); // needs to be recalculated because line may have changed

  std::string resource_path = line.substr(start, end - start);
  std::string mod_id = "mod" + std::to_string(mod_dir_number);
  std::size_t const first_separator = resource_path.find_first_of("/");
  if (first_separator != std::string::npos) {
    // resource is in a subdirectory, insert "_mod" + mod_dir_number before the first directory separator to prevent conflicts while still allowing subdirectories to be used as intended
    line.insert(start + first_separator, "_" + mod_id);
    (*output_file) << line;
    return;
  }
  // resource is in the root of the mod directory, insert "_mod" + mod_dir_number before the file extension to prevent conflicts
  std::size_t const end_of_filename = line.find_last_of(".", end);
  if (end_of_filename == std::string::npos || end_of_filename < start) {
    error_logger << "Icon line has no file extension, exiting: " << line;
    exit(1);
  }
  line.insert(end_of_filename, "_" + mod_id);
  (*output_file) << line;

}

void handle_era_line(std::string &line, std::ofstream *output_file) {
  std::tuple<int, std::size_t, std::size_t> id_info = get_id_from_line(line);
  int id = std::get<0>(id_info);
  // replace the id with the global era_merge_mode, do not replace if id is 0 (deactivated nation) because it is probably not a working nation
  if (era_merge_mode < 0 || id == 0) {
    // no merging, leave line as is
    (*output_file) << line;
    return;
  }
  std::size_t start = std::get<1>(id_info);
  std::size_t end = std::get<2>(id_info);
  line.replace(start, end - start, std::to_string(era_merge_mode));
  (*output_file) << line;
}

void handle_modname_line(std::string &line, std::ofstream *output_file) {
  std::tuple<std::string, std::size_t, std::size_t> str_info = get_string_from_line(line);
  std::string name = std::get<0>(str_info);
  std::size_t start = std::get<1>(str_info);
  if (start == std::string::npos) {
    error_logger << "#modname line is incorrect, exiting: " << line << "\n";
    exit(1);
  }
  std::string result = "Merged - " + std::format("{:03}", mod_file_number++) + " - " + name;
  line.replace(start, name.length(), result);
  (*output_file) << line;
}

/*
 * updates current_id and current_name to the current block's target
 */
void track_current_target(const std::string &line) {
  std::tuple<int, std::size_t, std::size_t> id_info = get_id_from_line(line);
  std::tuple<std::string, std::size_t, std::size_t> str_info = get_string_from_line(line);
  int id = std::get<0>(id_info);
  std::string name = std::get<0>(str_info);
  if (id != INT32_MAX) {
    current_id = id;
  }
  if (name != "") {
    current_name = name;
  }
}

/*
 * Checks if the current #select command is targeting a basegame id and sets the tracking status accordingly
 */
void track_base_modification_status(uint8_t type, const std::string &line) {
  int id = std::get<0>(get_id_from_line(line));
  std::string name = std::get<0>(get_string_from_line(line));
  if (id == INT32_MAX) {
    if (name == "") {
      error_logger << "Line: " << line << "\n";
      error_logger << "No id or name found for #select command, this would likely cause errors, exiting: " << line << "\n";
      exit(1);
    }
    // check if the name is that of a basegame entity
    std::tuple<int, std::string> *remapped_tuple = get_current_of_type(name, type);

    // check if this targets a basegame entity
    if (remapped_tuple == nullptr) {
      return; // name is not of a basegame entity
    }

    current_command_type = type;
    return;
  }
  std::tuple<int, std::string> *remapped_tuple = get_current_of_type(id, type);
  if (remapped_tuple == nullptr) {
    return; // id is not of a basegame entity, or is the same as the old one, nothing to do
  }
  current_command_type = type;
}

/*
 * Sets the type of the current #favrit command, as it can be either SPELLS or ITEMS
 */
void set_favrit_command_type(const std::string &line, uint8_t *type) {
  // while this shouldn't be allowed we cannot rule out the possibility
  warning_logger << "#favrit command found, if this uses an id target instead of a name remapping will not work though this is likely not allowed by dominions anyway" << "\n";
  std::tuple<std::string, std::size_t, std::size_t> str_info = get_string_from_line(line);
  std::string target = std::get<0>(str_info);
  if (target == "") {
    error_logger << "Line: " << line << "\n";
    error_logger << "No target found for #favrit command, this would likely cause errors, exiting: " << line << "\n";
    exit(1);
  }
  // check if the target is that of a basegame entity of type spell or item, as those are the only two types that #favrit can target and the only ones we track for this command
  std::tuple<int, std::string> *remapped_tuple_spell = get_current_of_type(target, SPELLS);
  std::tuple<int, std::string> *remapped_tuple_item = get_current_of_type(target, ITEMS);

  if (remapped_tuple_spell != nullptr) {
    *type = SPELLS;
  } else if (remapped_tuple_item != nullptr) {
    *type = ITEMS;
  } else {
    error_logger << "Line: " << line << "\n";
    error_logger << "Target found for #favrit command is not a basegame spell or item, this would likely cause errors, exiting: " << line << "\n";
    exit(1);
  }
}

/*
 * Sets global flags based on type of id modification
 * This is necessary for commands like #copyspr because it needs to know the last id selection command type
 */
void set_global_flags(uint8_t type, uint8_t action, uint8_t special, const std::string &line) {
  std::tuple<int, std::size_t, std::size_t> id_info = get_id_from_line(line);
  int id = std::get<0>(id_info);
  std::tuple<std::string, std::size_t, std::size_t> str_info = get_string_from_line(line);
  std::string name = std::get<0>(str_info);

  // only set these flags at block start, otherwise commands like #startitem can overwrite the type
  if (special & BLOCK_START) {
    if (type == ITEMS) {
      last_of_item_monster = ITEMS;
    } else if (type == MONSTERS) {
      last_of_item_monster = MONSTERS;
    }
  }

  // clear flags on end
  if (action == END && !prereading_block) {
    last_of_item_monster = -1;

    // insert remapping for basegame spell if applicable
    if (copy_base_spell >= 0) {
      debug_logger << "Inserting remapping for basegame spell id " << copy_base_spell << " to new id " << last_claimed_id_for_type[SPELLS] << "\n";
      insert_into_remap(copy_base_spell, last_claimed_id_for_type[SPELLS], SPELLS);
    } else if (copy_base_spell == -1) {
      debug_logger << "Inserting remapping for basegame spell name " << copy_base_spell_name << " to new id " << last_claimed_id_for_type[SPELLS] << "\n";
      insert_into_remap(copy_base_spell_name, last_claimed_id_for_type[SPELLS], SPELLS);
    }

    // reset command remap tracking
    current_command_type = -1;
    current_id = INT32_MAX;
    current_name = "";

    // reset enchantment tracking
    current_modification_is_special_spell = -1;
  }

  // handle newspell/copyspell tracking for remapping basegame spells
  if (action == NEWSPELL) {
    copy_base_spell = -2; // indicates that #newspell has been found
  } else if (special & COPYSPELL) {
    if (copy_base_spell == -2) {
      // if no id is found, it must use a name
      if (id == INT32_MAX) {
        copy_base_spell = -1; // indicates that a name is being used
        std::tuple<std::string, std::size_t, std::size_t> str_info = get_string_from_line(line);
        if (std::get<0>(str_info) == "") {
          error_logger << "Line: " << line << "\n";
          error_logger << "str_info: " << std::get<0>(str_info) << "\n";
          error_logger << "#copyspell command after #newspell has no valid id or name, exiting: " << line << "\n";
          exit(1);
        }
        copy_base_spell_name = std::get<0>(str_info);
      } else {
        copy_base_spell = id; // indicates that a basegame spell id is being used
      }
    }
  } else {
    // clear tracking if other command is found
    copy_base_spell = -3;
    copy_base_spell_name = "";
  }

  if (special & BASE_MODIFICATION_SELECTOR) {
    track_base_modification_status(type, line);
  }

  // check if a spell target is an enchantment/summon spell
  if (special & SPELL_TRACKING) {
    debug_logger << "Checking if copied base spell id " << id << " with name: " << name << " is a special spell" << "\n";
    if (id != INT32_MAX) {
      debug_logger << "Checking by id" << "\n";
      if (is_special_spell(id, ENCHANTMENT_SPELL)) {
        debug_logger << "Is enchantment spell" << "\n";
        current_modification_is_special_spell = ENCHANTMENT_SPELL;
      } else if (id != INT32_MAX && is_special_spell(id, SUMMON_SPELL)) {
        debug_logger << "Is summon spell" << "\n";
        current_modification_is_special_spell = SUMMON_SPELL;
      } else {
        debug_logger << "Is not special spell" << "\n";
        current_modification_is_special_spell = -1;
      }
    } else if (name != "") {
      debug_logger << "Checking by name" << "\n";
      if (is_special_spell(name, ENCHANTMENT_SPELL)) {
        debug_logger << "Is enchantment spell" << "\n";
        current_modification_is_special_spell = ENCHANTMENT_SPELL;
      } else if (is_special_spell(name, SUMMON_SPELL)) {
        debug_logger << "Is summon spell" << "\n";
        current_modification_is_special_spell = SUMMON_SPELL;
      } else {
        debug_logger << "Is not special spell" << "\n";
        current_modification_is_special_spell = -1;
      }
    } else {
      error_logger << "Line: " << line << "\n";
      error_logger << "#copyspell command has no valid id or name, exiting: " << line << "\n";
      exit(1);
    }
  }

  if (special & EFFECT) {
    // check if effect is in effect_enchantment_values set
    int effect = id; // just copying for clarity
    debug_logger << "Checking if effect id " << effect << " causes enchantment" << "\n";
    if (id != INT32_MAX && effect_enchantment_values.find(effect) != effect_enchantment_values.end()) {
      debug_logger << "Effect is enchantment" << "\n";
      current_modification_is_special_spell = ENCHANTMENT_SPELL;
    } else if (id != INT32_MAX && effect_summon_values.find(effect) != effect_summon_values.end()) {
      debug_logger << "Effect is summon" << "\n";
      current_modification_is_special_spell = SUMMON_SPELL;
    } else {
      debug_logger << "Effect is not enchantment" << "\n";
      current_modification_is_special_spell = -1;
    }
  }

  // set current_id and current_name only while prereading because later commands take precedence
  if (special & (BLOCK_START | BASE_MODIFICATION_SELECTOR)) {
    track_current_target(line);
  }
}

/*
 * Claims a new id for given type, tracks that the given id has been remapped to the new id
 * @param id The original id being remapped
 * @param type The type of id being remapped
 * @param pre_assigned If true, indicates that the id was pre-assigned during prescanning and a new id should be claimed from the queue
 */
int claim_new_id(int id, uint8_t type, bool pre_assigned = false, std::string line = "") {
  int new_id;
  // check if given id was found during prescanning and assign accordingly
  // we are not prescanning certain types like enchantments, because their order should not matter
  auto found_it = found_ids[type].find(abs(id));
  // only utilize pre_assigned if there is no id, enchantments and events are not scanned so we filter them out
  if (((pre_assigned && id == INT32_MAX) || found_it != found_ids[type].end()) && type != ENCHANTMENTS && type != EVENTS) {
    debug_logger << "Reusing previously found id " << abs(id) << " for type " << (int)type << "\n";
    // if something referenced an id before creation, it will already have been assigned and not get found here
    if (new_id_queue[type].empty()) {
      if (id == INT32_MAX) {
        warning_logger << "Command found that provides no id assigns new id even though all ids for type " << type_to_string.find(type)->second << " were already assigned, this likely just means that a different command referencing some at that point uninitialized id was used before this one." << "\n";
      } else {
      warning_logger << "Mod targets id outside base game range without initializing it for type " << type_to_string.find(type)->second << " with id " << id
                     << ", this is probably a modding error but should not break anything." << "\n";
      }
      new_id = next_id_for_type[type]++;
    } else {
      new_id = new_id_queue[type].front();
      new_id_queue[type].pop();
    }
    if (found_it != found_ids[type].end()) {
      found_ids[type].erase(found_it); // remove from set to prevent reusing again
    }

    id_found_in_file[type].push_back(abs(id));        // track that we used this id
    id_remapped_in_file[type].push_back(abs(new_id)); // track the new id assigned
    // if the id was negative return a negative new id
  } else {
    // should only happen if something references an id that is not defined in the mod, or it targets a basegame id that the modinspector has no entry for, but just in case we claim a new id
    if (type != ENCHANTMENTS && type != EVENTS) {
      warning_logger << "Claiming new id for id " << abs(id) << " of type " << type_to_string.find(type)->second
                     << " that was not found during prescanning, this means the id is neither a basegame id nor was defined in this mod file. This most likely means, that there is a typo or it was forgotten. This probably causes something to not work as intended by the mod author, but behave similar to the unmerged mod"
                     << "\n";
      warning_logger << "Line: " << line << "\n";
    }
    new_id = next_id_for_type[type]++;
    id_found_in_file[type].push_back(id);        // track that we used this id
    id_remapped_in_file[type].push_back(new_id); // track the new id assigned
  }
  // if the id was negative return a negative new id
  if (id < 0) {
    return -new_id;
  }
  last_claimed_id_for_type[type] = new_id;           // track last claimed id for this type
  return new_id;
}

/*
 * Gets the remapped id for a given id and type, properly returns negative ids when negative ones are provided (for montags other than #montag)
 * @return The remapped id, or INT32_MAX if not found
 */
int get_remapped_id(int id, uint8_t type) {
  auto it = std::find(id_found_in_file[type].begin(), id_found_in_file[type].end(), abs(id));
  if (it != id_found_in_file[type].end()) {
    int index = std::distance(id_found_in_file[type].begin(), it);
    if (id < 0) {
      return -id_remapped_in_file[type][index];
    }
    return id_remapped_in_file[type][index];
  } else {
    return INT32_MAX; // indicates not found
  }
}

/*
 * Replaces newspell with an equivalent selectspell command so we actually know which id is being used
 * Also tracks that a newspell was used so we can handle remapping of basegame spells properly
 */
void handle_newspell_command(std::ofstream *output_file) {
  int new_id = claim_new_id(INT32_MAX, SPELLS,
                            true); // We don't care for the specific new id here, INT32_MAX indicates no id found, if it turns out to be a remapping of a basegame spell it will be replaced elsewhere
  (*output_file) << "#selectspell " << new_id << "\n";
  // could potentially add a flag to track researchlevel being set, but seems mostly unnecessary
  (*output_file) << "#researchlevel 0"; // While #newspell does not seem to always require a researchlevel, this seems necessary with #selectspell targetting a new id, since this seems unnecessary
                                        // when #school -1 is used some modders skip it so we simply add it here
}

/*
 * Replaces newitem with an equivalent selectspell command so we actually know which id is being used
 * Also tracks that a newitem was used so we can handle remapping of basegame spells properly
 */
void handle_newitem_command(std::ofstream *output_file) {
  int new_id = claim_new_id(INT32_MAX, ITEMS,
                            true); // We don't care for the specific new id here, INT32_MAX indicates no id found, if it turns out to be a remapping of a basegame spell it will be replaced elsewhere
  (*output_file) << "#newitem " << new_id;
}

void handle_name_remapping(std::string &line, std::ofstream *output_file) {
  // track name this sets
  std::tuple<std::string, std::size_t, std::size_t> name_info = get_string_from_line(line);
  std::string name = std::get<0>(name_info);

  // if no basegame target is selected, we don't track a modification here
  if (current_command_type == -1) {
    current_name = name; // track current name for various other uses
    // no basegame target selected so name probably alters a new id, write command as is
    (*output_file) << line;
    return;
  }
  debug_logger << "Handling name remapping in line: " << line << "\n";
  std::size_t const start = std::get<1>(name_info);

  // if no name is found, something is broken
  if (start == std::string::npos) {
    error_logger << "No name found after #name command, exiting" << "\n";
    exit(1);
  }
  std::tuple<int, std::string> *remapped_tuple;
  if (current_id != INT32_MAX) {
    remapped_tuple = get_current_of_type(current_id, current_command_type);
  } else {
    remapped_tuple = get_current_of_type(current_name, current_command_type);
  }

  // check if name is in basegame and remap and update them if so
  if (remapped_tuple == nullptr) {
    error_logger << "No basegame entity found even though current_command_type is set, exiting" << "\n";
    exit(1);
  }

  // update map with new name
  if (current_id != INT32_MAX) {
    insert_into_remap(current_id, name, current_command_type);
  } else {
    insert_into_remap(current_name, name, current_command_type);
  }

  current_name = name; // update current name to the new name for future reference
  // write the line as is
  (*output_file) << line;
}

// TODO: can probably remove most of this as all #new commands should have been found during prescanning and therefore have an assigned id already
void remap_new_id_in_line(std::string &line, uint8_t type, std::ofstream *output_file, bool pre_assined = false) {
  debug_logger << "Assigning new id in line: " << line << "\n";
  // get next id for type
  // extract id
  std::tuple<int, std::size_t, std::size_t> id_info = get_id_from_line(line);
  std::size_t start = std::get<1>(id_info);
  std::size_t const end = std::get<2>(id_info);
  int id = std::get<0>(id_info);
  std::tuple<std::string, std::size_t, std::size_t> str_info = get_string_from_line(line);
  std::string name = std::get<0>(str_info);
  std::transform(name.begin(), name.end(), name.begin(), ::tolower);
  start =
      start == std::string::npos
          ? std::get<1>(str_info)
          : start; // if no id is found, we try to find a name, if there is also no name we will just add the new id at the end of the line so it doesn't really matter which start we use in that case

  // if no id/name is found, we just add the next id for type
  if (start == std::string::npos || found_ids[type].find(name) != found_ids[type].end()) {
    debug_logger << "No ID found in line." << "\n";
    int new_id = claim_new_id(id, type, pre_assined, line); // We don't care for the specific new id here, INT32_MAX indicates no id found,
                                                     // even if it exists if it was referenced by something else before (should be rare though)
    (*output_file) << line << " " << new_id;         // write line with new id appended
    return;
  }
  // check if id has already been assigned and simply replace in case it has
  int remapped_id = get_remapped_id(id, type);
  if (remapped_id != INT32_MAX) {
    debug_logger << "ID " << id << " already remapped to " << remapped_id << " in this file." << "\n";
    line.replace(start, end - start, std::to_string(remapped_id));
    (*output_file) << line;
    return;
  }

  int new_id = claim_new_id(id, type, pre_assined, line);
  // replace in line
  line.replace(start, end - start, std::to_string(new_id));
  debug_logger << "Assigned new ID " << new_id << " in line: " << line << "\n";
  // write changed line to output file
  (*output_file) << line;
}

/*
 * Replaces the integer id in line with the remapped id for the given type, modifies line in place
 */
int8_t remap_int_id_in_line(std::string &line, uint8_t type, std::ofstream *output_file) {
  debug_logger << "Remapping int id in line: " << line << "\n";
  std::tuple<int, std::size_t, std::size_t> id_info = get_id_from_line(line);
  int id = std::get<0>(id_info);
  std::size_t const start = std::get<1>(id_info);
  std::size_t const end = std::get<2>(id_info);

  if (start == std::string::npos) {
    return -1; // no id found
  }

  if (abs(id) < next_id_for_type_base[type]) {
    std::tuple<int, std::string> *remapped_id_name = get_current_of_type(id, type);
    if (remapped_id_name == nullptr) {
      // there are some values like 0 and sometimes -1 that are special cases but don't necessarily appear in the remap table
      if (type == SOUND || type == ENCHANTMENTS || type == POPTYPES || type == MONTAG || type == EVENTS || id <= 0) {
        // we don't track remapping for these types so we just write them as is
        debug_logger << "Int ID " << id << " is below base for type " << (int)type << " but remapping is not tracked for this type or it is 0, writing int id as is." << "\n";
        return 0;
      }
      error_logger << "ID target is below base for type but was not found in map, this suggests the tsv file is missing data or the mod is trying to modify a reserved but not present id." << "\n";
      error_logger << "This might cause issues in some cases." << "\n";
      error_logger << "Line: " << line << "\n";
      return 0;
    }
    // TODO: probably track these ids and report if multiple mods modify the same basegame id as that could cause conflicts but we can't automatically resolve them
    int remapped_id = std::get<0>(*remapped_id_name);
    // check if the remapped id was only reamapped in this file, because in that case the mod really wants to change the contents at the basegame id rather than the remapped one
    if (remapped_id >= next_id_for_type_start_of_file[type]) {
      // write line as is
      debug_logger << "ID " << id << " is below base for type " << (int)type << " but was remapped in this file, not remapping." << "\n";
      return 0;
    }
    debug_logger << "ID " << id << " is a basegame id remapped to " << remapped_id << "." << "\n";
    // replace in line
    line.replace(start, end - start, std::to_string(remapped_id));
    return 0;
  }

  // get remapped id
  int remapped_id = get_remapped_id(id, type);
  // if not found, treat as new id
  if (remapped_id == INT32_MAX) {
    remap_new_id_in_line(line, type, output_file);
    return 1;
  }
  debug_logger << "ID " << id << " already remapped to " << remapped_id << " in this file." << "\n";
  // replace in line
  line.replace(start, end - start, std::to_string(remapped_id));
  return 0;
}

int8_t remap_string_id_in_line(std::string &line, uint8_t type) {
  debug_logger << "Remapping string id in line: " << line << "\n";
  std::tuple<std::string, std::size_t, std::size_t> str_info = get_string_from_line(line);
  std::string name = std::get<0>(str_info);
  std::size_t const start = std::get<1>(str_info);
  std::size_t const end = std::get<2>(str_info);

  if (start == std::string::npos) {
    return -1; // no name found
  }

  std::tuple<int, std::string> *remapped_id_name = get_current_of_type(name, type);
  if (remapped_id_name == nullptr) {
    debug_logger << "Name " << name << " is not a basegame name, writing name as is." << "\n";
    return 0;
  }

  std::string remapped_name = std::get<1>(*remapped_id_name);
  debug_logger << "Name " << name << " is a basegame name remapped to name " << remapped_name << "." << "\n";
  // replace in line
  line.replace(start, end - start, remapped_name);
  return 0;
}

void remap_id_in_line(std::string &line, uint8_t type, std::ofstream *output_file) {
  int8_t string_remap_return = remap_string_id_in_line(line, type); // remap this first so it stays in the line even if a new int id is assigned
  int8_t int_remap_return = remap_int_id_in_line(line, type, output_file);
  if (int_remap_return == -1 && string_remap_return == -1) {
    error_logger << "No id or name found to remap in line, just writing as is: " << line << "\n";
  }

  // only write line if it was not already written in remap_new_id_in_line
  if (int_remap_return != 1) {
    (*output_file) << line;
  }
}

/*
 * Sets a type for #copyspr based on last id selection command and then remaps the id in line accordingly
 */
void handle_copyspr_command(std::string &line, std::ofstream *output_file) {
  if (last_of_item_monster == ITEMS) {
    debug_logger << "Handling #copyspr as item." << "\n";
    remap_id_in_line(line, ITEMS, output_file);
  } else if (last_of_item_monster == MONSTERS) {
    debug_logger << "Handling #copyspr as monster." << "\n";
    remap_id_in_line(line, MONSTERS, output_file);
  } else {
    error_logger << "#copyspr command found without preceding id selection command, cannot determine type, exiting: " << line << "\n";
    exit(1);
  }
}

/*
 * Several commands that take a monster id can also take a montag if the id is negative, as this is not the case for all monster id commands we handle this separately
 */
void handle_possible_montag(std::string &line, uint8_t type, std::ofstream *output_file) {
  // check if line contains a negative id
  std::tuple<int, std::size_t, std::size_t> id_info = get_id_from_line(line);
  int id = std::get<0>(id_info);
  std::size_t const start = std::get<1>(id_info);
  // if no id is found, we can just write the line as is, probably uses a name
  if (start == std::string::npos) {
    debug_logger << "No ID found in line." << "\n";
    (*output_file) << line;
    return;
  }

  // remap as montag if negative
  if (id < 0) {
    type = MONTAG;
    debug_logger << "ID is negative, handling as montag." << "\n";
  } else {
    debug_logger << "ID is non-negative, handling as monster." << "\n";
  }
  remap_id_in_line(line, type, output_file);
}

/*
 *  Decides whether to use remap_new_id_in_line or remap_id_in_line for ambiguous commands.
 */
void decide_ambiguous_command(std::string &line, uint8_t type, std::ofstream *output_file) {
  // all ambiguous commands take ids in all cases
  // extract id
  std::tuple<int, std::size_t, std::size_t> id_info = get_id_from_line(line);
  int id = std::get<0>(id_info);
  if (id == INT32_MAX) {
    std::tuple<std::string, std::size_t, std::size_t> str_info = get_string_from_line(line);
    std::string name = std::get<0>(str_info);
    if (name != "") {
      // this is probably a remap rather than a new id, at least most ambiguous commands don't allow names for new ids
      debug_logger << "Ambiguous command got a name argument: " << name << ", treating as remap." << "\n";
      remap_id_in_line(line, type, output_file);
      return;
    }

    error_logger << "Ambiguous command got neither id nor name argument, this should be illegal, exiting." << "\n";
    exit(1);
  }

  // check if this modifies a basegame id
  if (abs(id) < next_id_for_type_base[type]) {
    // because this could be a remapped base id, we need to remap it properly
    remap_id_in_line(line, type, output_file);
    return;
  }

  // check if already present in id_found_in_file
  auto it = std::find(id_found_in_file[type].begin(), id_found_in_file[type].end(), id); // this should always be present at this point
  if (it != id_found_in_file[type].end()) {
    // already remapped
    remap_id_in_line(line, type, output_file);
  } else {
    // new id
    remap_new_id_in_line(line, type, output_file, true);
  }
}

void handle_damage_command(std::string &line, std::ofstream *output_file) {
  debug_logger << "Handling #damage command in line: " << line << "\n";
  // only getting id to check if it overlaps with monster/montag ids, not strictly necessary
  std::tuple<int, std::size_t, std::size_t> id_info = get_id_from_line(line);
  int id = std::get<0>(id_info);
  int8_t type;
  if (id < 0) {
    type = MONTAG;
  } else {
    type = MONSTERS;
  }
  int remapped_id;
  if (id == INT32_MAX) {
    remapped_id = INT32_MAX;
  } else {
    remapped_id = get_remapped_id(id, type);
  }

  if (current_modification_is_special_spell == ENCHANTMENT_SPELL) {
    // handle as enchantment
    debug_logger << "Handling #damage as enchantment." << "\n";
    // insert enchantment id and name
    insert_special_spell(current_id, current_name, ENCHANTMENT_SPELL);
    remap_id_in_line(line, ENCHANTMENTS, output_file);
  } else if (current_modification_is_special_spell == SUMMON_SPELL) { // definitely a summon spell
    // handle as summon
    debug_logger << "Handling #damage as summon." << "\n";
    // insert summon id and name
    insert_special_spell(current_id, current_name, SUMMON_SPELL);
    handle_possible_montag(line, MONSTERS, output_file);

  } else if (remapped_id != INT32_MAX) {
    // std::cout << "Handling #damage as monster." << "\n";
    warning_logger << "#damage value overlaps with monster id, this might be a coincidence or missed case, if it is the latter, please report this. Enable 'DEBUG' logging for more information"
                   << "\n";
    warning_logger << "Line: " << line << "\n";
    (*output_file) << line;
  } else {
    // just write line as is
    (*output_file) << line;
  }
}

void handle_action(std::string &trimmed_line, const std::string &command, uint8_t id_type, uint8_t action, std::ofstream &target_file) {
  switch (action) {
  case NEW_ID:
    remap_new_id_in_line(trimmed_line, id_type, &target_file, true);
    break;
  case REMAP_ID:
    remap_id_in_line(trimmed_line, id_type, &target_file);
    break;
  case AMBIGUOUS:
    decide_ambiguous_command(trimmed_line, id_type, &target_file);
    break;
  case POSSIBLE_MONTAG:
    handle_possible_montag(trimmed_line, id_type, &target_file);
    break;
  case DAMAGE:
    handle_damage_command(trimmed_line, &target_file);
    break;
  case COPYSPR:
    handle_copyspr_command(trimmed_line, &target_file);
    break;
  case END:
    // just write line as is, global flags already handled
    target_file << trimmed_line;
    break;
  case NEWSPELL:
    handle_newspell_command(&target_file);
    break;
  case NAME:
    handle_name_remapping(trimmed_line, &target_file);
    break;
  case MODNAME:
    handle_modname_line(trimmed_line, &target_file);
    break;
  case RESOURCE_FILE:
    handle_resource(trimmed_line, &target_file);
    break;
  case NEWITEM:
    handle_newitem_command(&target_file);
    break;
  case ERA:
    handle_era_line(trimmed_line, &target_file);
    break;
  case NOTHING:
    // just write line as is
    target_file << trimmed_line;
    break;
  default:
    error_logger << "Unknown action for command: " << command << "\n";
    exit(1);
  }
}

void handle_dm_file(const std::string &file_path, const fs::path &target_path) {
  debug_logger << "Handling DM file: " << file_path << "\n";
  std::ifstream dm_file(file_path, std::ios::in | std::ios::binary);
  if (!dm_file.is_open()) {
    error_logger << "Failed to open DM file: " << file_path << strerror(errno) << "\n";
    return;
  }
  std::ofstream target_file(target_path);
  if (!target_file.is_open()) {
    error_logger << "Failed to open target DM file: " << target_path << strerror(errno) << "\n";
    return;
  }
  std::string line;
  std::streampos current_pos;
  while (std::getline(dm_file, line)) {
    debug_logger << "Processing line: " << line << "\n";
    // preprocess line
    std::tuple<std::string, std::string> preprocessed = preprocess_line(line);
    std::string trimmed_line = std::get<0>(preprocessed);
    std::string comment = std::get<1>(preprocessed);
    debug_logger << "Trimmed line: " << trimmed_line << "\n";

    // get command
    std::size_t command_end = trimmed_line.find_first_of(" \t\n");
    std::string command;
    command = trimmed_line.substr(0, command_end);
    auto it = command_map.find(command);

    // in some cases we need to read some lines ahead to properly set global flags, for example under certain circumstances we have no way of knowing if a spell is a summoning spell without reading
    // the effect line and can't remap the #damage line properly
    if (it != command_map.end()) {
      uint8_t id_type = std::get<0>(it->second);
      uint8_t action = std::get<1>(it->second);
      uint8_t special_case = std::get<2>(it->second);

      switch (special_case & (BLOCK_START | BLOCK_END)) {
      case BLOCK_START:
        // if we have just finished prereading a block, the variable is still set and we simply continue to write the line as normal
        if (!prereading_block) {
          debug_logger << "Starting block handling for command: " << command << "\n";
          current_pos = dm_file.tellg();
          if (!line.empty() && line.back() == '\r') {
              // File uses \r\n line endings
              current_pos -= std::streamoff(line.length() + 2);
          } else {
              // File uses \n line endings
              current_pos -= std::streamoff(line.length() + 1);
          }
          prereading_block = true;
          debug_logger << "Current position saved: " << current_pos << "\n";
        } else {
          debug_logger << "Finished prereading block, continuing processing." << "\n";
          prereading_block = false;
        }
        break;
      case BLOCK_END:
        if (prereading_block) {
          dm_file.seekg(current_pos);
          debug_logger << "Returning to saved position: " << dm_file.tellg() << "\n";
          continue; // reprocess line now that block has been preread
        }
        break;
      case NO_FLAGS:
        // nothing to do
        break;
      default:
        error_logger << "Unknown block handling for command: " << command << "\n";
        exit(1);
      }

      // #favrit command needs special handling to determine type
      if (special_case & FAVRIT_TYPE) {
        set_favrit_command_type(trimmed_line, &id_type);
      }

      debug_logger << "Command: " << command << ", Type: " << (int)id_type << ", Action: " << (int)action << ", Special Case: " << (int)special_case << "\n";

      // only set global flags while prereading blocks (this includes the #end of blocks)
      if (prereading_block || (special_case & BLOCK_END)) {
        set_global_flags(id_type, action, special_case, trimmed_line);
      }

      if (prereading_block) {
        debug_logger << "Currently prereading block, skipping processing for line: " << trimmed_line << "\n";
        continue;
      }
      handle_action(trimmed_line, command, id_type, action, target_file);
    } else if (prereading_block) {
      debug_logger << "Currently prereading block, skipping processing for line: " << trimmed_line << " as it does not contain relevant information." << "\n";
      continue;
    } else {
      // if we reach here, just copy the line as is
      target_file << trimmed_line;
    }

    // reappend comment if present
    if (!comment.empty()) {
      target_file << " " << comment;
    }
    target_file << "\n";
  }
  // write a single empty line at the end of the file, since the last line is not read by dominions and the author might have forgotten to add it in
  target_file << "\n"
                 "-- End of file added by merger tool";
  dm_file.close();
  target_file.close();
}

void prescan_handle_command(std::string &line, std::tuple<int, std::size_t, std::size_t> id_info, const std::string &command) {
    int id = std::get<0>(id_info);
    uint8_t id_type = std::get<0>(command_map.find(command)->second);
    debug_logger << "Prescan found command: " << command << "\n";
    // extract id
    std::tuple<std::string, std::size_t, std::size_t> str_info = get_string_from_line(line);
    std::string name = std::get<0>(str_info);
    if (id != INT32_MAX) {
      // check if this id targets the basegame or has already been found
      if (abs(id) >= next_id_for_type_base[id_type] && found_ids[id_type].find(abs(id)) == found_ids[id_type].end()) {
        // new id
        debug_logger << "Prescan treating command as NEW_ID for id: " << id << "\n";
        int new_id = next_id_for_type[id_type]++;
        debug_logger << "Prescan assigned new ID: " << new_id << "\n";
        new_id_queue[id_type].push(new_id);
        found_ids[id_type].insert(abs(id));
      } else {
        debug_logger << "Prescan treating command as REMAP_ID for id: " << id << ", nothing to be done here.\n";
      }
    } else if (name != "") {
      // check if this name targets the basegame or has already been found
      // transform name to lowercase to since the game is case insensitive in this regard
      std::transform(name.begin(), name.end(), name.begin(), ::tolower);
      if (found_ids[id_type].find(name) != found_ids[id_type].end() && get_current_of_type(name, id_type) != nullptr) {
        // new id
        debug_logger << "Prescan treating command as NEW_ID for name: " << name << "\n";
        int new_id = next_id_for_type[id_type]++;
        debug_logger << "Prescan assigned new ID: " << new_id << "\n";
        new_id_queue[id_type].push(new_id);
        found_ids[id_type].insert(name);
      }
    } else {
      debug_logger << "Prescan treating command as NEW_ID with no id or name, just assigning new ID without tracking: " << line << "\n";
      int new_id = next_id_for_type[id_type]++;
      debug_logger << "Prescan assigned new ID: " << new_id << "\n";
      new_id_queue[id_type].push(new_id);
    }
}

/*
 *  Prescans a mod file to track which new ids are used so we can properly remap them later.
 *  Keeps a queue so that all #new... commands can simply pop from it to get their assigned id.
 */
void prescan_mod_file(const std::string &file_path) {
  // check if all new id queues are empty
  for (int i = 0; i < ID_TYPE_COUNT; ++i) {
    if (new_id_queue[i].size() != 0) {
      error_logger << "New id queue for type " << type_to_string.find(i)->second << " is not empty before prescan, something went wrong." << "\n";
      error_logger << "Queue size: " << new_id_queue[i].size() << "\n";
      error_logger << "Next ID is: " << new_id_queue[i].front() << "\n";
      exit(1);
    }
  }

  debug_logger << "Prescanning DM file: " << file_path << "\n";
  std::ifstream dm_file(file_path);
  if (!dm_file.is_open()) {
    error_logger << "Failed to open DM file for prescan: " << file_path << strerror(errno) << "\n";
    return;
  }
  std::string line;
  while (std::getline(dm_file, line)) {
    debug_logger << "Prescan processing line: " << line << "\n";
    // preprocess line
    std::tuple<std::string, std::string> preprocessed = preprocess_line(line);
    std::string trimmed_line = std::get<0>(preprocessed);
    debug_logger << "Prescan trimmed line: " << trimmed_line << "\n";

    // get command
    std::size_t command_end = trimmed_line.find_first_of(" \t\n");
    std::string command;
    command = trimmed_line.substr(0, command_end);
    auto it = command_map.find(command);

    if (it != command_map.end()) {
      uint8_t action = std::get<1>(it->second);
      std::tuple<int, std::size_t, std::size_t> id_info = get_id_from_line(trimmed_line);

      switch (action) {
      case NEWSPELL:
      case NEWITEM: {
        // both these cases can not take an id as argument but will simply ignore it if present, as this number can be below the minimum id for the type, we just remove it
        id_info = std::make_tuple(INT32_MAX, std::string::npos, std::string::npos);
        prescan_handle_command(trimmed_line, id_info, command);
        break;
      }
      case NEW_ID:
      case AMBIGUOUS: {
        prescan_handle_command(trimmed_line, id_info, command);
        break;
      }
      default: {
        // nothing to do
        break;
      }
      }
    }
  }
  dm_file.close();
  for (int i = 0; i < ID_TYPE_COUNT; ++i) {
    debug_logger << "Prescan found " << new_id_queue[i].size() << " new ids for type " << type_to_string.find(i)->second << "\n";
  }
}

/*
 * Copies the contents of a single mod directory to the target path.
 * @param mod_dir The path to the mod directory to copy from.
 * @param target_path The path to the target directory to copy to.
 * all types
 */
void copy_single_mod(const std::string &mod_dir, const std::string &target_path, std::vector<std::string> &dmfile_process_order) {
  // Clear the vectors tracking ids for this mod
  for (int i = 0; i < ID_TYPE_COUNT; ++i) {
    id_found_in_file[i].clear();
    id_remapped_in_file[i].clear();
  }

  // we need to process the files in lexicographical order to ensure a consistent processing order
  std::vector<fs::directory_entry> entries;
  for (const fs::directory_entry &entry : fs::directory_iterator(mod_dir)) {
    entries.push_back(entry);
  }
  std::sort(entries.begin(), entries.end());

  // Copy the contents of mod_dir to target_path
  for (const fs::directory_entry &entry : entries) {
    // handle files differently based on their type

    // copy directories as they contain the sprites
    if (entry.is_directory()) {
      debug_logger << "Copying: " << entry.path() << "\n";
      fs::copy(entry.path(), fs::path(target_path) / (entry.path().filename().stem().string() + "_mod" + std::to_string(mod_dir_number) + entry.path().extension().string()), fs::copy_options::recursive | fs::copy_options::skip_existing);
      continue;
    }
    // dm files require specific handling including id remapping
    if (std::string(entry.path().extension().string()) == ".dm") {
      info_logger << "Processing: " << entry.path() << "\n";
      dmfile_process_order.push_back(entry.path().string()); // track processing order
      prescan_mod_file(entry.path().string());               // prescan to track new ids
      handle_dm_file(entry.path().string(), fs::path(target_path) / entry.path().filename());
      continue;
    }
    std::string extension = entry.path().extension().string();
    if (resource_file_types.find(extension) != resource_file_types.end()) {
      debug_logger << "Copying: " << entry.path() << "\n";
      // copy because it is probably the ws/ingame icon, "_mod_" + mod_number before extension so it can be uniquely identified
      fs::copy(entry.path(), fs::path(target_path) / (entry.path().filename().stem().string() + "_mod" + std::to_string(mod_dir_number) + entry.path().extension().string()),
               fs::copy_options::skip_existing);
      continue;
    }
    debug_logger << "Skipping file: " << entry.path() << ", as it is not important" << "\n";
  }
  mod_dir_number++; // increment mod number for unique icon naming
  check_max_id_violation(next_id_for_type);
}

void copy_all_mods(Arg_Parser parser, const std::vector<std::string> &mod_dirs, const std::string &target_path, std::vector<std::string> &dmfile_process_order) {
  era_merge_mode = stoi(parser.get_option("era-merge", parser.get_option("e", "-1")));
  if (era_merge_mode > 3) {
    error_logger << "--era-merge option has invalid value." << "\n";
    exit(1);
  }
  load_basegame_data();

  fs::create_directories(target_path);

  // set next_id_for_type to modmanual minimums, while some mods utilize lower ranges, they sometimes break things so we only allow target ids above the modmanual minimums
  next_id_for_type[SOUND] = 149, next_id_for_type[WEAPONS] = 999, next_id_for_type[ARMOR] = 399, next_id_for_type[MONSTERS] = 5000, next_id_for_type[NAMETYPES] = 169, next_id_for_type[SPELLS] = 1999,
  next_id_for_type[ENCHANTMENTS] = 200, next_id_for_type[ITEMS] = 699, next_id_for_type[MAGIC_SITES] = 1699, next_id_for_type[NATIONS] = 149, next_id_for_type[POPTYPES] = 124,
  next_id_for_type[MONTAG] = 999, next_id_for_type[EVENTS] = 299;

  for (const auto &mod_dir : mod_dirs) {
    // set next_id_for_type_start_of_file to current values, so we can remap properly
    std::memcpy(next_id_for_type_start_of_file, next_id_for_type, sizeof(next_id_for_type));
    copy_single_mod(mod_dir, target_path, dmfile_process_order);
  }
}
