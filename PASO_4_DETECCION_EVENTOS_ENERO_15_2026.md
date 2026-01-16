# 🔮 PASO 4: Detección de Eventos Seguros

**Fecha**: Enero 15, 2026  
**Estado**: ✅ **COMPLETADO**

---

## 🎯 OBJETIVO

Crear el sistema de detección de eventos seguros donde el Oráculo puede responder sin interrumpir el gameplay.

---

## 📝 ARCHIVOS CREADOS

### 1. Source/oracle/oracle_events.h

**Enum de Eventos**:
```cpp
enum class OracleEvent {
    PLAYER_DEATH,       // ☠️ Jugador murió
    LEVEL_CLEARED,      // 🧹 Nivel limpiado
    ENTERED_TOWN,       // 🏘️ Entró a ciudad
    BOOK_INTERACTION,   // 📖 Leyó un libro
    ALTAR_INTERACTION,  // 🕯️ Usó un altar
    NPC_INTERACTION     // 💬 Habló con un NPC
};
```

**API Pública**:
```cpp
class OracleEvents {
public:
    static void TriggerEvent(OracleEvent event, const std::string& context);
    static bool IsEventSafe(OracleEvent event);
    static const char* EventToString(OracleEvent event);
};
```

### 2. Source/oracle/oracle_events.cpp

**Implementación**:
- `TriggerEvent()` - Verifica pregunta pendiente y dispara respuesta
- `IsEventSafe()` - Valida que el evento es seguro
- `EventToString()` - Convierte evento a string (logging)

**Por ahora**: Muestra mensaje placeholder hasta implementar Ollama (Paso 5/6)

---

## 🔗 HOOKS IMPLEMENTADOS

### Hook 1: Muerte del Jugador

**Archivo**: `Source/player.cpp`  
**Función**: `StartPlayerKill()`  
**Línea**: ~2785

```cpp
if (&player == MyPlayer) {
    NetSendCmdParam1(true, CMD_PLRDEAD, ...);
    gamemenu_off();
    
    // 🔮 ORÁCULO: Trigger evento de muerte
    OracleEvents::TriggerEvent(
        OracleEvent::PLAYER_DEATH,
        StrCat("Level ", setlevel ? "Quest" : std::to_string(currlevel))
    );
}
```

**Contexto**: Incluye nivel donde murió el jugador

### Hook 2: Entrada a Ciudad

**Archivo**: `Source/town_cinematic.cpp`  
**Función**: `OnEnterTown()`  
**Línea**: ~60

```cpp
void OnEnterTown() {
    // ... código existente ...
    
    // 🔮 ORÁCULO: Trigger evento de entrada a ciudad
    OracleEvents::TriggerEvent(OracleEvent::ENTERED_TOWN, "Tristram");
}
```

**Contexto**: "Tristram" (nombre de la ciudad)

---

## 🎮 CÓMO FUNCIONA

### Flujo Completo

```
1. Jugador escribe en chat: "¿Por qué sigo muriendo?"
   ↓
2. Sistema guarda pregunta pendiente
   ↓
3. Jugador continúa jugando...
   ↓
4. Jugador muere (EVENTO SEGURO)
   ↓
5. StartPlayerKill() llama OracleEvents::TriggerEvent(PLAYER_DEATH)
   ↓
6. TriggerEvent() verifica:
   - ¿Hay pregunta pendiente? ✅
   - ¿Evento es seguro? ✅
   ↓
7. Por ahora: Muestra "🔮 EL ORÁCULO MEDITA TU PREGUNTA..."
   ↓
8. Próximos pasos: Llamará a Ollama y mostrará respuesta real
```

### Ejemplo Real

```
Jugador: [Chat] "Este nivel es muy difícil"
Sistema: "🔮 El Infierno ha escuchado tus palabras..."

[Jugador continúa jugando]
[Jugador muere en Level 5]

Sistema: "🔮 EL ORÁCULO MEDITA TU PREGUNTA..."
         "(Sistema de respuestas en desarrollo - Paso 5/6)"

[En pasos futuros, mostrará respuesta real de Ollama]
```

