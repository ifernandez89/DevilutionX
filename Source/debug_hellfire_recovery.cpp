// 🔥 DEBUG: Sistema de debugging agresivo para Hellfire recovery
// Este archivo contiene funciones de debugging para identificar exactamente qué está fallando

#include "debug_hellfire_recovery.h"
#include "items.h"
#include "player.h"
#include "utils/log.hpp"
#include "game_mode.hpp"
#include "loadsave.h"
#include <fstream>

namespace devilution {

void DebugPrintAllPlayerItems(const Player& player, const std::string& context) {
    LogInfo("🔍 DEBUG: Printing all player items - Context: {}", context);
    
    // Debug inventario principal
    LogInfo("📦 MAIN INVENTORY ({} items):", player._pNumInv);
    for (int i = 0; i < player._pNumInv; i++) {
        const Item& item = player.InvList[i];
        if (!item.isEmpty()) {
            LogInfo("  [{}] {} (IDidx={}, Spell={}, dwBuff=0x{:X}, MiscId={})", 
                   i, item._iIName, static_cast<int>(item.IDidx), 
                   static_cast<int>(item._iSpell), item.dwBuff,
                   static_cast<int>(AllItemsList[item.IDidx].iMiscId));
            
            // Verificar si es un libro
            if (AllItemsList[item.IDidx].iMiscId == IMISC_BOOK) {
                LogInfo("    📖 BOOK DETECTED! Spell: {}, Hellfire flag: {}", 
                       static_cast<int>(item._iSpell),
                       (item.dwBuff & CF_HELLFIRE) ? "YES" : "NO");
            }
        }
    }
    
    // Debug items equipados
    LogInfo("⚔️ EQUIPPED ITEMS:");
    for (int i = 0; i < NUM_INVLOC; i++) {
        const Item& item = player.InvBody[i];
        if (!item.isEmpty()) {
            LogInfo("  [{}] {} (IDidx={}, dwBuff=0x{:X})", 
                   i, item._iIName, static_cast<int>(item.IDidx), item.dwBuff);
        }
    }
    
    // Debug cinturón
    LogInfo("🎒 BELT ITEMS:");
    for (int i = 0; i < MaxBeltItems; i++) {
        const Item& item = player.SpdList[i];
        if (!item.isEmpty()) {
            LogInfo("  [{}] {} (IDidx={}, dwBuff=0x{:X})", 
                   i, item._iIName, static_cast<int>(item.IDidx), item.dwBuff);
        }
    }
}

void DebugSearchForInfernoAnywhere() {
    LogInfo("🔥 DEBUG: Searching for ANY Inferno-related items ANYWHERE...");
    
    // Buscar en items del suelo
    LogInfo("🌍 GROUND ITEMS:");
    for (int i = 0; i < MAXITEMS; i++) {
        const Item& item = Items[i];
        if (!item.isEmpty()) {
            if (strstr(item._iIName, "Inferno") != nullptr || 
                strstr(item._iName, "Inferno") != nullptr ||
                item._iSpell == SpellID::Inferno) {
                LogInfo("  FOUND INFERNO ITEM ON GROUND: {} (IDidx={}, Spell={}, dwBuff=0x{:X})", 
                       item._iIName, static_cast<int>(item.IDidx), 
                       static_cast<int>(item._iSpell), item.dwBuff);
            }
        }
    }
    
    // Buscar en todos los jugadores
    LogInfo("👥 ALL PLAYERS:");
    for (int p = 0; p < MAX_PLRS; p++) {
        const Player& player = Players[p];
        if (player.plractive) {
            LogInfo("  Player {}: {}", p, player._pName);
            
            // Buscar en inventario
            for (int i = 0; i < player._pNumInv; i++) {
                const Item& item = player.InvList[i];
                if (!item.isEmpty() && (strstr(item._iIName, "Inferno") != nullptr || 
                    item._iSpell == SpellID::Inferno)) {
                    LogInfo("    FOUND INFERNO IN INVENTORY: {}", item._iIName);
                }
            }
        }
    }
}

void DebugSaveGameFlags() {
    LogInfo("🔍 DEBUG: Save game flags analysis:");
    LogInfo("  gbIsHellfire: {}", gbIsHellfire ? "TRUE" : "FALSE");
    LogInfo("  gbIsHellfireSaveGame: {}", gbIsHellfireSaveGame ? "TRUE" : "FALSE");
    LogInfo("  gbIsMultiplayer: {}", gbIsMultiplayer ? "TRUE" : "FALSE");
    LogInfo("  gbIsSpawn: {}", gbIsSpawn ? "TRUE" : "FALSE");
}

void DebugTestInfernoSpellCreation() {
    LogInfo("🔥 DEBUG: Testing Inferno spell creation...");
    
    if (MyPlayer == nullptr) {
        LogError("MyPlayer is null!");
        return;
    }
    
    // Intentar crear un libro de Inferno manualmente
    Item testBook = {};
    
    // Buscar el ID correcto para libros
    for (int i = 0; i < static_cast<int>(AllItemsList.size()); i++) {
        if (AllItemsList[i].iMiscId == IMISC_BOOK) {
            LogInfo("  Found book item ID: {} ({})", i, AllItemsList[i].iName);
            
            // Crear libro de prueba
            testBook.IDidx = static_cast<_item_indexes>(i);
            testBook._iSpell = SpellID::Inferno;
            testBook.dwBuff = CF_HELLFIRE;  // Forzar flag de Hellfire
            strcpy(testBook._iName, "Book of Inferno (TEST)");
            strcpy(testBook._iIName, "Book of Inferno (TEST)");
            
            LogInfo("  Created test book: {} with Hellfire flag", testBook._iIName);
            break;
        }
    }
}

void DebugFullDiagnostics() {
    LogInfo("🚨 FULL DEBUG DIAGNOSTICS STARTING...");
    LogInfo(std::string(50, '='));
    
    DebugSaveGameFlags();
    
    if (MyPlayer != nullptr) {
        DebugPrintAllPlayerItems(*MyPlayer, "Full Diagnostics");
    } else {
        LogError("MyPlayer is NULL!");
    }
    
    DebugSearchForInfernoAnywhere();
    DebugTestInfernoSpellCreation();
    
    LogInfo("🚨 FULL DEBUG DIAGNOSTICS COMPLETE");
    LogInfo(std::string(50, '='));
}

} // namespace devilution