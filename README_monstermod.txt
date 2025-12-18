This mod is a monster capturing/summoning mod. The player starts off with a grenade launcher that never runs out, and this is used to capture monsters. Once captured, they can be summoned using commands, and they will fight for you. Every monster killed gives some exp to your current active monster. The more exp, the more powerful your monster's attacks, capping out at 3x its base power at 600 exp.

To install:
Compile the .dll and add it to a new folder in the quake 2 folder. To launch, open quake 2 and use the set game command for whatever the folder is named.

Capturable monsters and their number:
Soldier light = 1
Soldier = 2 
Soldier SS = 3 
Flyer = 4 
Parasite = 5 
Berserker = 6 
Tank = 7 
Medic = 8 
Mutant = 9 
Brain = 10

Commands: 
monsterhelp: In-game help screen with basic info/commands
monstercmds: list of additional commands, mostly for testing
monsterlist: list of your current monsters and their exp
monsterclear: clear all monster slots and set all exp to 0
monster1/monster2/monster3: summons the monster from that slot, making it your active monster
monsterreturn: returns the current monster - you need to return the current active monster before sending out another
monsterfollow: respawns your current monster next to you. Good for if it’s lagging behind

Additional commands (listed under monstercmds):
monset1: preset team #1 - Soldier light, soldier, soldier ss
monset2: preset team #2 - flyer, parasite, berserker
monset3: preset team #3 - Tank, Medic, Mutant
monset4: preset team #4 - brain, other two slots empty
monwho: list of all catchable monsters and what their number is
monexp: gives all slots with a monster +100 exp