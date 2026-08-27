#include "nightmare/world/ambient_animals.hpp"

#include <SDL.h>

#include "engine/random.hpp"
#include "levels/gendung.h"
#include "nightmare/restoration/registry.hpp"

namespace devilution {
namespace nightmare {

namespace {
uint32_t g_nextAnimalSoundTick = 0;

const char *const AnimalSoundPool[] = {
	"sfx\\animals\\cow.wav",
	"sfx\\animals\\dog.wav",
	"sfx\\animals\\pig.wav",
	"sfx\\animals\\sheep.wav"
};

const size_t AnimalSoundPoolSize = sizeof(AnimalSoundPool) / sizeof(AnimalSoundPool[0]);

} // namespace

void InitTristramAmbientAnimals()
{
	g_nextAnimalSoundTick = SDL_GetTicks() + (45 + GenerateRnd(75)) * 1000;
}

void UpdateTristramAmbientAnimals()
{
	if (leveltype != DTYPE_TOWN)
		return;

	const uint32_t now = SDL_GetTicks();
	if (now >= g_nextAnimalSoundTick) {
		const int index = GenerateRnd(AnimalSoundPoolSize);
		PlayNightmareRestorationWav(AnimalSoundPool[index]);
		g_nextAnimalSoundTick = now + (45 + GenerateRnd(75)) * 1000;
	}
}

} // namespace nightmare
} // namespace devilution
