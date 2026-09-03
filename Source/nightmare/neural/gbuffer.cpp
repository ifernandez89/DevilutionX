#include "nightmare/neural/gbuffer.hpp"
#include "engine/palette.h"
#include "levels/gendung.h"
#include <algorithm>
#include <cstring>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#define DVL_WASM_API EMSCRIPTEN_KEEPALIVE
#else
#define DVL_WASM_API
#endif

namespace devilution {
namespace nightmare {
namespace neural {

GBufferManager &GBufferManager::Instance()
{
	static GBufferManager instance;
	return instance;
}

GBufferManager::GBufferManager()
{
	Initialize(640, 480);
}

void GBufferManager::Initialize(int width, int height)
{
	width_ = width;
	height_ = height;
	size_t pixelCount = static_cast<size_t>(width_) * height_;

	rgbBuffer_.resize(pixelCount, 0xFF000000);
	depthBuffer_.resize(pixelCount, 1.0f);
	lightBuffer_.resize(pixelCount, 255);
	semanticBuffer_.resize(pixelCount, static_cast<uint8_t>(SemanticId::Void));
}

void GBufferManager::BeginFrame(int width, int height)
{
	if (!enabled_)
		return;

	if (width != width_ || height != height_ || rgbBuffer_.empty()) {
		Initialize(width, height);
	} else {
		std::fill(rgbBuffer_.begin(), rgbBuffer_.end(), 0xFF000000);
		std::fill(depthBuffer_.begin(), depthBuffer_.end(), 1.0f);
		std::fill(lightBuffer_.begin(), lightBuffer_.end(), 0);
		std::fill(semanticBuffer_.begin(), semanticBuffer_.end(), static_cast<uint8_t>(SemanticId::Void));
	}

	currentSemantic_ = SemanticId::Floor;
	currentDepth_ = 0.5f;
}

void GBufferManager::EndFrame()
{
	// Frame completion hook if needed
}

void GBufferManager::WritePixel(int x, int y, uint32_t rgba, float depth, uint8_t light, SemanticId semantic)
{
	if (!enabled_ || x < 0 || x >= width_ || y < 0 || y >= height_)
		return;

	size_t idx = static_cast<size_t>(y) * width_ + x;
	rgbBuffer_[idx] = rgba;
	depthBuffer_[idx] = depth;
	lightBuffer_[idx] = light;
	semanticBuffer_[idx] = static_cast<uint8_t>(semantic);
}

void GBufferManager::StampSemanticRect(int x, int y, int w, int h, SemanticId semantic, float depth, uint8_t light)
{
	if (!enabled_)
		return;

	int x0 = std::max(0, x);
	int y0 = std::max(0, y);
	int x1 = std::min(width_, x + w);
	int y1 = std::min(height_, y + h);

	uint8_t semVal = static_cast<uint8_t>(semantic);

	for (int py = y0; py < y1; ++py) {
		size_t rowStart = static_cast<size_t>(py) * width_;
		for (int px = x0; px < x1; ++px) {
			size_t idx = rowStart + px;
			semanticBuffer_[idx] = semVal;
			depthBuffer_[idx] = depth;
			lightBuffer_[idx] = light;
		}
	}
}

void GBufferManager::StampDiamondTile(Point screenPos, SemanticId semantic, float depth, uint8_t light)
{
	if (!enabled_)
		return;

	// Standard isometric diamond: 64 wide x 32 high
	const int halfW = 32;
	const int halfH = 16;
	int cx = screenPos.x + halfW;
	int cy = screenPos.y + halfH;
	uint8_t semVal = static_cast<uint8_t>(semantic);

	for (int dy = -halfH; dy < halfH; ++dy) {
		int py = cy + dy;
		if (py < 0 || py >= height_)
			continue;

		// Isometric diamond span at row dy: dx_max = halfW * (1 - |dy|/halfH)
		int span = halfW - (std::abs(dy) * halfW / halfH);
		int x0 = std::max(0, cx - span);
		int x1 = std::min(width_, cx + span);

		size_t rowStart = static_cast<size_t>(py) * width_;
		for (int px = x0; px < x1; ++px) {
			size_t idx = rowStart + px;
			semanticBuffer_[idx] = semVal;
			depthBuffer_[idx] = depth;
			lightBuffer_[idx] = light;
		}
	}
}

void GBufferManager::CaptureSceneSurface(const Surface &out, const SDL_Color *palette)
{
	if (!enabled_)
		return;

	const SDL_Color *pal = palette != nullptr ? palette : system_palette.data();
	int captureW = std::min(width_, out.w());
	int captureH = std::min(height_, out.h());

	for (int y = 0; y < captureH; ++y) {
		const uint8_t *srcRow = out.at(0, y);
		uint32_t *dstRow = &rgbBuffer_[static_cast<size_t>(y) * width_];

		for (int x = 0; x < captureW; ++x) {
			uint8_t palIndex = srcRow[x];
			const SDL_Color &c = pal[palIndex];
			// Store as RGBA (0xAABBGGRR in little endian)
			dstRow[x] = (static_cast<uint32_t>(0xFF) << 24) |
			            (static_cast<uint32_t>(c.b) << 16) |
			            (static_cast<uint32_t>(c.g) << 8) |
			            (static_cast<uint32_t>(c.r));
		}
	}
}

void GBufferManager::CaptureLightValue(int x, int y, uint8_t lightVal)
{
	if (!enabled_ || x < 0 || x >= width_ || y < 0 || y >= height_)
		return;

	size_t idx = static_cast<size_t>(y) * width_ + x;
	lightBuffer_[idx] = lightVal;
}

} // namespace neural
} // namespace nightmare
} // namespace devilution

// C-Export API
extern "C" {

DVL_WASM_API int Nightmare_IsTown()
{
	return (devilution::leveltype == devilution::DTYPE_TOWN && devilution::currlevel == 0) ? 1 : 0;
}

DVL_WASM_API void Nightmare_GBuffer_SetEnabled(int enabled)
{
	devilution::nightmare::neural::GBufferManager::Instance().SetEnabled(enabled != 0);
}

DVL_WASM_API int Nightmare_GBuffer_IsEnabled()
{
	return devilution::nightmare::neural::GBufferManager::Instance().IsEnabled() ? 1 : 0;
}

DVL_WASM_API int Nightmare_GBuffer_GetWidth()
{
	return devilution::nightmare::neural::GBufferManager::Instance().GetWidth();
}

DVL_WASM_API int Nightmare_GBuffer_GetHeight()
{
	return devilution::nightmare::neural::GBufferManager::Instance().GetHeight();
}

DVL_WASM_API int Nightmare_GBuffer_GetBiome()
{
	return static_cast<int>(devilution::nightmare::neural::GBufferManager::Instance().GetCurrentBiome());
}

DVL_WASM_API const uint32_t* Nightmare_GBuffer_GetRGB()
{
	return devilution::nightmare::neural::GBufferManager::Instance().GetRGBData();
}

DVL_WASM_API const float* Nightmare_GBuffer_GetDepth()
{
	return devilution::nightmare::neural::GBufferManager::Instance().GetDepthData();
}

DVL_WASM_API const uint8_t* Nightmare_GBuffer_GetLight()
{
	return devilution::nightmare::neural::GBufferManager::Instance().GetLightData();
}

DVL_WASM_API const uint8_t* Nightmare_GBuffer_GetSemantic()
{
	return devilution::nightmare::neural::GBufferManager::Instance().GetSemanticData();
}

}
