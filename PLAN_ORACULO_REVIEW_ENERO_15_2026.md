# 🔮 REVISIÓN DEL PLAN DEL ORÁCULO

**Fecha**: Enero 15, 2026  
**Estado**: ✅ **PLAN REVISADO Y OPTIMIZADO**

---

## 📋 RESUMEN DE LA REVISIÓN

He revisado el plan original del Oráculo y tengo **buenas noticias**: Ya tenemos **mucha infraestructura lista** que podemos reutilizar.

---

## ✅ LO QUE YA TENEMOS IMPLEMENTADO

### 1. Cliente HTTP Multi-Plataforma ✅
**Archivo**: `Source/ai/ai_text_variation.cpp`

Ya tenemos:
- Cliente HTTP para Windows (WinHTTP)
- Cliente HTTP para Linux/Mac (libcurl)
- Timeout configurable
- Manejo de errores robusto
- JSON builder y parser

**Reutilizable para**: `oracle_ollama.cpp`

### 2. Sistema de Cache ✅
**Archivo**: `Source/ai/ai_text_variation.cpp`

Ya tenemos:
- Cache en memoria (100 entradas)
- Sistema de key-value
- Eviction simple

**Mejora necesaria**: Añadir persistencia a disco

### 3. Integración con Ollama ✅
**Archivo**: `Source/ai/ai_text_variation.cpp`

Ya tenemos:
- Endpoint configurado: `http://localhost:11434/api/generate`
- Modelo: `qwen2.5:3b-instruct`
- Parámetros optimizados (temperature, top_p, etc.)
- Timeout: 8000ms (podemos reducir a 5000ms para el Oráculo)

### 4. Sistema de Fallback ✅
**Archivo**: `Source/ai/ai_text_variation.cpp`

Ya tenemos:
- 6 puntos de fallback
- Nunca bloquea el juego
- Graceful degradation

### 5. Validación Lore-Safe ✅
**Archivo**: `Source/ai/ai_text_variation.cpp`

Ya tenemos:
- Validación de palabras prohibidas
- Validación de longitud
- 100% lore-safe (con fix de pronombres)

---

## 🔧 LO QUE NECESITAMOS IMPLEMENTAR

### 1. Sistema de Preguntas Pendientes (NUEVO)
**Archivo**: `Source/oracle/oracle_system.h/cpp`

```cpp
struct PendingQuestion {
    std::string text;
    std::string context;
    PlayerState state;
    uint32_t timestamp;
};

class OracleSystem {
    static std::optional<PendingQuestion> pendingQuestion;
public:
    static void AddQuestion(const std::string& question);
    static bool HasPendingQuestion();
    static PendingQuestion GetPendingQuestion();
    static void ClearPendingQuestion();
};
```

**Estimación**: 30 minutos

### 2. Detección de Eventos Seguros (NUEVO)
**Archivo**: `Source/oracle/oracle_events.h/cpp`

```cpp
enum class OracleEvent {
    PLAYER_DEATH,
    LEVEL_CLEARED,
    ENTERED_TOWN,
    BOOK_INTERACTION,
    ALTAR_INTERACTION
};

class OracleEvents {
public:
    static void TriggerEvent(OracleEvent event, const std::string& context);
};
```

**Hooks necesarios**:
- `Source/player.cpp` - Muerte del jugador
- `Source/gendung.cpp` - Nivel limpiado
- `Source/towners.cpp` - Interacción con libros/altares

**Estimación**: 1.5 horas

### 3. Prompt Maestro del Oráculo (NUEVO)
**Archivo**: `Source/oracle/oracle_prompt.h`

El prompt ya está bien definido en el plan original. Solo necesitamos:
- Crear el archivo
- Implementar `BuildPrompt()`
- Ajustar parámetros según estado del jugador

**Estimación**: 30 minutos

### 4. Cliente Asíncrono (ADAPTAR EXISTENTE)
**Archivo**: `Source/oracle/oracle_ollama.h/cpp`

**Opción 1**: Reutilizar código de `ai_text_variation.cpp`
- Copiar funciones HTTP
- Añadir wrapper asíncrono con `std::thread`
- Callback para respuesta

**Opción 2**: Refactorizar código existente
- Mover cliente HTTP a módulo compartido
- Usar desde IA y Oráculo

**Recomendación**: Opción 1 (más rápido, menos riesgo)

**Estimación**: 1 hora (vs 2 horas original)

### 5. Cache Persistente (MEJORAR EXISTENTE)
**Archivo**: `Source/oracle/oracle_cache.h/cpp`

Ya tenemos cache en memoria. Solo necesitamos:
- Añadir `SaveToDisk()`
- Añadir `LoadFromDisk()`
- Formato: JSON o binario simple

**Estimación**: 45 minutos (vs 1 hora original)

