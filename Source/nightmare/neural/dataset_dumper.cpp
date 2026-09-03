#include "nightmare/neural/dataset_dumper.hpp"
#include "nightmare/neural/gbuffer.hpp"
#include "utils/file_util.h"
#include "utils/log.hpp"
#include "utils/str_cat.hpp"

#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>

#if defined(_WIN32)
#include <direct.h>
#else
#include <sys/stat.h>
#endif

namespace devilution {
namespace nightmare {
namespace neural {

namespace {

void CreateDirRecursive(const std::string &path)
{
#if defined(_WIN32)
	_mkdir(path.c_str());
#else
	mkdir(path.c_str(), 0755);
#endif
}

} // namespace

DatasetDumper &DatasetDumper::Instance()
{
	static DatasetDumper instance;
	return instance;
}

DatasetDumper::DatasetDumper() = default;

void DatasetDumper::OnSceneRendered(Point playerPos, int levelType, uint32_t gameTick)
{
	if (!enabled_)
		return;

	if (capturedFrames_ >= maxFrames_) {
		enabled_ = false;
		Log("NIGHTMARE Dataset Dumper: Completed capture of {} frames.", capturedFrames_);
		return;
	}

	frameCounter_++;
	if (frameCounter_ % intervalFrames_ != 0)
		return;

	DumpFrameTuple(capturedFrames_, playerPos, levelType, gameTick);
	capturedFrames_++;
}

void DatasetDumper::DumpFrameTuple(int frameIndex, Point playerPos, int levelType, uint32_t gameTick)
{
	GBufferManager &gbuffer = GBufferManager::Instance();
	if (!gbuffer.IsEnabled())
		return;

	int width = gbuffer.GetWidth();
	int height = gbuffer.GetHeight();

	CreateDirRecursive(outputDir_);

	std::ostringstream frameFolderStream;
	frameFolderStream << outputDir_ << "/frame_" << std::setw(6) << std::setfill('0') << frameIndex;
	std::string frameFolder = frameFolderStream.str();
	CreateDirRecursive(frameFolder);

	// 1. Save RGB raw buffer
	{
		std::string rgbPath = frameFolder + "/rgb.raw";
		std::ofstream ofs(rgbPath, std::ios::binary);
		if (ofs.is_open()) {
			ofs.write(reinterpret_cast<const char*>(gbuffer.GetRGBData()), gbuffer.GetRGBByteSize());
		}
	}

	// 2. Save Depth raw buffer
	{
		std::string depthPath = frameFolder + "/depth.raw";
		std::ofstream ofs(depthPath, std::ios::binary);
		if (ofs.is_open()) {
			ofs.write(reinterpret_cast<const char*>(gbuffer.GetDepthData()), gbuffer.GetDepthByteSize());
		}
	}

	// 3. Save Light raw buffer
	{
		std::string lightPath = frameFolder + "/light.raw";
		std::ofstream ofs(lightPath, std::ios::binary);
		if (ofs.is_open()) {
			ofs.write(reinterpret_cast<const char*>(gbuffer.GetLightData()), gbuffer.GetLightByteSize());
		}
	}

	// 4. Save Semantic raw buffer
	{
		std::string semPath = frameFolder + "/semantic.raw";
		std::ofstream ofs(semPath, std::ios::binary);
		if (ofs.is_open()) {
			ofs.write(reinterpret_cast<const char*>(gbuffer.GetSemanticData()), gbuffer.GetSemanticByteSize());
		}
	}

	// 5. Save Metadata JSON
	{
		auto now = std::chrono::system_clock::now();
		auto epochMs = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

		std::string metaPath = frameFolder + "/metadata.json";
		std::ofstream ofs(metaPath);
		if (ofs.is_open()) {
			ofs << "{\n"
			    << "  \"frame_index\": " << frameIndex << ",\n"
			    << "  \"width\": " << width << ",\n"
			    << "  \"height\": " << height << ",\n"
			    << "  \"player_x\": " << playerPos.x << ",\n"
			    << "  \"player_y\": " << playerPos.y << ",\n"
			    << "  \"level_type\": " << levelType << ",\n"
			    << "  \"game_tick\": " << gameTick << ",\n"
			    << "  \"timestamp_ms\": " << epochMs << ",\n"
			    << "  \"channels\": [\"rgb\", \"depth\", \"light\", \"semantic\"]\n"
			    << "}\n";
		}
	}

	// 6. Append entry to dataset_manifest.jsonl
	{
		std::string manifestPath = outputDir_ + "/manifest.jsonl";
		std::ofstream ofs(manifestPath, std::ios::app);
		if (ofs.is_open()) {
			ofs << "{\"frame\": " << frameIndex << ", \"folder\": \"" << frameFolder << "\", \"tick\": " << gameTick << "}\n";
		}
	}
}

} // namespace neural
} // namespace nightmare
} // namespace devilution
