#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>
#include <unordered_set>

// Types of ids
#define SOUND 0
#define WEAPONS 1
#define ARMOR 2
#define MONSTERS 3
#define NAMETYPES 4
#define SPELLS 5
#define ENCHANTMENTS 6
#define ITEMS 7
#define MAGIC_SITES 8
#define NATIONS 9
#define POPTYPES 10
#define MONTAG 11
#define EVENTS 12

#define ID_TYPE_COUNT 13


// Actions for commands
#define NEW_ID 0
#define REMAP_ID 1
#define AMBIGUOUS 2
#define POSSIBLE_MONTAG 3
#define DAMAGE 4
#define COPYSPR 5   // #copyspr needs to be handled differently because on its own it is impossible to guess if it refers to a monster or item
#define END 6     // used for #end, clears global flags as they will not be relevant outside of their blocks
#define NEWSPELL 7  // tracks the usage of #newspell, this is in order to check if a basegame spell is being remapped to another id, in this case it should be followed by some:\n#copyspell $some_base_id\n#end
#define NAME 8    // used to indicate that a #name command is being used which could be renaming a basegame target
#define MODNAME 9
#define RESOURCE_FILE 10 // resource file commands often target images in sub directories that are renamed to include the mod number for uniqueness
#define NEWITEM 11 // #newitem has no id so we replace it with #selectitem and a new id
#define ERA 12  // #era command, merge era that certain nations start in
#define IMG_FILE 13


// special case constants, for when a command should expand one of the earlier ones but do an additional special thing
#define BLOCK_START (1 << 0)  // marks a block that needs to be read ahead of time to set global flags
#define BLOCK_END (1 << 1)    // marks the end of such a block
#define COPYSPELL (1 << 2)    // used to indicate that the current command is #copyspell which needs special handling for basegame spell remapping
#define BASE_MODIFICATION_SELECTOR (2 << 3)   // command selects a basegame id which might be modified later on (e.g. remapping a name which we need to track)
#define EFFECT (2 << 4)  // used for the #effect command to set a global flag for enchantment tracking
#define SPELL_TRACKING (2 << 5) // used to indicate that the current command is selecting/copying a spell that might contain relevant information
#define FAVRIT_TYPE (2 << 6) // #favrit command can have either a spell or item as its target and has multiple int parameters before the target, though supposedly only takes names as arguments


#define NO_FLAGS 0    // for special because simple NOTHING will be true for all flags
#define NOTHING UINT8_MAX


// special spell types for enchantment/monster summon tracking
#define ENCHANTMENT_SPELL 0
#define SUMMON_SPELL 1

#define SPECIAL_SPELL_TYPE_COUNT 2


static const std::unordered_map<std::uint8_t, std::string> type_to_string = {
    {SOUND, "sound"},
    {WEAPONS, "weapons"},
    {ARMOR, "armor"},
    {MONSTERS, "monsters"},
    {NAMETYPES, "nametypes"},
    {SPELLS, "spells"},
    {ENCHANTMENTS, "enchantments"},
    {ITEMS, "items"},
    {MAGIC_SITES, "sites"},
    {NATIONS, "nations"},
    {POPTYPES, "poptypes"},
    {MONTAG, "montag"},
    {EVENTS, "events"}
};



// contains #effect ids that cause spells to be enchantments, need to also check against basegame spells because they are sometimes copied
// seems to be listed as 83 rather than 10083 in effects_spells.csv in dom6modinspector so simply check for entries with 81-86 (tough 86 does not seem to be used)
static const std::unordered_set<int> effect_enchantment_values = {
  42, 81, 82,  83,  84,  85,  86, // just how these are called in effects_spells.tsv
  10042,    // Seems to be enchantment specifically for Events
  10081, 10082, 10083, 10084, 10085, 10086  // https://btcallahan.github.io/dominions-modding-assistant/spell_helper.html has these as global/local enchantments
};

// contains #effect ids that cause spells to summon/polymorph/etc. creatures, need to also check against basegame spells because they are sometimes copied
// not entirely sure about 167 (lichcraft)
static const std::unordered_set<int> effect_summon_values = {
  1, 21, 26, 31, 37, 38, 43, 50, 54, 60, 62, 68, 89, 93, 119, 130, 137, 141, 167,
  10001, 10021, 10026, 10031, 10038, 10037, 10043, 10050, 10054, 10060, 10062, 10068, 10089, 10093, 10119, 10130, 10137, 10141, 10167
};
