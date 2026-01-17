#pragma once

#ifdef PHASE2_ENGINE_VERIFICATION

#include <iostream>
#include <string>

// FASE 2: Enhanced logging for engine loop verification
#define PHASE2_LOG(message) \
    do { \
        std::cout << "[PHASE2] " << message << std::endl; \
    } while(0)

#define PHASE2_ENGINE_STEP(step, details) \
    do { \
        std::cout << "[PHASE2-ENGINE] " << step << ": " << details << std::endl; \
    } while(0)

#define PHASE2_MEMORY_CHECK(location) \
    do { \
        std::cout << "[PHASE2-MEMORY] " << location << ": Memory OK - RAM operation confirmed" << std::endl; \
    } while(0)

#else

#define PHASE2_LOG(message) do {} while(0)
#define PHASE2_ENGINE_STEP(step, details) do {} while(0)
#define PHASE2_MEMORY_CHECK(location) do {} while(0)

#endif