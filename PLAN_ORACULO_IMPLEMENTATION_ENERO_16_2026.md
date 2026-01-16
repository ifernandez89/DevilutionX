# 🔮 PLAN DE IMPLEMENTACIÓN: "EL ORÁCULO"
**Nightmare Edition - Sistema de IA Narrativa Asíncrona**

---

## 📋 RESUMEN EJECUTIVO

Sistema de IA narrativa que responde preguntas del jugador de forma **críptica y atmosférica**, usando Ollama local, sin bloquear gameplay, activándose solo en eventos seguros del juego.

**Filosofía**: El Infierno responde cuando quiere, no cuando el jugador pregunta.

---

## 🎯 OBJETIVOS

### 1. Modificar Tecla V (Versión)
- ✅ Mostrar: Solo versión del juego "Nightmare Edition v1.0.0"
- ✅ Cambiar: "DevilutionX" → "Nightmare Edition" en todos los textos
- ✅ Mantener: Funciones de chat activas por defecto (ya implementado)
- ⚠️ **IMPORTANTE**: La tecla V NO activa/desactiva el Oráculo

### 2. Implementar "El Oráculo"
- Sistema **SIEMPRE ACTIVO** corriendo en segundo plano
- Usuario deja preguntas en el chat normal (con "?" al inicio)
- El Oráculo responde automáticamente en eventos seguros
- **NO requiere activación manual**

---

## 🏗️ ARQUITECTURA DEL SISTEMA

### Componentes Principales

```
┌─────────────────────────────────────────────────────┐
│                  GAMEPLAY LOOP                      │
│              (nunca se bloquea)                     │
└──────────────┬──────────────────────────────────────┘
               │
               ├─► Pregunta guardada localmente
               │   (sin red, sin IA)
               │
               ├─► Evento seguro detectado
               │   (muerte, ciudad, libro, etc.)
               │
               ├─► Thread secundario → Ollama
               │   (consulta asíncrona)
               │
               └─► Respuesta → Cache → Lectura
                   (texto decorativo)
```

### Flujo de Datos

1. **Fase Silenciosa** (durante gameplay)
   - Jugador escribe pregunta en chat
   - Se guarda en `pending_question` (local)
   - NO se envía a Ollama
   - Gameplay continúa normal

2. **Fase de Activación** (evento seguro)
   - Detecta: muerte, limpieza nivel, entrada ciudad, libro/altar
   - Verifica: ¿hay pregunta pendiente? ¿Ollama disponible?
   - Dispara consulta asíncrona

3. **Fase de Consulta** (thread separado)
   - POST a `http://localhost:11434/api/generate`
   - Modelo: `qwen2.5:3b-instruct`
   - Timeout: 5 segundos
   - Si falla: descarta, continúa normal

4. **Fase de Lectura** (cache persistente)
   - Respuesta guardada en cache
   - Asociada a evento/NPC/situación
   - Reutilizable (no vuelve a llamar IA)

---

## 📁 ESTRUCTURA DE ARCHIVOS

### Nuevos Archivos

```
Source/oracle/
├── oracle_system.h          # Sistema principal del Oráculo
├── oracle_system.cpp        # Implementación
├── oracle_prompt.h          # Prompt maestro y configuración
├── oracle_events.h          # Triggers de eventos
├── oracle_events.cpp        # Detección de eventos seguros
├── oracle_ollama.h          # Cliente HTTP para Ollama
├── oracle_ollama.cpp        # Comunicación asíncrona
└── oracle_cache.h           # Sistema de cache persistente
└── oracle_cache.cpp         # Almacenamiento de respuestas
```

### Archivos a Modificar

```
Source/control/control_chat.cpp     # Captura de preguntas
Source/diablo.cpp                    # Detección eventos (muerte, nivel)
Source/player.cpp                    # Hook muerte del jugador
Source/gendung.cpp                   # Hook limpieza de nivel
Source/towners.cpp                   # Hook interacción NPCs/libros
Source/init.cpp                      # Cambiar "DevilutionX" → "Nightmare"
Source/VERSION                       # Actualizar versión
```

---

## 🔧 IMPLEMENTACIÓN DETALLADA

### FASE 1: Modificar Textos de Versión (30 min)

#### 1.1 Cambiar "DevilutionX" → "Nightmare Edition"
```cpp
// Buscar en Source/ donde aparece "DevilutionX"
// Cambiar TODOS los textos visibles al usuario:
// - "DevilutionX" → "Nightmare Edition"
// - Mantener referencias internas (variables, funciones)
// - Actualizar mensajes de versión
```

