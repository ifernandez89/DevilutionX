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

constexpr size_t MaxInvasionMonsters = 100;

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
	uint8_t boss_phase; // 0: not spawned, 1: dramatic pause, 2: spawned/fighting, 3: dead
	uint32_t dramatic_pause_start_tick;
	_monster_id boss_selected;
	UniqueMonsterType boss_unique_type;
	uint8_t monster_count;
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
	void SpawnBossEncounter();

	TristramInvasionState state_;
};

} // namespace invasion
} // namespace nightmare
} // namespace devilution
