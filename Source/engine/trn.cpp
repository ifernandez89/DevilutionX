#include "engine/trn.hpp"

#include <cstdint>

#include <fmt/format.h>

#ifdef _DEBUG
#include "debug.h"
#endif
#include "engine/load_file.hpp"
#include "lighting.h"
#include "player.h"
#include "utils/str_cat.hpp"

namespace devilution {

uint8_t *GetInfravisionTRN()
{
	return InfravisionTable.data();
}

uint8_t *GetStoneTRN()
{
	return StoneTable.data();
}

uint8_t *GetPauseTRN()
{
	return PauseTable.data();
}

std::optional<std::array<uint8_t, 256>> GetClassTRN(Player &player)
{
	std::array<uint8_t, 256> trn;
	char path[64];

	const PlayerSpriteData &spriteData = GetPlayerSpriteDataForClass(player._pClass);
	
	// FEATURE: Custom Sorcerer Color - Violet/Purple theme
	if (player._pClass == HeroClass::Sorcerer) {
		// Create custom violet/purple TRN for Sorcerer
		for (int i = 0; i < 256; i++) {
			trn[i] = i; // Default: no change
		}
		
		// Transform ALL red/orange colors to violet/purple spectrum
		// More comprehensive color mapping for better coverage
		
		// Dark reds to dark purples
		for (int i = 224; i <= 231; i++) {
			trn[i] = 200 + (i - 224); // Map to purple range 200-207
		}
		
		// Orange/brown tones to violet
		for (int i = 232; i <= 239; i++) {
			trn[i] = 160 + (i - 232); // Map to violet range 160-167
		}
		
		// Light reds to light purples
		for (int i = 240; i <= 247; i++) {
			trn[i] = 208 + (i - 240); // Map to light purple range 208-215
		}
		
		// Additional red tones that might be used
		trn[176] = 203; // Dark red -> Dark purple
		trn[177] = 204; // Red -> Purple
		trn[178] = 205; // Medium red -> Medium purple
		trn[179] = 206; // Light red -> Light purple
		trn[180] = 163; // Bright red -> Violet
		trn[181] = 164; // Very bright red -> Light violet
		
		return trn;
	}
	
	*BufCopy(path, "plrgfx\\", spriteData.trn, ".trn") = '\0';

#ifdef _DEBUG
	if (!debugTRN.empty()) {
		*BufCopy(path, debugTRN.c_str()) = '\0';
	}
#endif
	if (LoadOptionalFileInMem(path, &trn[0], 256)) {
		return trn;
	}
	return std::nullopt;
}

std::optional<std::array<uint8_t, 256>> GetPlayerGraphicTRN(const char *pszName)
{
	char path[MaxMpqPathSize];
	*BufCopy(path, pszName, ".trn") = '\0';

	std::array<uint8_t, 256> trn;
	if (LoadOptionalFileInMem(path, &trn[0], 256)) {
		return trn;
	}
	return std::nullopt;
}

} // namespace devilution
