# 🔮 GUÍA: CÓMO VERIFICAR SI OLLAMA ESTÁ FUNCIONANDO

**Fecha**: 16 de Enero, 2026  
**Sistema**: Oracle System - Nightmare Edition

---

## 🎯 RESUMEN RÁPIDO

El sistema tiene **2 modos**:
- **Modo Ollama**: Genera variaciones únicas usando IA
- **Modo Fallback**: Usa textos preseteados (sin Ollama)

---

## ✅ MÉTODO 1: VERIFICACIÓN EXTERNA (MÁS FÁCIL)

### Paso 1: Ejecutar el script de testing
```bash
TEST_OLLAMA_STATUS.bat
```

Este script verifica:
1. ✅ Si Ollama está corriendo en `localhost:11434`
2. ✅ Si el modelo `qwen2.5:3b-instruct` está instalado
3. ✅ Si Ollama responde correctamente a consultas

### Paso 2: Interpretar resultados

**Si todo está ✅**:
- Ollama está funcionando
- El juego DEBERÍA usar IA para generar respuestas

**Si algo está ❌**:
- El juego usará textos preseteados (fallback)
- Para arreglar:
  ```bash
  # Iniciar Ollama
  ollama serve
  
  # Instalar modelo (en otra terminal)
  ollama pull qwen2.5:3b-instruct
  ```

---

## 🔍 MÉTODO 2: VERIFICACIÓN EN EL JUEGO (OBSERVACIÓN)

### Diferencias entre Ollama y Fallback

| Característica | Con Ollama (IA) | Sin Ollama (Fallback) |
|----------------|-----------------|----------------------|
| **Textos** | Variaciones sutiles cada vez | Exactamente iguales siempre |
| **Repetición** | Nunca idénticos | Siempre idénticos |
| **Longitud** | Puede variar ligeramente | Siempre igual |
| **Palabras** | Sinónimos, reordenamientos | Palabra por palabra igual |

### Prueba práctica

1. **Muere 3 veces en el mismo lugar**
2. **Lee los mensajes del Oráculo**

**Ejemplo con Ollama (IA)**:
```
Muerte 1: "The darkness grows stronger with each passing moment. Many have fallen before you."
Muerte 2: "With every moment, the darkness strengthens. Countless souls fell before your arrival."
Muerte 3: "Each moment brings greater darkness. Many perished here before you came."
```
👉 **Variaciones sutiles** = Ollama funcionando

**Ejemplo sin Ollama (Fallback)**:
```
Muerte 1: "The darkness grows stronger with each passing moment. Many have fallen before you."
Muerte 2: "The darkness grows stronger with each passing moment. Many have fallen before you."
Muerte 3: "The darkness grows stronger with each passing moment. Many have fallen before you."
```
👉 **Texto idéntico** = Usando fallback

---

## 🐛 MÉTODO 3: VERIFICACIÓN CON LOGS (MODO DEBUG)

### Opción A: Compilar en modo Debug

```bash
# Limpiar build anterior
rmdir /s /q build_NOW

# Configurar en modo Debug
cmake -B build_NOW -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug

# Compilar
cmake --build build_NOW -j 4

# Copiar ejecutable
copy build_NOW\devilutionx.exe nightmare.exe
```

### Opción B: Buscar en logs existentes

El código tiene logs en `#ifdef _DEBUG`:

**Archivo**: `Source/oracle/oracle_ollama.cpp`

Busca estos mensajes:

```cpp
// SI VES ESTO = Ollama está siendo consultado
LogVerbose("Oracle: Querying Ollama with prompt: \"{}\"", prompt);

// SI VES ESTO = Ollama respondió exitosamente
LogVerbose("Oracle: Ollama response: \"{}\"", *parsed);

// SI VES ESTO = Ollama falló
LogVerbose("Oracle: Ollama query failed (no response)");
LogVerbose("Oracle: Failed to parse Ollama response");
```

### Dónde buscar logs

1. **Consola del juego** (si compilaste en Debug)
2. **Archivo de log** (si existe):
   - `debug_logs/`
   - `devilutionx.log`
   - Salida estándar redirigida

---

## 🔧 MÉTODO 4: VERIFICACIÓN CON CÓDIGO

### Agregar logging temporal

Edita `Source/oracle/oracle_events.cpp` y busca la función que procesa eventos.

Agrega esto al inicio:

