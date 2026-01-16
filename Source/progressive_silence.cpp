#include "progressive_silence.h"
#include "diablo.h"
#include "utils/log.hpp"
#include "engine/random.hpp"

#ifdef USE_SDL3
#include <SDL3/SDL_timer.h>
#else
#include <SDL.h>
#endif

namespace devilution {

// ============================================================================
// 🎵 GLOBAL STATE
// ============================================================================

static ProgressiveSilenceState g_progressiveSilence;

// ============================================================================
// 🎵 IMPLEMENTATION
// ============================================================================

void InitProgressiveSilence()
{
    g_progressiveSilence = ProgressiveSilenceState{};
    g_progressiveSilence.enabled = true;
    LogVerbose("🎵 Progressive Silence System initialized");
}

int GetSilenceBehaviorForLevel(int level)
{
    if (level <= ProgressiveSilence::NORMAL_LEVELS) {
        return 0; // Normal music
    } else if (level <= ProgressiveSilence::PAUSE_LEVELS) {
        return 1; // Music with pauses
    } else {
        return 2; // Deep silence mode
    }
}

bool ShouldStartMusicImmediately()
{
    if (!g_progressiveSilence.enabled) return true;
    
    int behavior = GetSilenceBehaviorForLevel(currlevel);
    
    switch (behavior) {
        case 0: // Normal levels (1-4)
            return true;
            
        case 1: // Pause levels (5-8)
            // 70% chance to start immediately, 30% chance to delay
            return GenerateRnd(100) < 70;
            
        case 2: // Deep levels (9+)
            // Only 25% chance to play music at all
            return GenerateRnd(100) < ProgressiveSilence::DEEP_MUSIC_CHANCE;
            
        default:
            return true;
    }
}

uint32_t GetPauseDurationForLevel(int level)
{
    int behavior = GetSilenceBehaviorForLevel(level);
    
    switch (behavior) {
        case 1: // Pause levels (5-8)
            return ProgressiveSilence::MIN_PAUSE_MS + 
                   GenerateRnd(ProgressiveSilence::MAX_PAUSE_MS - ProgressiveSilence::MIN_PAUSE_MS);
                   
        case 2: // Deep levels (9+)
            return ProgressiveSilence::DEEP_MIN_PAUSE_MS + 
                   GenerateRnd(ProgressiveSilence::DEEP_MAX_PAUSE_MS - ProgressiveSilence::DEEP_MIN_PAUSE_MS);
                   
        default:
            return 0;
    }
}

void ProgressiveSilence_StartMusic(_music_id track)
{
    if (!g_progressiveSilence.enabled) {
        music_start(track);
        return;
    }
    
    // Always stop current music first
    music_stop();
    
    if (ShouldStartMusicImmediately()) {
        // Start music normally
        music_start(track);
        g_progressiveSilence.musicPaused = false;
        g_progressiveSilence.pendingTrack = NUM_MUSIC;
        
        LogVerbose("🎵 Progressive Silence: Starting music immediately for level {}", currlevel);
    } else {
        // Delay music start
        g_progressiveSilence.musicPaused = true;
        g_progressiveSilence.pauseStartTime = SDL_GetTicks();
        g_progressiveSilence.pauseDuration = GetPauseDurationForLevel(currlevel);
        g_progressiveSilence.pendingTrack = track;
        
        LogVerbose("🎵 Progressive Silence: Delaying music for {}ms on level {}", 
                  g_progressiveSilence.pauseDuration, currlevel);
    }
}

void UpdateProgressiveSilence()
{
    if (!g_progressiveSilence.enabled) return;
    if (!g_progressiveSilence.musicPaused) return;
    if (g_progressiveSilence.pendingTrack == NUM_MUSIC) return;
    
    uint32_t currentTime = SDL_GetTicks();
    uint32_t elapsed = currentTime - g_progressiveSilence.pauseStartTime;
    
    if (elapsed >= g_progressiveSilence.pauseDuration) {
        // Time to start the pending music
        _music_id trackToStart = g_progressiveSilence.pendingTrack;
        
        // Clear pause state
        g_progressiveSilence.musicPaused = false;
        g_progressiveSilence.pendingTrack = NUM_MUSIC;
        g_progressiveSilence.pauseStartTime = 0;
        g_progressiveSilence.pauseDuration = 0;
        
        // Start the music
        music_start(trackToStart);
        
        LogVerbose("🎵 Progressive Silence: Starting delayed music for level {}", currlevel);
        
        // For deep levels, schedule another pause after music plays for a while
        if (GetSilenceBehaviorForLevel(currlevel) == 2) {
            // In deep levels, music will play for 30-60 seconds then pause again
            // This creates the "rare music" effect
            // TODO: Implement music auto-pause after playing (future enhancement)
        }
    }
}

void ProgressiveSilence_ForceStop()
{
    music_stop();
    g_progressiveSilence.musicPaused = false;
    g_progressiveSilence.pendingTrack = NUM_MUSIC;
    g_progressiveSilence.pauseStartTime = 0;
    g_progressiveSilence.pauseDuration = 0;
    
    LogVerbose("🎵 Progressive Silence: Force stopped");
}

const ProgressiveSilenceState& GetProgressiveSilenceState()
{
    return g_progressiveSilence;
}

} // namespace devilution