**Archivos a modificar**:
- `Source/init.cpp` (línea 55-70) - Versión MPQ
- `Source/lua/lua_global.cpp` (línea 276) - Variable Lua
- `Source/DiabloUI/support_lines.cpp` (línea 12) - Texto de soporte
- `VERSION` - Archivo de versión
- Cualquier otro texto visible al usuario

#### 1.2 Verificar Chat Activo
```cpp
// Ya implementado en control_chat.cpp línea 309:
bool IsChatAvailable()
{
    return true; // ✅ Ya activo por defecto en Nightmare Edition
}
```

**NOTA**: La tecla V solo muestra versión, NO controla el Oráculo

---

### FASE 2: Sistema de Preguntas Pendientes (1h)

#### 2.1 Estructura de Datos
```cpp
// oracle_system.h
struct PendingQuestion {
    std::string text;           // Pregunta del jugador
    std::string context;        // "Dungeon Level 5", "Tristram", etc.
    PlayerState state;          // FRIENDLY o ATTACK
    uint32_t timestamp;         // Cuándo se hizo
    bool processed;             // Ya fue respondida?
};

enum class PlayerState {
    FRIENDLY,  // Jugador en buena situación
    ATTACK     // Jugador murió, bajo HP, etc.
};

class OracleSystem {
public:
    static void AddQuestion(const std::string& question);
    static bool HasPendingQuestion();
    static PendingQuestion GetPendingQuestion();
    static void ClearPendingQuestion();
    
private:
    static std::optional<PendingQuestion> pendingQuestion;
};
```

#### 2.2 Captura desde Chat
```cpp
// Modificar Source/control/control_chat.cpp
// En función ResetChatMessage() línea 33

void ResetChatMessage()
{
    // Si el mensaje empieza con "?" es una pregunta para el Oráculo
    if (TalkMessage[0] == '?') {
        std::string question = TalkMessage + 1; // Skip '?'
        
        // Guardar pregunta pendiente (NO enviar a Ollama aún)
        OracleSystem::AddQuestion(question);
        
        // Feedback visual al jugador
        AddConsoleMessage("🔮 Tu pregunta ha sido escuchada por el Infierno...");
        AddConsoleMessage("    El Oráculo responderá cuando lo considere oportuno.");
        
        return; // No enviar como chat normal
    }
    
    // Verificar comandos existentes
    if (CheckChatCommand(TalkMessage))
        return;
        
    // Chat normal (si estuviera en multiplayer)
    // ...
}
```

**Flujo del usuario**:
1. Jugador presiona Enter (abre chat)
2. Escribe: `?¿Por qué sigo muriendo aquí?`
3. Presiona Enter
4. Ve mensaje: "Tu pregunta ha sido escuchada..."
5. **Continúa jugando normalmente**
6. Cuando muere/entra a ciudad/etc → El Oráculo responde

---

### FASE 3: Detección de Eventos Seguros (1.5h)

#### 3.1 Enum de Eventos
```cpp
// oracle_events.h
enum class OracleEvent {
    PLAYER_DEATH,       // ☠️ Jugador murió
    LEVEL_CLEARED,      // 🧹 Nivel limpiado
    ENTERED_TOWN,       // 🏘️ Entró a ciudad
    BOOK_INTERACTION,   // 📖 Leyó libro
    ALTAR_INTERACTION,  // 🕯️ Usó altar
    DUNGEON_EXIT        // 🚪 Salió de dungeon
};

class OracleEvents {
public:
    static void TriggerEvent(OracleEvent event, const std::string& context);
    static bool IsEventSafe(OracleEvent event);
};
```

#### 3.2 Hooks en el Juego
```cpp
// Source/player.cpp - En función de muerte
void StartPlayerKill(Player &player)
{
    // ... código existente ...
    
    // ORACLE: Trigger muerte
    if (&player == MyPlayer) {
        OracleEvents::TriggerEvent(
            OracleEvent::PLAYER_DEATH,
            StrCat("Level ", setlevel ? "Quest" : std::to_string(currlevel))
        );
    }
}

// Source/gendung.cpp - Cuando se limpia un nivel
void CheckLevelCleared()
{
    if (AllMonstersKilled()) {
        OracleEvents::TriggerEvent(
            OracleEvent::LEVEL_CLEARED,
            StrCat("Level ", currlevel, " cleared")
        );
    }
}

// Source/towners.cpp - Interacción con libros
void TalkToTowner(Towner &towner)
{
    if (towner.type == TownerType::BOOK) {
        OracleEvents::TriggerEvent(
            OracleEvent::BOOK_INTERACTION,
            "Ancient Book"
        );
    }
}
```

