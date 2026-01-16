# 🔮 ORACLE UI INTEGRATION - DISEÑO FINAL
**Fecha**: Enero 16, 2026  
**Estado**: ✅ DECISIÓN DE DISEÑO CERRADA

---

## 🎯 PRINCIPIO RECTOR

### El Oráculo Nunca Interrumpe. Solo Informa.

```
❌ NO:
- Ventanas modales
- Popups que bloquean
- Sonidos intrusivos
- Animaciones llamativas
- Requerir input del jugador

✅ SÍ:
- Susurros sutiles
- Fade in/out suave
- Usar sistemas existentes
- Cero UI nueva
- Completamente opcional
```

---

## 🏗️ ARQUITECTURA DE ROUTING

### Canales Definidos

```
┌─────────────────────────────────────────────────────────────┐
│                    ORACLE MESSAGE ROUTING                   │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  EVENTO DEL JUEGO                                           │
│       ↓                                                     │
│  ORACLE GENERA SUSURRO                                      │
│       ↓                                                     │
│  ¿CHAT ACTIVO?                                              │
│       ├─ SÍ → CANAL A: Chat "voice" line                   │
│       └─ NO → CANAL B: Debug overlay (parcial)             │
│                                                             │
│  MENSAJE:                                                   │
│  - Fade in (1 segundo)                                      │
│  - Visible (3-5 segundos)                                   │
│  - Fade out (1 segundo)                                     │
│  - No persiste                                              │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

---

## 📺 CANAL A: Chat "Voice" Line

### Cuándo Usar
```cpp
if (IsChatWindowOpen() || IsPlayerTyping()) {
    UseChannelA();
}
```

### Implementación
```cpp
// Reusar área "voice" del chat
// Reducir a EXACTAMENTE 1 línea
// Solo lectura (no editable)
// Solo para susurros del Oráculo

void ShowOracleInChat(const std::string& message) {
    // Área de voz del chat (1 línea)
    chatVoiceLine = message;
    chatVoiceLineVisible = true;
    chatVoiceLineTimer = 5000; // 5 segundos
    
    // Estilo
    chatVoiceLineColor = GRAY_SUBTLE;
    chatVoiceLinePrefix = "🔮 ";
}
```

### Características
- **Posición**: Dentro de la ventana de chat
- **Tamaño**: 1 línea exacta
- **Color**: Gris sutil (no blanco brillante)
- **Prefijo**: 🔮 (opcional, sutil)
- **Tono**: Susurro, contextual, narrativo
- **Duración**: 5 segundos
- **Interacción**: Ninguna (solo lectura)

---

## 🐛 CANAL B: Debug Overlay (Parcial)

### Cuándo Usar
```cpp
if (!IsChatWindowOpen() && !IsPlayerTyping()) {
    UseChannelB();
}
```

### Implementación
```cpp
// Usar sistema de debug overlay EXISTENTE
// Pero SOLO para mensajes del Oráculo
// NO activar debug completo

void ShowOracleInDebug(const std::string& message) {
    // Debug overlay parcial
    oracleDebugLine = message;
    oracleDebugLineVisible = true;
    oracleDebugLineTimer = 4000; // 4 segundos
    
    // Posición
    oracleDebugLineX = 10;  // Margen izquierdo
    oracleDebugLineY = 50;  // Debajo del HUD
    
    // Estilo
    oracleDebugLineColor = GRAY_DARK;
    oracleDebugLineAlpha = 0.8f; // Semi-transparente
}
```

### Características Críticas

**✅ VISIBLE POR DEFECTO**
```cpp
// NO requiere F12
// NO activa debug completo
// SOLO muestra mensajes del Oráculo
bool oracleDebugEnabled = true;  // Siempre true
```

**✅ DEBUG OVERLAY PARCIAL**
```cpp
void RenderDebugOverlay() {
    // Debug normal (requiere F12)
    if (debugMode) {
        RenderFullDebugInfo();
    }
    
    // Oracle messages (SIEMPRE visible)
    if (oracleDebugLineVisible) {
        RenderOracleMessage();  // No requiere F12
    }
}
```

**✅ NO STACKING**
```cpp
// Solo 1 mensaje a la vez
// Nuevo mensaje reemplaza anterior
if (newOracleMessage) {
    oracleDebugLine = newMessage;  // Reemplaza
    ResetTimer();
}
```

### Características
- **Posición**: Esquina superior izquierda (debajo del HUD)
- **Tamaño**: 1 línea
- **Color**: Gris oscuro semi-transparente
- **Duración**: 4 segundos
- **Fade**: In (1s) → Visible (2s) → Out (1s)
- **Stacking**: No (solo 1 mensaje)
- **F12**: NO requerido

---

## 🎨 COMPORTAMIENTO VISUAL

### Fade In/Out
```cpp
struct OracleMessage {
    std::string text;
    float alpha;           // 0.0 - 1.0
    uint32_t startTime;
    uint32_t duration;     // 4000-5000ms
    
