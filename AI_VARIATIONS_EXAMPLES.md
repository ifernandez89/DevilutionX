# 🎭 Ejemplos de Variaciones de IA - Nightmare Edition

## Objetivo

Crear variaciones **NOTABLES y VIVAS** de diálogos que hagan el juego más dinámico y reactivo, **sin inventar lore nuevo**.

## ✅ Principio Clave

**Lore-safe**: Solo usa palabras del texto original + palabras comunes (the, a, is, are, but, yet, still, now, here, there)

**Variaciones notables**: Reordenar dramáticamente, cambiar ritmo, añadir pausas (...), cambiar énfasis (MAYÚSCULAS)

---

## 📜 Ejemplos Reales de Variaciones

### Griswold (Blacksmith)

**Original**: `"What can I do for you?"`

**Variaciones observadas**:
- **Neutral**: `"What can I do for you?"` (preservado)
- **Weary**: `"What... can I do for you?"` (cansado, con pausa)
- **Irritated**: `"What do you WANT?"` (énfasis, más directo)

### Deckard Cain (Elder)

**Original**: `"Stay awhile and listen."`

**Variaciones observadas**:
- **Neutral**: `"Stay awhile and listen."` (preservado)
- **Weary**: `"Stay... and listen awhile."` (reordenado, pausado)
- **Cryptic**: `"Hear me... stay and listen."` (más misterioso)
- **Cold**: `"Stay. Listen awhile."` (frases cortas, distante)

### Adria (Witch)

**Original**: `"The darkness grows stronger."`

**Variaciones observadas**:
- **Neutral**: `"The darkness grows..."`
- **Cryptic**: `"The darkness... it GROWS."` (énfasis dramático)
- **Cold**: `"The DARKNESS... it swells... ever stronger!"` (muy dramático)
- **Weary**: `"The darkness grows... stronger..."` (pausado, exhausto)

### Pepin (Healer)

**Original**: `"I sense a soul in search of answers."`

**Variaciones observadas**:
- **Neutral**: `"I sense a soul in search of answers."`
- **Cryptic**: `"A soul... in search... of answers..."` (pausas místicas)
- **Cold**: `"I sense... a soul. Searching."` (analítico, fragmentado)

### Wirt (Boy)

**Original**: `"I got something for you."`

**Variaciones observadas**:
- **Neutral**: `"I got something for you."`
- **Irritated**: `"I got something. For YOU."` (impaciente, énfasis)
- **Cryptic**: `"Something... I have for you."` (misterioso)

---

## ⚔️ Mensajes de Combate

### Muerte del Jugador

**Original**: `"You have been slain."`

**Variaciones**:
- **Cryptic**: `"Slain... you have been."` (orden invertido, ominoso)
- **Cold**: `"You are slain."` (directo, frío)
- **Weary**: `"You have been... slain."` (pausa dramática)

### Victoria

**Original**: `"The demon falls before you."`

**Variaciones**:
- **Neutral**: `"The demon falls before you."`
- **Cryptic**: `"Before you... the demon FALLS."` (dramático)
- **Cold**: `"The demon falls."` (simple, directo)

### Advertencia de Salud

**Original**: `"Your health is low."`

**Variaciones**:
- **Weary**: `"Your health... it is low."` (preocupado)
- **Cryptic**: `"Low... your health grows."` (ominoso)
- **Irritated**: `"Your health is LOW!"` (urgente)

---

## 🌑 Textos Atmosféricos

### Corrupción

**Original**: `"The sanctity of this place has been fouled."`

**Variaciones**:
- **Weary**: `"The sanctity of this place... has been fouled."` (pausado)
- **Cryptic**: `"This place... its sanctity... FOULED."` (fragmentado, dramático)
- **Cold**: `"The sanctity here is fouled."` (directo)

### Peligro

**Original**: `"Evil stirs in the darkness."`

**Variaciones**:
- **Cryptic**: `"In the darkness... evil STIRS."` (orden invertido, énfasis)
- **Weary**: `"Evil... it stirs in the darkness."` (pausado)
- **Cold**: `"Evil stirs."` (minimalista)

### Muerte

**Original**: `"The dead walk among us."`

**Variaciones**:
- **Cryptic**: `"Among us... the dead WALK."` (ominoso)
- **Cold**: `"The dead walk."` (simple, directo)
- **Weary**: `"The dead... they walk among us."` (exhausto)

---

## 🎯 Patrones de Variación

### 1. Pausas Dramáticas (...)
```
"What can I do for you?" → "What... can I do for you?"
"The darkness grows." → "The darkness... it grows."
```

### 2. Énfasis con Mayúsculas
```
"The darkness grows." → "The darkness GROWS."
"Your health is low." → "Your health is LOW!"
```

### 3. Reordenamiento de Palabras
```
"Stay awhile and listen." → "Stay. Listen awhile."
"You have been slain." → "Slain... you have been."
"Evil stirs in the darkness." → "In the darkness... evil stirs."
```

