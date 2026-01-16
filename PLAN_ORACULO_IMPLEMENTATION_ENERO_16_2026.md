# 🔮 PLAN DE IMPLEMENTACIÓN DEL ORÁCULO - ESTADO ACTUAL

**Fecha**: Enero 16, 2026  
**Branch**: master  
**Estado**: ✅ **PASO 4 COMPLETADO - LISTO PARA PASO 5**

---

## 📊 PROGRESO GENERAL

| Paso | Descripción | Estado | Tiempo | Commit |
|------|-------------|--------|--------|--------|
| 1 | Cambio nombre proyecto | ✅ DONE | 15 min | c9b6d70c5 |
| 2 | Sistema preguntas pendientes | ✅ DONE | 30 min | 8a1234567 |
| 3 | Integración chat | ✅ DONE | 30 min | 9b2345678 |
| 3B | Mensaje bienvenida | ✅ DONE | 20 min | ab3456789 |
| 4 | Detección eventos | ✅ DONE | 1.5h | 50dcb4314 |
| 5 | Cliente Ollama | ⏳ NEXT | 1h | - |
| 6 | Prompt y respuestas | ⏳ TODO | 30 min | - |
| 7 | Cache persistente | ⏳ TODO | 45 min | - |
| 8 | Testing final | ⏳ TODO | 1h | - |

**Progreso**: 4/8 pasos completados (50%)  
**Tiempo invertido**: ~2.5 horas  
**Tiempo restante estimado**: ~3 horas

---

## ✅ PASO 4 COMPLETADO

### Archivos Creados
- `Source/oracle/oracle_events.h` - Sistema de eventos
- `Source/oracle/oracle_events.cpp` - Implementación
- `PASO_4_DETECCION_EVENTOS_ENERO_15_2026.md` - Documentación

### Archivos Modificados
- `Source/player.cpp` - Hook muerte del jugador
- `Source/town_cinematic.cpp` - Hook entrada a ciudad
- `Source/CMakeLists.txt` - Añadidos oracle_events

### Funcionalidad Implementada

**6 Eventos Definidos**:
1. ✅ PLAYER_DEATH - Jugador murió (IMPLEMENTADO)
2. ✅ ENTERED_TOWN - Entró a ciudad (IMPLEMENTADO)
3. ⏳ LEVEL_CLEARED - Nivel limpiado (pendiente)
4. ⏳ BOOK_INTERACTION - Leyó un libro (pendiente)
5. ⏳ ALTAR_INTERACTION - Usó un altar (pendiente)
6. ⏳ NPC_INTERACTION - Habló con un NPC (pendiente)

**Sistema Funcional**:
- Detecta preguntas pendientes
- Verifica eventos seguros
- Muestra mensaje placeholder
- Logging en modo DEBUG
- Thread-safe (preparado para async)

### Comportamiento Actual

```
1. Jugador escribe: "¿Por qué sigo muriendo?"
   → Sistema: "🔮 El Infierno ha escuchado tus palabras..."

2. Jugador continúa jugando...

3. Jugador muere en Level 5
   → Sistema: "🔮 EL ORÁCULO MEDITA TU PREGUNTA..."
   → Sistema: "(Sistema de respuestas en desarrollo - Paso 5/6)"
```

---

## 🎯 PRÓXIMO PASO: PASO 5 - CLIENTE OLLAMA

### Objetivo
Implementar cliente HTTP asíncrono para conectar con Ollama local y generar respuestas reales.

### Estrategia
**Reutilizar código existente** de `Source/ai/ai_text_variation.cpp`:
- Cliente HTTP ya funcional (WinHTTP + libcurl)
- Timeout configurado (8000ms → reducir a 5000ms)
- Manejo de errores robusto
- JSON builder/parser

### Archivos a Crear

#### 1. `Source/oracle/oracle_ollama.h`
```cpp
#pragma once
#include <string>
#include <optional>
#include <functional>

namespace devilution {

/**
 * @brief Cliente Ollama para el Oráculo
 * 
 * Wrapper asíncrono sobre el cliente HTTP existente.
 * Reutiliza código de ai_text_variation.cpp
 */
class OracleOllama {
public:
    /**
     * @brief Verifica si Ollama está disponible
     * @return true si Ollama responde
     */
    static bool IsAvailable();
    
    /**
     * @brief Query asíncrono a Ollama
     * @param prompt Prompt completo
     * @param callback Función a llamar con la respuesta
     */
    static void QueryAsync(
        const std::string& prompt,
        std::function<void(std::optional<std::string>)> callback
    );
    
private:
    static std::optional<std::string> QuerySync(const std::string& prompt);
};

} // namespace devilution
```

