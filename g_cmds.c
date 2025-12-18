#include "g_local.h"
#include "m_player.h"


char *ClientTeam (edict_t *ent)
{
	char		*p;
	static char	value[512];

	value[0] = 0;

	if (!ent->client)
		return value;

	strcpy(value, Info_ValueForKey (ent->client->pers.userinfo, "skin"));
	p = strchr(value, '/');
	if (!p)
		return value;

	if ((int)(dmflags->value) & DF_MODELTEAMS)
	{
		*p = 0;
		return value;
	}

	// if ((int)(dmflags->value) & DF_SKINTEAMS)
	return ++p;
}

qboolean OnSameTeam (edict_t *ent1, edict_t *ent2)
{
	char	ent1Team [512];
	char	ent2Team [512];

	if (!((int)(dmflags->value) & (DF_MODELTEAMS | DF_SKINTEAMS)))
		return false;

	strcpy (ent1Team, ClientTeam (ent1));
	strcpy (ent2Team, ClientTeam (ent2));

	if (strcmp(ent1Team, ent2Team) == 0)
		return true;
	return false;
}


void SelectNextItem (edict_t *ent, int itflags)
{
	gclient_t	*cl;
	int			i, index;
	gitem_t		*it;

	cl = ent->client;

	if (cl->chase_target) {
		ChaseNext(ent);
		return;
	}

	// scan  for the next valid one
	for (i=1 ; i<=MAX_ITEMS ; i++)
	{
		index = (cl->pers.selected_item + i)%MAX_ITEMS;
		if (!cl->pers.inventory[index])
			continue;
		it = &itemlist[index];
		if (!it->use)
			continue;
		if (!(it->flags & itflags))
			continue;

		cl->pers.selected_item = index;
		return;
	}

	cl->pers.selected_item = -1;
}

void SelectPrevItem (edict_t *ent, int itflags)
{
	gclient_t	*cl;
	int			i, index;
	gitem_t		*it;

	cl = ent->client;

	if (cl->chase_target) {
		ChasePrev(ent);
		return;
	}

	// scan  for the next valid one
	for (i=1 ; i<=MAX_ITEMS ; i++)
	{
		index = (cl->pers.selected_item + MAX_ITEMS - i)%MAX_ITEMS;
		if (!cl->pers.inventory[index])
			continue;
		it = &itemlist[index];
		if (!it->use)
			continue;
		if (!(it->flags & itflags))
			continue;

		cl->pers.selected_item = index;
		return;
	}

	cl->pers.selected_item = -1;
}

void ValidateSelectedItem (edict_t *ent)
{
	gclient_t	*cl;

	cl = ent->client;

	if (cl->pers.inventory[cl->pers.selected_item])
		return;		// valid

	SelectNextItem (ent, -1);
}


//=================================================================================

