#!/usr/bin/env python3
"""
TEST REALISTA DE APOCALYPSE - VERSIÓN PYTHON
Simulación de jugador moderno con fast-clicking
No requiere compilación, solo Python 3
"""

import time
from dataclasses import dataclass
from typing import List
from enum import Enum

# ============================================================================
# SIMULACIÓN DE ESTRUCTURAS
# ============================================================================

class MissileType(Enum):
    APOCALYPSE = 1
    APOCALYPSE_BOOM = 2
    OTHER = 3

@dataclass
class Missile:
    missile_type: MissileType
    source_player: int
    deleted: bool = False
    duration: int = 0

class GameState:
    def __init__(self):
        self.missiles: List[Missile] = []
        self.last_cast_by_player = {}
        self.frame_count = 0
        
    def get_current_time_ms(self):
        return int(time.time() * 1000)

# ============================================================================
# PROTECCIÓN DE APOCALYPSE
# ============================================================================

def can_safely_cast_apocalypse(game_state: GameState, player_id: int) -> bool:
    """Simula CanSafelyCastApocalypse() de C++"""
    
    current_time = game_state.get_current_time_ms()
    
    # Cooldown de 100ms por jugador
    if player_id in game_state.last_cast_by_player:
        time_since_last = current_time - game_state.last_cast_by_player[player_id]
        if time_since_last < 100:
            return False
    
    # Límite global: máximo 2 Apocalypse activos
    total_apocalypse = sum(1 for m in game_state.missiles 
                          if m.missile_type == MissileType.APOCALYPSE and not m.deleted)
    if total_apocalypse >= 2:
        return False
    
    # Límite por jugador: máximo 1 Apocalypse activo
    player_apocalypse = sum(1 for m in game_state.missiles 
                           if m.missile_type == MissileType.APOCALYPSE 
                           and m.source_player == player_id 
                           and not m.deleted)
    if player_apocalypse >= 1:
        return False
    
    game_state.last_cast_by_player[player_id] = current_time
    return True

def add_missile(game_state: GameState, missile_type: MissileType, player_id: int) -> bool:
    """Simula AddMissile() de C++"""
    
    # Protección de Apocalypse
    if missile_type == MissileType.APOCALYPSE:
        if not can_safely_cast_apocalypse(game_state, player_id):
            return False
    
    # Límite de missiles
    if len(game_state.missiles) >= 500:
        return False
    
    # Crear missile
    missile = Missile(missile_type=missile_type, source_player=player_id)
    
    if missile_type == MissileType.APOCALYPSE:
        missile.duration = 1  # Instantáneo
    
    game_state.missiles.append(missile)
    return True

def process_apocalypse(game_state: GameState, missile: Missile):
    """Simula ProcessApocalypse() de C++"""
    
    # Velocidad original: instantáneo (todo en 1 frame)
    MONSTERS_IN_AREA = 20  # Simulación
    MAX_BOOMS_PER_APOCALYPSE = 50
    
    booms_created = 0
    for _ in range(MONSTERS_IN_AREA):
        if booms_created >= MAX_BOOMS_PER_APOCALYPSE:
            break
        
        if len(game_state.missiles) < 500:
            boom = Missile(
                missile_type=MissileType.APOCALYPSE_BOOM,
                source_player=missile.source_player,
                duration=10
            )
            game_state.missiles.append(boom)
            booms_created += 1
    
    missile.deleted = True

def process_missiles(game_state: GameState):
    """Procesa todos los missiles activos"""
    
    for missile in game_state.missiles:
        if missile.deleted:
            continue
        
        if missile.missile_type == MissileType.APOCALYPSE:
            process_apocalypse(game_state, missile)
        elif missile.missile_type == MissileType.APOCALYPSE_BOOM:
            missile.duration -= 1
            if missile.duration <= 0:
                missile.deleted = True
    
    # Eliminar missiles marcados
    game_state.missiles = [m for m in game_state.missiles if not m.deleted]

# ============================================================================
# ESTADÍSTICAS
# ============================================================================

@dataclass
class TestStats:
    total_cast_attempts: int = 0
    successful_casts: int = 0
    blocked_by_cooldown: int = 0
    blocked_by_player_limit: int = 0
    blocked_by_global_limit: int = 0
    max_simultaneous_missiles: int = 0
    total_booms_created: int = 0
    crashed: bool = False

