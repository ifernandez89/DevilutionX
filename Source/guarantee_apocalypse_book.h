#pragma once

/**
 * @file guarantee_apocalypse_book.h
 * 
 * 📖 BOOK OF APOCALYPSE GUARANTEE SYSTEM (INFERNO MASTER EDITION)
 * 
 * OBJETIVO:
 * Garantizar que el jugador tenga acceso al Book of Apocalypse, uno de los
 * hechizos más poderosos del juego, sin depender del RNG de las tiendas.
 * 
 * FILOSOFÍA:
 * "Los jugadores merecen acceso a las herramientas más poderosas del juego
 *  sin depender completamente de la suerte."
 * 
 * CARACTERÍSTICAS:
 * ✅ Garantía automática al alcanzar nivel suficiente
 * ✅ Solo se crea si el jugador no lo tiene
 * ✅ Compatible con Hellfire y Diablo original
 * ✅ No duplica libros existentes
 * ✅ Integración transparente
 */

namespace devilution {

// Forward declarations
struct Player;

// ============================================================================
// 📖 BOOK OF APOCALYPSE GUARANTEE CONSTANTS
// ============================================================================

namespace ApocalypseGuarantee {
    constexpr int MIN_LEVEL_FOR_APOCALYPSE = 19;    // Nivel mínimo para obtener Apocalypse
    constexpr int APOCALYPSE_SPELL_LEVEL = 8;       // Nivel del hechizo Apocalypse
    constexpr int CHECK_INTERVAL_TICKS = 60 * 5;    // Verificar cada 5 segundos
}

// ============================================================================
// 📖 BOOK OF APOCALYPSE GUARANTEE API
// ============================================================================

/**
 * Initialize the Book of Apocalypse guarantee system
 */
void InitApocalypseBookGuarantee();

/**
 * Update the guarantee system (called periodically)
 */
void UpdateApocalypseBookGuarantee();

/**
 * Check if player should receive Book of Apocalypse
 * @param player The player to check
 * @return true if player should receive the book
 */
bool ShouldReceiveApocalypseBook(const Player& player);

/**
 * Check if player already has Book of Apocalypse or the spell
 * @param player The player to check
 * @return true if player already has access to Apocalypse
 */
bool PlayerHasApocalypseAccess(const Player& player);

/**
 * Create and give Book of Apocalypse to player
 * @param player The player to give the book to
 * @return true if book was successfully created and given
 */
bool GiveApocalypseBookToPlayer(Player& player);

/**
 * Find empty inventory slot for the book
 * @param player The player
 * @return inventory slot index, or -1 if no space
 */
int FindEmptyInventorySlot(const Player& player);

} // namespace devilution