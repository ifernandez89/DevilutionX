/**
 * @file ai_stubs.cpp
 * @brief Temporary stubs for AI functions to allow compilation
 * 
 * These are placeholder implementations for AI functions that are
 * referenced but not yet implemented. This allows the VFS system
 * to compile and be tested.
 */

#include <string>

namespace devilution {

/**
 * @brief Stub for AI text variation initialization
 */
void InitAITextVariation()
{
    // Stub implementation - AI system not yet integrated
}

/**
 * @brief Stub for AI chat message processing
 * @param message The chat message to process
 */
void ProcessChatMessageWithAI(const std::string& message)
{
    // Stub implementation - AI system not yet integrated
    (void)message; // Suppress unused parameter warning
}

/**
 * @brief Stub for AI text variation cleanup
 */
void CleanupAITextVariation()
{
    // Stub implementation - AI system not yet integrated
}

} // namespace devilution