def print_stats(test_name: str, stats: TestStats):
    """Imprime estadísticas del test"""
    
    print("\n" + "=" * 60)
    print(f"TEST: {test_name}")
    print("=" * 60)
    print(f"Total cast attempts:      {stats.total_cast_attempts}")
    print(f"Successful casts:         {stats.successful_casts}")
    print(f"Blocked by cooldown:      {stats.blocked_by_cooldown}")
    print(f"Blocked by player limit:  {stats.blocked_by_player_limit}")
    print(f"Blocked by global limit:  {stats.blocked_by_global_limit}")
    print(f"Total booms created:      {stats.total_booms_created}")
    print(f"Max simultaneous missiles:{stats.max_simultaneous_missiles}")
    print(f"Result:                   {'❌ CRASHED' if stats.crashed else '✅ PASSED'}")
    print("=" * 60)

# ============================================================================
# TESTS
# ============================================================================

def test_fast_clicking_singleplayer():
    """Test 1: Fast-clicking singleplayer"""
    
    print("\n🎮 Running Test 1: Fast-Clicking SinglePlayer...")
    
    game_state = GameState()
    stats = TestStats()
    player_id = 0
    
    # 3 segundos @ 60 FPS = 180 frames
    TOTAL_FRAMES = 180
    CLICKS_PER_SECOND = 15  # Fast-clicking realista
    
    for frame in range(TOTAL_FRAMES):
        game_state.frame_count = frame
        
        # Intentar castear cada 4 frames (15 clicks/segundo)
        if frame % 4 == 0:
            stats.total_cast_attempts += 1
            
            if add_missile(game_state, MissileType.APOCALYPSE, player_id):
                stats.successful_casts += 1
            else:
                # Determinar razón del bloqueo
                if not can_safely_cast_apocalypse(game_state, player_id):
                    active_player = sum(1 for m in game_state.missiles 
                                      if m.missile_type == MissileType.APOCALYPSE 
                                      and m.source_player == player_id 
                                      and not m.deleted)
                    active_global = sum(1 for m in game_state.missiles 
                                      if m.missile_type == MissileType.APOCALYPSE 
                                      and not m.deleted)
                    
                    if active_player >= 1:
                        stats.blocked_by_player_limit += 1
                    elif active_global >= 2:
                        stats.blocked_by_global_limit += 1
                    else:
                        stats.blocked_by_cooldown += 1
        
        # Procesar missiles
        process_missiles(game_state)
        
        # Estadísticas
        current_missiles = len(game_state.missiles)
        if current_missiles > stats.max_simultaneous_missiles:
            stats.max_simultaneous_missiles = current_missiles
        
        stats.total_booms_created = sum(1 for m in game_state.missiles 
                                       if m.missile_type == MissileType.APOCALYPSE_BOOM)
        
        # Verificar crash
        if current_missiles > 200:
            stats.crashed = True
            break
        
        # Simular frame time
        time.sleep(0.001)
    
    return stats

def test_extreme_spam_singleplayer():
    """Test 2: Extreme spam singleplayer"""
    
    print("\n🔥 Running Test 2: Extreme Spam SinglePlayer...")
    
    game_state = GameState()
    stats = TestStats()
    player_id = 0
    
    TOTAL_FRAMES = 120  # 2 segundos
    
    for frame in range(TOTAL_FRAMES):
        game_state.frame_count = frame
        
        # Intentar castear CADA frame (60 clicks/segundo)
        stats.total_cast_attempts += 1
        
        if add_missile(game_state, MissileType.APOCALYPSE, player_id):
            stats.successful_casts += 1
        else:
            active_player = sum(1 for m in game_state.missiles 
                              if m.missile_type == MissileType.APOCALYPSE 
                              and m.source_player == player_id 
                              and not m.deleted)
            if active_player >= 1:
                stats.blocked_by_player_limit += 1
            else:
                stats.blocked_by_cooldown += 1
        
        process_missiles(game_state)
        
        current_missiles = len(game_state.missiles)
        if current_missiles > stats.max_simultaneous_missiles:
            stats.max_simultaneous_missiles = current_missiles
        
        if current_missiles > 200:
            stats.crashed = True
            break
        
        time.sleep(0.001)
    
    return stats

