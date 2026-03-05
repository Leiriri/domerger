
#include <algorithm>
#include <tuple>
#include <string>
#include <unordered_map>


/*
 * Extracts the first integer from line that is separated from the command by a space or tab
 * @return A tuple Containing the extracted id as int, start index of id in line, end index of id in line
 */
std::tuple<int, std::size_t, std::size_t> get_id_from_line(const std::string &line);



/*
 * Extracts the first string from line that is separated from the command by a space or tab
 * Identifies strings by surrounding quotes, TODO: check if any commands allow using unquoted strings
 */
std::tuple<std::string, std::size_t, std::size_t> get_string_from_line(const std::string &line);



/*
  * Preprocesses line by trimming leading whitespace and removing UTF-8 BOM if present, also removes carriage returns and splits possible commands from comments
  * @return A tuple containing the preprocessed command part and the comment
  */
std::tuple<std::string, std::string> preprocess_line(const std::string &line);

/*
 * Outputs error messages if any id type exceeds the maximum id for that type, takes the current id table as an argument.
 */
void check_max_id_violation(int current_id_table[]);



/*
 * A helper class to maintain a mapping between names and ids for a certain type, as well as a mapping from id to the actual item for easy retrieval
 */
template <class Item> class DualMap {
private:
  std::unordered_map<std::string, int> name_to_id;
  std::unordered_map<int, std::string> id_to_name; // necessary to retrieve name key
  std::unordered_map<int, Item> id_to_item;

public:
  // map id to item and name to id
  void insert(int id, std::string name, Item item) {
    // change all strings to lowercase for case insensitive search as some commands like #selectspell are case insensitive
    std::transform(name.begin(), name.end(), name.begin(), ::tolower);
    id_to_item.insert({id, item});
    id_to_name.insert({id, name});
    name_to_id.insert({name, id});
  }

  Item *find_by_id(int id) {
    auto it = id_to_item.find(id);
    if (it == id_to_item.end()) {
      return nullptr;
    }
    return &it->second;
  }

  Item *find_by_name(std::string name) {
    // change all strings to lowercase for case insensitive search as some commands like #selectspell are case insensitive
    std::transform(name.begin(), name.end(), name.begin(), ::tolower);
    auto it = name_to_id.find(name);
    if (it == name_to_id.end()) {
      return nullptr;
    }
    return find_by_id(it->second);
  }

  int *find_id_by_name(std::string name) {
    // change all strings to lowercase for case insensitive search as some commands like #selectspell are case insensitive
    std::transform(name.begin(), name.end(), name.begin(), ::tolower);
    auto it = name_to_id.find(name);
    if (it == name_to_id.end()) {
      return nullptr;
    }
    return &it->second;
  }

  std::string *find_name_by_id(int id) {
    auto it = id_to_name.find(id);
    if (it == id_to_name.end()) {
      return nullptr;
    }
    return &it->second;
  }

  // only supports updating the target, not the name and id combination
  void update_item(int id, Item item) { id_to_item[id] = item; }
};
