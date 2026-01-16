#include "hellfire_book_fix.h"
#include "player.h"
#include "items.h"
#include "utils/log.hpp"

namespace devilution {

void ScanPlayerInventoryForHellfireItems(Player& player) {
    // 🛡️ SAFETY CHECKS: Prevenir crashes durante operaciones de menú
    if (!gbRunGame) {
        LogVerbose("🛑 SAFETY: Hellfire recovery skipped - game not running");
        return;
    }
    
    if (!player.plractive) {
        LogVerbose("🛑 SAFETY: Hellfire recovery skipped - player not active");
        return;
    }
    
    // Verificar que el jugador tenga datos válidos
    if (player._pNumInv < 0 || player._pNumInv > InventoryGridCells) {
        LogVerbose("🛑 SAFETY: Hellfire recovery skipped - invalid inventory count: {}", player._pNumInv);
        return;
    }
    
    LogVerbose("🔍 Scanning player inventory for lost Hellfire items...");
    
    int recoveredItems = 0;
    
    // Escanear inventario principal de forma muy segura
    for (int i = 0; i < player._pNumInv && i < InventoryGridCells; i++) {
        Item& item = player.InvList[i];
        if (!item.isEmpty() && item.IDidx >= 0 && item.IDidx < static_cast<int>(AllItemsList.size())) {
            uint32_t oldBuff = item.dwBuff;
            ForceHellfireItemDetection(item);  // Sin verbose logging
            if ((item.dwBuff & CF_HELLFIRE) != 0 && (oldBuff & CF_HELLFIRE) == 0) {
                recoveredItems++;
                LogInfo("✅ RECOVERED Hellfire item in inventory: {}", item._iIName);
            }
        }
    }
    
    // Escanear items equipados de forma muy segura
    for (int i = 0; i < NUM_INVLOC; i++) {
        Item& item = player.InvBody[i];
        if (!item.isEmpty() && item.IDidx >= 0 && item.IDidx < static_cast<int>(AllItemsList.size())) {
            uint32_t oldBuff = item.dwBuff;
            ForceHellfireItemDetection(item);  // Sin verbose logging
            if ((item.dwBuff & CF_HELLFIRE) != 0 && (oldBuff & CF_HELLFIRE) == 0) {
                recoveredItems++;
                LogInfo("✅ RECOVERED Hellfire item equipped: {}", item._iIName);
            }
        }
    }
    
    // Escanear cinturón de forma muy segura
    for (int i = 0; i < MaxBeltItems; i++) {
        Item& item = player.SpdList[i];
        if (!item.isEmpty() && item.IDidx >= 0 && item.IDidx < static_cast<int>(AllItemsList.size())) {
            uint32_t oldBuff = item.dwBuff;
            ForceHellfireItemDetection(item);  // Sin verbose logging
            if ((item.dwBuff & CF_HELLFIRE) != 0 && (oldBuff & CF_HELLFIRE) == 0) {
                recoveredItems++;
                LogInfo("✅ RECOVERED Hellfire item in belt: {}", item._iIName);
            }
        }
    }
    
    if (recoveredItems > 0) {
        LogInfo("🔥 HELLFIRE RECOVERY COMPLETE: {} items recovered!", recoveredItems);
    }
}

void ScanGroundItemsForHellfireItems() {
    // 🛡️ SAFETY CHECKS: Prevenir crashes durante operaciones de menú
    if (!gbRunGame) {
        LogVerbose("🛑 SAFETY: Ground scan skipped - game not running");
        return;
    }
    
    LogVerbose("🔍 Scanning ground items for lost Hellfire items...");
    
    int recoveredItems = 0;
    
    for (int i = 0; i < MAXITEMS; i++) {
        Item& item = Items[i];
        if (!item.isEmpty() && item.IDidx >= 0 && item.IDidx < static_cast<int>(AllItemsList.size())) {
            uint32_t oldBuff = item.dwBuff;
            ForceHellfireItemDetection(item);
            if ((item.dwBuff & CF_HELLFIRE) != 0 && (oldBuff & CF_HELLFIRE) == 0) {
                recoveredItems++;
                LogInfo("✅ RECOVERED Hellfire item on ground: {}", item._iIName);
            }
        }
    }
    
    if (recoveredItems > 0) {
        LogInfo("🔥 GROUND HELLFIRE RECOVERY COMPLETE: {} items recovered!", recoveredItems);
    } else {
        LogVerbose("🔍 No lost Hellfire items found on ground");
    }
}

void EmergencyHellfireItemRecovery() {
    // 🛡️ CRITICAL SAFETY CHECKS: Prevenir crashes durante operaciones de menú
    if (!gbRunGame) {
        LogVerbose("🛑 SAFETY: Emergency recovery skipped - game not running");
        return;
    }
    
    if (MyPlayer == nullptr) {
        LogVerbose("🛑 SAFETY: Emergency recovery skipped - MyPlayer is null");
        return;
    }
    
    if (!MyPlayer->plractive) {
        LogVerbose("🛑 SAFETY: Emergency recovery skipped - player not active");
        return;
    }
    
    // Verificar que no estemos en menús o estados inválidos
    if (MyPlayer->_pNumInv < 0 || MyPlayer->_pNumInv > InventoryGridCells) {
        LogVerbose("🛑 SAFETY: Emergency recovery skipped - invalid inventory state");
        return;
    }
    
    LogInfo("🚨 EMERGENCY HELLFIRE ITEM RECOVERY INITIATED");
    
    // Escanear inventario del jugador principal de forma muy segura
    ScanPlayerInventoryForHellfireItems(*MyPlayer);
    
    LogInfo("🔥 EMERGENCY HELLFIRE RECOVERY COMPLETE");
}

} // namespace devilution