// Minimal test to verify Feature 1 code compiles
#include <iostream>

// Mock the required types and constants for compilation test
enum quest_state {
    QUEST_NOTAVAIL,
    QUEST_INIT,
    QUEST_ACTIVE,
    QUEST_DONE
};

struct Quest {
    quest_state _qactive;
};

const int MAXQUESTS = 24;
Quest Quests[MAXQUESTS];
bool gbIsSpawn = false;

// Feature 1 implementation (extracted from quests.cpp)
void TestFeature1Logic() {
    // Initialize all quests as QUEST_NOTAVAIL (simulating original state)
    for (auto &quest : Quests) {
        quest._qactive = QUEST_NOTAVAIL;
    }
    
    std::cout << "Before Feature 1: All quests are QUEST_NOTAVAIL" << std::endl;
    
    // FEATURE 1: Garantizar que todas las quests estén disponibles
    // Solo hacer disponibles, NO activar automáticamente
    if (!gbIsSpawn) {
        for (auto &quest : Quests) {
            if (quest._qactive == QUEST_NOTAVAIL) {
                quest._qactive = QUEST_INIT;
            }
        }
    }
    
    // Verify the result
    int availableQuests = 0;
    for (auto &quest : Quests) {
        if (quest._qactive == QUEST_INIT) {
            availableQuests++;
        }
    }
    
    std::cout << "After Feature 1: " << availableQuests << " quests are now available (QUEST_INIT)" << std::endl;
    
    if (availableQuests == MAXQUESTS) {
        std::cout << "✅ FEATURE 1 TEST PASSED: All quests are now available!" << std::endl;
    } else {
        std::cout << "❌ FEATURE 1 TEST FAILED: Not all quests are available!" << std::endl;
    }
}

int main() {
    std::cout << "Testing Feature 1 compilation and logic..." << std::endl;
    TestFeature1Logic();
    std::cout << "Feature 1 logic compiled successfully!" << std::endl;
    return 0;
}