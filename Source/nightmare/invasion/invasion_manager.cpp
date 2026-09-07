#include "nightmare/invasion/invasion_manager.hpp"

#include <SDL.h>
#include <algorithm>
#include <cstring>

#include "engine/load_file.hpp"
#include "engine/point.hpp"
#include "engine/sound.h"
#include "levels/gendung.h"
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
	// Cathedral Courtyard
	{ { 23, 27 }, MT_BALROG },
	{ { 28, 28 }, MT_WINGED },
	{ { 22, 33 }, MT_BALROG },
	{ { 29, 36 }, MT_WINGED },
	// Roads & Riverbanks
	{ { 36, 38 }, MT_WINGED },
	{ { 40, 44 }, MT_BALROG },
	{ { 45, 48 }, MT_WINGED },
	{ { 42, 54 }, MT_BALROG },
	{ { 48, 55 }, MT_WINGED },
	// Town Center (Near well/campfire, leaving {61, 71} clear for portal)
	{ { 54, 62 }, MT_WINGED },
	{ { 58, 64 }, MT_BALROG },
	{ { 64, 65 }, MT_WINGED },
	{ { 53, 68 }, MT_BALROG },
	{ { 57, 72 }, MT_WINGED },
	{ { 66, 73 }, MT_BALROG },
	// Towards Blacksmith & Eastern Path
	{ { 65, 52 }, MT_WINGED },
	{ { 69, 50 }, MT_BALROG },
	// Bridge Approach to Peninsula
	{ { 72, 58 }, MT_BALROG },
	{ { 74, 55 }, MT_WINGED },
	{ { 76, 59 }, MT_BALROG }
};

const Point GuardPositions[] = {
	{ 80, 68 },
	{ 83, 69 },
	{ 80, 72 },
	{ 82, 74 }
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

	// Penumbra atmosphere
	for (int i = 0; i < MAXDUNX; i++) {
		for (int j = 0; j < MAXDUNY; j++) {
			dLight[i][j] = 3;
			dPreLight[i][j] = 3;
		}
	}

	// Cathedral entrance spotlight
	AddLight(Point { 25, 31 }, 9);

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

	preloadType(MT_WINGED, PLACE_SCATTER);
	preloadType(MT_BALROG, PLACE_SCATTER);

	if (Quests[Q_SKELKING]._qactive != QUEST_DONE) {
		preloadType(UniqueMonsterType::SkeletonKing, PLACE_UNIQUE);
		preloadType(MT_WSKELAX, PLACE_SCATTER);
	} else {
		preloadType(UniqueMonsterType::WarlordOfBlood, PLACE_UNIQUE);
		preloadType(MT_GUARD, PLACE_SCATTER);
	}

	if (state_.monster_count == 0) {
		SpawnInitialInvasionForce();
	} else {
		RestoreInvasionSnapshot();
	}
}