---

## ✅ EVENTOS IMPLEMENTADOS

### ✅ Implementados (2/6)
1. **PLAYER_DEATH** - Muerte del jugador
2. **ENTERED_TOWN** - Entrada a ciudad

### ⚠️ Pendientes (4/6)
3. **LEVEL_CLEARED** - Nivel limpiado (todos los monstruos muertos)
4. **BOOK_INTERACTION** - Leyó un libro
5. **ALTAR_INTERACTION** - Usó un altar
6. **NPC_INTERACTION** - Habló con un NPC

**Nota**: Los 2 eventos implementados son suficientes para testear el sistema. Los demás se pueden añadir después.

---

## 🧪 TESTING

### Test 1: Muerte del Jugador
```
1. Compilar juego
2. Iniciar juego
3. Escribir en chat: "¿Por qué muero tanto?"
4. Dejar que un monstruo te mate
5. Verificar mensaje: "🔮 EL ORÁCULO MEDITA TU PREGUNTA..."
```

**Resultado esperado**: ✅ Mensaje mostrado al morir

### Test 2: Entrada a Ciudad
```
1. Escribir en chat: "¿Qué debo hacer?"
2. Entrar a un portal o volver a ciudad
3. Verificar mensaje del Oráculo
```

**Resultado esperado**: ✅ Mensaje mostrado al entrar a ciudad

### Test 3: Sin Pregunta Pendiente
```
1. NO escribir nada en chat
2. Morir
3. Verificar que NO aparece mensaje del Oráculo
```

**Resultado esperado**: ✅ Sin mensaje (correcto)

### Test 4: Logging (DEBUG)
```
1. Compilar en modo DEBUG
2. Escribir pregunta y morir
3. Verificar logs:
   "Oracle: Event PLAYER_DEATH triggered with question: ..."
```

**Resultado esperado**: ✅ Logs muestran evento

---

## 📊 IMPACTO

| Aspecto | Cambio | Riesgo |
|---------|--------|--------|
| player.cpp | Hook añadido | Bajo |
| town_cinematic.cpp | Hook añadido | Bajo |
| Nuevos archivos | 2 archivos | Ninguno |
| Gameplay | Sin cambios | Ninguno |

**Riesgo total**: **BAJO** (hooks mínimos, bien aislados)

---

## 🔄 PRÓXIMOS PASOS

### PASO 5: Cliente Ollama
- Implementar cliente HTTP asíncrono
- Conectar con Ollama local
- Timeout y fallback

### PASO 6: Prompt y Respuestas
- Crear prompt maestro críptico
- Generar respuestas atmosféricas
- Mostrar en pantalla con estilo

### PASO 7: Cache y Optimización
- Cache persistente
- Reutilizar respuestas
- Optimizar performance

---

## 📝 NOTAS TÉCNICAS

### Por Qué Estos Eventos Son Seguros

1. **PLAYER_DEATH**: Juego pausado, no hay combate activo
2. **ENTERED_TOWN**: Zona segura, no hay enemigos

### Eventos Futuros

Los eventos pendientes también son seguros:
- **LEVEL_CLEARED**: Todos los enemigos muertos
- **BOOK_INTERACTION**: Jugador leyendo (pausa natural)
- **ALTAR_INTERACTION**: Jugador interactuando (pausa)
- **NPC_INTERACTION**: Jugador hablando (pausa)

### Thread Safety

Por ahora es single-threaded. En Paso 5 (cliente asíncrono) añadiremos mutex para proteger `pendingQuestion`.

---

**Autor**: Kiro AI Assistant  
**Fecha**: Enero 15, 2026  
**Versión**: Paso 4 Completado

---

*"Los eventos están listos. El Oráculo aguarda su voz."* 🔮