---

### FASE 4: Cliente Ollama Asíncrono (2h)

#### 4.1 Estructura HTTP
```cpp
// oracle_ollama.h
class OllamaClient {
public:
    struct Request {
        std::string model = "qwen2.5:3b-instruct";
        std::string prompt;
        int max_tokens = 200;
        float temperature = 0.8;
    };
    
    struct Response {
        bool success;
        std::string text;
        std::string error;
    };
    
    static void QueryAsync(
        const Request& request,
        std::function<void(Response)> callback
    );
    
    static bool IsAvailable();
    
private:
    static constexpr const char* OLLAMA_URL = "http://localhost:11434/api/generate";
    static constexpr int TIMEOUT_MS = 5000;
};
```

#### 4.2 Implementación con libcurl o SDL_net
```cpp
// oracle_ollama.cpp
#include <thread>
#include <curl/curl.h> // O usar SDL_net si ya está disponible

void OllamaClient::QueryAsync(const Request& req, std::function<void(Response)> callback)
{
    // Lanzar en thread separado
    std::thread([req, callback]() {
        Response response;
        
        CURL* curl = curl_easy_init();
        if (!curl) {
            response.success = false;
            response.error = "Failed to initialize CURL";
            callback(response);
            return;
        }
        
        // Construir JSON request
        std::string jsonData = StrCat(
            "{\"model\":\"", req.model, "\",",
            "\"prompt\":\"", EscapeJson(req.prompt), "\",",
            "\"stream\":false}"
        );
        
        // Configurar CURL
        curl_easy_setopt(curl, CURLOPT_URL, OLLAMA_URL);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonData.c_str());
        curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, TIMEOUT_MS);
        
        // Ejecutar request
        CURLcode res = curl_easy_perform(curl);
        
        if (res != CURLE_OK) {
            response.success = false;
            response.error = curl_easy_strerror(res);
        } else {
            // Parsear respuesta JSON
            response.success = true;
            response.text = ParseOllamaResponse(responseBuffer);
        }
        
        curl_easy_cleanup(curl);
        callback(response);
        
    }).detach(); // Thread independiente
}

bool OllamaClient::IsAvailable()
{
    // Ping rápido a Ollama
    CURL* curl = curl_easy_init();
    if (!curl) return false;
    
    curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:11434/api/tags");
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 1000);
    curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
    
    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    
    return res == CURLE_OK;
}
```

---

### FASE 5: Prompt Maestro (30 min)

#### 5.1 Definición del Prompt
```cpp
// oracle_prompt.h
namespace OraclePrompt {

constexpr const char* MASTER_PROMPT = R"(
Rol:
Eres una entidad antigua del mundo de Diablo.
No eres un asistente.
No ayudas al jugador directamente.
No das instrucciones claras ni consejos mecánicos.
Hablas como un oráculo, grimorio o voz del Infierno.

Restricciones ABSOLUTAS:
- NO expliques mecánicas de juego
- NO menciones estadísticas, números, resistencias, builds ni items concretos
- NO digas qué debe hacer explícitamente
- NO optimices
- NO hables como tutorial
- NO rompas la cuarta pared
- NO menciones que eres una IA

Estilo:
- Críptico
- Oscuro
- Breve pero denso (2 a 6 líneas máximo)
- Poético, amenazante o solemne
- Basado únicamente en el LORE y la atmósfera de Diablo
- Puede juzgar al jugador
- Puede hablar en metáforas
- Puede ser ambiguo o incompleto

Tono según estado:
- Si FRIENDLY → guía enigmático, antiguo, resignado
- Si ATTACK → burlón, cruel, acusador, infernal

Formato:
- 2 a 6 líneas como máximo
- Sin listas
- Sin títulos
- Sin emojis
- Sin explicaciones fuera del personaje

Pregunta del jugador: {QUESTION}
Evento actual: {EVENT}
Estado del jugador: {STATE}
Contexto: {CONTEXT}

Responde como el Infierno mismo:
)";

std::string BuildPrompt(
    const std::string& question,
    const std::string& event,
    const std::string& state,
    const std::string& context
);

} // namespace OraclePrompt
```

---

### FASE 6: Sistema de Cache (1h)