```cpp
void OracleEvents::OnPlayerDeath()
{
    // TEMPORAL: Verificar si Ollama está disponible
    bool ollamaAvailable = OracleOllama::IsAvailable();
    
    char debugMsg[256];
    sprintf(debugMsg, "🔮 DEBUG: Ollama %s", 
            ollamaAvailable ? "DISPONIBLE" : "NO DISPONIBLE");
    EventPlrMsg(debugMsg, UiFlags::ColorWhitegold);
    
    // ... resto del código
}
```

Recompila y ejecuta. Verás el mensaje en pantalla.

---

## 📊 MÉTODO 5: MONITOREO DE RED

### Usar Wireshark o tcpdump

Si Ollama está funcionando, verás tráfico HTTP a `localhost:11434`:

```bash
# Windows (PowerShell como admin)
netstat -ano | findstr "11434"

# Si ves conexiones = Ollama está siendo usado
```

### Usar Process Monitor

1. Descarga Process Monitor (Sysinternals)
2. Filtra por `nightmare.exe`
3. Busca conexiones de red a puerto `11434`

---

## 🎮 MÉTODO 6: TESTING INTERACTIVO

### Crear un comando de debug

Edita `Source/control/control_chat_commands.cpp` y agrega:

```cpp
// Comando: /oracle_test
if (command == "oracle_test") {
    bool available = OracleOllama::IsAvailable();
    
    if (available) {
        EventPlrMsg("🔮 Ollama está DISPONIBLE y funcionando", UiFlags::ColorGreen);
        
        // Hacer consulta de prueba
        OracleOllama::QueryAsync(
            "Test: Say only 'The Oracle awakens'",
            [](std::optional<std::string> response) {
                if (response.has_value()) {
                    EventPlrMsg(StrCat("🔮 Respuesta: ", *response), UiFlags::ColorWhitegold);
                } else {
                    EventPlrMsg("🔮 Error: No se recibió respuesta", UiFlags::ColorRed);
                }
            }
        );
    } else {
        EventPlrMsg("🔮 Ollama NO está disponible - usando fallback", UiFlags::ColorRed);
    }
    
    return true;
}
```

Luego en el juego escribe: `/oracle_test`

---

## 🚨 TROUBLESHOOTING

### Problema: Ollama está corriendo pero el juego no lo usa

**Posibles causas**:

1. **Puerto incorrecto**:
   - Verifica que Ollama esté en puerto `11434`
   - Comando: `netstat -ano | findstr "11434"`

2. **Firewall bloqueando**:
   - Permite `nightmare.exe` en el firewall
   - Permite conexiones a `localhost:11434`

3. **Modelo no cargado**:
   - Verifica: `ollama list`
   - Debe aparecer: `qwen2.5:3b-instruct`

4. **Timeout muy corto**:
   - El código usa `5000ms` (5 segundos)
   - Si Ollama es lento, puede fallar
   - Edita `OLLAMA_TIMEOUT_MS` en `oracle_ollama.cpp`

### Problema: Mensajes no aparecen en absoluto

**Verifica**:

1. ¿Estás escribiendo en el chat?
   - El sistema solo responde si dejas preguntas
   - Escribe algo y presiona Enter

2. ¿Estás en un evento válido?
   - Muerte del jugador
   - Entrada a ciudad
   - Nivel limpiado
   - Interacción con libro/altar

3. ¿El sistema está inicializado?
   - Busca mensaje de bienvenida al iniciar
   - Debe aparecer: "🔮 El Infierno te observa..."

---

## 📝 RESUMEN DE INDICADORES

| Indicador | Ollama Funcionando | Ollama NO Funcionando |
|-----------|-------------------|----------------------|
| Textos repetidos | ❌ Nunca idénticos | ✅ Siempre idénticos |
| Variaciones | ✅ Sutiles cambios | ❌ Cero cambios |
| Logs "Querying Ollama" | ✅ Aparecen | ❌ No aparecen |
| Logs "query failed" | ❌ No aparecen | ✅ Aparecen |
| Tráfico puerto 11434 | ✅ Presente | ❌ Ausente |
| Script TEST_OLLAMA_STATUS | ✅ Todo verde | ❌ Errores rojos |

---

## 🎯 RECOMENDACIÓN FINAL

**La forma MÁS FÁCIL de verificar**:

1. Ejecuta `TEST_OLLAMA_STATUS.bat`
2. Si todo está ✅, Ollama funciona
3. Juega y muere 3 veces
4. Compara los textos:
   - **Iguales** = Fallback
   - **Diferentes** = Ollama

**Tiempo estimado**: 2 minutos

---

**Estado**: Guía completa para verificación del sistema Oráculo 🔮
