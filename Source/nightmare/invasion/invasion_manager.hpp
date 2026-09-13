#pragma once

#include <cstddef>
#include <cstdint>

#include "engine/point.hpp"
#include "monster.h"

namespace devilution {

class MpqWriter;
using SaveWriter = MpqWriter;

namespace nightmare {
namespace invasion {

constexpr size_t MinInvasionMinions = 13;
constexpr size_t TargetInvasionMinions = 14;
constexpr size_t MaxInvasionMinions = 15;
constexpr size_t MaxInvasionMonsters = 100;

enum class InvasionTier : uint8_t {
	None = 0,
	MinorScout = 1,   // Cathedral explored: 4 scouts
	MediumRaid = 2,   // Catacombs explored: 8 raiders
	MajorAssault = 3, // Caves/Hell explored: 12 assault demons
	GrandSiege = 4,   // Endgame: full siege with Leoric and Heralds
};

struct InvasionMonsterSnapshot {
	int32_t current_hp;
	uint8_t x;
	uint8_t y;
	_monster_id type;
	bool is_boss;
	bool is_alive;
};

struct TristramInvasionState {
	bool active;
	bool completed;
	InvasionTier current_tier;
	uint8_t highest_tier_completed;
	uint8_t boss_phase; // 0: not spawned, 1: dramatic pause, 2: spawned/fighting, 3: dead
	uint32_t dramatic_pause_start_tick;
	_monster_id boss_selected;
	UniqueMonsterType boss_unique_type;
	uint8_t monster_count;
	uint32_t total_kills;
	uint16_t pending_reinforcements;
	bool leoric_alive;
	bool butcher_spawned;
	bool nakrul_spawned;
	bool diablo_spawned;
	InvasionMonsterSnapshot monsters[MaxInvasionMonsters];
};

class InvasionManager {
public:
	static InvasionManager &Get();

	bool IsInvaded() const;
	bool IsCombatActive() const;
	bool IsCompleted() const;

	void CheckInvasionTrigger();
	void OnTownEntry();
	void Update();
	void OnMonsterDeath(Monster &monster);

	void SaveInvasionSnapshot();
	void RestoreInvasionSnapshot();

	TristramInvasionState &GetState() { return state_; }
	const TristramInvasionState &GetState() const { return state_; }

	void ResetInvasionState();

private:
	InvasionManager();
	void SpawnInitialInvasionForce();
	void SpawnCommanderLeoric();
	void SpawnHeraldButcher();
	void SpawnHeraldNaKrul();
	void SpawnHeraldDiablo();
	bool SpawnOneReinforcement();
	Point GetWalkablePerimeterSpawn();

	TristramInvasionState state_;
};

} // namespace invasion
} // namespace nightmare
} // namespace devilution
