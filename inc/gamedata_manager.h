#pragma once


#include "constants.h"
#include <variant>
#include <tuple>
#include <unordered_set>


void load_basegame_data();
bool is_special_spell(std::variant<int, std::string> id, uint8_t special_type);
void insert_special_spell(int spell_id, std::string spell_name, uint8_t special_type);
void insert_into_remap(std::variant<int, std::string> old_id, std::variant<int, std::string> new_id, uint8_t type);
std::tuple<int, std::string> *get_current_of_type(std::variant<int, std::string> old_id, uint8_t type);
