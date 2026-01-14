#pragma once

/**
 * @file safety_iter.h
 * 
 * SAFETY LAYER - Phase 1: Safe Iteration Patterns
 * 
 * PRINCIPIO RECTOR: No mutar durante iteración.
 * 
 * Este header proporciona patrones seguros para iteración sin invalidación.
 * - Previene undefined behavior durante iteración
 * - Defer pattern estilo Diablo para mutaciones
 * - Macros conceptuales para claridad de código
 * - Compatible con containers STL y arrays C
 */

#include <vector>
#include <functional>
#include <cstddef>

namespace devilution {

// ============================================================================
// 🧱 1.3 PATRONES DE ITERACIÓN SEGURA
// ============================================================================

/**
 * Estructura para manejar acciones diferidas durante iteración
 * Evita invalidación de iteradores al diferir mutaciones
 */
struct DeferredActions {
    std::vector<std::function<void()>> actions;
    
    void Add(std::function<void()> action) {
        actions.emplace_back(std::move(action));
    }
    
    void ExecuteAll() {
        for (auto& action : actions) {
            action();
        }
        actions.clear();
    }
    
    void Clear() {
        actions.clear();
    }
    
    bool HasActions() const {
        return !actions.empty();
    }
};

// ============================================================================
// 🧱 MACROS PARA ITERACIÓN SEGURA ESTILO DIABLO
// ============================================================================

/**
 * Inicia un bloque de iteración segura
 * Uso: BEGIN_SAFE_ITER() al inicio del loop
 */
#define BEGIN_SAFE_ITER() \
    DeferredActions __deferred_actions; \
    bool __mutation_requested = false

/**
 * Marca que se requiere una mutación (defer hasta el final)
 * Uso: DEFER_ACTION() cuando necesites mutar durante iteración
 */
#define DEFER_ACTION() \
    __mutation_requested = true

/**
 * Agrega una acción específica para ejecutar al final
 * Uso: DEFER_SPECIFIC_ACTION(lambda o función)
 */
#define DEFER_SPECIFIC_ACTION(action) \
    __deferred_actions.Add(action)

/**
 * Finaliza el bloque de iteración segura y ejecuta acciones diferidas
 * Uso: END_SAFE_ITER(acción_a_ejecutar) al final del loop
 */
#define END_SAFE_ITER(action) \
    do { \
        if (__mutation_requested) { \
            action; \
        } \
        __deferred_actions.ExecuteAll(); \
    } while(0)

/**
 * Versión simplificada sin acción específica
 */
#define END_SAFE_ITER_SIMPLE() \
    __deferred_actions.ExecuteAll()

// ============================================================================
// 🧱 TEMPLATES PARA ITERACIÓN SEGURA
// ============================================================================

/**
 * Itera sobre un container de forma segura, permitiendo mutaciones diferidas
 * 
 * @param container El container a iterar
 * @param processor Función que procesa cada elemento
 * @param mutator Función opcional que se ejecuta al final si hay mutaciones
 */
template<typename Container, typename Processor, typename Mutator = std::function<void()>>
inline void SafeIterate(Container& container, Processor&& processor, Mutator&& mutator = [](){}) {
    DeferredActions deferred;
    bool needsMutation = false;
    
    for (auto it = container.begin(); it != container.end(); ++it) {
        auto result = processor(*it, deferred, needsMutation);
        
        // Si el processor retorna false, detener iteración
        if constexpr (std::is_same_v<decltype(result), bool>) {
            if (!result) break;
        }
    }
    
    // Ejecutar mutaciones diferidas
    if (needsMutation) {
        mutator();
    }
    deferred.ExecuteAll();
}

/**
 * Itera sobre un array C de forma segura
 * 
 * @param array Puntero al array
 * @param size Tamaño del array
 * @param processor Función que procesa cada elemento
 */
template<typename T, typename Processor>
inline void SafeIterateArray(T* array, size_t size, Processor&& processor) {
    DeferredActions deferred;
    bool needsMutation = false;
    
    for (size_t i = 0; i < size; ++i) {
        processor(array[i], i, deferred, needsMutation);
    }
    
    deferred.ExecuteAll();
}

// ============================================================================
// 🧱 PATRONES ESPECÍFICOS PARA DIABLO
// ============================================================================

/**
 * Patrón seguro para iterar monsters con posible eliminación
 */
#define SAFE_MONSTER_ITER_BEGIN() \
    BEGIN_SAFE_ITER(); \
    std::vector<size_t> __monsters_to_remove

#define SAFE_MONSTER_MARK_FOR_REMOVAL(index) \
    __monsters_to_remove.push_back(index)

#define SAFE_MONSTER_ITER_END() \
    do { \
        /* Remover monsters marcados (en orden inverso para mantener índices válidos) */ \
        for (auto it = __monsters_to_remove.rbegin(); it != __monsters_to_remove.rend(); ++it) { \
            /* Aquí iría la lógica específica de remoción de monsters */ \
        } \
        END_SAFE_ITER_SIMPLE(); \
    } while(0)

/**
 * Patrón seguro para iterar missiles con posible eliminación
 */
#define SAFE_MISSILE_ITER_BEGIN() \
    BEGIN_SAFE_ITER(); \
    std::vector<std::list<Missile>::iterator> __missiles_to_remove

#define SAFE_MISSILE_MARK_FOR_REMOVAL(iterator) \
    __missiles_to_remove.push_back(iterator)

#define SAFE_MISSILE_ITER_END(missile_list) \
    do { \
        /* Remover missiles marcados */ \
        for (auto it : __missiles_to_remove) { \
            missile_list.erase(it); \
        } \
        END_SAFE_ITER_SIMPLE(); \
    } while(0)

// ============================================================================
// 🧱 HELPERS PARA DEBUGGING
// ============================================================================

#ifdef _DEBUG
/**
 * Versión con logging para debugging
 */
#define BEGIN_SAFE_ITER_DEBUG(name) \
    BEGIN_SAFE_ITER(); \
    const char* __iter_name = name

#define DEFER_ACTION_DEBUG(msg) \
    do { \
        DEFER_ACTION(); \
        /* En debug podríamos loggear la acción diferida */ \
    } while(0)

#define END_SAFE_ITER_DEBUG(action) \
    do { \
        if (__mutation_requested) { \
            /* Log de mutación en debug */ \
            action; \
        } \
        __deferred_actions.ExecuteAll(); \
    } while(0)

#else
// En release, las versiones debug son idénticas a las normales
#define BEGIN_SAFE_ITER_DEBUG(name) BEGIN_SAFE_ITER()
#define DEFER_ACTION_DEBUG(msg) DEFER_ACTION()
#define END_SAFE_ITER_DEBUG(action) END_SAFE_ITER(action)
#endif

// ============================================================================
// 🧱 VALIDADORES DE ITERACIÓN
// ============================================================================

/**
 * Verifica si es seguro iterar sobre un container
 */
template<typename Container>
inline bool IsSafeToIterate(const Container& container) {
    // Verificaciones básicas de sanidad
    return !container.empty();
}

/**
 * Verifica si es seguro mutar un container durante iteración
 * (spoiler: nunca es seguro, por eso usamos defer)
 */
template<typename Container>
inline bool IsSafeToMutate(const Container& container) {
    // Siempre retorna false - las mutaciones deben ser diferidas
    return false;
}

} // namespace devilution