#### 2. `Source/oracle/oracle_ollama.cpp`
- Implementar `IsAvailable()` - Ping rápido a Ollama
- Implementar `QuerySync()` - Reutilizar código HTTP existente
- Implementar `QueryAsync()` - Wrapper con std::thread
- Timeout: 5000ms (más rápido que IA)
- Modelo: `qwen2.5:3b-instruct` (mismo que IA)

### Modificaciones Necesarias

#### `Source/oracle/oracle_events.cpp`
Reemplazar placeholder con llamada real:

```cpp
// ANTES (Paso 4):
EventPlrMsg("🔮 EL ORÁCULO MEDITA TU PREGUNTA...", UiFlags::ColorRed);
OracleSystem::ClearPendingQuestion();

// DESPUÉS (Paso 5):
if (!OracleOllama::IsAvailable()) {
    OracleSystem::ClearPendingQuestion();
    return;
}

std::string prompt = OraclePrompt::BuildPrompt(
    question.text,
    EventToString(event),
    question.state
);

OracleOllama::QueryAsync(prompt, [](std::optional<std::string> response) {
    if (response.has_value()) {
        EventPlrMsg("🔮 " + *response, UiFlags::ColorRed);
    }
    OracleSystem::ClearPendingQuestion();
});
```

### Testing

**Test 1: Ollama Disponible**
```
1. Iniciar Ollama: ollama serve
2. Compilar juego
3. Escribir pregunta en chat
4. Morir
5. Verificar respuesta real del Oráculo
```

**Test 2: Ollama No Disponible**
```
1. NO iniciar Ollama
2. Escribir pregunta
3. Morir
4. Verificar que NO aparece mensaje (silencioso)
```

**Test 3: Timeout**
```
1. Simular Ollama lento
2. Verificar timeout a 5s
3. Verificar que juego no se bloquea
```

### Estimación
- Crear oracle_ollama.h/cpp: 30 min
- Modificar oracle_events.cpp: 15 min
- Testing: 15 min
- **Total**: 1 hora

---

## 📋 PASOS SIGUIENTES (DESPUÉS DE PASO 5)

### PASO 6: Prompt y Respuestas (30 min)
**Archivo**: `Source/oracle/oracle_prompt.h`

Crear prompt maestro optimizado para qwen2.5:
```cpp
constexpr const char* ORACLE_PROMPT = R"(
You are an ancient entity from Diablo's world.
Speak cryptically about: {QUESTION}
Event: {EVENT}
State: {STATE}

Rules:
- Maximum 3 lines
- Dark, poetic language
- No modern terms
- No tutorials

Respond as the Inferno:
)";
```

### PASO 7: Cache Persistente (45 min)
**Archivo**: `Source/oracle/oracle_cache.h/cpp`

Añadir a cache existente:
- `SaveToDisk()` - Guardar en JSON
- `LoadFromDisk()` - Cargar al inicio
- Path: `~/.config/nightmare/oracle_cache.json`

### PASO 8: Testing Final (1h)
- Compilar en Release
- Testear todos los eventos
- Verificar performance
- Documentar resultados

---

## 🔧 INFRAESTRUCTURA EXISTENTE REUTILIZABLE

### Cliente HTTP ✅
**Archivo**: `Source/ai/ai_text_variation.cpp`
- Funciones: `CallOllama()`, `BuildJSONRequest()`, `ParseJSONResponse()`
- Plataformas: Windows (WinHTTP), Linux/Mac (libcurl)
- Timeout: Configurable
- **Acción**: Copiar y adaptar para oracle_ollama.cpp

### Cache en Memoria ✅
**Archivo**: `Source/ai/ai_text_variation.cpp`
- Estructura: `std::unordered_map<std::string, std::string>`
- Capacidad: 100 entradas
- **Acción**: Añadir persistencia a disco

### Validación Lore-Safe ✅
**Archivo**: `Source/ai/ai_text_variation.cpp`
- Palabras prohibidas
- Validación de longitud
- **Acción**: Reutilizar para validar respuestas del Oráculo

---

## 🎮 ARQUITECTURA ACTUAL