/*
==================
Cmd_Give_f

Give items to a client
==================
*/
void Cmd_Give_f (edict_t *ent)
{
	char		*name;
	gitem_t		*it;
	int			index;
	int			i;
	qboolean	give_all;
	edict_t		*it_ent;

	if (deathmatch->value && !sv_cheats->value)
	{
		gi.cprintf (ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
		return;
	}

	name = gi.args();

	if (Q_stricmp(name, "all") == 0)
		give_all = true;
	else
		give_all = false;

	if (give_all || Q_stricmp(gi.argv(1), "health") == 0)
	{
		if (gi.argc() == 3)
			ent->health = atoi(gi.argv(2));
		else
			ent->health = ent->max_health;
		if (!give_all)
			return;
	}

	if (give_all || Q_stricmp(name, "weapons") == 0)
	{
		for (i=0 ; i<game.num_items ; i++)
		{
			it = itemlist + i;
			if (!it->pickup)
				continue;
			if (!(it->flags & IT_WEAPON))
				continue;
			ent->client->pers.inventory[i] += 1;
		}
		if (!give_all)
			return;
	}

	if (give_all || Q_stricmp(name, "ammo") == 0)
	{
		for (i=0 ; i<game.num_items ; i++)
		{
			it = itemlist + i;
			if (!it->pickup)
				continue;
			if (!(it->flags & IT_AMMO))
				continue;
			Add_Ammo (ent, it, 1000);
		}
		if (!give_all)
			return;
	}

	if (give_all || Q_stricmp(name, "armor") == 0)
	{
		gitem_armor_t	*info;

		it = FindItem("Jacket Armor");
		ent->client->pers.inventory[ITEM_INDEX(it)] = 0;

		it = FindItem("Combat Armor");
		ent->client->pers.inventory[ITEM_INDEX(it)] = 0;

		it = FindItem("Body Armor");
		info = (gitem_armor_t *)it->info;
		ent->client->pers.inventory[ITEM_INDEX(it)] = info->max_count;

		if (!give_all)
			return;
	}

	if (give_all || Q_stricmp(name, "Power Shield") == 0)
	{
		it = FindItem("Power Shield");
		it_ent = G_Spawn();
		it_ent->classname = it->classname;
		SpawnItem (it_ent, it);
		Touch_Item (it_ent, ent, NULL, NULL);
		if (it_ent->inuse)
			G_FreeEdict(it_ent);

		if (!give_all)
			return;
	}

	if (give_all)
	{
		for (i=0 ; i<game.num_items ; i++)
		{
			it = itemlist + i;
			if (!it->pickup)
				continue;
			if (it->flags & (IT_ARMOR|IT_WEAPON|IT_AMMO))
				continue;
			ent->client->pers.inventory[i] = 1;
		}
		return;
	}

	it = FindItem (name);
	if (!it)
	{
		name = gi.argv(1);
		it = FindItem (name);
		if (!it)
		{
			gi.cprintf (ent, PRINT_HIGH, "unknown item\n");
			return;
		}
	}

	if (!it->pickup)
	{
		gi.cprintf (ent, PRINT_HIGH, "non-pickup item\n");
		return;
	}

	index = ITEM_INDEX(it);

	if (it->flags & IT_AMMO)
	{
		if (gi.argc() == 3)
			ent->client->pers.inventory[index] = atoi(gi.argv(2));
		else
			ent->client->pers.inventory[index] += it->quantity;
	}
	else
	{
		it_ent = G_Spawn();
		it_ent->classname = it->classname;
		SpawnItem (it_ent, it);
		Touch_Item (it_ent, ent, NULL, NULL);
		if (it_ent->inuse)
			G_FreeEdict(it_ent);
	}
}


/*
==================
Cmd_God_f

Sets client to godmode

argv(0) god
==================
*/
void Cmd_God_f (edict_t *ent)
{
	char	*msg;

	if (deathmatch->value && !sv_cheats->value)
	{
		gi.cprintf (ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
		return;
	}

	ent->flags ^= FL_GODMODE;
	if (!(ent->flags & FL_GODMODE) )
		msg = "godmode OFF\n";
	else
		msg = "godmode ON\n";

	gi.cprintf (ent, PRINT_HIGH, msg);
}


/*
==================
Cmd_Notarget_f

Sets client to notarget

argv(0) notarget
==================
*/
void Cmd_Notarget_f (edict_t *ent)
{
	char	*msg;

	if (deathmatch->value && !sv_cheats->value)
	{
		gi.cprintf (ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
		return;
	}

	ent->flags ^= FL_NOTARGET;
	if (!(ent->flags & FL_NOTARGET) )
		msg = "notarget OFF\n";
	else
		msg = "notarget ON\n";

	gi.cprintf (ent, PRINT_HIGH, msg);
}


/*
==================
Cmd_Noclip_f

argv(0) noclip
==================
*/
void Cmd_Noclip_f (edict_t *ent)
{
	char	*msg;

	if (deathmatch->value && !sv_cheats->value)
	{
		gi.cprintf (ent, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
		return;
	}

	if (ent->movetype == MOVETYPE_NOCLIP)
	{
		ent->movetype = MOVETYPE_WALK;
		msg = "noclip OFF\n";
	}
	else
	{
		ent->movetype = MOVETYPE_NOCLIP;
		msg = "noclip ON\n";
	}

	gi.cprintf (ent, PRINT_HIGH, msg);
}


/*
==================
Cmd_Use_f

Use an inventory item
==================
*/
void Cmd_Use_f (edict_t *ent)
{
	int			index;
	gitem_t		*it;
	char		*s;

	s = gi.args();
	it = FindItem (s);
	if (!it)
	{
		gi.cprintf (ent, PRINT_HIGH, "unknown item: %s\n", s);
		return;
	}
	if (!it->use)
	{
		gi.cprintf (ent, PRINT_HIGH, "Item is not usable.\n");
		return;
	}
	index = ITEM_INDEX(it);
	if (!ent->client->pers.inventory[index])
	{
		gi.cprintf (ent, PRINT_HIGH, "Out of item: %s\n", s);
		return;
	}

	it->use (ent, it);
}


/*
==================
Cmd_Drop_f

Drop an inventory item
==================
*/
void Cmd_Drop_f (edict_t *ent)
{
	int			index;
	gitem_t		*it;
	char		*s;

	s = gi.args();
	it = FindItem (s);
	if (!it)
	{
		gi.cprintf (ent, PRINT_HIGH, "unknown item: %s\n", s);
		return;
	}
	if (!it->drop)
	{
		gi.cprintf (ent, PRINT_HIGH, "Item is not dropable.\n");
		return;
	}
	index = ITEM_INDEX(it);
	if (!ent->client->pers.inventory[index])
	{
		gi.cprintf (ent, PRINT_HIGH, "Out of item: %s\n", s);
		return;
	}

	it->drop (ent, it);
}


/*
=================
Cmd_Inven_f
=================
*/
void Cmd_Inven_f (edict_t *ent)
{
	int			i;
	gclient_t	*cl;

	cl = ent->client;

	cl->showscores = false;
	cl->showhelp = false;

	if (cl->showinventory)
	{
		cl->showinventory = false;
		return;
	}

	cl->showinventory = true;

	gi.WriteByte (svc_inventory);
	for (i=0 ; i<MAX_ITEMS ; i++)
	{
		gi.WriteShort (cl->pers.inventory[i]);
	}
	gi.unicast (ent, true);
}

/*
=================
Cmd_InvUse_f
=================
*/
void Cmd_InvUse_f (edict_t *ent)
{
	gitem_t		*it;

	ValidateSelectedItem (ent);

	if (ent->client->pers.selected_item == -1)
	{
		gi.cprintf (ent, PRINT_HIGH, "No item to use.\n");
		return;
	}

	it = &itemlist[ent->client->pers.selected_item];
	if (!it->use)
	{
		gi.cprintf (ent, PRINT_HIGH, "Item is not usable.\n");
		return;
	}
	it->use (ent, it);
}

/*
=================
Cmd_WeapPrev_f
=================
*/
void Cmd_WeapPrev_f (edict_t *ent)
{
	gclient_t	*cl;
	int			i, index;
	gitem_t		*it;
	int			selected_weapon;

	cl = ent->client;

	if (!cl->pers.weapon)
		return;

	selected_weapon = ITEM_INDEX(cl->pers.weapon);

	// scan  for the next valid one
	for (i=1 ; i<=MAX_ITEMS ; i++)
	{
		index = (selected_weapon + i)%MAX_ITEMS;
		if (!cl->pers.inventory[index])
			continue;
		it = &itemlist[index];
		if (!it->use)
			continue;
		if (! (it->flags & IT_WEAPON) )
			continue;
		it->use (ent, it);
		if (cl->pers.weapon == it)
			return;	// successful
	}
}

/*
=================
Cmd_WeapNext_f
=================
*/
void Cmd_WeapNext_f (edict_t *ent)
{
	gclient_t	*cl;
	int			i, index;
	gitem_t		*it;
	int			selected_weapon;

	cl = ent->client;

	if (!cl->pers.weapon)
		return;

	selected_weapon = ITEM_INDEX(cl->pers.weapon);

	// scan  for the next valid one
	for (i=1 ; i<=MAX_ITEMS ; i++)
	{
		index = (selected_weapon + MAX_ITEMS - i)%MAX_ITEMS;
		if (!cl->pers.inventory[index])
			continue;
		it = &itemlist[index];
		if (!it->use)
			continue;
		if (! (it->flags & IT_WEAPON) )
			continue;
		it->use (ent, it);
		if (cl->pers.weapon == it)
			return;	// successful
	}
}

/*
=================
Cmd_WeapLast_f
=================
*/
void Cmd_WeapLast_f (edict_t *ent)
{
	gclient_t	*cl;
	int			index;
	gitem_t		*it;

	cl = ent->client;

	if (!cl->pers.weapon || !cl->pers.lastweapon)
		return;

	index = ITEM_INDEX(cl->pers.lastweapon);
	if (!cl->pers.inventory[index])
		return;
	it = &itemlist[index];
	if (!it->use)
		return;
	if (! (it->flags & IT_WEAPON) )
		return;
	it->use (ent, it);
}

/*
=================
Cmd_InvDrop_f
=================
*/
void Cmd_InvDrop_f (edict_t *ent)
{
	gitem_t		*it;

	ValidateSelectedItem (ent);

	if (ent->client->pers.selected_item == -1)
	{
		gi.cprintf (ent, PRINT_HIGH, "No item to drop.\n");
		return;
	}

	it = &itemlist[ent->client->pers.selected_item];
	if (!it->drop)
	{
		gi.cprintf (ent, PRINT_HIGH, "Item is not dropable.\n");
		return;
	}
	it->drop (ent, it);
}

/*
=================
Cmd_Kill_f
=================
*/
void Cmd_Kill_f (edict_t *ent)
{
	if((level.time - ent->client->respawn_time) < 5)
		return;
	ent->flags &= ~FL_GODMODE;
	ent->health = 0;
	meansOfDeath = MOD_SUICIDE;
	player_die (ent, ent, ent, 100000, vec3_origin);
}

/*
=================
Cmd_PutAway_f
=================
*/
void Cmd_PutAway_f (edict_t *ent)
{
	ent->client->showscores = false;
	ent->client->showhelp = false;
	ent->client->showinventory = false;
}


int PlayerSort (void const *a, void const *b)
{
	int		anum, bnum;

	anum = *(int *)a;
	bnum = *(int *)b;

	anum = game.clients[anum].ps.stats[STAT_FRAGS];
	bnum = game.clients[bnum].ps.stats[STAT_FRAGS];

	if (anum < bnum)
		return -1;
	if (anum > bnum)
		return 1;
	return 0;
}

/*
=================
Cmd_Players_f
=================
*/
void Cmd_Players_f (edict_t *ent)
{
	int		i;
	int		count;
	char	small[64];
	char	large[1280];
	int		index[256];

	count = 0;
	for (i = 0 ; i < maxclients->value ; i++)
		if (game.clients[i].pers.connected)
		{
			index[count] = i;
			count++;
		}

	// sort by frags
	qsort (index, count, sizeof(index[0]), PlayerSort);

	// print information
	large[0] = 0;

	for (i = 0 ; i < count ; i++)
	{
		Com_sprintf (small, sizeof(small), "%3i %s\n",
			game.clients[index[i]].ps.stats[STAT_FRAGS],
			game.clients[index[i]].pers.netname);
		if (strlen (small) + strlen(large) > sizeof(large) - 100 )
		{	// can't print all of them in one packet
			strcat (large, "...\n");
			break;
		}
		strcat (large, small);
	}

	gi.cprintf (ent, PRINT_HIGH, "%s\n%i players\n", large, count);
}

/*
=================
Cmd_Wave_f
=================
*/
void Cmd_Wave_f (edict_t *ent)
{
	int		i;

	i = atoi (gi.argv(1));

	// can't wave when ducked
	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
		return;

	if (ent->client->anim_priority > ANIM_WAVE)
		return;

	ent->client->anim_priority = ANIM_WAVE;

	switch (i)
	{
	case 0:
		gi.cprintf (ent, PRINT_HIGH, "flipoff\n");
		ent->s.frame = FRAME_flip01-1;
		ent->client->anim_end = FRAME_flip12;
		break;
	case 1:
		gi.cprintf (ent, PRINT_HIGH, "salute\n");
		ent->s.frame = FRAME_salute01-1;
		ent->client->anim_end = FRAME_salute11;
		break;
	case 2:
		gi.cprintf (ent, PRINT_HIGH, "taunt\n");
		ent->s.frame = FRAME_taunt01-1;
		ent->client->anim_end = FRAME_taunt17;
		break;
	case 3:
		gi.cprintf (ent, PRINT_HIGH, "wave\n");
		ent->s.frame = FRAME_wave01-1;
		ent->client->anim_end = FRAME_wave11;
		break;
	case 4:
	default:
		gi.cprintf (ent, PRINT_HIGH, "point\n");
		ent->s.frame = FRAME_point01-1;
		ent->client->anim_end = FRAME_point12;
		break;
	}
}

/*
==================
Cmd_Say_f
==================
*/
void Cmd_Say_f (edict_t *ent, qboolean team, qboolean arg0)
{
	int		i, j;
	edict_t	*other;
	char	*p;
	char	text[2048];
	gclient_t *cl;

	if (gi.argc () < 2 && !arg0)
		return;

	if (!((int)(dmflags->value) & (DF_MODELTEAMS | DF_SKINTEAMS)))
		team = false;

	if (team)
		Com_sprintf (text, sizeof(text), "(%s): ", ent->client->pers.netname);
	else
		Com_sprintf (text, sizeof(text), "%s: ", ent->client->pers.netname);

	if (arg0)
	{
		strcat (text, gi.argv(0));
		strcat (text, " ");
		strcat (text, gi.args());
	}
	else
	{
		p = gi.args();

		if (*p == '"')
		{
			p++;
			p[strlen(p)-1] = 0;
		}
		strcat(text, p);
	}

	// don't let text be too long for malicious reasons
	if (strlen(text) > 150)
		text[150] = 0;

	strcat(text, "\n");

	if (flood_msgs->value) {
		cl = ent->client;

        if (level.time < cl->flood_locktill) {
			gi.cprintf(ent, PRINT_HIGH, "You can't talk for %d more seconds\n",
				(int)(cl->flood_locktill - level.time));
            return;
        }
        i = cl->flood_whenhead - flood_msgs->value + 1;
        if (i < 0)
            i = (sizeof(cl->flood_when)/sizeof(cl->flood_when[0])) + i;
		if (cl->flood_when[i] && 
			level.time - cl->flood_when[i] < flood_persecond->value) {
			cl->flood_locktill = level.time + flood_waitdelay->value;
			gi.cprintf(ent, PRINT_CHAT, "Flood protection:  You can't talk for %d seconds.\n",
				(int)flood_waitdelay->value);
            return;
        }
		cl->flood_whenhead = (cl->flood_whenhead + 1) %
			(sizeof(cl->flood_when)/sizeof(cl->flood_when[0]));
		cl->flood_when[cl->flood_whenhead] = level.time;
	}

	if (dedicated->value)
		gi.cprintf(NULL, PRINT_CHAT, "%s", text);

	for (j = 1; j <= game.maxclients; j++)
	{
		other = &g_edicts[j];
		if (!other->inuse)
			continue;
		if (!other->client)
			continue;
		if (team)
		{
			if (!OnSameTeam(ent, other))
				continue;
		}
		gi.cprintf(other, PRINT_CHAT, "%s", text);
	}
}

void Cmd_PlayerList_f(edict_t *ent)
{
	int i;
	char st[80];
	char text[1400];
	edict_t *e2;

	// connect time, ping, score, name
	*text = 0;
	for (i = 0, e2 = g_edicts + 1; i < maxclients->value; i++, e2++) {
		if (!e2->inuse)
			continue;

		Com_sprintf(st, sizeof(st), "%02d:%02d %4d %3d %s%s\n",
			(level.framenum - e2->client->resp.enterframe) / 600,
			((level.framenum - e2->client->resp.enterframe) % 600)/10,
			e2->client->ping,
			e2->client->resp.score,
			e2->client->pers.netname,
			e2->client->resp.spectator ? " (spectator)" : "");
		if (strlen(text) + strlen(st) > sizeof(text) - 50) {
			sprintf(text+strlen(text), "And more...\n");
			gi.cprintf(ent, PRINT_HIGH, "%s", text);
			return;
		}
		strcat(text, st);
	}
	gi.cprintf(ent, PRINT_HIGH, "%s", text);
}

void Cmd_monsterslot_f(edict_t* ent) 
{
	gi.cprintf(ent, PRINT_HIGH, "monster 1: %i; exp: %i\n", ent->client->pers.mon_slot_1, ent->client->pers.slot_1_exp);
	gi.cprintf(ent, PRINT_HIGH, "monster 2: %i; exp: %i\n", ent->client->pers.mon_slot_2, ent->client->pers.slot_2_exp);
	gi.cprintf(ent, PRINT_HIGH, "monster 3: %i; exp: %i\n", ent->client->pers.mon_slot_3, ent->client->pers.slot_3_exp);
}

void Cmd_clearmonsters_f(edict_t* ent)
{
	ent->client->pers.mon_slot_1 = 0;
	ent->client->pers.mon_slot_2 = 0;
	ent->client->pers.mon_slot_3 = 0;
	ent->client->pers.current_monsters = 0;
	ent->client->pers.slot_1_exp = 0;
	ent->client->pers.slot_2_exp = 0;
	ent->client->pers.slot_3_exp = 0;
	gi.cprintf(ent, PRINT_HIGH, "monsters cleared\n");
}

void Cmd_Spawnplayermonster_f(edict_t* ent, int slot)
{
	if (!ent)
	{
		return;
	}

	int monx;
	int mony;
	int monz;
	int montype;

	if (ent->client) {
		monx = ent->s.origin[0];
		mony = ent->s.origin[1];
		monz = ent->s.origin[2];
		if (ent->client->pers.active_slot != 0) {
			gi.cprintf(ent, PRINT_HIGH, "you have an active monster right now - call it back before sending out a new one!\n");
			return;
		}
		if (slot == 1) {
			montype = ent->client->pers.mon_slot_1;
			ent->client->pers.active_slot = 1;
		}
		else if (slot == 2) {
			montype = ent->client->pers.mon_slot_2;
			ent->client->pers.active_slot = 2;
		}
		else if (slot == 3) {
			montype = ent->client->pers.mon_slot_3;
			ent->client->pers.active_slot = 3;
		}
		else {
			gi.cprintf(ent, PRINT_HIGH, "no slot picked\n");
			return;
		}
		gi.cprintf(ent, PRINT_HIGH, "slot: %i\n", slot);
		gi.cprintf(ent, PRINT_HIGH, "monster type: %i\n", montype);
	}
	else {
		gi.cprintf(ent, PRINT_HIGH, "no player\n");
		return;
	}

	ent = G_Spawn();

	monx = monx + 45;
	mony = mony + 45;
	monz = monz + 30;
	// set position
	ent->s.origin[0] = monx;
	ent->s.origin[1] = mony;
	ent->s.origin[2] = monz;
	// angles
	// flags
	//if (gi.argc() >= 9)
	//{
	//	ent->spawnflags = atoi(gi.argv(8));
	//}
	if (montype > 0 && montype <= 10) {
		if (montype == 1)
			ent->classname = "monster_soldier_light";
		else if (montype == 2)
			ent->classname = "monster_soldier";
		else if (montype == 3)
			ent->classname = "monster_soldier_ss";
		else if (montype == 4)
			ent->classname = "monster_flyer";
		else if (montype == 5)
			ent->classname = "monster_parasite";
		else if (montype == 6)
			ent->classname = "monster_berserk";
		else if (montype == 7)
			ent->classname = "monster_tank";
		else if (montype == 8)
			ent->classname = "monster_medic";
		else if (montype == 9)
			ent->classname = "monster_mutant";
		else if (montype == 10)
			ent->classname = "monster_brain";
		else {
			gi.cprintf(ent, PRINT_HIGH, "no monster type\n");
			return;
		}
		ent->monsterinfo.is_mine = 1;
		ent->monsterinfo.aiflags |= AI_GOOD_GUY;
		ED_CallSpawn(ent);
	}
}

void Cmd_playmonster1_f(edict_t* ent)
{
	if (ent->client->pers.mon_slot_1 == 0)
		gi.cprintf(ent, PRINT_HIGH, "no monster in that slot\n");
	else if (ent->client->pers.mon_slot_1 > 0 && ent->client->pers.mon_slot_1 <= 10)
		Cmd_Spawnplayermonster_f(ent,1);
}

void Cmd_playmonster2_f(edict_t* ent)
{
	if (ent->client->pers.mon_slot_2 == 0)
		gi.cprintf(ent, PRINT_HIGH, "no monster in that slot\n");
	else if (ent->client->pers.mon_slot_2 > 0 && ent->client->pers.mon_slot_2 <= 10)
		Cmd_Spawnplayermonster_f(ent, 2);
}

void Cmd_playmonster3_f(edict_t* ent)
{
	if (ent->client->pers.mon_slot_3 == 0)
		gi.cprintf(ent, PRINT_HIGH, "no monster in that slot\n");
	else if (ent->client->pers.mon_slot_3 > 0 && ent->client->pers.mon_slot_3 <= 10)
		Cmd_Spawnplayermonster_f(ent, 3);
}

void Cmd_returnmonster_f(edict_t* ent) 
{
	for (int i = 0; i < globals.num_edicts; i++)
	{
		edict_t* cur = &g_edicts[i];
		qboolean print = false;

		/* Ensure that the entity is valid. */
		if (!cur->classname)
		{
			continue;
		}
		if (strncmp(cur->classname, "monster_", 8) == 0)
		{
			if (cur->monsterinfo.is_mine == 1) {
				print = true;
			}
		}
		if (print)
		{
			G_FreeEdict(cur);
		}
	}
	ent->client->pers.active_slot = 0;
}

void Cmd_monsterfollow_f(edict_t* ent) 
{
	int holdslot;

	holdslot = ent->client->pers.active_slot;
	Cmd_returnmonster_f(ent);
	Cmd_Spawnplayermonster_f(ent, holdslot);

}

void Cmd_monsterset1_f(edict_t* ent)
{
	ent->client->pers.mon_slot_1 = 1;
	ent->client->pers.mon_slot_2 = 2;
	ent->client->pers.mon_slot_3 = 3;
	ent->client->pers.current_monsters = 3;
	ent->client->pers.slot_1_exp = 0;
	ent->client->pers.slot_2_exp = 0;
	ent->client->pers.slot_3_exp = 0;
	gi.cprintf(ent, PRINT_HIGH, "1: soldier light, 2: soldier, 3: soldier ss\n");
}

void Cmd_monsterset2_f(edict_t* ent)
{
	ent->client->pers.mon_slot_1 = 4;
	ent->client->pers.mon_slot_2 = 5;
	ent->client->pers.mon_slot_3 = 6;
	ent->client->pers.current_monsters = 3;
	ent->client->pers.slot_1_exp = 0;
	ent->client->pers.slot_2_exp = 0;
	ent->client->pers.slot_3_exp = 0;
	gi.cprintf(ent, PRINT_HIGH, "1: flyer, 2: parasite, 3: berserker\n");
}

void Cmd_monsterset3_f(edict_t* ent)
{
	ent->client->pers.mon_slot_1 = 7;
	ent->client->pers.mon_slot_2 = 8;
	ent->client->pers.mon_slot_3 = 9;
	ent->client->pers.current_monsters = 3;
	ent->client->pers.slot_1_exp = 0;
	ent->client->pers.slot_2_exp = 0;
	ent->client->pers.slot_3_exp = 0;
	gi.cprintf(ent, PRINT_HIGH, "1: tank, 2: medic, 3: mutant\n");
}

void Cmd_monsterset4_f(edict_t* ent)
{
	ent->client->pers.mon_slot_1 = 10;
	ent->client->pers.mon_slot_2 = 0;
	ent->client->pers.mon_slot_3 = 0;
	ent->client->pers.current_monsters = 2;
	ent->client->pers.slot_1_exp = 0;
	ent->client->pers.slot_2_exp = 0;
	ent->client->pers.slot_3_exp = 0;
	gi.cprintf(ent, PRINT_HIGH, "1: brain, 2: empty, 3: empty\n");
}

void Cmd_monsterwho_f(edict_t* ent) 
{
	gi.cprintf(ent, PRINT_HIGH, "1 - Light Soldier  6 - Berserker\n");
	gi.cprintf(ent, PRINT_HIGH, "2 - Soldier        7 - Tank\n");
	gi.cprintf(ent, PRINT_HIGH, "3 - Soldier SS     8 - Medic\n");
	gi.cprintf(ent, PRINT_HIGH, "4 - Flyer          9 - Mutant\n");
	gi.cprintf(ent, PRINT_HIGH, "5 - Parasite      10 - Brain\n");
}

void Cmd_monsterexp_f(edict_t* ent) 
{
	if (ent->client->pers.mon_slot_1 != 0) 
	{
		ent->client->pers.slot_1_exp = ent->client->pers.slot_1_exp + 100;
		if (ent->client->pers.slot_1_exp > 600)
			ent->client->pers.slot_1_exp = 600;
	}
	if (ent->client->pers.mon_slot_2 != 0)
	{
		ent->client->pers.slot_2_exp = ent->client->pers.slot_2_exp + 100;
		if (ent->client->pers.slot_2_exp > 600)
			ent->client->pers.slot_2_exp = 600;
	}
	if (ent->client->pers.mon_slot_3 != 0)
	{
		ent->client->pers.slot_3_exp = ent->client->pers.slot_3_exp + 100;
		if (ent->client->pers.slot_3_exp > 600)
			ent->client->pers.slot_3_exp = 600;
	}
}
void Cmd_monsterhelp_f(edict_t* ent) 
{
	gi.cprintf(ent, PRINT_HIGH, "This mod lets you capture and summon monsters!\n");
	gi.cprintf(ent, PRINT_HIGH, "Use the grenades/grenade launcher to catch monsters\n");
	gi.cprintf(ent, PRINT_HIGH, "Defeat other monsters to earn exp for your monster and make them stronger!\n");
	gi.cprintf(ent, PRINT_HIGH, "Commands -\n");
	gi.cprintf(ent, PRINT_HIGH, "monsterlist - Check your monsters\n");
	gi.cprintf(ent, PRINT_HIGH, "monsterclear - Get rid of all monsters\n");
	gi.cprintf(ent, PRINT_HIGH, "monster1/monster2/monster3 - Summon Monsters\n");
	gi.cprintf(ent, PRINT_HIGH, "monsterreturn - Call back monster\n");
	gi.cprintf(ent, PRINT_HIGH, "monsterfollow - bring your active monster to you\n");
	gi.cprintf(ent, PRINT_HIGH, "monstercmds - See extra commands\n");
}

void Cmd_monstercommands_f(edict_t* ent) 
{
	gi.cprintf(ent, PRINT_HIGH, "monset1/monset2/monset3/monset4 - preset teams\n");
	gi.cprintf(ent, PRINT_HIGH, "monwho - which monster is what number\n");
	gi.cprintf(ent, PRINT_HIGH, "monexp - give every monster +100 exp\n");
}

/*
=================
ClientCommand
=================
*/
void ClientCommand (edict_t *ent)
{
	char	*cmd;

	if (!ent->client)
		return;		// not fully in game yet

	cmd = gi.argv(0);

	if (Q_stricmp (cmd, "players") == 0)
	{
		Cmd_Players_f (ent);
		return;
	}
	if (Q_stricmp (cmd, "say") == 0)
	{
		Cmd_Say_f (ent, false, false);
		return;
	}
	if (Q_stricmp (cmd, "say_team") == 0)
	{
		Cmd_Say_f (ent, true, false);
		return;
	}
	if (Q_stricmp (cmd, "score") == 0)
	{
		Cmd_Score_f (ent);
		return;
	}
	if (Q_stricmp (cmd, "help") == 0)
	{
		Cmd_Help_f (ent);
		return;
	}

	if (level.intermissiontime)
		return;

	if (Q_stricmp(cmd, "use") == 0)
		Cmd_Use_f(ent);
	else if (Q_stricmp(cmd, "drop") == 0)
		Cmd_Drop_f(ent);
	else if (Q_stricmp(cmd, "give") == 0)
		Cmd_Give_f(ent);
	else if (Q_stricmp(cmd, "god") == 0)
		Cmd_God_f(ent);
	else if (Q_stricmp(cmd, "notarget") == 0)
		Cmd_Notarget_f(ent);
	else if (Q_stricmp(cmd, "noclip") == 0)
		Cmd_Noclip_f(ent);
	else if (Q_stricmp(cmd, "inven") == 0)
		Cmd_Inven_f(ent);
	else if (Q_stricmp(cmd, "invnext") == 0)
		SelectNextItem(ent, -1);
	else if (Q_stricmp(cmd, "invprev") == 0)
		SelectPrevItem(ent, -1);
	else if (Q_stricmp(cmd, "invnextw") == 0)
		SelectNextItem(ent, IT_WEAPON);
	else if (Q_stricmp(cmd, "invprevw") == 0)
		SelectPrevItem(ent, IT_WEAPON);
	else if (Q_stricmp(cmd, "invnextp") == 0)
		SelectNextItem(ent, IT_POWERUP);
	else if (Q_stricmp(cmd, "invprevp") == 0)
		SelectPrevItem(ent, IT_POWERUP);
	else if (Q_stricmp(cmd, "invuse") == 0)
		Cmd_InvUse_f(ent);
	else if (Q_stricmp(cmd, "invdrop") == 0)
		Cmd_InvDrop_f(ent);
	else if (Q_stricmp(cmd, "weapprev") == 0)
		Cmd_WeapPrev_f(ent);
	else if (Q_stricmp(cmd, "weapnext") == 0)
		Cmd_WeapNext_f(ent);
	else if (Q_stricmp(cmd, "weaplast") == 0)
		Cmd_WeapLast_f(ent);
	else if (Q_stricmp(cmd, "kill") == 0)
		Cmd_Kill_f(ent);
	else if (Q_stricmp(cmd, "putaway") == 0)
		Cmd_PutAway_f(ent);
	else if (Q_stricmp(cmd, "wave") == 0)
		Cmd_Wave_f(ent);
	else if (Q_stricmp(cmd, "playerlist") == 0)
		Cmd_PlayerList_f(ent);
	else if (Q_stricmp(cmd, "monsterlist") == 0)
		Cmd_monsterslot_f(ent);
	else if (Q_stricmp(cmd, "monsterclear") == 0)
		Cmd_clearmonsters_f(ent);
	else if (Q_stricmp(cmd, "monster1") == 0)
		Cmd_playmonster1_f(ent);
	else if (Q_stricmp(cmd, "monster2") == 0)
		Cmd_playmonster2_f(ent);
	else if (Q_stricmp(cmd, "monster3") == 0)
		Cmd_playmonster3_f(ent);
	else if (Q_stricmp(cmd, "monsterreturn") == 0)
		Cmd_returnmonster_f(ent);
	else if (Q_stricmp(cmd, "monsterfollow") == 0)
		Cmd_monsterfollow_f(ent);
	else if (Q_stricmp(cmd, "monset1") == 0)
		Cmd_monsterset1_f(ent);
	else if (Q_stricmp(cmd, "monset2") == 0)
		Cmd_monsterset2_f(ent);
	else if (Q_stricmp(cmd, "monset3") == 0)
		Cmd_monsterset3_f(ent);
	else if (Q_stricmp(cmd, "monset4") == 0)
		Cmd_monsterset4_f(ent);
	else if (Q_stricmp(cmd, "monwho") == 0)
		Cmd_monsterwho_f(ent);
	else if (Q_stricmp(cmd, "monexp") == 0)
		Cmd_monsterexp_f(ent);
	else if (Q_stricmp(cmd, "monsterhelp") == 0)
		Cmd_monsterhelp_f(ent);
	else if (Q_stricmp(cmd, "monstercmds") == 0)
		Cmd_monstercommands_f(ent);
	else	// anything that doesn't match a command will be a chat
		Cmd_Say_f (ent, false, true);
}
