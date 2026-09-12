/**
 * @file portal.cpp
 *
 * Implementation of functionality for handling town portals.
 */
#include "portal.h"

#include "lighting.h"
#include "missiles.h"
#include "multi.h"
#include "player.h"
#include "tables/misdat.h"

namespace devilution {

/** In-game state of portals. */
Portal Portals[MAXPORTAL];

namespace {

/** Current portal number (a portal array index). */
size_t portalindex;

/** Coordinate of each player's portal in town. */
Point PortalTownPosition[MAXPORTAL] = {
	{ 57, 40 },
	{ 59, 40 },
	{ 61, 40 },
	{ 63, 40 },
};

} // namespace

void InitPortals()
{
	for (auto &portal : Portals) {
		portal.open = false;
	}
}

void SetPortalStats(int i, bool o, Point position, int lvl, dungeon_type lvltype, bool isSetLevel)
{
	if (i < 0 || i >= MAXPORTAL)
		return;
	Portals[i].open = o;
	Portals[i].position = position;
	Portals[i].level = lvl;
	Portals[i].ltype = lvltype;
	Portals[i].setlvl = isSetLevel;
}

void AddPortalMissile(const Player &player, Point position, bool sync)
{
	auto *missile = AddMissile({ 0, 0 }, position, Direction::South, MissileID::TownPortal, TARGET_MONSTERS, player, 0, 0, /*parent=*/nullptr, SfxID::None);
	if (missile != nullptr) {
		// Don't show portal opening animation if we sync existing portals
		if (sync)
			missile->setFrameGroup<PortalFrame>(PortalFrame::Idle);

		if (leveltype != DTYPE_TOWN)
			missile->_mlid = AddLight(missile->position.tile, 15);
	}
}

void SyncPortals()
{
	for (int i = 0; i < MAXPORTAL; i++) {
		if (!Portals[i].open)
			continue;
		const Player &player = Players[i];
		if (leveltype == DTYPE_TOWN)
			AddPortalMissile(player, PortalTownPosition[i], true);
		else {
			int lvl = currlevel;
			if (setlevel)
				lvl = setlvlnum;
			if (Portals[i].level == lvl && Portals[i].setlvl == setlevel)
				AddPortalMissile(player, Portals[i].position, true);
		}
	}
}

void AddPortalInTown(const Player &player)
{
	const size_t id = player.getId();
	if (id >= MAXPORTAL)
		return;
	AddPortalMissile(player, PortalTownPosition[id], false);
}

void ActivatePortal(const Player &player, Point position, int lvl, dungeon_type dungeonType, bool isSetLevel)
{
	const size_t id = player.getId();
	if (id >= MAXPORTAL)
		return;
	Portal &portal = Portals[id];
	portal.open = true;

	if (lvl != 0) {
		portal.position = position;
		portal.level = lvl;
		portal.ltype = dungeonType;
		portal.setlvl = isSetLevel;
	}
}

void DeactivatePortal(const Player &player)
{
	const size_t id = player.getId();
	if (id >= MAXPORTAL)
		return;
	Portals[id].open = false;
}

bool PortalOnLevel(const Player &player)
{
	const size_t id = player.getId();
	if (id >= MAXPORTAL)
		return false;
	const Portal &portal = Portals[id];
	if (portal.setlvl == setlevel && portal.level == (setlevel ? static_cast<int>(setlvlnum) : currlevel))
		return true;

	return leveltype == DTYPE_TOWN;
}

void RemovePortalMissile(const Player &player)
{
	const size_t id = player.getId();
	if (id >= MAXPORTAL)
		return;
	Missiles.remove_if([id](Missile &missile) {
		if ((missile._mitype == MissileID::TownPortal || missile._mitype == MissileID::RedPortal) && missile._misource == static_cast<int>(id)) {
			if (InDungeonBounds(missile.position.tile)) {
				dFlags[missile.position.tile.x][missile.position.tile.y] &= ~DungeonFlag::Missile;
			}

			if (Portals[id].level != 0)
				AddUnLight(missile._mlid);

			return true;
		}
		return false;
	});
}

void SetCurrentPortal(size_t p)
{
	portalindex = p;
}

void GetPortalLevel()
{
	if (leveltype != DTYPE_TOWN) {
		setlevel = false;
		currlevel = 0;
		MyPlayer->setLevel(0);
		leveltype = DTYPE_TOWN;
		return;
	}

	const size_t idx = (portalindex < MAXPORTAL) ? portalindex : 0;

	if (Portals[idx].setlvl) {
		setlevel = true;
		setlvlnum = (_setlevels)Portals[idx].level;
		currlevel = Portals[idx].level;
		MyPlayer->setLevel(setlvlnum);
		setlvltype = leveltype = Portals[idx].ltype;
	} else {
		setlevel = false;
		currlevel = Portals[idx].level;
		MyPlayer->setLevel(currlevel);
		leveltype = Portals[idx].ltype;
	}

	if (idx == MyPlayerId) {
		NetSendCmd(true, CMD_DEACTIVATEPORTAL);
		DeactivatePortal(*MyPlayer);
	}
}

void GetPortalLvlPos()
{
	const size_t idx = (portalindex < MAXPORTAL) ? portalindex : 0;
	if (leveltype == DTYPE_TOWN) {
		ViewPosition = PortalTownPosition[idx] + Displacement { 1, 1 };
	} else {
		Point pos = Portals[idx].position;
		if (idx != MyPlayerId) {
			pos.x++;
			pos.y++;
		}
		if (InDungeonBounds(pos)) {
			ViewPosition = pos;
		}
	}
}

bool PosOkPortal(int lvl, Point position)
{
	for (auto &portal : Portals) {
		if (portal.open
		    && portal.level == lvl
		    && ((portal.position == position)
		        || (portal.position == position - Displacement { 1, 1 })))
			return true;
	}
	return false;
}

} // namespace devilution
