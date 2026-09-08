#include "nightmare/invasion/invasion_manager.hpp"

#include <SDL.h>
#include <algorithm>
#include <cstring>
#include <iterator>

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

// Initial squad of exactly 14 minions positioned across key town approaches
const SpawnCoord InitialSquadSpawns[] = {
	{ { 60, 20 }, MT_BALROG },
	{ { 64, 18 }, MT_SUCCUBUS },
	{ { 27, 31 }, MT_BALROG },
	{ { 29, 36 }, MT_WINGED },
	{ { 36, 38 }, MT_BACID },
	{ { 40, 44 }, MT_BGOATMC },
	{ { 65, 52 }, MT_SUCCUBUS },
	{ { 69, 50 }, MT_BALROG },
	{ { 80, 20 }, MT_BACID },
	{ { 83, 22 }, MT_WINGED },
	{ { 72, 58 }, MT_BGOATMC },
	{ { 54, 62 }, MT_WSKELBW },
	{ { 58, 64 }, MT_WSKELAX },
	{ { 64, 65 }, MT_WSKELBW },
};

// 7 regular minion types for the reinforcement pool
const _monster_id MinionPool[] = {
	MT_WSKELAX,
	MT_WSKELBW,
	MT_BGOATMC,
	MT_BACID,
	MT_WINGED,
	MT_BALROG,
	MT_SUCCUBUS,
};

void ConfigureMinionStats(Monster &monster, _monster_id mtype)
{
	monster.intelligence = 2;
	monster.activeForTicks = 0; // Proximity-based awakening to prevent whole-map pathfinding CPU freeze

	switch (mtype) {
	case MT_BALROG:
		monster.maxHitPoints = 2500 << 6;
		monster.minDamage = 35;
		monster.maxDamage = 60;
		monster.armorClass = 65;
		break;
	case MT_SUCCUBUS:
		monster.maxHitPoints = 1600 << 6;
		monster.minDamage = 30;
		monster.maxDamage = 50;
		monster.armorClass = 55;
		break;
	case MT_WINGED:
		monster.maxHitPoints = 1200 << 6;
		monster.minDamage = 25;
		monster.maxDamage = 45;
		monster.armorClass = 50;
		break;
	case MT_BACID:
		monster.maxHitPoints = 1100 << 6;
		monster.minDamage = 20;
		monster.maxDamage = 45;
		monster.armorClass = 45;
		break;
	case MT_BGOATMC:
		monster.maxHitPoints = 1000 << 6;
		monster.minDamage = 22;
		monster.maxDamage = 42;
		monster.armorClass = 50;
		break;
	case MT_WSKELBW:
		monster.maxHitPoints = 750 << 6;
		monster.minDamage = 18;
		monster.maxDamage = 35;
		monster.armorClass = 40;
		break;
	case MT_WSKELAX:
	default:
		monster.maxHitPoints = 850 << 6;
		monster.minDamage = 20;
		monster.maxDamage = 40;
		monster.armorClass = 50;
		break;
	}
	monster.hitPoints = monster.maxHitPoints;
}

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
	state_.total_kills = 0;
	state_.pending_reinforcements = 0;
	state_.leoric_alive = false;
	state_.butcher_spawned = false;
	state_.nakrul_spawned = false;
	state_.diablo_spawned = false;
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
			state_.total_kills = 0;
			state_.pending_reinforcements = 0;
			state_.leoric_alive = true;
			state_.butcher_spawned = false;
			state_.nakrul_spawned = false;
			state_.diablo_spawned = false;
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

	// Precache the 11 invasion monster types
	const auto preloadType = [](auto mtype, placeflag pflag) {
		const auto typeIndex = AddMonsterType(mtype, pflag);
		if (typeIndex) {
			InitMonsterGFX(LevelMonsterTypes[*typeIndex]);
		}
		return typeIndex;
	};

	// Comandante
	preloadType(MT_SKING, PLACE_UNIQUE);
	// Heraldos
	preloadType(MT_CLEAVER, PLACE_UNIQUE);
	preloadType(MT_NAKRUL, PLACE_SPECIAL);
	preloadType(MT_DIABLO, PLACE_SPECIAL);
	// Núcleo de Esbirros
	preloadType(MT_WSKELAX, PLACE_SCATTER);
	preloadType(MT_WSKELBW, PLACE_SCATTER);
	preloadType(MT_BGOATMC, PLACE_SCATTER);
	preloadType(MT_BACID, PLACE_SCATTER);
	preloadType(MT_WINGED, PLACE_SCATTER);
	preloadType(MT_BALROG, PLACE_SCATTER);
	preloadType(MT_SUCCUBUS, PLACE_SCATTER);

	if (state_.monster_count == 0) {
		SpawnInitialInvasionForce();
	} else {
		RestoreInvasionSnapshot();
	}
}