#### 6.1 Cache Persistente
```cpp
// oracle_cache.h
class OracleCache {
public:
    struct CachedResponse {
        std::string question;
        std::string response;
        std::string event;
        uint32_t timestamp;
    };
    
    static void SaveResponse(const CachedResponse& response);
    static std::optional<std::string> GetResponse(const std::string& question);
    static void Clear();
    static void LoadFromDisk();
    static void SaveToDisk();
    
private:
    static std::vector<CachedResponse> cache;
    static constexpr const char* CACHE_FILE = "oracle_cache.dat";
};
```

#### 6.2 Persistencia
```cpp
// oracle_cache.cpp
void OracleCache::SaveToDisk()
{
    // Guardar en archivo binario o JSON
    std::ofstream file(CACHE_FILE, std::ios::binary);
    if (!file) return;
    
    for (const auto& entry : cache) {
        // Serializar cada entrada
        WriteString(file, entry.question);
        WriteString(file, entry.response);
        WriteString(file, entry.event);
        file.write(reinterpret_cast<const char*>(&entry.timestamp), sizeof(uint32_t));
    }
}

void OracleCache::LoadFromDisk()
{
    std::ifstream file(CACHE_FILE, std::ios::binary);
    if (!file) return;
    
    cache.clear();
    while (file.good()) {
        CachedResponse entry;
        entry.question = ReadString(file);
        entry.response = ReadString(file);
        entry.event = ReadString(file);
        file.read(reinterpret_cast<char*>(&entry.timestamp), sizeof(uint32_t));
        cache.push_back(entry);
    }
}
```

---

### FASE 7: Integración y Display (1.5h)

#### 7.1 Mostrar Respuesta en Pantalla
```cpp
// Usar sistema existente de mensajes o crear overlay especial

void DisplayOracleResponse(const std::string& response)
{
    // Opción 1: Usar sistema de mensajes existente
    AddConsoleMessage("═══════════════════════════");
    AddConsoleMessage("🔮 EL ORÁCULO HABLA:");
    AddConsoleMessage(response);
    AddConsoleMessage("═══════════════════════════");
    
    // Opción 2: Overlay especial con efecto visual
    // - Fondo oscuro semi-transparente
    // - Texto en color dorado/rojo
    // - Animación de fade in/out
    // - Duración: 8-10 segundos
}
```

#### 7.2 Integración Completa
```cpp
// oracle_system.cpp - Función principal

void OracleEvents::TriggerEvent(OracleEvent event, const std::string& context)
{
    // 1. Verificar si hay pregunta pendiente
    if (!OracleSystem::HasPendingQuestion())
        return;
    
    // 2. Verificar si Ollama está disponible
    if (!OllamaClient::IsAvailable()) {
        OracleSystem::ClearPendingQuestion();
        return;
    }
    
    // 3. Obtener pregunta
    PendingQuestion question = OracleSystem::GetPendingQuestion();
    
    // 4. Verificar cache
    auto cachedResponse = OracleCache::GetResponse(question.text);
    if (cachedResponse.has_value()) {
        DisplayOracleResponse(*cachedResponse);
        OracleSystem::ClearPendingQuestion();
        return;
    }
    
    // 5. Construir prompt
    std::string prompt = OraclePrompt::BuildPrompt(
        question.text,
        EventToString(event),
        question.state == PlayerState::FRIENDLY ? "FRIENDLY" : "ATTACK",
        context
    );
    
    // 6. Query asíncrono a Ollama
    OllamaClient::Request request;
    request.prompt = prompt;
    
    OllamaClient::QueryAsync(request, [question, event](OllamaClient::Response response) {
        if (response.success) {
            // Guardar en cache
            OracleCache::SaveResponse({
                question.text,
                response.text,
                EventToString(event),
                SDL_GetTicks()
            });
            
            // Mostrar respuesta
            DisplayOracleResponse(response.text);
        }
        
        // Limpiar pregunta pendiente
        OracleSystem::ClearPendingQuestion();
    });
}
```

---

## 🧪 TESTING

### Test Cases

1. **Test Sin Ollama Instalado** ⚠️ CRÍTICO
   - Ollama no instalado en el sistema
   - Usuario hace pregunta con "?"
   - Trigger evento
   - ✅ Debe: Juego funciona 100% normal, sin errores, sin mensajes
   - ✅ Debe: Diálogos originales intactos

2. **Test Ollama No Corriendo**
   - Ollama instalado pero no corriendo
   - Pregunta pendiente
   - Trigger evento
   - ✅ Debe: Limpiar pregunta silenciosamente, continuar normal

