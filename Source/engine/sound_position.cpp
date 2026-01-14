#include "engine/sound_position.hpp"

#include "engine/sound_defs.hpp"
#include "player.h"

// 🎵 SPATIAL AUDIO ENHANCEMENT - Integración del sistema mejorado
#include "audio/enhanced_audio.h"

namespace devilution {

bool CalculateSoundPosition(Point soundPosition, int *plVolume, int *plPan)
{
	// 🎵 SPATIAL AUDIO ENHANCEMENT - Usar sistema mejorado si está disponible
	if (IsEnhancedAudioEnabled()) {
		const Point playerPosition { MyPlayer->position.tile };
		
		// Usar nuestro sistema de spatial audio mejorado
		int enhancedVolume = 0;  // Volumen base
		int enhancedPan = 0;     // Pan base
		
		if (CalculateSpatialAudio(soundPosition, playerPosition, enhancedVolume, &enhancedVolume, &enhancedPan)) {
			*plVolume = enhancedVolume;
			*plPan = enhancedPan;
			return true;
		} else {
			// Fuera de rango del spatial audio mejorado
			return false;
		}
	}
	
	// Código original como fallback
	const Point playerPosition { MyPlayer->position.tile };
	const Displacement delta = soundPosition - playerPosition;

	const int pan = (delta.deltaX - delta.deltaY) * 256;
	*plPan = std::clamp(pan, PAN_MIN, PAN_MAX);

	const int volume = playerPosition.ApproxDistance(soundPosition) * -64;

	if (volume <= ATTENUATION_MIN)
		return false;

	*plVolume = volume;

	return true;
}

} // namespace devilution