void InvasionManager::SpawnCommanderLeoric()
{
	const Point bossCandidates[] = {
		{ 83, 64 }, { 82, 64 }, { 84, 64 },
		{ 83, 63 }, { 83, 65 }, { 81, 63 },
	};

	state_.boss_selected = MT_SKING;
	state_.boss_unique_type = UniqueMonsterType::SkeletonKing;

	const size_t bossTypeIdx = GetMonsterTypeIndex(MT_SKING);
	if (bossTypeIdx < LevelMonsterTypeCount && state_.monster_count < MaxInvasionMonsters) {
		Monster *boss = nullptr;
		Point usedPos = bossCandidates[0];

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
			PrepareUniqueMonst(*boss, UniqueMonsterType::SkeletonKing, 0, 0, UniqueMonstersData[static_cast<size_t>(UniqueMonsterType::SkeletonKing)]);
			boss->maxHitPoints = 25000 << 6;
			boss->hitPoints = boss->maxHitPoints;
			boss->intelligence = 3;
			boss->activeForTicks = UINT8_MAX;
			boss->minDamage = 75;
			boss->maxDamage = 120;
			boss->armorClass = 95;

			state_.leoric_alive = true;

			auto &snap = state_.monsters[state_.monster_count++];
			snap.x = static_cast<uint8_t>(usedPos.x);
			snap.y = static_cast<uint8_t>(usedPos.y);
			snap.type = MT_SKING;
			snap.current_hp = boss->hitPoints;
			snap.is_boss = true;
			snap.is_alive = true;
		}
	}
}