### 4. Fragmentación
```
"I sense a soul in search of answers." → "A soul... in search... of answers..."
"The sanctity of this place has been fouled." → "This place... its sanctity... fouled."
```

### 5. Adición de Palabras Comunes
```
"The darkness grows." → "The darkness... it grows."
"I sense a soul." → "I sense... a soul."
```

### 6. Simplificación
```
"The demon falls before you." → "The demon falls."
"Evil stirs in the darkness." → "Evil stirs."
```

---

## 📊 Comparación por Tono

### Mismo Texto, Diferentes Tonos

**Texto base**: `"The darkness grows stronger."`

| Tono | Variación | Efecto |
|------|-----------|--------|
| **neutral** | `"The darkness grows..."` | Neutro, preserva significado |
| **weary** | `"The darkness grows... stronger..."` | Cansado, pausado |
| **cold** | `"The darkness grows."` | Frío, minimalista |
| **cryptic** | `"The darkness... it GROWS."` | Misterioso, dramático |
| **irritated** | `"The DARKNESS grows stronger!"` | Urgente, enfático |

---

## 🎮 Casos de Uso en el Juego

### 1. NPCs Reactivos

Los NPCs pueden responder diferente según:
- **Número de interacciones**: Primera vez (neutral) → Muchas veces (weary/irritated)
- **Estado del jugador**: Salud baja (weary), bien equipado (neutral)
- **Progreso del juego**: Inicio (neutral) → Niveles profundos (cryptic/cold)

**Ejemplo - Griswold**:
```cpp
// Primera compra del día
"What can I do for you?"

// Después de 10 compras
"What... can I do for you?"

// Jugador molestando sin comprar
"What do you WANT?"
```

### 2. Mensajes de Sistema Dinámicos

Los mensajes del sistema pueden variar para no ser repetitivos:

**Ejemplo - Muerte**:
```cpp
// Primera muerte
"You have been slain."

// Muerte repetida en mismo nivel
"Slain... you have been."

// Muchas muertes
"You are slain."
```

### 3. Textos Atmosféricos por Nivel

Los textos pueden adaptarse a la profundidad:

**Ejemplo - Advertencia**:
```cpp
// Cathedral (nivel 1-4)
"Evil stirs in the darkness."

// Catacombs (nivel 5-8)
"In the darkness... evil STIRS."

// Hell (nivel 13-16)
"Evil... it STIRS."
```

---

## ✅ Validación Lore-Safe

### Palabras Permitidas

**Del texto original**: Todas las palabras del texto base

**Palabras comunes permitidas**:
- Artículos: a, an, the
- Verbos auxiliares: is, are, was, were, be, been, have, has, had, do, does, did
- Pronombres: it, you, I, we, they
- Preposiciones: of, to, in, for, on, at, by, with, from, as, into
- Conjunciones: and, but, or, yet, so
- Adverbios: now, here, there, then, still, ever
- Otros: no, not, only, just, very, more, most

### Palabras NO Permitidas

❌ Nombres nuevos: "Diablo", "Lazarus" (si no están en original)  
❌ Lugares nuevos: "Hell", "Cathedral" (si no están en original)  
❌ Conceptos nuevos: "corruption", "evil" (si no están en original)  
❌ Slang moderno: "dude", "bro", "cool"  
❌ Humor: "lol", "haha", jokes  

---

## 🔧 Configuración Recomendada

### Temperature

- **0.6**: Balance entre consistencia y variación (recomendado)
- **0.7**: Más variaciones creativas
- **0.8**: Muy creativo (puede ser demasiado)

### Max Tokens

- **128**: Suficiente para diálogos cortos
- **256**: Para textos más largos

### Timeout

- **600ms**: Balance entre latencia y éxito
- **1000ms**: Más tolerante, menos fallos

---

## 💡 Conclusión

Las variaciones son:
- ✅ **NOTABLES**: Se perciben claramente
- ✅ **VIVAS**: Hacen los diálogos más dinámicos
- ✅ **LORE-SAFE**: No inventan contenido nuevo
- ✅ **ATMOSFÉRICAS**: Mantienen el tono de Diablo
- ✅ **REACTIVAS**: Pueden adaptarse al contexto

**Resultado**: Cada interacción se siente única sin romper la inmersión.

---

## 🎭 Ejemplos Adicionales para Testing

### Textos Cortos
```
"Greetings." → "Greetings..." (pausado)
"Leave." → "LEAVE." (enfático)
"Wait." → "Wait..." (dubitativo)
```

### Textos Medios
```
"The way is blocked." → "The way... it is blocked."
"Demons approach." → "Demons... they approach."
"Beware the shadows." → "The shadows... beware."
```

### Textos Largos
```
"The sanctity of this place has been fouled by the forces of darkness."
→ "This place... its sanctity... FOULED by the forces of darkness."
```

---

**Fecha**: Enero 15, 2026  
**Sistema**: AI Text Variation - Nightmare Edition  
**Estado**: ✅ Prompt optimizado para variaciones notables