### 6. Display de Respuestas (NUEVO)
**Archivo**: `Source/oracle/oracle_display.cpp`

Opciones:
1. Usar sistema de mensajes existente (`AddConsoleMessage`)
2. Crear overlay especial con efecto visual

**Recomendación**: Empezar con opción 1, mejorar después

**Estimación**: 30 minutos

### 7. Modificar Tecla V (SIMPLE)
**Archivos**: `Source/init.cpp`, `Source/VERSION`

Cambiar:
- "DevilutionX" → "Nightmare Edition"
- Actualizar versión

**Estimación**: 15 minutos

---

## 📊 ESTIMACIÓN REVISADA

| Fase | Tarea | Original | Revisado | Ahorro |
|------|-------|----------|----------|--------|
| 1 | Modificar Tecla V | 30 min | 15 min | -15 min |
| 2 | Sistema Preguntas | 1h | 30 min | -30 min |
| 3 | Detección Eventos | 1.5h | 1.5h | 0 |
| 4 | Cliente Ollama | 2h | 1h | -1h |
| 5 | Prompt Maestro | 30 min | 30 min | 0 |
| 6 | Sistema Cache | 1h | 45 min | -15 min |
| 7 | Integración Display | 1.5h | 30 min | -1h |
| 8 | Testing | 2h | 1.5h | -30 min |
| **TOTAL** | | **10h** | **6.5h** | **-3.5h** |

**Ahorro**: 35% de tiempo gracias a infraestructura existente

---

## 🎯 MEJORAS SUGERIDAS AL PLAN

### 1. Reutilizar Código Existente

**En lugar de crear desde cero**:
```cpp
// oracle_ollama.cpp
#include "ai/ai_text_variation.h"

// Reutilizar funciones HTTP existentes
std::optional<std::string> CallOllamaSync(const std::string& prompt) {
    // Usar CallOllama() de ai_text_variation.cpp
    // Wrapper simple
}
```

**Beneficios**:
- Menos código duplicado
- Misma lógica de timeout
- Mismo manejo de errores
- Ya testeado y funcional

### 2. Compartir Configuración

**Archivo único**: `Source/ai/ai_config.h`

```cpp
struct AIConfig {
    std::string ollamaURL = "http://localhost:11434";
    std::string model = "qwen2.5:3b-instruct";
    int timeoutMs = 8000;
    // ...
};

// Usado por:
// - ai_text_variation.cpp (micro-variaciones)
// - oracle_system.cpp (respuestas crípticas)
```

**Beneficios**:
- Configuración centralizada
- Fácil de modificar
- Consistencia garantizada

### 3. Sistema de Eventos Unificado

**En lugar de hooks dispersos**:
```cpp
// Source/events/game_events.h
enum class GameEvent {
    PLAYER_DEATH,
    LEVEL_CLEARED,
    ENTERED_TOWN,
    BOOK_INTERACTION,
    // ...
};

class GameEvents {
public:
    static void Trigger(GameEvent event, const std::string& context);
    static void Subscribe(GameEvent event, std::function<void(std::string)> callback);
};

// Uso:
GameEvents::Subscribe(GameEvent::PLAYER_DEATH, [](std::string context) {
    OracleSystem::OnPlayerDeath(context);
});
```

**Beneficios**:
- Más limpio
- Más extensible
- Fácil de testear

### 4. Prompt Optimizado para qwen2.5

**Basado en nuestros tests** (100% lore-safe):

```cpp
constexpr const char* ORACLE_PROMPT = R"(
You are an ancient entity from Diablo's world.
Speak cryptically about the player's question.
Use dark, poetic language.
Maximum 4 lines.
No modern terms, no tutorials, no mechanics.

Question: {QUESTION}
Event: {EVENT}
State: {STATE}

Respond as the Inferno itself:
)";
```

**Más corto** = Menos bloqueo cognitivo en CPU débil

### 5. Cache Inteligente

**Indexar por**:
- Pregunta (normalizada)
- Evento
- Estado del jugador

```cpp
std::string cacheKey = StrCat(
    NormalizeQuestion(question),
    "|", EventToString(event),
    "|", StateToString(state)
);
```

**Beneficios**:
- Más hits de cache
- Menos llamadas a Ollama
- Mejor performance

---

## 🚨 PUNTOS CRÍTICOS A CONSIDERAR

### 1. Thread Safety ⚠️

**Problema**: Cliente asíncrono usa threads

**Solución**:
```cpp
// Usar mutex para acceso a pendingQuestion
std::mutex g_oracleMutex;

void OracleSystem::AddQuestion(const std::string& question) {
    std::lock_guard<std::mutex> lock(g_oracleMutex);
    pendingQuestion = {question, ...};
}
```

### 2. Latencia Perceptible ⚠️

**Problema**: 3-5s de latencia puede ser notable

