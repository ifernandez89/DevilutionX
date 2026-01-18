#pragma once

#ifdef PHASE5_AUDIO_VERIFICATION

#include <iostream>
#include <string>
#include <chrono>

// FASE 5: Audio System Verification Logging Macros
#define PHASE5_LOG(message) \
    do { \
        auto now = std::chrono::steady_clock::now(); \
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count(); \
        std::cout << "[FASE5-AUDIO][" << ms << "] " << message << std::endl; \
    } while(0)

#define PHASE5_AUDIO_INIT(system, status) \
    do { \
        PHASE5_LOG("🔊 AUDIO INIT: " + std::string(system) + " - " + std::string(status)); \
    } while(0)

#define PHASE5_AUDIO_DEVICE(device_name, channels, frequency) \
    do { \
        PHASE5_LOG("🎵 AUDIO DEVICE: " + std::string(device_name) + " | Channels: " + std::to_string(channels) + " | Freq: " + std::to_string(frequency) + "Hz"); \
    } while(0)

#define PHASE5_AUDIO_LOAD(sound_name, size) \
    do { \
        PHASE5_LOG("📂 AUDIO LOAD: " + std::string(sound_name) + " (" + std::to_string(size) + " bytes)"); \
    } while(0)

#define PHASE5_AUDIO_PLAY(sound_name, volume, channel) \
    do { \
        PHASE5_LOG("▶️ AUDIO PLAY: " + std::string(sound_name) + " | Vol: " + std::to_string(volume) + " | Ch: " + std::to_string(channel)); \
    } while(0)

#define PHASE5_AUDIO_STOP(sound_name, channel) \
    do { \
        PHASE5_LOG("⏹️ AUDIO STOP: " + std::string(sound_name) + " | Ch: " + std::to_string(channel)); \
    } while(0)

#define PHASE5_AUDIO_MIXER(mixer_status, active_channels) \
    do { \
        PHASE5_LOG("🎛️ AUDIO MIXER: " + std::string(mixer_status) + " | Active: " + std::to_string(active_channels) + " channels"); \
    } while(0)

#define PHASE5_AUDIO_BUFFER(buffer_type, size, samples) \
    do { \
        PHASE5_LOG("📦 AUDIO BUFFER: " + std::string(buffer_type) + " (" + std::to_string(size) + " bytes, " + std::to_string(samples) + " samples)"); \
    } while(0)

#define PHASE5_AUDIO_SYSTEM_CHECK(system, status) \
    do { \
        PHASE5_LOG("🔍 AUDIO CHECK: " + std::string(system) + " - " + std::string(status)); \
    } while(0)

#else

// No-op macros when PHASE5_AUDIO_VERIFICATION is not defined
#define PHASE5_LOG(message)
#define PHASE5_AUDIO_INIT(system, status)
#define PHASE5_AUDIO_DEVICE(device_name, channels, frequency)
#define PHASE5_AUDIO_LOAD(sound_name, size)
#define PHASE5_AUDIO_PLAY(sound_name, volume, channel)
#define PHASE5_AUDIO_STOP(sound_name, channel)
#define PHASE5_AUDIO_MIXER(mixer_status, active_channels)
#define PHASE5_AUDIO_BUFFER(buffer_type, size, samples)
#define PHASE5_AUDIO_SYSTEM_CHECK(system, status)

#endif // PHASE5_AUDIO_VERIFICATION