    void Update() {
        uint32_t elapsed = SDL_GetTicks() - startTime;
        
        if (elapsed < 1000) {
            // Fade in (0-1 segundo)
            alpha = elapsed / 1000.0f;
        } else if (elapsed < duration - 1000) {
            // Visible (1-4 segundos)
            alpha = 1.0f;
        } else if (elapsed < duration) {
            // Fade out (4-5 segundos)
            alpha = 1.0f - ((elapsed - (duration - 1000)) / 1000.0f);
        } else {
            // Desaparecer
            alpha = 0.0f;
            text.clear();
        }
    }
};
```

### Estilo Visual
```cpp
// Colores sutiles (no brillantes)
constexpr uint8_t ORACLE_GRAY_CHAT = 200;   // Gris claro para chat
constexpr uint8_t ORACLE_GRAY_DEBUG = 180;  // Gris oscuro para debug

// Font
constexpr Font ORACLE_FONT = FONT_SMALL;    // Pequeño, discreto

// Prefijo opcional
constexpr const char* ORACLE_PREFIX = "🔮 "; // Muy sutil
```

---

## 🔄 FLUJO COMPLETO

### 1. Evento del Juego
```cpp
// Jugador muere
void OnPlayerDeath() {
    if (OracleSystem::HasPendingQuestion()) {
        OracleEvents::TriggerEvent(PLAYER_DEATH);
    }
}
```

### 2. Oracle Genera Susurro
```cpp
void TriggerEvent(OracleEvent event) {
    // Obtener pregunta pendiente
    auto question = OracleSystem::GetPendingQuestion();
    
    // Consultar Ollama (async)
    OracleOllama::QueryAsync(question, [](auto response) {
        if (response) {
            ShowOracleMessage(*response);
        }
    });
}
```

### 3. Routing del Mensaje
```cpp
void ShowOracleMessage(const std::string& message) {
    // Evaluar canal
    if (IsChatWindowOpen() || IsPlayerTyping()) {
        // CANAL A: Chat voice line
        ShowOracleInChat(message);
    } else {
        // CANAL B: Debug overlay
        ShowOracleInDebug(message);
    }
}
```

### 4. Renderizado
```cpp
void RenderFrame() {
    RenderWorld();
    RenderUI();
    
    // Chat (si está abierto)
    if (IsChatWindowOpen()) {
        RenderChat();
        RenderOracleChatLine();  // CANAL A
    }
    
    // Debug overlay (siempre para Oracle)
    RenderOracleDebugLine();     // CANAL B (no requiere F12)
}
```

### 5. Fade Out Automático
```cpp
void UpdateOracleMessages() {
    // Chat voice line
    if (chatVoiceLineTimer > 0) {
        chatVoiceLineTimer -= deltaTime;
        if (chatVoiceLineTimer <= 0) {
            chatVoiceLineVisible = false;
        }
    }
    
    // Debug line
    if (oracleDebugLineTimer > 0) {
        oracleDebugLineTimer -= deltaTime;
        if (oracleDebugLineTimer <= 0) {
            oracleDebugLineVisible = false;
        }
    }
}
```

---

## 🛡️ SAFETY & COMPATIBILITY

### Deshabilitación Completa
```cpp
// Si Oracle está deshabilitado
if (!OracleSystem::IsEnabled()) {
    // Juego se comporta EXACTAMENTE como vanilla
    // Cero overhead
    // Cero mensajes
    return;
}
```

### F12 Debug Toggle Intacto
```cpp
// F12 sigue funcionando SOLO para debug normal
void OnF12Pressed() {
    debugMode = !debugMode;
    // NO afecta Oracle messages
    // Oracle messages siempre visibles (si están activos)
}
```

### No Refactorizar Sistemas Existentes
```cpp
// ❌ NO HACER: Refactorizar chat completo
// ❌ NO HACER: Modificar debug system profundamente
// ✅ HACER: Reusar sistemas existentes
// ✅ HACER: Añadir solo routing mínimo
```

---

## 📋 IMPLEMENTACIÓN PASO A PASO

### Paso 1: Preparar Estructuras
```cpp
// En oracle_ui.h
struct OracleUIMessage {
    std::string text;
    float alpha;
    uint32_t startTime;
    uint32_t duration;
    bool visible;
    
    void Update();
    void Render();
};

