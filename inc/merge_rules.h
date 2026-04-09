#pragma once

#include "constants.h"
#include "arg_parser.h"
#include <string>
#include <unordered_map>
#include <vector>

void copy_all_mods(Arg_Parser parser, const std::vector<std::string> &mod_dirs, const std::string &target_path, std::vector<std::string> &dmfile_process_order);

// TODO: #selectbless may needs to be handled if any bless is renamed. This would also require creating a list of bless names for basegame remapping as this is not in the mod inspector yet

// TODO: add BLOCK_START to all relevant commands


// maps a command string to a tuple of (type, action, special, block_handling)
static const std::unordered_map<std::string, std::tuple<uint8_t, uint8_t, uint8_t>> command_map = {
    {"#selectsound", std::make_tuple(SOUND, AMBIGUOUS, BLOCK_START)},
    {"#selectnametype", std::make_tuple(NAMETYPES, AMBIGUOUS, BLOCK_START)},
    {"#selectnation", std::make_tuple(NATIONS, AMBIGUOUS, BASE_MODIFICATION_SELECTOR | BLOCK_START)},
    {"#selectpoptype", std::make_tuple(POPTYPES, AMBIGUOUS, BLOCK_START)},
    {"#selectspell", std::make_tuple(SPELLS, AMBIGUOUS, BASE_MODIFICATION_SELECTOR | SPELL_TRACKING | BLOCK_START)},
    {"#selectweapon", std::make_tuple(WEAPONS, AMBIGUOUS, BASE_MODIFICATION_SELECTOR | BLOCK_START)},
    {"#selectarmor", std::make_tuple(ARMOR, AMBIGUOUS, BASE_MODIFICATION_SELECTOR | BLOCK_START)},
    {"#selectsite", std::make_tuple(MAGIC_SITES, AMBIGUOUS, BASE_MODIFICATION_SELECTOR | BLOCK_START)},
    {"#selectitem", std::make_tuple(ITEMS, AMBIGUOUS, BASE_MODIFICATION_SELECTOR | BLOCK_START)},
    {"#selectevent", std::make_tuple(NOTHING, NOTHING, BLOCK_START)}, // TODO: is selectevent the same pool as event ids?
    {"#montag", std::make_tuple(MONTAG, AMBIGUOUS, NO_FLAGS)},

    {"#newweapon", std::make_tuple(WEAPONS, NEW_ID, BLOCK_START)},
    {"#newarmor", std::make_tuple(ARMOR, NEW_ID, BLOCK_START)},
    {"#newmonster", std::make_tuple(MONSTERS, NEW_ID, BLOCK_START)},
    {"#newsite", std::make_tuple(MAGIC_SITES, NEW_ID, BLOCK_START)},
    {"#newnation", std::make_tuple(NATIONS, NEW_ID, BLOCK_START)},
    {"#newspell", std::make_tuple(SPELLS, NEWSPELL, BLOCK_START)},
    {"#newitem", std::make_tuple(ITEMS, NEWITEM, BLOCK_START)},

    {"#newtemplate", std::make_tuple(NATIONS, REMAP_ID, BLOCK_START)},
    {"#newmerc", std::make_tuple(NOTHING, NOTHING, BLOCK_START)},   // has no id or anything but starts a block

    {"#selectmonster", std::make_tuple(MONSTERS, AMBIGUOUS, BASE_MODIFICATION_SELECTOR | BLOCK_START)},   // set as ambiguous because it can techinically create new ids tough replacing all #newmonster with #selectmonster breaks sprites for some reason
    {"#copyweapon", std::make_tuple(WEAPONS, REMAP_ID, NO_FLAGS)},
    {"#sound", std::make_tuple(SOUND, REMAP_ID, NO_FLAGS)},
    {"#secondaryeffect", std::make_tuple(WEAPONS, REMAP_ID, NO_FLAGS)},
    {"#secondaryeffectalways", std::make_tuple(WEAPONS, REMAP_ID, NO_FLAGS)},
    {"#copyarmor", std::make_tuple(ARMOR, REMAP_ID, NO_FLAGS)},
    {"#copystats", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#godsite", std::make_tuple(MAGIC_SITES, REMAP_ID, NO_FLAGS)},
    {"#monpresentrec", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)},
    {"#ownsmonrec", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#weapon", std::make_tuple(WEAPONS, REMAP_ID, NO_FLAGS)},
    {"#armor", std::make_tuple(ARMOR, REMAP_ID, NO_FLAGS)},
    {"#startitem", std::make_tuple(ITEMS, REMAP_ID, NO_FLAGS)},
    {"#bugshape", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#buguwshape", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#bugswarmshape", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#bugswarmuwshape", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#raiseshape", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)}, // can be montag
    {"#shapechange", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#prophetshape", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)},   // can be montag
    {"#firstshape", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)},     // can be montag
    {"#secondshape", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)},    // can be montag
    {"#secondtmpshape", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)}, // can be montag
    {"#forestshape", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#plainshape", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#foreignshape", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#homeshape", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#domshape", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#notdomshape", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#springshape", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#summershape", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#autumnshape", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#wintershape", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#xpshapemon", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)}, // can be montag
    {"#landshape", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)},
    {"#watershape", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)},
    {"#twiceborn", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#lich", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#battleshape", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#worldshape", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#animated", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#domsummon", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)},
    {"#domsummon2", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)},
    {"#domsummon20", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)},
    {"#raredomsummon", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)},
    {"#templetrainer", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)},   // Have found no example of this using montag, but it just seem fitting
    {"#makemonsters1", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)},
    {"#makemonsters2", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)},
    {"#makemonsters3", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)},
    {"#makemonsters4", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)},
    {"#makemonsters5", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)},
    {"#summon1", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)},
    {"#summon2", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)},
    {"#summon3", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)},
    {"#summon4", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)},
    {"#summon5", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)},
    {"#battlesum1", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)},
    {"#battlesum2", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)},
    {"#battlesum3", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)},
    {"#battlesum4", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)},
    {"#battlesum5", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)},
    {"#battlesum1d2", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)},
    {"#battlesum1d3", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)},
    {"#battlesumwarm", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)},
    {"#batstartsum1", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)},
    {"#batstartsum2", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)},
    {"#batstartsum3", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)},
    {"#batstartsum4", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)},
    {"#batstartsum5", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)},
    {"#batstartsum1d3", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)},
    {"#batstartsum1d6", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)},
    {"#batstartsum2d6", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)},
    {"#batstartsum3d6", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)},
    {"#batstartsum4d6", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)},
    {"#batstartsum5d6", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)},
    {"#batstartsum6d6", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)},
    {"#batstartsum7d6", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)},
    {"#batstartsum8d6", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)},
    {"#batstartsum9d6", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)},
    {"#slaver", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#mountmnr", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#coridermnr", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#copysite", std::make_tuple(MAGIC_SITES, REMAP_ID, NO_FLAGS)},
    {"#homemon", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#homecom", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#mon", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)},
    {"#com", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)},
    {"#nat", std::make_tuple(NATIONS, REMAP_ID, NO_FLAGS)},
    {"#natmon", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#natcom", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#summon", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#summonlvl2", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#summonlvl3", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#summonlvl4", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#wallcom", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#wallunit", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#uwwallunit", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#uwwallcom", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#defcom", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#defunit", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)},
    {"#startcom", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#addforeignunit", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#addforeigncom", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#plainrec", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#forestrec", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#mountainrec", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#swamprec", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#wasterec", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#farmrec", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#caverec", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#driprec", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#coastrec", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#searec", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#deeprec", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#kelprec", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#plainfortrec", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#forestfortrec", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#mountainfortrec", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#swampfortrec", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#wastefortrec", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#farmfortrec", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#cavefortrec", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#dripfortrec", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#coastfortrec", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#seafortrec", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#deepfortrec", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#kelpfortrec", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#foreignfortrec", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#startscout", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#plaincom", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#forestcom", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#mountaincom", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#swampcom", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#wastecom", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#farmcom", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#cavecom", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#dripcom", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#coastcom", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#seacom", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#deepcom", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#kelpcom", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#plainfortcom", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#forestfortcom", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#mountainfortcom", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#swampfortcom", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#wastefortcom", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#farmfortcom", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#cavefortcom", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#dripfortcom", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#coastfortcom", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#seafortcom", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#deepfortcom", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#kelpfortcom", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#foreignfortcom", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#startunittype1", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#startunittype2", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#addrecunit", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#addreccom", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#uwrec", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#uwcom", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#landrec", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#landcom", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#hero1", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#hero2", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#hero3", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#hero4", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#hero5", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#hero6", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#hero7", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#hero8", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#hero9", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#hero10", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#multihero1", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#multihero2", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#multihero3", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#multihero4", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#multihero5", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#multihero6", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#multihero7", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#defcom1", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)},
    {"#defcom2", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)},
    {"#defunit1", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)},
    {"#defunit1b", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)},
    {"#defunit1c", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)},
    {"#defunit1d", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)},
    {"#defunit2", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)},
    {"#defunit2b", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)},
    {"#guardcom", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#guardunit", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#foreignwallcom", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#foreignwallunit", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#foreignguardcom", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#foreignguardunit", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#addgod", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#delgod", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#cheapgod20", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#cheapgod40", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#guardspirit", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)}, // can be montag
    {"#copyspell", std::make_tuple(SPELLS, REMAP_ID, COPYSPELL | SPELL_TRACKING)},          // can be used to remap spells to different ids and needs special handling
    {"#nextspell", std::make_tuple(SPELLS, REMAP_ID, NO_FLAGS)},
    {"#strikesound", std::make_tuple(SOUND, REMAP_ID, NO_FLAGS)},
    {"#onlyatsite", std::make_tuple(MAGIC_SITES, REMAP_ID, NO_FLAGS)},
    {"#onlysitedst", std::make_tuple(MAGIC_SITES, REMAP_ID, NO_FLAGS)},
    {"#restricted", std::make_tuple(NATIONS, REMAP_ID, NO_FLAGS)},
    {"#notfornation", std::make_tuple(NATIONS, REMAP_ID, NO_FLAGS)},
    {"#farsumcom", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#onlymnr", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)}, // can be montag
    {"#notmnr", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)},  // can be montag
    {"#copyitem", std::make_tuple(ITEMS, REMAP_ID, NO_FLAGS)},
    {"#nationrebate", std::make_tuple(NATIONS, REMAP_ID, NO_FLAGS)},
    {"#danceweapon", std::make_tuple(WEAPONS, REMAP_ID, NO_FLAGS)},
    {"#likespop", std::make_tuple(POPTYPES, REMAP_ID, NO_FLAGS)},
    {"#futuresite", std::make_tuple(MAGIC_SITES, REMAP_ID, NO_FLAGS)},
    {"#startsite", std::make_tuple(MAGIC_SITES, REMAP_ID, NO_FLAGS)},
    {"#islandsite", std::make_tuple(MAGIC_SITES, REMAP_ID, NO_FLAGS)},
    {"#damage", std::make_tuple(-1, DAMAGE, NO_FLAGS)},    // we have no clue about the type, damage is a special case
    {"#damagemon", std::make_tuple(-1, DAMAGE, NO_FLAGS)}, // we have no clue about the type, damage is a special case, even though this specifies a string it seems to also take ids
    {"#nametype", std::make_tuple(NAMETYPES, REMAP_ID, NO_FLAGS)},
    {"#onebattlespell", std::make_tuple(SPELLS, REMAP_ID, NO_FLAGS)},   // officially does not support modded spell ids, but adding doesn't hurt (especially if spells get renamed)
    {"#3castbattlespell", std::make_tuple(SPELLS, REMAP_ID, NO_FLAGS)}, // officially does not support modded spell ids, but adding doesn't hurt (especially if spells get renamed)
    {"#coastunit1", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},     // deprecated command but we lose nothing by remapping ids just in case
    {"#coastunit2", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},     // deprecated command but we lose nothing by remapping ids just in case
    {"#coastunit3", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},     // deprecated command but we lose nothing by remapping ids just in case
    {"#coastcom1", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},      // deprecated command but we lose nothing by remapping ids just in case
    {"#coastcom2", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},      // deprecated command but we lose nothing by remapping ids just in case
    {"#copyspr", std::make_tuple(-1, COPYSPR, NO_FLAGS)},               // copyspr could be either of MONSTERS or ITEMS, handled separately
    {"#end", std::make_tuple(-1, END, BLOCK_END)},
    {"#name", std::make_tuple(-1, NAME, NO_FLAGS)},                     // name could be renaming a basegame id, handled separately
    {"#unit", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},                     // mercenary stuff
    {"#fireboost", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#airboost", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#waterboost", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#earthboost", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#astralboost", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#deathboost", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#natureboost", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#glamourboost", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#bloodboost", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#holyboost", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#researchgoal", std::make_tuple(SPELLS, REMAP_ID, NO_FLAGS)}, // template stuff
    {"#form", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#favrit", std::make_tuple(NOTHING, REMAP_ID, NO_FLAGS)},   // this could be either a ritual or item name
  //
    // event commands
    {"#req_nation", std::make_tuple(NATIONS, REMAP_ID, NO_FLAGS)},
    {"#req_nonation", std::make_tuple(NATIONS, REMAP_ID, NO_FLAGS)},
    {"#req_notnation", std::make_tuple(NATIONS, REMAP_ID, NO_FLAGS)},     // doesn't seem to be listed in the manual but is used by Warhammer
    {"#req_fornation", std::make_tuple(NATIONS, REMAP_ID, NO_FLAGS)},
    {"#req_notfornation", std::make_tuple(NATIONS, REMAP_ID, NO_FLAGS)},
    {"#req_notforally", std::make_tuple(NATIONS, REMAP_ID, NO_FLAGS)},
    {"#req_poptype", std::make_tuple(POPTYPES, REMAP_ID, NO_FLAGS)},
    {"#req_notpoptype", std::make_tuple(POPTYPES, REMAP_ID, NO_FLAGS)},
    {"#req_nositenbr", std::make_tuple(MAGIC_SITES, REMAP_ID, NO_FLAGS)},
    {"#req_fullowner", std::make_tuple(NATIONS, REMAP_ID, NO_FLAGS)},
    {"#req_domowner", std::make_tuple(NATIONS, REMAP_ID, NO_FLAGS)},
    {"#req_godismnr", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#req_godisnotmnr", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#req_pretismnr", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#req_monster", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#req_2monsters", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#req_5monsters", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#req_nomonster", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#req_mnr", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#req_nomnr", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#req_deadmnr", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#req_realmnr", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#req_norealmnr", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#req_monsterbs", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#req_mnrbs", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#req_targmnr", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#req_targnomnr", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#req_targrealmnr", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#req_targnorealmnr", std::make_tuple(MONSTERS, REMAP_ID, NO_FLAGS)},
    {"#req_targitem", std::make_tuple(ITEMS, REMAP_ID, NO_FLAGS)},
    {"#req_targnoitem", std::make_tuple(ITEMS, REMAP_ID, NO_FLAGS)},
    {"#req_targowner", std::make_tuple(NATIONS, REMAP_ID, NO_FLAGS)},
    {"#req_targnotowner", std::make_tuple(NATIONS, REMAP_ID, NO_FLAGS)},
    {"#req_targally", std::make_tuple(NATIONS, REMAP_ID, NO_FLAGS)},
    {"#req_targnotally", std::make_tuple(NATIONS, REMAP_ID, NO_FLAGS)},
    {"#req_worlditem", std::make_tuple(ITEMS, REMAP_ID, NO_FLAGS)},
    {"#req_noworlditem", std::make_tuple(ITEMS, REMAP_ID, NO_FLAGS)},
    {"#nation", std::make_tuple(NATIONS, REMAP_ID, NO_FLAGS)},
    {"#extramsg", std::make_tuple(NATIONS, REMAP_ID, NO_FLAGS)},
    {"#addsite", std::make_tuple(MAGIC_SITES, REMAP_ID, NO_FLAGS)},
    {"#maybeaddsite", std::make_tuple(MAGIC_SITES, REMAP_ID, NO_FLAGS)},
    {"#removesite", std::make_tuple(MAGIC_SITES, REMAP_ID, NO_FLAGS)},
    {"#hiddensite", std::make_tuple(MAGIC_SITES, REMAP_ID, NO_FLAGS)},
    {"#maybehiddensite", std::make_tuple(MAGIC_SITES, REMAP_ID, NO_FLAGS)},
    {"#setpoptype", std::make_tuple(POPTYPES, REMAP_ID, NO_FLAGS)},
    {"#assassin", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)}, // not listed in manual, but also allows montag
    {"#assowner", std::make_tuple(NATIONS, REMAP_ID, NO_FLAGS)},
    {"#assownerench", std::make_tuple(ENCHANTMENTS, REMAP_ID, NO_FLAGS)},
    {"#assfollower1", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)},   // not listed in manual, but also allows montag
    {"#assfollower2", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)},   // not listed in manual, but also allows montag
    {"#assfollower3", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)},   // not listed in manual, but also allows montag
    {"#assfollower1d3", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)}, // not listed in manual, but also allows montag
    {"#stealthcom", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)},     // not listed in manual, but probably also allows montag TODO: verify
    {"#com", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)},            // not listed in manual, but also allows montag
    {"#2com", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)},           // not listed in manual, but also allows montag
    {"#4com", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)},           // not listed in manual, but also allows montag
    {"#5com", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)},           // not listed in manual, but also allows montag
    {"#1unit", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)},          // not listed in manual, but also allows montag
    {"#1d3units", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)},        // not listed in manual, but also allows montag
    {"#2d3units", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)},        // not listed in manual, but also allows montag
    {"#3d3units", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)},        // not listed in manual, but also allows montag
    {"#4d3units", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)},        // not listed in manual, but also allows montag
    {"#1d6units", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)},        // not listed in manual, but also allows montag
    {"#2d6units", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)},        // not listed in manual, but also allows montag
    {"#3d6units", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)},        // not listed in manual, but also allows montag
    {"#4d6units", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)},        // not listed in manual, but also allows montag
    {"#5d6units", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)},        // not listed in manual, but also allows montag
    {"#6d6units", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)},        // not listed in manual, but also allows montag
    {"#7d6units", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)},        // not listed in manual, but also allows montag
    {"#8d6units", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)},        // not listed in manual, but also allows montag
    {"#9d6units", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)},        // not listed in manual, but also allows montag
    {"#10d6units", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)},       // not listed in manual, but also allows montag
    {"#11d6units", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)},       // not listed in manual, but also allows montag
    {"#12d6units", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)},       // not listed in manual, but also allows montag
    {"#13d6units", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)},       // not listed in manual, but also allows montag
    {"#14d6units", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)},       // not listed in manual, but also allows montag
    {"#15d6units", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)},       // not listed in manual, but also allows montag
    {"#16d6units", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)},       // not listed in manual, but also allows montag
    {"#var0units", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)},      // not listed in manual, but also allows montag TODO: verify
    {"#killmon", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)},        // not listed in manual, but also allows montag TODO: verify
    {"#kill2d6mon", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)},     // not listed in manual, but also allows montag TODO: verify
    {"#killcom", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)},        // not listed in manual, but also allows montag TODO: verify
    {"#transform", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)},
    {"#forcetransform", std::make_tuple(MONSTERS, POSSIBLE_MONTAG, NO_FLAGS)},

    {"#code", std::make_tuple(EVENTS, REMAP_ID, NO_FLAGS)},
    {"#code2", std::make_tuple(EVENTS, REMAP_ID, NO_FLAGS)},
    {"#resetcode", std::make_tuple(EVENTS, REMAP_ID, NO_FLAGS)},
    {"#id", std::make_tuple(EVENTS, REMAP_ID, NO_FLAGS)},
    {"#codedelay", std::make_tuple(EVENTS, REMAP_ID, NO_FLAGS)},
    {"#codedelay2", std::make_tuple(EVENTS, REMAP_ID, NO_FLAGS)},
    {"#resetcodedelay", std::make_tuple(EVENTS, REMAP_ID, NO_FLAGS)},
    {"#resetcodedelay2", std::make_tuple(EVENTS, REMAP_ID, NO_FLAGS)},
    {"#req_code", std::make_tuple(EVENTS, REMAP_ID, NO_FLAGS)},
    {"#req_notcode", std::make_tuple(EVENTS, REMAP_ID, NO_FLAGS)},
    {"#req_anycode", std::make_tuple(EVENTS, REMAP_ID, NO_FLAGS)},
    {"#req_notanycode", std::make_tuple(EVENTS, REMAP_ID, NO_FLAGS)},
    {"#req_nearbycode", std::make_tuple(EVENTS, REMAP_ID, NO_FLAGS)},
    {"#req_nearowncode", std::make_tuple(EVENTS, REMAP_ID, NO_FLAGS)},

    // enchantment commands
    {"#effect", std::make_tuple(NOTHING, NOTHING, EFFECT)},
    {"#enchrebate10", std::make_tuple(ENCHANTMENTS, REMAP_ID, NO_FLAGS)},
    {"#enchrebate20", std::make_tuple(ENCHANTMENTS, REMAP_ID, NO_FLAGS)},
    {"#enchrebate50", std::make_tuple(ENCHANTMENTS, REMAP_ID, NO_FLAGS)},
    {"#enchrebate75", std::make_tuple(ENCHANTMENTS, REMAP_ID, NO_FLAGS)},
    {"#enchrebate100", std::make_tuple(ENCHANTMENTS, REMAP_ID, NO_FLAGS)},
    {"#enchrebate25p", std::make_tuple(ENCHANTMENTS, REMAP_ID, NO_FLAGS)},
    {"#enchrebate50p", std::make_tuple(ENCHANTMENTS, REMAP_ID, NO_FLAGS)},
    {"#req_noench", std::make_tuple(ENCHANTMENTS, REMAP_ID, NO_FLAGS)},
    {"#req_ench", std::make_tuple(ENCHANTMENTS, REMAP_ID, NO_FLAGS)},
    {"#req_myench", std::make_tuple(ENCHANTMENTS, REMAP_ID, NO_FLAGS)},
    {"#req_friendlyench", std::make_tuple(ENCHANTMENTS, REMAP_ID, NO_FLAGS)},
    {"#req_hostileench", std::make_tuple(ENCHANTMENTS, REMAP_ID, NO_FLAGS)},
    {"#req_enchdom", std::make_tuple(ENCHANTMENTS, REMAP_ID, NO_FLAGS)},
    {"#req_enchtarget", std::make_tuple(ENCHANTMENTS, REMAP_ID, NO_FLAGS)},
    {"#req_enchnearby", std::make_tuple(ENCHANTMENTS, REMAP_ID, NO_FLAGS)},
    {"#nationench", std::make_tuple(ENCHANTMENTS, REMAP_ID, NO_FLAGS)},

    // some special cases
    {"#modname", std::make_tuple(NOTHING, MODNAME, NO_FLAGS)},
    {"#icon", std::make_tuple(NOTHING, RESOURCE_FILE, NO_FLAGS)},
    {"#sample", std::make_tuple(NOTHING, RESOURCE_FILE, NO_FLAGS)},
    {"#spr", std::make_tuple(NOTHING, RESOURCE_FILE, NO_FLAGS)},
    {"#spr1", std::make_tuple(NOTHING, RESOURCE_FILE, NO_FLAGS)},
    {"#spr2", std::make_tuple(NOTHING, RESOURCE_FILE, NO_FLAGS)},
    {"#unmountedspr1", std::make_tuple(NOTHING, RESOURCE_FILE, NO_FLAGS)},
    {"#unmountedspr2", std::make_tuple(NOTHING, RESOURCE_FILE, NO_FLAGS)},
    {"#xspr1", std::make_tuple(NOTHING, RESOURCE_FILE, NO_FLAGS)},
    {"#xspr2", std::make_tuple(NOTHING, RESOURCE_FILE, NO_FLAGS)},
    {"#indepflag", std::make_tuple(NOTHING, RESOURCE_FILE, NO_FLAGS)},
    {"#flag", std::make_tuple(NOTHING, RESOURCE_FILE, NO_FLAGS)},
    {"#era", std::make_tuple(NOTHING, ERA, NO_FLAGS)},
};

// TODO: probably add other file types that are supported
static const std::unordered_set<std::string> resource_file_types = {
  ".tga", ".png", ".jpg", ".sw", ".sw2", ".al"
};