void InvasionManager::SpawnInitialInvasionForce()
{
	state_.monster_count = 0;

	for (const auto &spawn : RegularSpawns) {
		if (state_.monster_count >= MaxInvasionMonsters)
			break;

		const size_t typeIdx = GetMonsterTypeIndex(spawn.type);
		if (typeIdx >= LevelMonsterTypeCount)
			continue;

		Monster *monster = AddMonster(spawn.pt, Direction::South, typeIdx, true);
		if (monster != nullptr) {
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
	const Point bossPos { 82, 70 };

	if (Quests[Q_SKELKING]._qactive != QUEST_DONE) {
		state_.boss_selected = MT_SKING;
		state_.boss_unique_type = UniqueMonsterType::SkeletonKing;

		const size_t kingTypeIdx = GetMonsterTypeIndex(MT_SKING);
		if (kingTypeIdx < LevelMonsterTypeCount) {
			Monster *king = AddMonster(bossPos, Direction::SouthWest, kingTypeIdx, true);
			if (king != nullptr) {
				PrepareUniqueMonst(*king, UniqueMonsterType::SkeletonKing, 0, 0, UniqueMonstersData[static_cast<size_t>(UniqueMonsterType::SkeletonKing)]);
			}
		}

		const size_t minionTypeIdx = GetMonsterTypeIndex(MT_WSKELAX);
		if (minionTypeIdx < LevelMonsterTypeCount) {
			for (const auto &guardPos : GuardPositions) {
				AddMonster(guardPos, Direction::SouthWest, minionTypeIdx, true);
			}
		}
	} else {
		state_.boss_selected = MT_VTEXLRD;
		state_.boss_unique_type = UniqueMonsterType::WarlordOfBlood;

		const _monster_id warlordBase = UniqueMonstersData[static_cast<size_t>(UniqueMonsterType::WarlordOfBlood)].mtype;
		const size_t warlordTypeIdx = GetMonsterTypeIndex(warlordBase);
		if (warlordTypeIdx < LevelMonsterTypeCount) {
			Monster *warlord = AddMonster(bossPos, Direction::SouthWest, warlordTypeIdx, true);
			if (warlord != nullptr) {
				PrepareUniqueMonst(*warlord, UniqueMonsterType::WarlordOfBlood, 0, 0, UniqueMonstersData[static_cast<size_t>(UniqueMonsterType::WarlordOfBlood)]);
			}
		}

		const size_t minionTypeIdx = GetMonsterTypeIndex(MT_GUARD);
		if (minionTypeIdx < LevelMonsterTypeCount) {
			for (const auto &guardPos : GuardPositions) {
				AddMonster(guardPos, Direction::SouthWest, minionTypeIdx, true);
			}
		}
	}
}

void InvasionManager::Update()
{
	if (!IsCombatActive())
		return;

	if (state_.boss_phase == 0) {
		bool anyRegularAlive = false;
		for (size_t i = 0; i < ActiveMonsterCount; i++) {
			const Monster &monster = Monsters[ActiveMonsters[i]];
			if (!monster.isUnique() && monster.hitPoints > 0 && monster.mode != MonsterMode::Death) {
				anyRegularAlive = true;
				break;
			}
		}

		if (!anyRegularAlive) {
			state_.boss_phase = 1;
			state_.dramatic_pause_start_tick = SDL_GetTicks();
		}
	} else if (state_.boss_phase == 1) {
		const uint32_t now = SDL_GetTicks();
		if (now - state_.dramatic_pause_start_tick >= 2500) {
			state_.boss_phase = 2;
			SpawnBossEncounter();
		}
	} else if (state_.boss_phase == 2) {
		bool bossAlive = false;
		for (size_t i = 0; i < ActiveMonsterCount; i++) {
			const Monster &monster = Monsters[ActiveMonsters[i]];
			if (monster.isUnique() && monster.hitPoints > 0 && monster.mode != MonsterMode::Death) {
				bossAlive = true;
				break;
			}
		}

		if (!bossAlive) {
			state_.boss_phase = 3;
			state_.completed = true;
			state_.active = false;
		}
	}
}

void InvasionManager::OnMonsterDeath(Monster &monster)
{
	if (monster.isUnique() && state_.boss_phase == 2) {
		state_.boss_phase = 3;
		state_.completed = true;
		state_.active = false;
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

		const size_t typeIdx = GetMonsterTypeIndex(snap.type);
		if (typeIdx >= LevelMonsterTypeCount)
			continue;

		Monster *monster = AddMonster(Point { snap.x, snap.y }, Direction::South, typeIdx, true);
		if (monster != nullptr) {
			monster->hitPoints = snap.current_hp;
			if (snap.is_boss && state_.boss_unique_type != UniqueMonsterType::None) {
				PrepareUniqueMonst(*monster, state_.boss_unique_type, 0, 0, UniqueMonstersData[static_cast<size_t>(state_.boss_unique_type)]);
				monster->hitPoints = snap.current_hp;
			}
		}
	}
}

} // namespace invasion
} // namespace nightmare
} // namespace devilution
