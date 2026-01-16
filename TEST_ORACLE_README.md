# 🔮 TEST DEL SISTEMA DEL ORÁCULO

Test standalone para probar el sistema completo del Oráculo sin necesidad de compilar todo el juego.

---

## 📋 REQUISITOS

### 1. Ollama Instalado y Corriendo
```bash
# Instalar Ollama
# https://ollama.ai/download

# Iniciar Ollama
ollama serve

# En otra terminal, verificar que funciona
ollama list

# Descargar modelo si no lo tienes
ollama pull qwen2.5:3b-instruct
```

### 2. Compilador C++ (MinGW-w64 o MSYS2)
```bash
# Verificar que g++ está disponible
g++ --version

# Si no está instalado:
# - Descargar MinGW-w64: https://www.mingw-w64.org/
# - O instalar MSYS2: https://www.msys2.org/
```

### 3. SDL2
```bash
# Debe estar en el PATH o en el directorio del proyecto
# Ya debería estar si compilaste DevilutionX antes
```

---

## 🚀 COMPILAR Y EJECUTAR

### Opción 1: Script Automático (Windows)
```bash
# Ejecutar el script de compilación
compile_test_oracle.bat

# Si compila exitosamente, ejecutar
test_oracle_complete.exe
```

### Opción 2: Compilación Manual
```bash
g++ -std=c++17 \
    test_oracle_complete.cpp \
    Source/oracle/oracle_dormant_texts.cpp \
    Source/oracle/oracle_prompt.cpp \
    Source/oracle/oracle_ollama.cpp \
    Source/oracle/oracle_validator.cpp \
    Source/oracle/oracle_cache.cpp \
    Source/utils/str_cat.cpp \
    Source/utils/str_case.cpp \
    -I. -ISource \
    -lwinhttp -lSDL2 \
    -o test_oracle_complete.exe

# Ejecutar
./test_oracle_complete.exe
```

---

## 🧪 TESTS INCLUIDOS

### Test 1: Verificar Disponibilidad de Ollama
- Verifica que Ollama está corriendo en localhost:11434
- Hace un ping rápido al servidor

### Test 2: Textos Dormidos
- Verifica que hay 17 textos dormidos disponibles
- Muestra ejemplos de cada categoría:
  - DEATH_WARNINGS
  - DARKNESS_LORE
  - ANCIENT_WISDOM
  - CRYPTIC_PROPHECY
  - INFERNO_WHISPERS
  - FATE_ECHOES

### Test 3: Construcción de Prompts
- Construye un prompt con textos base obligatorios
- Verifica que incluye:
  - EXAMPLE TEXTS (textos dormidos)
  - STRICT RULES (reglas de micro-variación)
  - Contexto del jugador

### Test 4: Query a Ollama (Asíncrono)
- Envía un prompt real a Ollama
- Espera respuesta (máximo 10 segundos)
- Muestra la respuesta generada

### Test 5: Validación de Respuestas
- Obtiene respuesta de Ollama
- Valida que sea micro-variación legítima:
  - Similitud mínima 60%
  - Sin palabras prohibidas
  - Longitud apropiada (±30%)
  - Tono consistente
- Muestra resultado de validación

### Test 6: Sistema de Cache
- Prueba cache MISS (primera consulta)
- Guarda respuesta en cache
- Prueba cache HIT (segunda consulta)
- Muestra estadísticas del cache

### Test 7: Flujo End-to-End Completo
- Simula flujo completo del Oráculo:
  1. Verificar cache
  2. Obtener textos dormidos
  3. Construir prompt
  4. Consultar Ollama
  5. Validar respuesta
  6. Mostrar respuesta
  7. Guardar en cache

### Test 8: Múltiples Preguntas (Cache Performance)
- Hace 5 preguntas (2 repetidas)
- Mide cache hits vs misses
- Verifica que preguntas repetidas usan cache
- Muestra estadísticas de performance

---

## 📊 EJEMPLO DE SALIDA

