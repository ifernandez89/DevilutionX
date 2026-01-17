#pragma once

#ifdef PHASE3_INPUT_VERIFICATION

#include <iostream>
#include <string>

// FASE 3: Enhanced logging for input system verification
#define PHASE3_LOG(message) \
    do { \
        std::cout << "[PHASE3] " << message << std::endl; \
    } while(0)

#define PHASE3_INPUT_EVENT(event_type, details) \
    do { \
        std::cout << "[PHASE3-INPUT] " << event_type << ": " << details << std::endl; \
    } while(0)

#define PHASE3_MOUSE_EVENT(x, y, button, action) \
    do { \
        std::cout << "[PHASE3-MOUSE] " << action << " at (" << x << "," << y << ") button=" << button << std::endl; \
    } while(0)

#define PHASE3_KEYBOARD_EVENT(key, action) \
    do { \
        std::cout << "[PHASE3-KEYBOARD] " << action << " key=" << key << std::endl; \
    } while(0)

#define PHASE3_INPUT_SYSTEM_CHECK(system, status) \
    do { \
        std::cout << "[PHASE3-SYSTEM] " << system << ": " << status << std::endl; \
    } while(0)

#else

#define PHASE3_LOG(message) do {} while(0)
#define PHASE3_INPUT_EVENT(event_type, details) do {} while(0)
#define PHASE3_MOUSE_EVENT(x, y, button, action) do {} while(0)
#define PHASE3_KEYBOARD_EVENT(key, action) do {} while(0)
#define PHASE3_INPUT_SYSTEM_CHECK(system, status) do {} while(0)

#endif