# General
This program does not check whether two mods assign something the same name and reference it later, if mods reference something by its name without also referencing its id this can theoretically cause conflicts.
While the merged files are likely to work fine and will almost certainly work better than running the mods without merging, using multiple mods at once can make the game more unstable.

# Usage
The tool is currently a command line tool that takes various arguments that can be looked up with --help.
For merging you should copy all mod directories into a single directory and leave only a single version per mod.
For example, Dominions Enhanced has various version files, but the remapper can not detect which is the newest one and will fail to remap properly if it handles different versions of the same thing.
It is however fine if mods have multiple .dm files, like Sombre Warhammer with its poptype and main file, as long as they don't have significant overlaps there should be no issues.
Generally always use the latest version of a mod, the resulting merged mod will always be a static version.

Mods need to be enabled in the order provided by the program after execution and in the remapped mod name.
Failure to do so can cause crashes because remapped names will be wrong.
This can also happen if one of your merged mods is not loaded.

If separate mods are directly dependent on each other (e.g. one mod references a weapon defined by another mod) they need to be in the same directory for the remapper to handle this correctly, the .dm files should also be lexicographically ordered like the load order given on the mod page.

Below is an example of how the directory structure should look like, notice how there is only one version of the Dominions Enhanced .dm file, Because the others are simply different versions of the same mod.
The Warhammer mod has 4 different .dm files because they are for different purposes and do not define the same things.
```
├── merger
├── gamedata
│   ├── ...
├── mods
│   ├── DominionsEnhanced
│   │   ├── ... (subdirectories with tga/png files)
│   │   ├── banner.png
│   │   ├── DomEnhanced2_13.dm
│   └── Warhammer
│       ├── ... (subdirectories with tga/png files)
│       ├── banner.png
│       ├── .. (other banner files)
│       ├── Sombre_Warhammer_static_1.42.dm
│       ├── Warhammer_AI_Pretenders.dm
│       ├── Warhammer_AI_Starting_Force_Boost.dm
│       ├── Warhammer_Poptype_PD.dm
```


# Implementation details and limitations
## Spell remapping for #onebattlespell, #3castbattlespell
Since these commands can only reference base game spells, some mods (e.g. sombre warhammer) copy base game spells to new ids and then replace the old id to create a spell that works for these commands
Currently this works if spells are remapped using the following, could potentially be expanded to include #selectspell rather than just #newspell.
#newspell
#copyspell spell_id | spell_name
#end

This is used because the Sombre warhammer mod already does it this way and it seems like a reasonable way to handle it.


## Events
Per the manual, events can only be created with #newevent without specifying an id, so no remapping should be necessary here.
If magic sites are necessary and the event references a basegame one it should be remapped correctly. This is not currently implemented as it needs to be in brackets (special case not handled yet) and remapping base game magic site names is probably (almost) never done.
Information for this implementation can be found on page 5 of the event manual (regarding the #msg command)


## Enchantments and Summon/Transform Spells
Enchantments are harder to remap, because #copyspell and #effect will decide whether a spell is an enchantment or not, for this we need to check the #effect value and check if the target of #copyspell is an enchantment itself.
For the second part this project utilizes the tsv files from dom6modinspector which contain a list of all spells and their effect values so we can identify enchantments.
Summon/transform spells work the same way.


## Genuinely no clue
The following contains some things that I have no clue how to handle/whether they need to be handled at all.
#explspr (seems to only be in a set range, probably no new values allowed)
#flyspr (same)
#templepic (same)

Somehow it seems possible to do comments with a single - even though the docs say -- is needed, this might cause issues with parsing as we can't filter that reliably, at least some mods have stuff that seem like comments with a single -


## check later:
#selectbless (probably only base game blessings), #disbless
#newnation id, probably does not properly assign the nation to the specified id, at least they appear in the wrong order on nation list ingame


## TODO
TODO: do event vars need remapping? How do they work, need to look this up
TODO: Probably keep track of all new names created and replace name references with ids to prevent conflicts (Should be an at most rarely relevant edge case)
TODO: blesses can be renamed and referenced by name, if one mod renames a bless and a later mod references the bless by name it will cause issues, fixing this requires keeping track of all name changes
TODO: pull gamedata from dom6inspector in code rather than before to always use newest data


# Credits
Illwinter for making Dominions 6
The contributors to the dom6modinspector for getting all the information about the base game content, needed for some merging capabilities
The dominions modding community for making all the mods
