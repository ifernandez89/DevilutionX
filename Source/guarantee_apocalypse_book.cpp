#include "guarantee_apocalypse_book.h"
#include "player.h"
#include "items.h"
#include "spells.h"
#include "multi.h"
#include "utils/log.hpp"
#include "utils/utf8.hpp"

namespace devilution {

// ============================================================================
// 📖 GLOBAL STATE
// ============================================================================

static int g_lastCheckTime = 0;
static bool g_systemInitialized = false;

// ============================================================================
// 📖 IMPLEMENTATION
// ============================================================================

void InitApocalypseBookGuarantee()
{
    g_lastCheckTime = 0;
    g_systemInitialized = true;
    
    LogVerbose("📖 Book of Apocalypse Guarantee System initialized");
}

bool PlayerHasApocalypseAccess(const Player& player)
{
    // Check if player knows the Apocalypse spell
    if (player._pSplLvl[static_cast<uint8_t>(SpellID::Apocalypse)] > 0) {
        return true;
    }
    
    // Check if player has Book of Apocalypse in inventory
    for (int i = 0; i < player._pNumInv; i++) {
        const Item& item = player.InvList[i];
        if (item._itype == ItemType::Misc && 
            item._iMiscId == IMISC_BOOK && 
            item._iSpell == SpellID::Apocalypse) {
            return true;
        }
    }
    
    // Check belt for scrolls (alternative access)
    for (int i = 0; i < MaxBeltItems; i++) {
        const Item& item = player.SpdList[i];
        if (item._itype == ItemType::Misc && 
            item._iMiscId == IMISC_SCROLL && 
            item._iSpell == SpellID::Apocalypse) {
            return true; // Has scroll, counts as access
        }
    }
    
    return false;
}

bool ShouldReceiveApocalypseBook(const Player& player)
{
    // Must be high enough level
    if (player.getCharacterLevel() < ApocalypseGuarantee::MIN_LEVEL_FOR_APOCALYPSE) {
        return false;
    }
    
    // Must be a spellcaster (Sorcerer or have sufficient magic)
    if (player._pClass != HeroClass::Sorcerer && player._pMagic < 70) {
        return false; // Only sorcerers or high-magic characters get guaranteed access
    }
    
    // Must not already have access
    if (PlayerHasApocalypseAccess(player)) {
        return false;
    }
    
    // In Hellfire, Apocalypse is available normally
    // In Diablo original, we need to enable it
    return true;
}

int FindEmptyInventorySlot(const Player& player)
{
    for (int i = 0; i < InventoryGridCells; i++) {
        if (player.InvList[i].isEmpty()) {
            return i;
        }
    }
    return -1; // No empty slot found
}

bool GiveApocalypseBookToPlayer(Player& player)
{
    // Find empty inventory slot
    int emptySlot = FindEmptyInventorySlot(player);
    if (emptySlot == -1) {
        // No space in inventory - try to drop it on ground near player
        LogVerbose("📖 No inventory space for Apocalypse book, dropping on ground");
        
        // Create book on ground near player
        Point dropPosition = player.position.tile;
        CreateSpellBook(dropPosition, SpellID::Apocalypse, true, false);
        
        // Notify player
        NetSendCmdString(1 << MyPlayerId, "📖 Book of Apocalypse appeared near you!");
        
        LogVerbose("📖 Book of Apocalypse created on ground at ({}, {})", dropPosition.x, dropPosition.y);
        return true;
    }
    
    // Create the book item
    Item& book = player.InvList[emptySlot];
    
    // Initialize as Book of Apocalypse
    book._itype = ItemType::Misc;
    book._iMiscId = IMISC_BOOK;
    book._iSpell = SpellID::Apocalypse;
    book._iIdentified = true;
    book._iMagical = ITEM_QUALITY_NORMAL;
    book._ivalue = 2000; // High value book
    book._iIvalue = 2000;
    book._iCurs = ICURS_BOOK_RED; // Red book cursor
    
    // Set name and description
    CopyUtf8(book._iName, "Book of Apocalypse", sizeof(book._iName));
    CopyUtf8(book._iIName, "Book of Apocalypse", sizeof(book._iIName));
    
    // Update inventory count
    player._pNumInv++;
    
    // In Diablo original, enable Apocalypse spell
    if (!gbIsHellfire) {
        player._pSplLvl[static_cast<uint8_t>(SpellID::Apocalypse)] = 1;
    }
    
    // Notify player
    NetSendCmdString(1 << MyPlayerId, "📖 You have been granted the Book of Apocalypse!");
    
    LogVerbose("📖 Book of Apocalypse granted to player at level {}", player.getCharacterLevel());
    return true;
}

void UpdateApocalypseBookGuarantee()
{
    if (!g_systemInitialized) return;
    
    // Check every few seconds
    static int tickCounter = 0;
    if (++tickCounter < ApocalypseGuarantee::CHECK_INTERVAL_TICKS) {
        return;
    }
    tickCounter = 0;
    
    // Only check for main player
    if (!MyPlayer) return;
    
    Player& player = *MyPlayer;
    
    // Check if player should receive the book
    if (ShouldReceiveApocalypseBook(player)) {
        LogVerbose("📖 Player qualifies for Book of Apocalypse - attempting to grant");
        
        if (GiveApocalypseBookToPlayer(player)) {
            LogVerbose("📖 Book of Apocalypse successfully granted to player");
        } else {
            LogVerbose("📖 Failed to grant Book of Apocalypse (no space)");
        }
    }
}

} // namespace devilution