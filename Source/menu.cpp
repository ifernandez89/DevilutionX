/**
 * @file mainmenu.cpp
 *
 * Implementation of functions for interacting with the main menu.
 */

#include <cstdint>
#include "phase2_logging.h"

#ifdef USE_SDL3
#include <SDL3/SDL_surface.h>
#include <SDL3/SDL_timer.h>
#else
#include <SDL.h>
#endif

#include "DiabloUI/diabloui.h"
#include "DiabloUI/settingsmenu.h"
#include "engine/assets.hpp"
#include "engine/demomode.h"
#include "game_mode.hpp"
#include "init.hpp"
#include "movie.h"
#include "options.h"
#include "pfile.h"
#include "storm/storm_net.hpp"
#include "utils/language.h"

namespace devilution {

uint32_t gSaveNumber;

namespace {

_music_id NextTrack()
{
	if (gbIsSpawn) {
		return TMUSIC_INTRO;
	}

	switch (sgnMusicTrack) {
	case TMUSIC_INTRO:
		return TMUSIC_CATACOMBS;
	case TMUSIC_CATACOMBS:
		return TMUSIC_CAVES;
	case TMUSIC_CAVES:
		return TMUSIC_HELL;
	case TMUSIC_HELL:
		return gbIsHellfire ? TMUSIC_NEST : TMUSIC_INTRO;
	case TMUSIC_NEST:
		return gbIsHellfire ? TMUSIC_CRYPT : TMUSIC_INTRO;
	default:
		return TMUSIC_INTRO;
	}
}

void RefreshMusic()
{
	music_start(NextTrack());
}

bool InitMenu(_selhero_selections type)
{
	bool success;

	if (type == SELHERO_PREVIOUS)
		return true;

	success = StartGame(type != SELHERO_CONTINUE, type != SELHERO_CONNECT);
	if (success)
		RefreshMusic();

	return success;
}

bool InitSinglePlayerMenu()
{
	gbIsMultiplayer = false;
	return InitMenu(SELHERO_NEW_DUNGEON);
}

bool InitMultiPlayerMenu()
{
	gbIsMultiplayer = true;
	return InitMenu(SELHERO_CONNECT);
}

void PlayIntro()
{
	music_stop();
	if (gbIsHellfire)
		play_movie("gendata\\Hellfire.smk", true);
	else
		play_movie("gendata\\diablo1.smk", true);
	RefreshMusic();
}

bool DummyGetHeroInfo(_uiheroinfo * /*pInfo*/)
{
	return true;
}

} // namespace

bool mainmenu_select_hero_dialog(GameData *gameData)
{
	OptionEntryInt<uint32_t> *pSaveNumberFromOptions = nullptr;
	_selhero_selections dlgresult = SELHERO_NEW_DUNGEON;
	if (demo::IsRunning()) {
		pfile_ui_set_hero_infos(DummyGetHeroInfo);
		gbLoadGame = true;
	} else if (!gbIsMultiplayer) {
		pSaveNumberFromOptions = gbIsHellfire ? &GetOptions().Hellfire.lastSinglePlayerHero : &GetOptions().Diablo.lastSinglePlayerHero;
		gSaveNumber = **pSaveNumberFromOptions;
		UiSelHeroSingDialog(
		    pfile_ui_set_hero_infos,
		    pfile_ui_save_create,
		    pfile_delete_save,
		    pfile_ui_set_class_stats,
		    &dlgresult,
		    &gSaveNumber,
		    &gameData->nDifficulty);

		gbLoadGame = (dlgresult == SELHERO_CONTINUE);
	} else {
		pSaveNumberFromOptions = gbIsHellfire ? &GetOptions().Hellfire.lastMultiplayerHero : &GetOptions().Diablo.lastMultiplayerHero;
		gSaveNumber = **pSaveNumberFromOptions;
		UiSelHeroMultDialog(
		    pfile_ui_set_hero_infos,
		    pfile_ui_save_create,
		    pfile_delete_save,
		    pfile_ui_set_class_stats,
		    &dlgresult,
		    &gSaveNumber);
	}
	if (dlgresult == SELHERO_PREVIOUS) {
		return false;
	}

	if (pSaveNumberFromOptions != nullptr)
		pSaveNumberFromOptions->SetValue(gSaveNumber);

	return true;
}

void mainmenu_wait_for_button_sound()
{
	SDL_FillSurfaceRect(DiabloUiSurface(), nullptr, 0);
	UiFadeIn();
	SDL_Delay(350); // delay to let button pressed sound finish playing
}

void mainmenu_loop()
{
	PHASE2_LOG("mainmenu_loop() started - Main game loop active");
	PHASE2_ENGINE_STEP("MENU_LOOP_ENTRY", "Main menu loop executing");
	
	bool done;

	RefreshMusic();
	done = false;

	PHASE2_MEMORY_CHECK("mainmenu_loop - Loop initialized");
	PHASE2_ENGINE_STEP("MENU_LOOP_ACTIVE", "Menu loop running - engine fully operational");

	do {
		_mainmenu_selections menu = MAINMENU_NONE;
		if (demo::IsRunning())
			menu = MAINMENU_SINGLE_PLAYER;
		else if (!UiMainMenuDialog(gszProductName, &menu, 30))
			app_fatal(_("Unable to display mainmenu"));

		PHASE2_ENGINE_STEP("MENU_SELECTION", "Processing menu selection");

		switch (menu) {
		case MAINMENU_NONE:
			break;
		case MAINMENU_SINGLE_PLAYER:
			PHASE2_ENGINE_STEP("SINGLE_PLAYER", "Single player mode selected");
			if (!InitSinglePlayerMenu())
				done = true;
			break;
		case MAINMENU_MULTIPLAYER:
			PHASE2_ENGINE_STEP("MULTIPLAYER", "Multiplayer mode selected");
			if (!InitMultiPlayerMenu())
				done = true;
			break;
		case MAINMENU_ATTRACT_MODE:
			PHASE2_ENGINE_STEP("ATTRACT_MODE", "Attract mode activated");
			if (gbIsSpawn && !HaveIntro())
				done = false;
			else if (gbActive) {
				// 🌑 NIGHTMARE: Reproducir créditos automáticamente después de timeout
				// Mantener intro de estapada al inicio (se reproduce antes del menú)
				// Aquí solo reproducimos créditos como "screensaver"
				UiCreditsDialog();
			}
			break;
		case MAINMENU_SHOW_CREDITS:
			PHASE2_ENGINE_STEP("CREDITS", "Showing credits");
			UiCreditsDialog();
			break;
		case MAINMENU_SHOW_SUPPORT:
			PHASE2_ENGINE_STEP("SUPPORT", "Showing support dialog");
			UiSupportDialog();
			break;
		case MAINMENU_EXIT_DIABLO:
			PHASE2_ENGINE_STEP("EXIT", "Exit selected - terminating loop");
			mainmenu_wait_for_button_sound();
			done = true;
			break;
		case MAINMENU_SETTINGS:
			PHASE2_ENGINE_STEP("SETTINGS", "Settings menu selected");
			UiSettingsMenu();
			break;
		}

		PHASE2_MEMORY_CHECK("mainmenu_loop - Loop iteration complete");
	} while (!done);

	PHASE2_ENGINE_STEP("MENU_LOOP_EXIT", "Main menu loop completed");
	PHASE2_LOG("mainmenu_loop() finished - Engine loop lifecycle verified");
	
	music_stop();
}

} // namespace devilution