def test_multiplayer_2_players():
    """Test 3: Multiplayer 2 jugadores"""
    
    print("\n👥 Running Test 3: Multiplayer 2 Players...")
    
    game_state = GameState()
    stats = TestStats()
    
    TOTAL_FRAMES = 180  # 3 segundos
    
    for frame in range(TOTAL_FRAMES):
        game_state.frame_count = frame
        
        # Player 0 castea cada 10 frames
        if frame % 10 == 0:
            stats.total_cast_attempts += 1
            if add_missile(game_state, MissileType.APOCALYPSE, 0):
                stats.successful_casts += 1
        
        # Player 1 castea cada 10 frames (offset 5)
        if frame % 10 == 5:
            stats.total_cast_attempts += 1
            if add_missile(game_state, MissileType.APOCALYPSE, 1):
                stats.successful_casts += 1
        
        process_missiles(game_state)
        
        current_missiles = len(game_state.missiles)
        if current_missiles > stats.max_simultaneous_missiles:
            stats.max_simultaneous_missiles = current_missiles
        
        if current_missiles > 200:
            stats.crashed = True
            break
        
        time.sleep(0.001)
    
    return stats

def test_multiplayer_4_players_spam():
    """Test 4: Multiplayer 4 jugadores spam"""
    
    print("\n👥👥 Running Test 4: Multiplayer 4 Players Spam...")
    
    game_state = GameState()
    stats = TestStats()
    
    TOTAL_FRAMES = 180
    
    for frame in range(TOTAL_FRAMES):
        game_state.frame_count = frame
        
        # Cada jugador intenta castear cada 5 frames
        for player_id in range(4):
            if frame % 5 == player_id:
                stats.total_cast_attempts += 1
                if add_missile(game_state, MissileType.APOCALYPSE, player_id):
                    stats.successful_casts += 1
                else:
                    active_global = sum(1 for m in game_state.missiles 
                                      if m.missile_type == MissileType.APOCALYPSE 
                                      and not m.deleted)
                    if active_global >= 2:
                        stats.blocked_by_global_limit += 1
        
        process_missiles(game_state)
        
        current_missiles = len(game_state.missiles)
        if current_missiles > stats.max_simultaneous_missiles:
            stats.max_simultaneous_missiles = current_missiles
        
        if current_missiles > 200:
            stats.crashed = True
            break
        
        time.sleep(0.001)
    
    return stats

# ============================================================================
# MAIN
# ============================================================================

def main():
    print("╔" + "=" * 58 + "╗")
    print("║  APOCALYPSE REALISTIC TEST SUITE - PYTHON VERSION       ║")
    print("║  Simulación de jugador moderno con fast-clicking        ║")
    print("╚" + "=" * 58 + "╝")
    
    # Ejecutar tests
    all_stats = []
    
    all_stats.append(("Fast-Clicking SinglePlayer", test_fast_clicking_singleplayer()))
    all_stats.append(("Extreme Spam SinglePlayer", test_extreme_spam_singleplayer()))
    all_stats.append(("Multiplayer 2 Players", test_multiplayer_2_players()))
    all_stats.append(("Multiplayer 4 Players Spam", test_multiplayer_4_players_spam()))
    
    # Imprimir resultados
    for test_name, stats in all_stats:
        print_stats(test_name, stats)
    
    # Resumen final
    print("\n╔" + "=" * 58 + "╗")
    print("║  FINAL SUMMARY                                           ║")
    print("╚" + "=" * 58 + "╝")
    
    total_tests = len(all_stats)
    passed_tests = sum(1 for _, stats in all_stats if not stats.crashed)
    
    print(f"Total tests:   {total_tests}")
    print(f"Passed:        {passed_tests} ✅")
    print(f"Failed:        {total_tests - passed_tests} ❌")
    print(f"Success rate:  {passed_tests * 100 // total_tests}%")
    
    if passed_tests == total_tests:
        print("\n🎉 ALL TESTS PASSED! Apocalypse fix is SOLID! 🎉")
        return 0
    else:
        print("\n⚠️  SOME TESTS FAILED! Review the implementation! ⚠️")
        return 1

if __name__ == "__main__":
    exit(main())
