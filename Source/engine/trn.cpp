#include "engine/trn.hpp"

#include <cstdint>

#include <fmt/format.h>

#ifdef _DEBUG
#include "debug.h"
#endif
#include "engine/load_file.hpp"
#include "engine/palette.h"
#include "lighting.h"
#include "utils/str_cat.hpp"

namespace devilution {

namespace {

std::array<uint8_t, 256> GoldToStoneTable = []() {
	std::array<uint8_t, 256> trn;
	for (int i = 0; i < 256; ++i) {
		trn[i] = static_cast<uint8_t>(i);
	}
	// Map 16 shades of Yellow/Gold (192..207) to matching Slate Gray (240..255)
	for (int i = 0; i < 16; ++i) {
		trn[PAL16_YELLOW + i] = static_cast<uint8_t>(PAL16_GRAY + i);
	}
	// Map 8 shades of Dark Yellow to matching dark slate gray
	for (int i = 0; i < 8; ++i) {
		trn[PAL8_YELLOW + i] = static_cast<uint8_t>(PAL16_GRAY + 8 + i);
	}
	// Map 8 shades of Dark Orange to matching dark slate gray
	for (int i = 0; i < 8; ++i) {
		trn[PAL8_ORANGE + i] = static_cast<uint8_t>(PAL16_GRAY + 8 + i);
	}
	return trn;
}();

} // namespace

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

uint8_t *GetGoldToStoneTRN()
{
	return GoldToStoneTable.data();
}

std::optional<std::array<uint8_t, 256>> GetClassTRN(Player &player)
{
	std::array<uint8_t, 256> trn;
	char path[64];

	const PlayerSpriteData &spriteData = GetPlayerSpriteDataForClass(player._pClass);
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
