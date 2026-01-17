#include "area_dump.h"

#include <fstream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <cstdlib>

#include "player.h"
#include "levels/gendung.h"

namespace devilution {

void CaptureAreaDump(const std::string& trigger)
{
    // Generate timestamp for filename
    auto now = std::time(nullptr);
    auto tm = *std::localtime(&now);
    
    std::ostringstream filename;
    filename << "debug_logs/area_dump_" << trigger << "_"
             << std::put_time(&tm, "%Y%m%d_%H%M%S") << ".txt";
    
    std::ofstream file(filename.str());
    if (!file.is_open()) {
        return; // Failed to create file
    }
    
    file << "=== AREA DUMP SYSTEM - DETAILED ANALYSIS ===" << std::endl;
    file << "Timestamp: " << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << std::endl;
    file << "Trigger: " << trigger << std::endl;
    file << "Purpose: Identify corrupted objects causing location-based crashes" << std::endl;
    file << "=========================================" << std::endl << std::endl;
    
    // Player Information
    file << "=== PLAYER INFORMATION ===" << std::endl;
    if (MyPlayer != nullptr) {
        file << "Player Position: (" << MyPlayer->position.tile.x << ", " << MyPlayer->position.tile.y << ")" << std::endl;
        file << "Player Future: (" << MyPlayer->position.future.x << ", " << MyPlayer->position.future.y << ")" << std::endl;
        file << "Player Last: (" << MyPlayer->position.last.x << ", " << MyPlayer->position.last.y << ")" << std::endl;
        file << "Player Level: " << static_cast<int>(MyPlayer->plrlevel) << std::endl;
        file << "Player HP: " << MyPlayer->_pHitPoints << "/" << MyPlayer->_pMaxHP << std::endl;
    } else {
        file << "ERROR: MyPlayer is nullptr!" << std::endl;
    }
    file << std::endl;
    
    // Level Information
    file << "=== LEVEL INFORMATION ===" << std::endl;
    file << "Current Level: " << static_cast<int>(currlevel) << std::endl;
    file << "Level Type: " << static_cast<int>(leveltype) << std::endl;
    file << "Dungeon Size: " << DMAXX << "x" << DMAXY << std::endl;
    file << std::endl;
    
    // Simple Items Analysis - Just count and basic info
    file << "=== ITEMS ANALYSIS (BASIC) ===" << std::endl;
    if (MyPlayer != nullptr) {
        int playerX = MyPlayer->position.tile.x;
        int playerY = MyPlayer->position.tile.y;
        int radius = 10;
        
        file << "Scanning items in " << (radius * 2 + 1) << "x" << (radius * 2 + 1) << " area around player..." << std::endl;
        
        int itemCount = 0;
        for (int i = 0; i < MAXITEMS; i++) {
            if (!Items[i].isEmpty()) {
                int itemX = Items[i].position.x;
                int itemY = Items[i].position.y;
                
                // Check if item is in the area around player (using simple math)
                int deltaX = itemX - playerX;
                int deltaY = itemY - playerY;
                if (deltaX < 0) deltaX = -deltaX;
                if (deltaY < 0) deltaY = -deltaY;
                
                if (deltaX <= radius && deltaY <= radius) {
                    file << "Item[" << i << "]: ";
                    file << "Pos(" << itemX << "," << itemY << ") ";
                    file << "Type:" << static_cast<int>(Items[i]._itype) << " ";
                    file << "ID:" << static_cast<int>(Items[i].IDidx) << " ";
                    file << "Seed:" << Items[i]._iSeed << " ";
                    
                    // Basic validation
                    if (itemX < 0 || itemX >= DMAXX || itemY < 0 || itemY >= DMAXY) {
                        file << "**INVALID_POSITION** ";
                    }
                    file << std::endl;
                    itemCount++;
                }
            }
        }
        file << "Total items in area: " << itemCount << std::endl;
    }
    file << std::endl;
    
    // Dungeon Grid Analysis (small area around player)
    file << "=== DUNGEON GRID ANALYSIS (5x5 AROUND PLAYER) ===" << std::endl;
    if (MyPlayer != nullptr) {
        int playerX = MyPlayer->position.tile.x;
        int playerY = MyPlayer->position.tile.y;
        int radius = 2; // 5x5 grid around player
        
        for (int y = playerY - radius; y <= playerY + radius; y++) {
            for (int x = playerX - radius; x <= playerX + radius; x++) {
                if (x >= 0 && x < DMAXX && y >= 0 && y < DMAXY) {
                    file << std::setw(3) << static_cast<int>(dungeon[x][y]);
                } else {
                    file << " XX";
                }
            }
            file << std::endl;
        }
    }
    file << std::endl;
    
    file << "=== END OF AREA DUMP ===" << std::endl;
    file.close();
}

void CheckAreaDumpHotkey()
{
    // This function is not used - area dump is automatic on save load
}

void CaptureAreaDumpOnLoad()
{
    // Automatically capture area dump when save is loaded
    CaptureAreaDump("save_load");
}

} // namespace devilution