// Variables globales
OracleUIMessage g_oracleChatMessage;
OracleUIMessage g_oracleDebugMessage;
```

### Paso 2: Implementar Routing
```cpp
// En oracle_ui.cpp
void ShowOracleMessage(const std::string& message) {
    if (IsChatWindowOpen() || IsPlayerTyping()) {
        g_oracleChatMessage.text = message;
        g_oracleChatMessage.startTime = SDL_GetTicks();
        g_oracleChatMessage.duration = 5000;
        g_oracleChatMessage.visible = true;
    } else {
        g_oracleDebugMessage.text = message;
        g_oracleDebugMessage.startTime = SDL_GetTicks();
        g_oracleDebugMessage.duration = 4000;
        g_oracleDebugMessage.visible = true;
    }
}
```

### Paso 3: Integrar en Chat
```cpp
// En control_chat.cpp
void RenderChat() {
    // Chat normal
    RenderChatMessages();
    
    // Oracle voice line (si visible)
    if (g_oracleChatMessage.visible) {
        g_oracleChatMessage.Update();
        g_oracleChatMessage.Render();
    }
}
```

### Paso 4: Integrar en Debug Overlay
```cpp
// En debug_overlay.cpp (o crear oracle_debug_overlay.cpp)
void RenderDebugOverlay() {
    // Debug normal (requiere F12)
    if (debugMode) {
        RenderFullDebugInfo();
    }
    
    // Oracle messages (SIEMPRE visible si activo)
    if (g_oracleDebugMessage.visible) {
        g_oracleDebugMessage.Update();
        g_oracleDebugMessage.Render();
    }
}
```

### Paso 5: Update Loop
```cpp
// En main game loop
void UpdateFrame() {
    // ... resto del juego
    
    // Update Oracle UI
    g_oracleChatMessage.Update();
    g_oracleDebugMessage.Update();
}
```

---

## 🎯 VENTAJAS DE ESTE DISEÑO

### 1. Usa Sistemas Existentes
```
✅ Chat voice line (ya existe)
✅ Debug overlay (ya existe)
✅ Fade system (ya existe)
✅ Timer system (ya existe)
```

### 2. Cero UI Nueva
```
✅ No ventanas modales
✅ No popups
✅ No menús nuevos
✅ No paneles
```

### 3. No Interrumpe
```
✅ Nunca bloquea gameplay
✅ Nunca requiere input
✅ Fade out automático
✅ Completamente pasivo
```

### 4. Elegante y Sutil
```
✅ Susurros, no gritos
✅ Gris, no blanco brillante
✅ Fade suave, no pop
✅ 1 línea, no scroll
```

### 5. Arquitectura Madura
```
✅ Routing inteligente
✅ Contexto-aware
✅ Fail-safe
✅ Deshabilitación limpia
```

---

## 💬 EJEMPLOS VISUALES

### Ejemplo 1: Chat Abierto (Canal A)
```
┌─────────────────────────────────────────┐
│ CHAT WINDOW                             │
├─────────────────────────────────────────┤
│ Player1: anyone have a spare sword?    │
│ Player2: check the shop                │
│ Player3: I can trade                   │
├─────────────────────────────────────────┤
│ 🔮 The abyss swallows your fear...     │ ← Oracle (gris sutil)
├─────────────────────────────────────────┤
│ > _                                     │ ← Input
└─────────────────────────────────────────┘
```

### Ejemplo 2: Chat Cerrado (Canal B)
```
┌─────────────────────────────────────────┐
│ GAME WORLD                              │
│                                         │
│ 🔮 Darkness enfolds the bones of time  │ ← Oracle (debug overlay)
│                                         │
│                                         │
│         [Player]                        │
│                                         │
│                                         │
│ ┌─────────────────┐                    │
│ │ HP: 50/100      │                    │
│ │ Mana: 30/80     │                    │
│ └─────────────────┘                    │
└─────────────────────────────────────────┘
```

---

## 🎓 FILOSOFÍA FINAL

### "Parece que siempre estuvo ahí, pero nadie terminó"

```
El Oráculo:
- No grita
- No roba pantalla
- No exige atención
- Simplemente está

Usa chat cuando tiene sentido
Usa debug cuando no
Nunca pierde información
Nunca interrumpe

Esto no parece un mod.
Parece algo que Blizzard dejó a medias.
```

---

## ✅ DECISIÓN CERRADA

Este diseño es **final y definitivo**:
- ✅ Arquitectura madura
- ✅ Elegante y sutil
- ✅ Usa sistemas existentes
- ✅ Cero UI nueva
- ✅ No interrumpe nunca

**No requiere más iteraciones. Implementar tal cual.**

---

**Estado**: ✅ DISEÑO APROBADO - LISTO PARA IMPLEMENTACIÓN
