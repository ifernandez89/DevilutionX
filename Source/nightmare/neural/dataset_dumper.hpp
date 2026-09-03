#pragma once

#include <string>
#include <cstdint>
#include "engine/point.hpp"

namespace devilution {
namespace nightmare {
namespace neural {

class DatasetDumper {
public:
	static DatasetDumper &Instance();

	void SetEnabled(bool enable) { enabled_ = enable; }
	bool IsEnabled() const { return enabled_; }

	void SetOutputDir(const std::string &dir) { outputDir_ = dir; }
	const std::string &GetOutputDir() const { return outputDir_; }

	void SetMaxFrames(int maxFrames) { maxFrames_ = maxFrames; }
	int GetMaxFrames() const { return maxFrames_; }

	void SetIntervalFrames(int interval) { intervalFrames_ = interval; }
	int GetIntervalFrames() const { return intervalFrames_; }

	int GetCapturedCount() const { return capturedFrames_; }

	void OnSceneRendered(Point playerPos, int levelType, uint32_t gameTick);

private:
	DatasetDumper();

	bool enabled_ = false;
	std::string outputDir_ = "dataset_tristram";
	int maxFrames_ = 25000;
	int intervalFrames_ = 3;
	int frameCounter_ = 0;
	int capturedFrames_ = 0;

	void DumpFrameTuple(int frameIndex, Point playerPos, int levelType, uint32_t gameTick);
};

} // namespace neural
} // namespace nightmare
} // namespace devilution
