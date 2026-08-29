/**
 * @file level_atmosphere.cpp
 *
 * Implementation of NIGHTMARE Level Atmospheric and Gameplay Configuration.
 */
#include "nightmare/world/level_atmosphere.hpp"

#include <algorithm>
#include "levels/gendung.h"
#include "tables/monstdat.h"

namespace devilution {
namespace nightmare {

namespace {

// =============================================================================
// 1. LIGHT RADIUS OVERRIDES BY DUNGEON LEVEL (0 to 24)
// =============================================================================
// Vanilla value: Static 12 across all levels.
// NIGHTMARE: Dynamic base light radius creating varied psychological tension.
// (Bounded between 2 and 15 by CalcPlrLightRadius).
//
// Level 0:  Town (Tristram) = 12 (Full light)
// Level 1:  Cathedral - Horror = 8
// Level 2:  Cathedral - Infección = 7
// Level 3:  Cathedral - Francotiro = 6
// Level 4:  Cathedral - Umbral = 7
// Level 5:  Catacombs - Invasión = 9
// Level 6:  Catacombs - Hierro (Melee) = 10
// Level 7:  Catacombs - Hambre (Ranged) = 6
// Level 8:  Catacombs - Salón de los Señores = 9
// Level 9:  Caves - Cuevas Profundas = 9
// Level 10: Caves - Cavernas de Fuego = 8
// Level 11: Caves - Silencio Opresivo = 4
// Level 12: Caves - Ríos de Lava = 6
// Level 13: Hell - El Descenso = 7
// Level 14: Hell - Ríos Negros = 5
// Level 15: Hell - Salón de Sombras = 4
// Level 16: Hell - Cámara de Diablo = 3
// Level 17: Nest 1 - Entrada de la Colmena = 10
// Level 18: Nest 2 - Enjambre = 9
// Level 19: Nest 3 - Nido de Sombras = 5
// Level 20: Nest 4 - Cubil del Defiler = 8
// Level 21: Crypt 1 - Tumbas Olvidadas = 5
// Level 22: Crypt 2 - Cámara del Tesoro = 10
// Level 23: Crypt 3 - Cripta de Huesos = 4
// Level 24: Crypt 4 - Santuario de Na-Krul / Pesadilla = 3
// =============================================================================
constexpr int8_t NightmareBaseLightByLevel[25] = {
	12, // Level 0: Town
	8,  // Level 1: Cathedral - Horror
	7,  // Level 2: Cathedral - Infección
	6,  // Level 3: Cathedral - Francotiro
	7,  // Level 4: Cathedral - Umbral
	9,  // Level 5: Catacombs - Invasión
	10, // Level 6: Catacombs - Hierro
	6,  // Level 7: Catacombs - Hambre
	9,  // Level 8: Catacombs - Señores
	9,  // Level 9: Caves - Profundidades
	8,  // Level 10: Caves - Fuego
	4,  // Level 11: Caves - Silencio
	6,  // Level 12: Caves - Lava
	7,  // Level 13: Hell - Descenso
	5,  // Level 14: Hell - Ríos Negros
	4,  // Level 15: Hell - Sombras
	3,  // Level 16: Hell - Diablo
	10, // Level 17: Nest 1 - Entrada
	9,  // Level 18: Nest 2 - Enjambre
	5,  // Level 19: Nest 3 - Nido de Sombras
	8,  // Level 20: Nest 4 - Defiler
	5,  // Level 21: Crypt 1 - Tumbas
	10, // Level 22: Crypt 2 - Tesoro
	4,  // Level 23: Crypt 3 - Cripta de Huesos
	3   // Level 24: Crypt 4 - Na-Krul / Pesadilla
};

} // namespace

int GetNightmareBaseLightRadius(uint8_t level)
{
	// Vanilla fallback:
	// return 12;

	if (level < sizeof(NightmareBaseLightByLevel) / sizeof(NightmareBaseLightByLevel[0])) {
		return NightmareBaseLightByLevel[level];
	}
	return 12;
}

_music_id GetNightmareLevelMusic(dungeon_type dungeonType, uint8_t level, bool isSetLevel)
{
	// In quest set pieces, retain their designated theme
	if (isSetLevel) {
		switch (dungeonType) {
		case DTYPE_TOWN: return TMUSIC_TOWN;
		case DTYPE_CATHEDRAL: return TMUSIC_CATHEDRAL;
		case DTYPE_CATACOMBS: return TMUSIC_CATACOMBS;
		case DTYPE_CAVES: return TMUSIC_CAVES;
		case DTYPE_HELL: return TMUSIC_HELL;
		case DTYPE_NEST: return TMUSIC_NEST;
		case DTYPE_CRYPT: return TMUSIC_CRYPT;
		default: return TMUSIC_INTRO;
		}
	}

	// Dynamic music per floor to break expectations
	switch (level) {
	case 4:
		// Threshold: Cathedral with Crypt music introduces psychological dissonance
		return TMUSIC_CRYPT;
	case 8:
		// Lords: Catacombs with Hell music indicates proximity to the abyss
		return TMUSIC_HELL;
	case 11:
		// Silence: Caves with Caves ambient music
		return TMUSIC_CAVES;
	case 14:
		// Black Rivers: Caves music
		return TMUSIC_CAVES;
	case 15:
		// Shadow Hell: Hell with Crypt haunting soundtrack
		return TMUSIC_CRYPT;
	case 19:
		// Shadow Nest: Nest with Crypt haunting music
		return TMUSIC_CRYPT;
	case 22:
		// Treasure Chamber in Crypt: Cathedral solemn music
		return TMUSIC_CATHEDRAL;
	case 24:
		// Na-Krul / Nightmare Peak: Hell combat music
		return TMUSIC_HELL;
	default:
		// Standard biome fallback
		switch (dungeonType) {
		case DTYPE_TOWN: return TMUSIC_TOWN;
		case DTYPE_CATHEDRAL: return TMUSIC_CATHEDRAL;
		case DTYPE_CATACOMBS: return TMUSIC_CATACOMBS;
		case DTYPE_CAVES: return TMUSIC_CAVES;
		case DTYPE_HELL: return TMUSIC_HELL;
		case DTYPE_NEST: return TMUSIC_NEST;
		case DTYPE_CRYPT: return TMUSIC_CRYPT;
		default: return TMUSIC_INTRO;
		}
	}
}

void GetNightmareChestCounts(uint8_t level, int &c1min, int &c1max, int &c2min, int &c2max, int &c3min, int &c3max)
{
	// Vanilla baseline defaults:
	// c1min = 5; c1max = 10;
	// c2min = 3; c2max = 6;
	// c3min = 1; c3max = 5;

	switch (level) {
	case 7: // Level 7: Hunger (Scarcity)
		c1min = 2; c1max = 4;
		c2min = 1; c2max = 3;
		c3min = 0; c3max = 2;
		break;
	case 11: // Level 11: Silence (Minimal objects)
		c1min = 2; c1max = 4;
		c2min = 1; c2max = 2;
		c3min = 0; c3max = 1;
		break;
	case 8: // Level 8: Lords (Reward)
		c1min = 7; c1max = 12;
		c2min = 4; c2max = 8;
		c3min = 2; c3max = 6;
		break;
	case 22: // Level 22: Treasure Chamber (Massive Loot)
		c1min = 10; c1max = 18;
		c2min = 6;  c2max = 12;
		c3min = 4;  c3max = 8;
		break;
	case 24: // Level 24: Nightmare Final
		c1min = 6; c1max = 12;
		c2min = 4; c2max = 8;
		c3min = 2; c3max = 6;
		break;
	default:
		// Standard baseline
		c1min = 5; c1max = 10;
		c2min = 3; c2max = 6;
		c3min = 1; c3max = 5;
		break;
	}
}

void ApplyNightmareMonsterLevelOverrides()
{
	// Iterate loaded monster data and extend specific minDunLvl/maxDunLvl ranges
	for (auto &mon : MonstersData) {
		switch (mon.ai) {
		case MonsterAIID::Sneak:
			// Allow Hidden / Stalkers starting on Level 1-2
			// Vanilla: minDunLvl = 2..8
			mon.minDunLvl = std::min<int8_t>(mon.minDunLvl, 1);
			break;
		case MonsterAIID::GoatMelee:
		case MonsterAIID::GoatRanged:
			// Flesh Clan can appear in Cathedral Level 2-4
			// Vanilla: minDunLvl = 4..7
			mon.minDunLvl = std::min<int8_t>(mon.minDunLvl, 2);
			break;
		case MonsterAIID::Acid:
			// Acid beasts can appear earlier in Catacombs / Caves
			// Vanilla: minDunLvl = 6..12
			mon.minDunLvl = std::min<int8_t>(mon.minDunLvl, 5);
			break;
		case MonsterAIID::Succubus:
			// Succubi range extended to cover shadow levels
			// Vanilla: minDunLvl = 12..15
			mon.minDunLvl = std::min<int8_t>(mon.minDunLvl, 7);
			mon.maxDunLvl = std::max<int8_t>(mon.maxDunLvl, 24);
			break;
		case MonsterAIID::Counselor:
			// Mages available across high tier Hell/Crypt levels
			// Vanilla: minDunLvl = 13..16
			mon.maxDunLvl = std::max<int8_t>(mon.maxDunLvl, 24);
			break;
		default:
			break;
		}
	}
}

} // namespace nightmare
} // namespace devilution