**Solución**:
- Mostrar indicador visual: "🔮 El Oráculo medita..."
- Timeout más corto: 5s (vs 8s actual)
- Cache agresivo

### 3. Ollama No Disponible ⚠️

**Problema**: Usuario no tiene Ollama instalado

**Solución** (ya en el plan):
```cpp
if (!OllamaClient::IsAvailable()) {
    // Silenciosamente no hacer nada
    // Juego funciona 100% normal
    return;
}
```

### 4. Preguntas Inapropiadas ⚠️

**Problema**: Usuario pregunta cosas fuera de contexto

**Solución**:
```cpp
bool IsQuestionValid(const std::string& question) {
    // Filtrar preguntas muy cortas
    if (question.length() < 5) return false;
    
    // Filtrar spam
    if (HasRepeatedChars(question)) return false;
    
    // Filtrar palabras prohibidas
    if (HasBannedWords(question)) return false;
    
    return true;
}
```

---

## 🎮 FLUJO OPTIMIZADO

### Versión Original (Plan)
```
Pregunta → Guardar → Evento → Thread → Ollama → Callback → Display
```

### Versión Optimizada (Propuesta)
```
Pregunta → Validar → Guardar → Evento → Cache? → Ollama → Display
                                           ↓
                                        Respuesta inmediata
```

**Mejoras**:
1. Validación temprana (evita llamadas inútiles)
2. Cache check primero (70% de hits esperado)
3. Reutilizar código existente (menos bugs)

---

## 📝 ORDEN DE IMPLEMENTACIÓN OPTIMIZADO

### Día 1 (3h)
1. ✅ Modificar Tecla V (15 min)
2. ✅ Sistema Preguntas Pendientes (30 min)
3. ✅ Prompt Maestro (30 min)
4. ✅ Detección Eventos Seguros (1.5h)
5. ✅ Compilar y verificar (15 min)

### Día 2 (3.5h)
1. ✅ Cliente Ollama (reutilizar código) (1h)
2. ✅ Cache Persistente (45 min)
3. ✅ Display de Respuestas (30 min)
4. ✅ Integración Completa (45 min)
5. ✅ Testing (30 min)

**Total**: 6.5 horas (vs 10 horas original)

---

## ✅ RECOMENDACIONES FINALES

### 1. Empezar Simple
- Usar sistema de mensajes existente para display
- Cache en memoria primero, persistencia después
- Reutilizar código HTTP existente

### 2. Testear Incremental
- Cada fase debe compilar y funcionar
- No esperar al final para testear
- Verificar graceful degradation en cada paso

### 3. Documentar Mientras Implementas
- Comentar código con ejemplos
- Crear tests simples
- Documentar decisiones de diseño

### 4. Considerar Futuras Mejoras
- Overlay visual especial (Fase 2)
- Múltiples voces del Oráculo (Fase 3)
- Respuestas contextuales más ricas (Fase 4)

---

## 🎯 CONCLUSIÓN

El plan original es **excelente y muy completo**. Con la infraestructura que ya tenemos, podemos:

1. **Reducir tiempo de implementación** de 10h a 6.5h
2. **Reutilizar código testeado** (cliente HTTP, cache, validación)
3. **Mantener consistencia** (misma configuración, mismo modelo)
4. **Reducir bugs** (menos código nuevo = menos errores)

**Recomendación**: Proceder con implementación usando el plan optimizado.

---

## 📚 ARCHIVOS A CREAR

### Nuevos (Mínimos)
1. `Source/oracle/oracle_system.h` - Sistema principal
2. `Source/oracle/oracle_system.cpp` - Implementación
3. `Source/oracle/oracle_events.h` - Eventos
4. `Source/oracle/oracle_events.cpp` - Detección
5. `Source/oracle/oracle_prompt.h` - Prompt maestro

### Reutilizar/Adaptar
6. `Source/ai/ai_text_variation.cpp` - Cliente HTTP (copiar funciones)
7. `Source/ai/ai_text_variation.cpp` - Cache (añadir persistencia)

### Modificar
8. `Source/control/control_chat.cpp` - Captura preguntas
9. `Source/player.cpp` - Hook muerte
10. `Source/gendung.cpp` - Hook nivel limpiado
11. `Source/towners.cpp` - Hook libros/altares
12. `Source/init.cpp` - Cambiar "DevilutionX"
13. `VERSION` - Actualizar versión

**Total**: 5 archivos nuevos, 8 modificados

---

**Autor**: Kiro AI Assistant  
**Fecha**: Enero 15, 2026  
**Versión**: Plan Revisado y Optimizado

---

*"El plan es sólido. La infraestructura está lista."*  
*"El Oráculo aguarda su implementación."* 🔮✨

---

¿Quieres que empiece con la implementación? 🚀
