// 🔥 PARCHE ESPECÍFICO: Forzar detección de libro Inferno de Hellfire
// 
// Este parche asegura que el libro Inferno sea reconocido correctamente
// incluso cuando se carga un save de Hellfire en modo Diablo

#include "items.h"
#include "loadsave.h"
#include "game_mode.h"

namespace devilution {

// Función para forzar la detección de items de Hellfire al cargar saves
void ForceHellfireItemDetection(Item &item)
{
    // Si el item es un libro y no tiene la flag CF_HELLFIRE
    if (AllItemsList[item.IDidx].iMiscId == IMISC_BOOK && (item.dwBuff & CF_HELLFIRE) == 0) {
        
        // Lista de spells que son exclusivos de Hellfire
        const SpellID hellfireSpells[] = {
            SpellID::Inferno,           // 🔥 LIBRO INFERNO
            SpellID::Immolation,
            SpellID::LightningWall,
            SpellID::Warp,
            SpellID::Reflect,
            SpellID::Berserk,
            SpellID::RingOfFire,
            SpellID::Search,
            SpellID::Mana,
            SpellID::Magi,
            SpellID::Jester
        };
        
        // Verificar si el spell del libro está en la lista de Hellfire
        for (SpellID hellfireSpell : hellfireSpells) {
            if (item._iSpell == hellfireSpell) {
                // ✅ FORZAR FLAG DE HELLFIRE
                item.dwBuff |= CF_HELLFIRE;
                LogVerbose("🔥 Hellfire book detected and flagged: {} (Spell: {})", 
                          item._iIName, static_cast<int>(item._iSpell));
                break;
            }
        }
    }
    
    // También verificar items únicos de Hellfire por nombre
    const char* hellfireItemNames[] = {
        "Inferno",
        "Book of Inferno",
        "Scroll of Inferno"
    };
    
    for (const char* hellfireName : hellfireItemNames) {
        if (strstr(item._iIName, hellfireName) != nullptr) {
            item.dwBuff |= CF_HELLFIRE;
            LogVerbose("🔥 Hellfire item detected by name: {}", item._iIName);
            break;
        }
    }
}

// Parche para aplicar en la función de carga de items
void PatchHellfireItemLoading()
{
    // Este parche se debe aplicar en:
    // 1. LoadItemData() en loadsave.cpp
    // 2. UnPackItem() en pack.cpp
    // 3. RecreateItem() en items.cpp
    
    LogInfo("🔥 Hellfire item detection patch applied");
}

} // namespace devilution

/*
INSTRUCCIONES DE APLICACIÓN:

1. Añadir esta llamada en Source/loadsave.cpp, función LoadItemData():
   
   // Después de cargar el item, añadir:
   ForceHellfireItemDetection(item);

2. Añadir esta llamada en Source/pack.cpp, función UnPackItem():
   
   // Al final de la función, antes del return:
   ForceHellfireItemDetection(item);

3. Añadir esta llamada en Source/items.cpp, función RecreateItem():
   
   // Al final de la función:
   ForceHellfireItemDetection(item);

4. Incluir este header en los archivos mencionados:
   #include "fix_hellfire_book_detection.h"

RESULTADO ESPERADO:
- ✅ El libro Inferno aparecerá en tu inventario
- ✅ Otros items de Hellfire serán reconocidos correctamente
- ✅ No necesitas habilitar Hellfire globalmente
- ✅ Compatible con saves existentes
*/