#pragma once

/**
 * @file hellfire_book_fix.h
 * 
 * 🔥 HELLFIRE BOOK RECOVERY SYSTEM
 * 
 * PROBLEMA IDENTIFICADO:
 * Los libros de Hellfire (especialmente Inferno) desaparecen del inventario
 * cuando se cargan saves de Hellfire en DevilutionX Enhanced.
 * 
 * CAUSA:
 * - Falta de flag CF_HELLFIRE en items de saves de Hellfire vanilla
 * - UpdateHellfireFlag() solo funciona para items mágicos
 * - No se ejecuta en todos los puntos de carga de items
 * 
 * SOLUCIÓN MÚLTIPLE:
 * 1. Forzar detección en múltiples puntos del código
 * 2. Verificar por spell ID y nombre del item
 * 3. Aplicar fix en carga de save, unpacking, y recreación
 * 4. Logging detallado para debugging
 */

#include "items.h"
#include "spells.h"
#include "utils/log.hpp"
#include "loadsave.h"  // Para gbIsHellfireSaveGame

namespace devilution {

// ============================================================================
// 🔥 HELLFIRE SPELL DETECTION
// ============================================================================

/**
 * Verifica si un spell es exclusivo de Hellfire
 */
inline bool IsHellfireSpell(SpellID spell) {
    switch (spell) {
        case SpellID::Inferno:           // 🔥 EL LIBRO QUE NECESITAS
        case SpellID::Immolation:
        case SpellID::LightningWall:
        case SpellID::Warp:
        case SpellID::Reflect:
        case SpellID::Berserk:
        case SpellID::RingOfFire:
        case SpellID::Search:
        case SpellID::Mana:
        case SpellID::Magi:
        case SpellID::Jester:
            return true;
        default:
            return false;
    }
}

/**
 * Verifica si el nombre del item contiene referencias a Hellfire
 */
inline bool HasHellfireItemName(const Item& item) {
    const char* hellfireNames[] = {
        "Inferno",
        "Immolation", 
        "Lightning Wall",
        "Warp",
        "Reflect",
        "Berserk",
        "Ring of Fire",
        "Search",
        "Mana",
        "Magi",
        "Jester"
    };
    
    for (const char* name : hellfireNames) {
        if (strstr(item._iIName, name) != nullptr || 
            strstr(item._iName, name) != nullptr) {
            return true;
        }
    }
    return false;
}

/**
 * FUNCIÓN PRINCIPAL: Fuerza la detección y flag de items de Hellfire
 * Esta función debe ser llamada en TODOS los puntos donde se cargan items
 */
inline void ForceHellfireItemDetection(Item& item) {
    // Si ya tiene la flag, no hacer nada
    if ((item.dwBuff & CF_HELLFIRE) != 0) {
        return;
    }
    
    bool isHellfireItem = false;
    std::string reason;
    
    // MÉTODO 1: Verificar si es un libro con spell de Hellfire
    if (AllItemsList[item.IDidx].iMiscId == IMISC_BOOK) {
        if (IsHellfireSpell(item._iSpell)) {
            isHellfireItem = true;
            reason = fmt::format("Book with Hellfire spell: {}", static_cast<int>(item._iSpell));
        }
    }
    
    // MÉTODO 2: Verificar por nombre del item (MÁS AGRESIVO)
    if (!isHellfireItem && HasHellfireItemName(item)) {
        isHellfireItem = true;
        reason = fmt::format("Item name contains Hellfire reference: {}", item._iIName);
    }
    
    // MÉTODO 3: NUEVO - Verificar específicamente por SpellID sin importar el tipo
    if (!isHellfireItem && IsHellfireSpell(item._iSpell)) {
        isHellfireItem = true;
        reason = fmt::format("Item has Hellfire spell ID: {}", static_cast<int>(item._iSpell));
    }
    
    // MÉTODO 4: NUEVO - Si estamos cargando un save de Hellfire, ser más permisivo
    if (!isHellfireItem && gbIsHellfireSaveGame && AllItemsList[item.IDidx].iMiscId == IMISC_BOOK) {
        // En saves de Hellfire, todos los libros deberían tener la flag
        isHellfireItem = true;
        reason = fmt::format("Book in Hellfire save should have flag: {}", item._iIName);
    }
    
    // Si detectamos que es un item de Hellfire, aplicar la flag
    if (isHellfireItem) {
        item.dwBuff |= CF_HELLFIRE;
        LogInfo("🔥 HELLFIRE ITEM RECOVERED: {} - Reason: {}", item._iIName, reason);
        
        // Log adicional para debugging
        LogVerbose("   Item details: IDidx={}, Spell={}, MiscId={}", 
                  static_cast<int>(item.IDidx), 
                  static_cast<int>(item._iSpell),
                  static_cast<int>(AllItemsList[item.IDidx].iMiscId));
    }
}

/**
 * Versión con logging más detallado para debugging
 */
inline void ForceHellfireItemDetectionVerbose(Item& item, const std::string& context) {
    LogVerbose("🔍 Checking item for Hellfire detection in context: {}", context);
    LogVerbose("   Item: {} ({})", item._iIName, item._iName);
    LogVerbose("   IDidx: {}, Spell: {}, dwBuff: 0x{:X}", 
              static_cast<int>(item.IDidx), 
              static_cast<int>(item._iSpell),
              item.dwBuff);
    
    ForceHellfireItemDetection(item);
}

/**
 * Escanea todo el inventario del jugador buscando items de Hellfire perdidos
 */
void ScanPlayerInventoryForHellfireItems(Player& player);

/**
 * Escanea todos los items en el suelo buscando items de Hellfire perdidos
 */
void ScanGroundItemsForHellfireItems();

/**
 * Función de emergencia: fuerza la recuperación de todos los items de Hellfire
 */
void EmergencyHellfireItemRecovery();

} // namespace devilution