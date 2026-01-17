/**
 * @file test_webassembly_simple.cpp
 * @brief Simple WebAssembly test to verify Emscripten works
 */

#include <iostream>
#include <emscripten.h>

extern "C" {
    // Export a simple function to JavaScript
    EMSCRIPTEN_KEEPALIVE
    int test_function(int x, int y) {
        return x + y;
    }
    
    EMSCRIPTEN_KEEPALIVE
    void log_message(const char* message) {
        std::cout << "WebAssembly: " << message << std::endl;
    }
}

int main() {
    std::cout << "🌐 Nightmare Browser Edition - WebAssembly Test" << std::endl;
    std::cout << "✅ Emscripten compilation successful!" << std::endl;
    
    // Test basic functionality
    int result = test_function(5, 3);
    std::cout << "🧪 Test function result: " << result << std::endl;
    
    // Emscripten main loop (for browser)
    EM_ASM({
        console.log('🎮 Nightmare Browser Edition WebAssembly loaded!');
        console.log('🌐 Ready for browser deployment!');
    });
    
    return 0;
}