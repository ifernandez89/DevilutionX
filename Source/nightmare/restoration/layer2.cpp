#include "nightmare/restoration/layer2.hpp"

#include <memory>
#include <vector>

#include "engine/assets.hpp"
#include "engine/random.hpp"
#include "engine/sound.h"
#include "nightmare/restoration/registry.hpp"

namespace devilution {
namespace nightmare {

namespace {
std::vector<std::unique_ptr<TSnd>> g_layer2Snds;
} // namespace

void InitNightmareLayer2()
{
	g_layer2Snds.clear();
}

bool IsLayer2AssetAvailable(const char *assetPath)
{
	if (assetPath == nullptr || *assetPath == '\0')
		return false;

	return FindAsset(assetPath).ok();
}

bool IsIncineratorAssetAvailable()
{
	return IsLayer2AssetAvailable("monsters\\fireman\\firem.clx") ||
	       IsLayer2AssetAvailable("monsters\\fireman\\firem.cel") ||
	       IsLayer2AssetAvailable("monsters/fireman/firem.clx") ||
	       IsLayer2AssetAvailable("monsters/fireman/firem.cel");
}

bool IsDarkMageAssetAvailable()
{
	return IsLayer2AssetAvailable("monsters\\darkmage\\dmage.clx") ||
	       IsLayer2AssetAvailable("monsters\\darkmage\\dmage.cel") ||
	       IsLayer2AssetAvailable("monsters/darkmage/dmage.clx") ||
	       IsLayer2AssetAvailable("monsters/darkmage/dmage.cel");
}

void PlayLayer2WavDefensive(const char *wavPath)
{
	if (wavPath == nullptr || *wavPath == '\0')
		return;

#ifndef NOSOUND
	for (auto &snd : g_layer2Snds) {
		if (snd && snd->DSB.IsLoaded())
			snd->DSB.Stop();
	}
	g_layer2Snds.clear();

	auto sndRes = SoundFileLoadWithStatus(wavPath, /*stream=*/false);
	if (sndRes.has_value() && *sndRes != nullptr) {
		auto snd = std::move(*sndRes);
		if (snd) {
			snd_play_snd(snd.get(), 0, 0);
			g_layer2Snds.push_back(std::move(snd));
		}
	}
#endif
}

_speech_id GetLayer2GossipDefensive(_speech_id defaultSpeech, int startCutId, int endCutId)
{
	if (GenerateRnd(100) < 30 && endCutId >= startCutId) {
		int chosen = startCutId + GenerateRnd(endCutId - startCutId + 1);
		return static_cast<_speech_id>(chosen);
	}
	return defaultSpeech;
}

} // namespace nightmare
} // namespace devilution