void InvasionManager::SpawnInitialInvasionForce()
{
	state_.monster_count = 0;
	state_.total_kills = 0;
	state_.pending_reinforcements = 0;
	state_.leoric_alive = true;
	state_.butcher_spawned = false;
	state_.nakrul_spawned = false;
	state_.diablo_spawned = false;

	// 1. Spawnea al Comandante
	SpawnCommanderLeoric();
	state_.boss_phase = 2;

	// 2. Spawnea el escuadrón inicial de 14 esbirros perimetrales
	size_t spawnedMinions = 0;
	for (const auto &spawn : InitialSquadSpawns) {
		if (spawnedMinions >= TargetInvasionMinions)
			break;
		if (!IsTileWalkable(spawn.pt))
			continue;

		const size_t typeIdx = GetMonsterTypeIndex(spawn.type);
		if (typeIdx >= LevelMonsterTypeCount)
			continue;

		Monster *monster = AddMonster(spawn.pt, Direction::South, typeIdx, true);
		if (monster != nullptr) {
			ConfigureMinionStats(*monster, spawn.type);
			spawnedMinions++;

			if (state_.monster_count < MaxInvasionMonsters) {
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
}

Point InvasionManager::GetWalkablePerimeterSpawn()
{
	static const Point s_perimeter[] = {
		{ 60, 20 }, { 64, 18 }, { 27, 31 }, { 36, 38 },
		{ 65, 52 }, { 69, 50 }, { 80, 20 }, { 75, 62 },
		{ 45, 48 }, { 58, 64 }, { 34, 46 }, { 72, 40 },
	};
	static size_t s_idx = 0;
	for (size_t i = 0; i < std::size(s_perimeter); i++) {
		Point pt = s_perimeter[(s_idx + i) % std::size(s_perimeter)];
		if (InDungeonBounds(pt) && IsTileWalkable(pt) && dMonster[pt.x][pt.y] == 0 && dPlayer[pt.x][pt.y] == 0) {
			s_idx = (s_idx + i + 1) % std::size(s_perimeter);
			return pt;
		}
	}

	auto freePos = FindClosestValidPosition([](Point target) {
		return InDungeonBounds(target) && IsTileWalkable(target) && dMonster[target.x][target.y] == 0 && dPlayer[target.x][target.y] == 0;
	}, { 58, 64 }, 1, 8);

	if (freePos)
		return *freePos;

	return { 58, 64 };
}

void InvasionManager::SpawnHeraldButcher()
{
	const size_t typeIdx = GetMonsterTypeIndex(MT_CLEAVER);
	if (typeIdx >= LevelMonsterTypeCount || ActiveMonsterCount >= MaxMonsters)
		return;

	const Point spawnPt = GetWalkablePerimeterSpawn();
	Monster *butcher = AddMonster(spawnPt, Direction::South, typeIdx, true);
	if (butcher != nullptr) {
		PrepareUniqueMonst(*butcher, UniqueMonsterType::Butcher, 0, 0, UniqueMonstersData[static_cast<size_t>(UniqueMonsterType::Butcher)]);
		butcher->maxHitPoints = 14000 << 6;
		butcher->hitPoints = butcher->maxHitPoints;
		butcher->intelligence = 3;
		butcher->activeForTicks = UINT8_MAX;
		butcher->minDamage = 65;
		butcher->maxDamage = 110;
		butcher->armorClass = 85;
		PlaySFX(SfxID::ButcherGreeting);
	}
}

void InvasionManager::SpawnHeraldNaKrul()
{
	const size_t typeIdx = GetMonsterTypeIndex(MT_NAKRUL);
	if (typeIdx >= LevelMonsterTypeCount || ActiveMonsterCount >= MaxMonsters)
		return;

	const Point spawnPt = GetWalkablePerimeterSpawn();
	Monster *nakrul = AddMonster(spawnPt, Direction::South, typeIdx, true);
	if (nakrul != nullptr) {
		PrepareUniqueMonst(*nakrul, UniqueMonsterType::NaKrul, 0, 0, UniqueMonstersData[static_cast<size_t>(UniqueMonsterType::NaKrul)]);
		nakrul->maxHitPoints = 22000 << 6;
		nakrul->hitPoints = nakrul->maxHitPoints;
		nakrul->intelligence = 3;
		nakrul->activeForTicks = UINT8_MAX;
		nakrul->minDamage = 80;
		nakrul->maxDamage = 130;
		nakrul->armorClass = 95;
		PlaySFX(SfxID::NaKrul4);
	}
}

void InvasionManager::SpawnHeraldDiablo()
{
	const size_t typeIdx = GetMonsterTypeIndex(MT_DIABLO);
	if (typeIdx >= LevelMonsterTypeCount || ActiveMonsterCount >= MaxMonsters)
		return;

	const Point spawnPt = GetWalkablePerimeterSpawn();
	Monster *diablo = AddMonster(spawnPt, Direction::South, typeIdx, true);
	if (diablo != nullptr) {
		diablo->maxHitPoints = 30000 << 6;
		diablo->hitPoints = diablo->maxHitPoints;
		diablo->intelligence = 3;
		diablo->activeForTicks = UINT8_MAX;
		diablo->minDamage = 90;
		diablo->maxDamage = 150;
		diablo->armorClass = 100;
		PlayEffect(*diablo, MonsterSound::Special);
		PlaySFX(SfxID::BigExplosion);
	}
}

bool InvasionManager::SpawnOneReinforcement()
{
	if (ActiveMonsterCount >= MaxMonsters)
		return false;

	static uint8_t s_reinforceIdx = 0;
	const _monster_id mtype = MinionPool[s_reinforceIdx % std::size(MinionPool)];
	s_reinforceIdx++;

	const size_t typeIdx = GetMonsterTypeIndex(mtype);
	if (typeIdx >= LevelMonsterTypeCount)
		return false;

	const Point spawnPt = GetWalkablePerimeterSpawn();
	Monster *monster = AddMonster(spawnPt, Direction::South, typeIdx, true);
	if (monster != nullptr) {
		ConfigureMinionStats(*monster, mtype);
		return true;
	}
	return false;
}

void InvasionManager::Update()
{
	if (!IsCombatActive())
		return;

	// 1. Count active living invasion monsters
	size_t activeMinions = 0;
	size_t activeBosses = 0;
	for (size_t i = 0; i < ActiveMonsterCount; i++) {
		const Monster &m = Monsters[ActiveMonsters[i]];
		if (m.hasNoLife() || m.isInvalid || m.mode == MonsterMode::Death || m.isPlayerMinion())
			continue;
		if (m.isUnique() || m.type().type == MT_DIABLO)
			activeBosses++;
		else
			activeMinions++;
	}

	// 2. Victory Condition: Leoric is dead and all remaining minions/heralds are cleared
	if (!state_.leoric_alive && activeMinions == 0 && activeBosses == 0 && state_.total_kills > 0) {
		state_.completed = true;
		state_.active = false;
		state_.boss_phase = 3;
		return;
	}

	// 3. Persistent Milestone Herald Spawns (Independent of minion replacements)
	if (state_.leoric_alive) {
		if (!state_.butcher_spawned && state_.total_kills >= 15) {
			SpawnHeraldButcher();
			state_.butcher_spawned = true;
		} else if (!state_.nakrul_spawned && state_.total_kills >= 35) {
			SpawnHeraldNaKrul();
			state_.nakrul_spawned = true;
		} else if (!state_.diablo_spawned && state_.total_kills >= 55) {
			SpawnHeraldDiablo();
			state_.diablo_spawned = true;
		}
	}

	// 4. Elastic Minion Reinforcements (Target: 14, Max: 15)
	// Only dispatch if Leoric is alive, pending count > 0, and active minions are below Target (14)
	if (state_.leoric_alive && state_.pending_reinforcements > 0 && activeMinions < TargetInvasionMinions) {
		if (SpawnOneReinforcement()) {
			state_.pending_reinforcements--;
		}
	}
}

void InvasionManager::OnMonsterDeath(Monster &monster)
{
	if (!IsCombatActive())
		return;

	if (monster.isPlayerMinion())
		return;

	// Did Leoric fall?
	if (monster.isUnique() && monster.uniqueType == UniqueMonsterType::SkeletonKing) {
		state_.leoric_alive = false;
		state_.boss_phase = 3;
		state_.pending_reinforcements = 0; // Seal the breach immediately!
		return;
	}

	state_.total_kills++;

	// Regular minion deaths feed the reinforcement queue while Leoric lives
	const bool isHeraldBoss = monster.isUnique() || monster.type().type == MT_DIABLO;
	if (state_.leoric_alive && !isHeraldBoss) {
		state_.pending_reinforcements++;
	}
}

void InvasionManager::SaveInvasionSnapshot()
{
	if (!state_.active)
		return;

	state_.monster_count = 0;
	for (size_t i = 0; i < ActiveMonsterCount && state_.monster_count < MaxInvasionMonsters; i++) {
		const Monster &monster = Monsters[ActiveMonsters[i]];
		if (monster.isPlayerMinion())
			continue;
		auto &snap = state_.monsters[state_.monster_count++];
		snap.current_hp = monster.hitPoints;
		snap.x = static_cast<uint8_t>(monster.position.tile.x);
		snap.y = static_cast<uint8_t>(monster.position.tile.y);
		snap.type = LevelMonsterTypes[monster.levelType].type;
		snap.is_boss = monster.isUnique() || snap.type == MT_DIABLO;
		snap.is_alive = (monster.hitPoints > 0 && monster.mode != MonsterMode::Death);
	}
}

void InvasionManager::RestoreInvasionSnapshot()
{
	for (size_t i = 0; i < state_.monster_count; i++) {
		const auto &snap = state_.monsters[i];
		if (!snap.is_alive || snap.current_hp <= 0)
			continue;

		if (!IsTileWalkable(Point { snap.x, snap.y }))
			continue;

		const size_t typeIdx = GetMonsterTypeIndex(snap.type);
		if (typeIdx >= LevelMonsterTypeCount)
			continue;

		Monster *monster = AddMonster(Point { snap.x, snap.y }, Direction::South, typeIdx, true);
		if (monster != nullptr) {
			if (snap.type == MT_SKING) {
				PrepareUniqueMonst(*monster, UniqueMonsterType::SkeletonKing, 0, 0, UniqueMonstersData[static_cast<size_t>(UniqueMonsterType::SkeletonKing)]);
				monster->maxHitPoints = 25000 << 6;
				monster->intelligence = 3;
				monster->activeForTicks = UINT8_MAX;
				monster->minDamage = 75;
				monster->maxDamage = 120;
				monster->armorClass = 95;
			} else if (snap.type == MT_CLEAVER) {
				PrepareUniqueMonst(*monster, UniqueMonsterType::Butcher, 0, 0, UniqueMonstersData[static_cast<size_t>(UniqueMonsterType::Butcher)]);
				monster->maxHitPoints = 14000 << 6;
				monster->intelligence = 3;
				monster->activeForTicks = UINT8_MAX;
				monster->minDamage = 65;
				monster->maxDamage = 110;
				monster->armorClass = 85;
			} else if (snap.type == MT_NAKRUL) {
				PrepareUniqueMonst(*monster, UniqueMonsterType::NaKrul, 0, 0, UniqueMonstersData[static_cast<size_t>(UniqueMonsterType::NaKrul)]);
				monster->maxHitPoints = 22000 << 6;
				monster->intelligence = 3;
				monster->activeForTicks = UINT8_MAX;
				monster->minDamage = 80;
				monster->maxDamage = 130;
				monster->armorClass = 95;
			} else if (snap.type == MT_DIABLO) {
				monster->maxHitPoints = 30000 << 6;
				monster->intelligence = 3;
				monster->activeForTicks = UINT8_MAX;
				monster->minDamage = 90;
				monster->maxDamage = 150;
				monster->armorClass = 100;
			} else {
				ConfigureMinionStats(*monster, snap.type);
			}
			monster->hitPoints = snap.current_hp;
		}
	}
}

} // namespace invasion
} // namespace nightmare
} // namespace devilution
