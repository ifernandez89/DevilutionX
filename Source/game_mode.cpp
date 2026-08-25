#include "game_mode.hpp"

#include <function_ref.hpp>

#include "options.h"

namespace devilution {
namespace {
void OptionSharewareChanged()
{
	gbIsSpawn = *GetOptions().GameMode.shareware;
}
const auto OptionChangeHandlerShareware = (GetOptions().GameMode.shareware.SetValueChangedCallback(OptionSharewareChanged), true);

void OptionGameModeChanged()
{
	const StartUpGameMode mode = *GetOptions().GameMode.gameMode;
	if (mode == StartUpGameMode::Hellfire) {
		GetOptions().Mods.SetHellfireEnabled(true);
	} else if (mode == StartUpGameMode::Diablo) {
		GetOptions().Mods.SetHellfireEnabled(false);
	}
}
const auto OptionChangeHandlerGameMode = (GetOptions().GameMode.gameMode.SetValueChangedCallback(OptionGameModeChanged), true);
} // namespace

bool gbRunGame;
bool gbIsSpawn;
bool gbIsHellfire;
bool gbVanilla;
bool forceHellfire;

} // namespace devilution