3. **Test Con Ollama Activo**
   - Ollama corriendo
   - Pregunta: "?¿Por qué sigo muriendo?"
   - Morir en nivel 5
   - ✅ Debe: Mostrar respuesta críptica del Oráculo

4. **Test Cache**
   - Misma pregunta dos veces
   - ✅ Debe: Segunda vez usar cache (no llamar Ollama)

5. **Test Timeout**
   - Ollama lento (>5s)
   - ✅ Debe: Timeout, limpiar pregunta, continuar sin interrupciones

6. **Test Gameplay**
   - Pregunta pendiente
   - Combate activo
   - ✅ Debe: NO bloquear, NO lag, respuesta solo en evento seguro

7. **Test Diálogos Originales**
   - Sin Ollama
   - Hablar con NPCs (Cain, Griswold, etc.)
   - ✅ Debe: Diálogos originales funcionan perfectamente

---

## 📊 ESTIMACIÓN DE TIEMPO

| Fase | Tarea | Tiempo |
|------|-------|--------|
| 1 | Modificar Tecla V | 30 min |
| 2 | Sistema Preguntas Pendientes | 1h |
| 3 | Detección Eventos Seguros | 1.5h |
| 4 | Cliente Ollama Asíncrono | 2h |
| 5 | Prompt Maestro | 30 min |
| 6 | Sistema de Cache | 1h |
| 7 | Integración y Display | 1.5h |
| 8 | Testing y Debugging | 2h |
| **TOTAL** | | **10 horas** |

---

## 🚀 ORDEN DE IMPLEMENTACIÓN

### Día 1 (4h)
1. ✅ Modificar Tecla V (30 min)
2. ✅ Sistema Preguntas Pendientes (1h)
3. ✅ Detección Eventos Seguros (1.5h)
4. ✅ Prompt Maestro (30 min)
5. ✅ Compilar y verificar (30 min)

### Día 2 (6h)
1. ✅ Cliente Ollama Asíncrono (2h)
2. ✅ Sistema de Cache (1h)
3. ✅ Integración y Display (1.5h)
4. ✅ Testing completo (2h)
5. ✅ Documentación final (30 min)

---

## 🔒 REGLAS DE SEGURIDAD

1. **NUNCA bloquear gameplay**
   - Todas las llamadas a Ollama son asíncronas
   - Timeout de 5 segundos máximo
   - Si falla, continúa normal

2. **NUNCA alterar lógica del juego**
   - Solo genera texto decorativo
   - No afecta stats, items, enemigos

3. **NUNCA reemplazar audio ni diálogos**
   - Solo texto adicional en pantalla
   - Audio original intacto
   - Diálogos de NPCs sin modificar

4. **SIEMPRE respetar eventos seguros**
   - Solo responde en momentos de pausa natural
   - Nunca durante combate activo

5. **⚠️ CRÍTICO: Graceful Degradation**
   - **Si Ollama NO está instalado**: Juego funciona 100% normal
   - **Si Ollama NO responde**: Juego continúa sin interrupciones
   - **Si hay timeout**: Se descarta la pregunta, gameplay intacto
   - **El Oráculo es OPCIONAL**: El juego NUNCA depende de él

---

## 📝 NOTAS FINALES

### Dependencias
- **libcurl** o **SDL_net**: Para HTTP requests
- **Ollama**: Debe estar instalado y corriendo localmente
- **Modelo**: `qwen2.5:3b-instruct` (ligero, rápido)

### Configuración Usuario
```ini
# nightmare_config/oracle.ini
[Oracle]
Enabled=true
OllamaURL=http://localhost:11434
Model=qwen2.5:3b-instruct
Timeout=5000
CacheEnabled=true
```

### Comandos de Chat
```
?<pregunta>          # Hacer pregunta al Oráculo
/oracle status       # Ver estado de Ollama
/oracle clear        # Limpiar cache
/oracle test         # Test de conexión
```

---

## ✅ CRITERIOS DE ÉXITO

1. ✅ Tecla V muestra "Nightmare Edition v1.0.0"
2. ✅ Chat activo por defecto en single player
3. ✅ Preguntas con "?" se guardan localmente
4. ✅ Respuestas solo en eventos seguros
5. ✅ Nunca bloquea gameplay
6. ✅ Respuestas crípticas y atmosféricas
7. ✅ Cache funcional (no repite llamadas)
8. ✅ Graceful degradation (funciona sin Ollama)

---

**¿Listo para empezar la implementación?** 🔮
