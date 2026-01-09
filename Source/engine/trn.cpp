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
		
		// Transform red colors (224-231) to violet/purple colors
		// Red range in Diablo palette typically 224-231
		trn[224] = 203; // Dark purple
		trn[225] = 204; // Purple
		trn[226] = 205; // Medium purple
		trn[227] = 206; // Light purple
		trn[228] = 207; // Bright purple
		trn[229] = 163; // Violet
		trn[230] = 164; // Light violet
		trn[231] = 165; // Bright violet
		
		// Also transform some brown/orange tones to complement
		trn[232] = 200; // Dark violet
		trn[233] = 201; // Medium violet
		trn[234] = 202; // Light violet
		
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
