#include "nightmare/invasion/invasion_manager.hpp"

#include <SDL.h>
#include <algorithm>
#include <cstring>

#include "engine/load_file.hpp"
#include "engine/point.hpp"
#include "engine/sound.h"
#include "levels/gendung.h"
#include "levels/tile_properties.hpp"
#include "lighting.h"
#include "loadsave.h"
#include "monster.h"
#include "pfile.h"
#include "quests.h"

namespace devilution {
namespace nightmare {
namespace invasion {

namespace {

struct SpawnCoord {
	Point pt;
	_monster_id type;
};

size_t GetMonsterTypeIndex(_monster_id type)
{
	for (size_t i = 0; i < LevelMonsterTypeCount; i++) {
		if (LevelMonsterTypes[i].type == type)
			return i;
	}
	return LevelMonsterTypeCount;
}

const SpawnCoord RegularSpawns[] = {
	// Cathedral Courtyard & Steps (8 enemies)
	{ { 27, 31 }, MT_BALROG },
	{ { 29, 32 }, MT_WINGED },
	{ { 25, 34 }, MT_BALROG },
	{ { 29, 36 }, MT_WINGED },
	{ { 28, 33 }, MT_WSKELAX },
	{ { 26, 36 }, MT_WSKELAX },
	{ { 24, 32 }, MT_WSKELAX },
	{ { 30, 30 }, MT_WSKELAX },

	// Roads to Town & Cemetery Path (10 enemies)
	{ { 36, 38 }, MT_WINGED },
	{ { 40, 44 }, MT_BALROG },
	{ { 45, 48 }, MT_WINGED },
	{ { 42, 54 }, MT_BALROG },
	{ { 48, 55 }, MT_WINGED },
	{ { 38, 41 }, MT_WSKELAX },
	{ { 43, 46 }, MT_WSKELAX },
	{ { 45, 52 }, MT_WSKELAX },
	{ { 34, 40 }, MT_WSKELAX },
	{ { 40, 48 }, MT_WSKELAX },

	// Town Center, Tavern & Campfire (10 enemies)
	{ { 54, 62 }, MT_WINGED },
	{ { 58, 64 }, MT_BALROG },
	{ { 64, 65 }, MT_WINGED },
	{ { 53, 68 }, MT_BALROG },
	{ { 57, 72 }, MT_WINGED },
	{ { 66, 73 }, MT_BALROG },
	{ { 56, 66 }, MT_WSKELAX },
	{ { 62, 68 }, MT_WSKELAX },
	{ { 50, 64 }, MT_WSKELAX },
	{ { 60, 60 }, MT_WSKELAX },

	// Blacksmith Area & Southern Road (6 enemies)
	{ { 65, 52 }, MT_WINGED },
	{ { 69, 50 }, MT_BALROG },
	{ { 67, 47 }, MT_WSKELAX },
	{ { 71, 52 }, MT_WSKELAX },
	{ { 64, 48 }, MT_BALROG },
	{ { 68, 55 }, MT_WSKELAX },

	// Lester the farmer & Northern Cow Pasture (16 enemies)
	{ { 60, 20 }, MT_BALROG },
	{ { 64, 18 }, MT_WINGED },
	{ { 56, 17 }, MT_WINGED },
	{ { 62, 24 }, MT_BALROG },
	{ { 58, 19 }, MT_WSKELAX },
	{ { 62, 17 }, MT_WSKELAX },
	{ { 65, 22 }, MT_WSKELAX },
	{ { 54, 15 }, MT_WSKELAX },
	{ { 58, 14 }, MT_WSKELAX },
	{ { 66, 15 }, MT_WSKELAX },
	{ { 68, 18 }, MT_WINGED },
	{ { 52, 20 }, MT_WSKELAX },
	{ { 50, 24 }, MT_BALROG },
	{ { 68, 24 }, MT_WSKELAX },
	{ { 56, 26 }, MT_WSKELAX },
	{ { 70, 20 }, MT_WSKELAX },

	// Adria the witch's shack & Eastern Clearing (7 enemies)
	{ { 80, 20 }, MT_BALROG },
	{ { 83, 22 }, MT_WINGED },
	{ { 78, 25 }, MT_WINGED },
	{ { 85, 21 }, MT_BALROG },
	{ { 82, 18 }, MT_WSKELAX },
	{ { 79, 23 }, MT_WSKELAX },
	{ { 84, 25 }, MT_WSKELAX },

	// Bridge Approach (West riverbank) (5 enemies)
	{ { 72, 58 }, MT_BALROG },
	{ { 73, 56 }, MT_WINGED },
	{ { 75, 62 }, MT_BALROG },
	{ { 74, 60 }, MT_WSKELAX },
	{ { 76, 63 }, MT_WSKELAX },

	// Peninsula Perimeter Commanders (2 enemies)
	{ { 81, 62 }, MT_WINGED },
	{ { 85, 65 }, MT_BALROG },

	// Extra assault wave -- mid-town flankers (10 more enemies)
	{ { 52, 58 }, MT_BALROG },
	{ { 55, 56 }, MT_WINGED },
	{ { 47, 60 }, MT_WSKELAX },
	{ { 44, 57 }, MT_BALROG },
	{ { 46, 64 }, MT_WINGED },
	{ { 70, 44 }, MT_WSKELAX },
	{ { 72, 40 }, MT_BALROG },
	{ { 74, 36 }, MT_WINGED },
	{ { 34, 46 }, MT_WSKELAX },
	{ { 32, 50 }, MT_BALROG },
};

const Point GuardPositions[] = {
	{ 82, 63 },
	{ 84, 63 },
	{ 82, 65 },
	{ 84, 65 },
	{ 81, 64 },
	{ 85, 64 },
	{ 83, 62 },
	{ 83, 66 }
};

} // namespace

InvasionManager &InvasionManager::Get()
{
	static InvasionManager instance;
	return instance;
}

InvasionManager::InvasionManager()
{
	ResetInvasionState();
}

void InvasionManager::ResetInvasionState()
{
	std::memset(&state_, 0, sizeof(state_));
	state_.active = false;
	state_.completed = false;
	state_.boss_phase = 0;
	state_.boss_selected = MT_INVALID;
	state_.boss_unique_type = UniqueMonsterType::None;
	state_.monster_count = 0;
}

bool InvasionManager::IsInvaded() const
{
	return state_.active || state_.completed;
}

bool InvasionManager::IsCombatActive() const
{
	return state_.active && !state_.completed;
}

bool InvasionManager::IsCompleted() const
{
	return state_.completed;
}

void InvasionManager::CheckInvasionTrigger()
{
	if (!state_.active && !state_.completed) {
		if (Quests[Q_NAKRUL]._qactive == QUEST_DONE) {
			state_.active = true;
			state_.completed = false;
			state_.boss_phase = 0;
			state_.monster_count = 0;
		}
	}
}

void InvasionManager::OnTownEntry()
{
	CheckInvasionTrigger();

	if (!IsInvaded())
		return;

	if (state_.completed)
		return;

	// Precache required monster GFX before placing monsters
	const auto preloadType = [](auto mtype, placeflag pflag) {
		const auto typeIndex = AddMonsterType(mtype, pflag);
		if (typeIndex) {
			InitMonsterGFX(LevelMonsterTypes[*typeIndex]);
		}
		return typeIndex;
	};

	const UniqueMonsterType bossUnique = (state_.monster_count > 0 && state_.boss_unique_type != UniqueMonsterType::None)
	    ? state_.boss_unique_type
	    : ((Quests[Q_SKELKING]._qactive == QUEST_DONE) ? UniqueMonsterType::WarlordOfBlood : UniqueMonsterType::SkeletonKing);
	const _monster_id bossMType = UniqueMonstersData[static_cast<size_t>(bossUnique)].mtype;

	preloadType(MT_WINGED, PLACE_SCATTER);
	preloadType(MT_BALROG, PLACE_SCATTER);
	preloadType(bossMType, PLACE_UNIQUE);
	preloadType(MT_WSKELAX, PLACE_SCATTER);

	if (state_.monster_count == 0) {
		SpawnInitialInvasionForce();
	} else {
		RestoreInvasionSnapshot();
	}
}

void InvasionManager::SpawnInitialInvasionForce()
{
	state_.monster_count = 0;

	// --- Spawn Boss FIRST to guarantee he always gets a slot ---
	// The boss encounter reserves its own slots before the army fills up.
	SpawnBossEncounter();
	state_.boss_phase = 2;

	// --- Then fill remaining slots with the regular invasion army ---
	for (const auto &spawn : RegularSpawns) {
		if (state_.monster_count >= MaxInvasionMonsters)
			break;

		// Skip spawn points that land on solid/unwalkable tiles (rooftops, walls)
		if (!IsTileWalkable(spawn.pt))
			continue;

		const size_t typeIdx = GetMonsterTypeIndex(spawn.type);
		if (typeIdx >= LevelMonsterTypeCount)
			continue;

		Monster *monster = AddMonster(spawn.pt, Direction::South, typeIdx, true);
		if (monster != nullptr) {
			monster->intelligence = 2;
			monster->activeForTicks = 0; // Proximity-based awakening to prevent WebAssembly CPU pathfinding freeze
			if (monster->type().type == MT_BALROG) {
				monster->maxHitPoints = 2500 << 6;
				monster->hitPoints = monster->maxHitPoints;
				monster->minDamage = 35;
				monster->maxDamage = 60;
				monster->armorClass = 65;
			} else if (monster->type().type == MT_WINGED) {
				monster->maxHitPoints = 1200 << 6;
				monster->hitPoints = monster->maxHitPoints;
				monster->minDamage = 25;
				monster->maxDamage = 45;
				monster->armorClass = 50;
			} else if (monster->type().type == MT_WSKELAX) {
				monster->maxHitPoints = 850 << 6;
				monster->hitPoints = monster->maxHitPoints;
				monster->minDamage = 20;
				monster->maxDamage = 40;
				monster->armorClass = 50;
			}

			auto &snap = state_.monsters[state_.monster_count++];
			snap.x = static_cast<uint8_t>(spawn.pt.x);
			snap.y = static_cast<uint8_t>(spawn.pt.y);
			snap.type = spawn.type;
			snap.current_hp = monster->hitPoints;
			snap.is_boss = false;
			snap.is_alive = true;
		}
	}
}

void InvasionManager::SpawnBossEncounter()
{
	// Primary position on the peninsula. Fallbacks spread nearby in case
	// the primary tile is blocked by another monster or object.
	const Point bossCandidates[] = {
		{ 83, 64 }, { 82, 64 }, { 84, 64 },
		{ 83, 63 }, { 83, 65 }, { 81, 63 },
	};

	const bool kingDead = (Quests[Q_SKELKING]._qactive == QUEST_DONE);
	const UniqueMonsterType bossUnique = kingDead ? UniqueMonsterType::WarlordOfBlood : UniqueMonsterType::SkeletonKing;
	const _monster_id bossMType = UniqueMonstersData[static_cast<size_t>(bossUnique)].mtype;

	state_.boss_selected = bossMType;
	state_.boss_unique_type = bossUnique;

	const size_t bossTypeIdx = GetMonsterTypeIndex(bossMType);
	if (bossTypeIdx < LevelMonsterTypeCount && state_.monster_count < MaxInvasionMonsters) {
		Monster *boss = nullptr;
		Point usedPos = bossCandidates[0];

		// Try each candidate position until one succeeds
		for (const auto &candidate : bossCandidates) {
			if (!IsTileWalkable(candidate))
				continue;
			boss = AddMonster(candidate, Direction::SouthWest, bossTypeIdx, true);
			if (boss != nullptr) {
				usedPos = candidate;
				break;
			}
		}

		if (boss != nullptr) {
			PrepareUniqueMonst(*boss, bossUnique, 0, 0, UniqueMonstersData[static_cast<size_t>(bossUnique)]);
			boss->maxHitPoints = 18000 << 6;
			boss->hitPoints = boss->maxHitPoints;
			boss->intelligence = 3;
			boss->activeForTicks = UINT8_MAX;
			boss->minDamage = 75;
			boss->maxDamage = 120;
			boss->armorClass = 95;

			auto &snap = state_.monsters[state_.monster_count++];
			snap.x = static_cast<uint8_t>(usedPos.x);
			snap.y = static_cast<uint8_t>(usedPos.y);
			snap.type = bossMType;
			snap.current_hp = boss->hitPoints;
			snap.is_boss = true;
			snap.is_alive = true;
		}
	}

	const size_t minionTypeIdx = GetMonsterTypeIndex(MT_WSKELAX);
	if (minionTypeIdx < LevelMonsterTypeCount) {
		for (const auto &guardPos : GuardPositions) {
			if (state_.monster_count >= MaxInvasionMonsters)
				break;
			if (!IsTileWalkable(guardPos))
				continue;
			Monster *guard = AddMonster(guardPos, Direction::SouthWest, minionTypeIdx, true);
			if (guard != nullptr) {
				guard->maxHitPoints = 1000 << 6;
				guard->hitPoints = guard->maxHitPoints;
				guard->intelligence = 3;
				guard->activeForTicks = UINT8_MAX;
				guard->minDamage = 25;
				guard->maxDamage = 45;
				guard->armorClass = 55;

				auto &snap = state_.monsters[state_.monster_count++];
				snap.x = static_cast<uint8_t>(guardPos.x);
				snap.y = static_cast<uint8_t>(guardPos.y);
				snap.type = MT_WSKELAX;
				snap.current_hp = guard->hitPoints;
				snap.is_boss = false;
				snap.is_alive = true;
			}
		}
	}
}

void InvasionManager::Update()
{
	if (!IsCombatActive())
		return;

	// Count all living invasion monsters (skip player golems)
	int aliveCount = 0;
	for (size_t i = 0; i < ActiveMonsterCount; i++) {
		const Monster &monster = Monsters[ActiveMonsters[i]];
		if (monster.type().type == MT_GOLEM || (monster.flags & MFLAG_TARGETS_MONSTER) != 0)
			continue;
		if (monster.hitPoints > 0 && monster.mode != MonsterMode::Death) {
			aliveCount++;
		}
	}

	// Only end the invasion when every single invasion monster is dead
	if (aliveCount == 0 && state_.monster_count > 0) {
		state_.boss_phase = 3;
		state_.completed = true;
		state_.active = false;
	}
}

void InvasionManager::OnMonsterDeath(Monster &monster)
{
	// Note boss death for bookkeeping, but do NOT end the invasion here.
	// Update() is the authority — invasion ends only when ALL monsters are dead.
	if (monster.isUnique()) {
		state_.boss_phase = 3;
	}
}

void InvasionManager::SaveInvasionSnapshot()
{
	if (!state_.active)
		return;

	state_.monster_count = 0;
	for (size_t i = 0; i < ActiveMonsterCount && state_.monster_count < MaxInvasionMonsters; i++) {
		const Monster &monster = Monsters[ActiveMonsters[i]];
		auto &snap = state_.monsters[state_.monster_count++];
		snap.current_hp = monster.hitPoints;
		snap.x = static_cast<uint8_t>(monster.position.tile.x);
		snap.y = static_cast<uint8_t>(monster.position.tile.y);
		snap.type = LevelMonsterTypes[monster.levelType].type;
		snap.is_boss = monster.isUnique();
		snap.is_alive = (monster.hitPoints > 0 && monster.mode != MonsterMode::Death);
	}
}

void InvasionManager::RestoreInvasionSnapshot()
{
	for (size_t i = 0; i < state_.monster_count; i++) {
		const auto &snap = state_.monsters[i];
		if (!snap.is_alive || snap.current_hp <= 0)
			continue;

		// Skip any snapshot positions that are no longer walkable
		if (!IsTileWalkable(Point { snap.x, snap.y }))
			continue;

		const size_t typeIdx = GetMonsterTypeIndex(snap.type);
		if (typeIdx >= LevelMonsterTypeCount)
			continue;

		Monster *monster = AddMonster(Point { snap.x, snap.y }, Direction::South, typeIdx, true);
		if (monster != nullptr) {
			// Restore aggression and base stats matching initial spawn values
			monster->intelligence = 2;
			monster->activeForTicks = 0;
			if (snap.is_boss && state_.boss_unique_type != UniqueMonsterType::None) {
				PrepareUniqueMonst(*monster, state_.boss_unique_type, 0, 0, UniqueMonstersData[static_cast<size_t>(state_.boss_unique_type)]);
				monster->hitPoints = snap.current_hp;
				monster->intelligence = 3;
				monster->activeForTicks = 0;
				monster->minDamage = 75;
				monster->maxDamage = 120;
				monster->armorClass = 95;
			} else {
				monster->hitPoints = snap.current_hp;
				// Restore type-specific stats
				if (snap.type == MT_BALROG) {
					monster->maxHitPoints = 2500 << 6;
					monster->minDamage = 35;
					monster->maxDamage = 60;
					monster->armorClass = 65;
				} else if (snap.type == MT_WINGED) {
					monster->maxHitPoints = 1200 << 6;
					monster->minDamage = 25;
					monster->maxDamage = 45;
					monster->armorClass = 50;
				} else if (snap.type == MT_WSKELAX) {
					monster->maxHitPoints = 850 << 6;
					monster->minDamage = 20;
					monster->maxDamage = 40;
					monster->armorClass = 50;
				}
			}
		}
	}
}

} // namespace invasion
} // namespace nightmare
} // namespace devilution