```
🔮 TEST COMPLETO DEL SISTEMA DEL ORÁCULO
========================================

================================================================
  TEST 1: Verificar Disponibilidad de Ollama
================================================================
Verificando si Ollama está corriendo...
✅ Ollama está disponible en localhost:11434

================================================================
  TEST 2: Textos Dormidos
================================================================
Verificando textos dormidos...
✅ Textos dormidos disponibles: 17

Ejemplos por categoría:

📜 DEATH_WARNINGS:
   "The darkness grows stronger with each passing moment. Many have fallen befo..."

📜 DARKNESS_LORE:
   "The corruption spreads like a plague through these ancient stones. What was ..."

...

================================================================
  TEST 4: Query a Ollama (Asíncrono)
================================================================
Construyendo prompt...
Enviando query a Ollama...
⏳ Esperando respuesta..........
✅ Respuesta recibida de Ollama

🔮 RESPUESTA DEL ORÁCULO:
----------------------------------------
The darkness grows ever stronger with each death.
Many have fallen in these cursed depths, and more shall follow.
----------------------------------------

================================================================
  TEST 5: Validación de Respuestas
================================================================
...
RESULTADO DE VALIDACIÓN:
----------------------------------------
Válida: ✅ SÍ
Similitud: 75.00%
Razón: Valid micro-variation
Texto base detectado:
"The darkness grows stronger with each passing moment. Many have fallen before..."
----------------------------------------
✅ Respuesta VÁLIDA - Es micro-variación reconocible

...

================================================================
  RESUMEN FINAL
================================================================
Tests ejecutados: 8
Tests exitosos: 8
Tests fallidos: 0
Tasa de éxito: 100%

✅ ¡TODOS LOS TESTS PASARON! 🎉

El sistema del Oráculo está funcionando perfectamente.
```

---

## 🔍 QUÉ VERIFICAR

### 1. Respuestas son Micro-Variaciones
Las respuestas de Ollama deben ser **reconocibles** como variaciones de los textos base:

**Texto Base**:
```
"The darkness grows stronger with each passing moment.
Many have fallen before you, and many more shall follow."
```

**Micro-Variación Válida** (✅):
```
"The darkness grows ever stronger with each death.
Many have fallen in these depths, and more shall follow."
```

**Cambios**: 2-3 palabras, estructura similar, tono idéntico

**Variación Inválida** (❌):
```
"You need to level up and get better equipment.
Try using potions and avoiding enemy attacks."
```

**Problema**: Contenido inventado, tono moderno, no es variación

### 2. Validación Funciona
- Similitud debe ser >60%
- Sin palabras prohibidas (lol, noob, tutorial, etc.)
- Longitud similar al texto base
- Tono dark/cryptic mantenido

### 3. Cache Funciona
- Primera pregunta: Cache MISS (3-5 segundos)
- Segunda pregunta (misma): Cache HIT (instantáneo)
- Hit rate debe aumentar con preguntas repetidas

---

## 🐛 TROUBLESHOOTING

### Error: "Ollama not available"
```bash
# Solución: Iniciar Ollama
ollama serve

# En otra terminal, verificar
ollama list
```

### Error: "g++ not found"
```bash
# Solución: Instalar MinGW-w64 o MSYS2
# Añadir al PATH: C:\msys64\mingw64\bin
```

### Error: "SDL2 not found"
```bash
# Solución: Verificar que SDL2 está en el PATH
# O copiar SDL2.dll al directorio del test
```

### Error: "Similarity too low"
```bash
# Esto significa que Ollama está generando contenido muy diferente
# Posibles causas:
# 1. Modelo diferente (debe ser qwen2.5:3b-instruct)
# 2. Prompt no está llegando correctamente
# 3. Temperatura muy alta

# Verificar modelo:
ollama list

# Debe mostrar: qwen2.5:3b-instruct
```

### Respuestas en Inglés vs Español
```bash
# El sistema está diseñado para inglés (textos originales de Diablo)
# Las preguntas pueden ser en español, pero las respuestas serán en inglés
# Esto es intencional para mantener autenticidad
```

---

## 📝 NOTAS

### Performance
- Primera consulta: 3-5 segundos (Ollama genera respuesta)
- Consultas cacheadas: <10ms (instantáneo)
- Hit rate esperado: 70-80% después de 100 preguntas

### Modelo Recomendado
- **qwen2.5:3b-instruct**: Rápido, bueno para micro-variaciones
- Alternativas: llama2:7b, mistral:7b (más lentos pero más creativos)

### Ajustar Parámetros
Si las respuestas son muy diferentes, editar `Source/oracle/oracle_ollama.cpp`:

```cpp
// Reducir temperatura para respuestas más conservadoras
"\"temperature\":0.6,"  // Era 0.8

// Reducir top_p para menos variación
"\"top_p\":0.8,"  // Era 0.9
```

---

## 🎯 OBJETIVO DEL TEST

Este test te permite:

1. ✅ Verificar que Ollama funciona correctamente
2. ✅ Ver cómo el modelo responde a los prompts
3. ✅ Verificar que las respuestas son micro-variaciones válidas
4. ✅ Probar el sistema de validación
5. ✅ Probar el sistema de cache
6. ✅ Medir performance (latencia, hit rate)
7. ✅ Ajustar parámetros si es necesario

**Sin necesidad de compilar todo el juego.**

---

**Autor**: Kiro AI Assistant  
**Fecha**: Enero 16, 2026

---

*"Prueba el Oráculo antes de integrarlo."*  
*"Verifica que las respuestas son auténticas."*  
*"Optimiza el sistema con datos reales."* 🔮✨🧪