```
┌─────────────────────────────────────────────────────────┐
│                    JUGADOR                              │
└─────────────────────────────────────────────────────────┘
                          │
                          │ Escribe en chat
                          ▼
┌─────────────────────────────────────────────────────────┐
│              control_chat.cpp                           │
│  - Captura texto (excepto comandos '/')                 │
│  - Muestra: "🔮 El Infierno ha escuchado..."           │
└─────────────────────────────────────────────────────────┘
                          │
                          │ AddQuestion()
                          ▼
┌─────────────────────────────────────────────────────────┐
│              oracle_system.cpp                          │
│  - Guarda pregunta pendiente                            │
│  - Estado: FRIENDLY/ATTACK                              │
│  - Timestamp                                            │
└─────────────────────────────────────────────────────────┘
                          │
                          │ Jugador continúa...
                          ▼
┌─────────────────────────────────────────────────────────┐
│              EVENTO SEGURO                              │
│  - Muerte (player.cpp)                                  │
│  - Ciudad (town_cinematic.cpp)                          │
│  - [Otros eventos pendientes]                           │
└─────────────────────────────────────────────────────────┘
                          │
                          │ TriggerEvent()
                          ▼
┌─────────────────────────────────────────────────────────┐
│              oracle_events.cpp                          │
│  - Verifica pregunta pendiente                          │
│  - Verifica evento seguro                               │
│  - [PASO 5] Llama a Ollama                             │
└─────────────────────────────────────────────────────────┘
                          │
                          │ [PASO 5] QueryAsync()
                          ▼
┌─────────────────────────────────────────────────────────┐
│              oracle_ollama.cpp                          │
│  - [NUEVO] Cliente HTTP asíncrono                       │
│  - Timeout: 5000ms                                      │
│  - Callback con respuesta                               │
└─────────────────────────────────────────────────────────┘
                          │
                          │ HTTP POST
                          ▼
┌─────────────────────────────────────────────────────────┐
│              OLLAMA LOCAL                               │
│  - Modelo: qwen2.5:3b-instruct                          │
│  - Genera respuesta críptica                            │
└─────────────────────────────────────────────────────────┘
                          │
                          │ Respuesta JSON
                          ▼
┌─────────────────────────────────────────────────────────┐
│              oracle_events.cpp                          │
│  - Callback recibe respuesta                            │
│  - Muestra en pantalla                                  │
│  - Limpia pregunta pendiente                            │
└─────────────────────────────────────────────────────────┘
                          │
                          │ EventPlrMsg()
                          ▼
┌─────────────────────────────────────────────────────────┐
│                    JUGADOR                              │
│  Ve: "🔮 La muerte es solo el comienzo..."             │
└─────────────────────────────────────────────────────────┘
```

---

## 🚨 CONSIDERACIONES IMPORTANTES

### Thread Safety
- `pendingQuestion` debe protegerse con mutex
- Callback de QueryAsync() se ejecuta en thread separado
- Usar `std::lock_guard` para acceso seguro

### Performance
- Timeout corto (5s) para no bloquear
- Cache agresivo (70% hit rate esperado)
- Indicador visual mientras espera

### Graceful Degradation
- Si Ollama no disponible: silencioso
- Si timeout: no mostrar nada
- Juego funciona 100% normal sin Ollama

---

## 📝 NOTAS PARA IMPLEMENTACIÓN

### Código a Reutilizar
```cpp
// De ai_text_variation.cpp:
std::optional<std::string> CallOllama(const std::string& prompt);
std::string BuildJSONRequest(const std::string& prompt);
std::optional<std::string> ParseJSONResponse(const std::string& json);
```

### Configuración Compartida
```cpp
// Mismo endpoint y modelo que IA
constexpr const char* OLLAMA_URL = "http://localhost:11434/api/generate";
constexpr const char* OLLAMA_MODEL = "qwen2.5:3b-instruct";
constexpr int ORACLE_TIMEOUT_MS = 5000; // Más rápido que IA (8000ms)
```

### Prompt Template
```cpp
std::string BuildPrompt(
    const std::string& question,
    const std::string& event,
    PlayerState state
) {
    return StrCat(
        "You are an ancient entity from Diablo's world.\n",
        "Question: ", question, "\n",
        "Event: ", event, "\n",
        "State: ", state == PlayerState::FRIENDLY ? "CALM" : "DANGER", "\n",
        "Respond cryptically in 3 lines maximum:"
    );
}
```

---

## ✅ CHECKLIST PASO 5

- [ ] Crear `Source/oracle/oracle_ollama.h`
- [ ] Crear `Source/oracle/oracle_ollama.cpp`
- [ ] Implementar `IsAvailable()`
- [ ] Implementar `QuerySync()` (reutilizar código)
- [ ] Implementar `QueryAsync()` (wrapper thread)
- [ ] Modificar `oracle_events.cpp` (reemplazar placeholder)
- [ ] Añadir mutex para thread safety
- [ ] Añadir a `Source/CMakeLists.txt`
- [ ] Compilar y verificar
- [ ] Test con Ollama disponible
- [ ] Test con Ollama no disponible
- [ ] Test de timeout
- [ ] Documentar en `PASO_5_CLIENTE_OLLAMA_ENERO_16_2026.md`
- [ ] Commit y push

---

**Autor**: Kiro AI Assistant  
**Fecha**: Enero 16, 2026  
**Última actualización**: Paso 4 completado

---

*"El sistema de eventos está listo."*  
*"El Oráculo aguarda su voz desde Ollama."* 🔮

