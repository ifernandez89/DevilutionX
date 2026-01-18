#pragma once

#ifdef PHASE4_RENDER_VERIFICATION

#include <iostream>
#include <string>
#include <chrono>

// FASE 4: Render System Verification Logging Macros
#define PHASE4_LOG(message) \
    do { \
        auto now = std::chrono::steady_clock::now(); \
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count(); \
        std::cout << "[FASE4-RENDER][" << ms << "] " << message << std::endl; \
    } while(0)

#define PHASE4_RENDER_INIT(system, status) \
    do { \
        PHASE4_LOG("🎨 RENDER INIT: " + std::string(system) + " - " + std::string(status)); \
    } while(0)

#define PHASE4_RENDER_FRAME(frame_count, fps) \
    do { \
        PHASE4_LOG("🖼️ RENDER FRAME: #" + std::to_string(frame_count) + " | FPS: " + std::to_string(fps)); \
    } while(0)

#define PHASE4_RENDER_SURFACE(surface_type, width, height) \
    do { \
        PHASE4_LOG("🖥️ RENDER SURFACE: " + std::string(surface_type) + " (" + std::to_string(width) + "x" + std::to_string(height) + ")"); \
    } while(0)

#define PHASE4_RENDER_DRAW(draw_type, x, y, w, h) \
    do { \
        PHASE4_LOG("✏️ RENDER DRAW: " + std::string(draw_type) + " at (" + std::to_string(x) + "," + std::to_string(y) + ") size " + std::to_string(w) + "x" + std::to_string(h)); \
    } while(0)

#define PHASE4_RENDER_PALETTE(palette_name, colors) \
    do { \
        PHASE4_LOG("🎨 RENDER PALETTE: " + std::string(palette_name) + " (" + std::to_string(colors) + " colors)"); \
    } while(0)

#define PHASE4_RENDER_BUFFER(buffer_type, size) \
    do { \
        PHASE4_LOG("📦 RENDER BUFFER: " + std::string(buffer_type) + " (" + std::to_string(size) + " bytes)"); \
    } while(0)

#define PHASE4_RENDER_SYSTEM_CHECK(system, status) \
    do { \
        PHASE4_LOG("🔍 RENDER CHECK: " + std::string(system) + " - " + std::string(status)); \
    } while(0)

#else

// No-op macros when PHASE4_RENDER_VERIFICATION is not defined
#define PHASE4_LOG(message)
#define PHASE4_RENDER_INIT(system, status)
#define PHASE4_RENDER_FRAME(frame_count, fps)
#define PHASE4_RENDER_SURFACE(surface_type, width, height)
#define PHASE4_RENDER_DRAW(draw_type, x, y, w, h)
#define PHASE4_RENDER_PALETTE(palette_name, colors)
#define PHASE4_RENDER_BUFFER(buffer_type, size)
#define PHASE4_RENDER_SYSTEM_CHECK(system, status)

#endif // PHASE4_RENDER_VERIFICATION