# 🔮 DECISIÓN DE DISEÑO: IDIOMA DEL ORÁCULO
**Fecha**: Enero 16, 2026  
**Decisión**: Inglés Canónico (Inmutable)

---

## 🎯 PRINCIPIO RECTOR

### El Oráculo NO es multilingüe
```
❌ NO: Traducción dinámica según idioma del juego
❌ NO: Prompts ambiguos por idioma
❌ NO: Dependencias de latencia extra

✅ SÍ: Inglés canónico siempre
✅ SÍ: Coherente, estable, predecible
✅ SÍ: Cacheable universalmente
```

---

## 🏆 DECISIÓN FINAL: INGLÉS CANÓNICO

### Por qué Inglés
1. **Lore auténtico**: Diablo I fue escrito y pensado en inglés
2. **Tono correcto**: Las metáforas y el ritmo funcionan mejor
3. **Sin modernización**: El modelo no "suaviza" frases
4. **Cache único**: Válido para todos los jugadores
5. **Atmósfera**: "Ancient tongue, unchanged by mortal translations"

### Cómo funciona
```
UI del juego: Idioma elegido por el jugador (español, francés, etc.)
Oráculo: SIEMPRE inglés (inmutable)
```

### Justificación lore-friendly
> "The Oracle speaks in an ancient tongue, unchanged by mortal translations."

No es una limitación técnica. Es **diseño intencional**.

---

## 🎭 PRESENTACIÓN AL JUGADOR

### En README / Opciones (sutil)
```
"The Oracle speaks in an ancient tongue, unchanged by mortal translations.
Its words echo from a time before the corruption, preserved in their original form."
```

### En el juego (primera vez)
Cuando el Oráculo responde por primera vez, mostrar brevemente:
```
🔮 The Oracle speaks in the ancient tongue...
```

Nada más. El jugador entiende que:
- Esto no es para ser entendido del todo
- Es parte de la atmósfera
- Es intencional, no un bug

---

## 💡 VENTAJAS DE ESTA DECISIÓN

### 1. Técnicas
- ✅ Cache único para todos los jugadores
- ✅ Sin complejidad de traducción
- ✅ Sin latencia extra
- ✅ Sin dependencias de idioma
- ✅ Prompts consistentes

### 2. Atmosféricas
- ✅ Profecías en "lengua antigua"
- ✅ Susurros incomprensibles
- ✅ Advertencias crípticas
- ✅ Sensación de misterio
- ✅ Autenticidad Diablo

### 3. De Diseño
- ✅ Coherente con lore
- ✅ Predecible
- ✅ Estable
- ✅ No rompe inmersión
- ✅ Suma atmósfera

---

## 🎨 VARIACIONES PERMITIDAS (Estilo, no Idioma)

### Acentos estilísticos en inglés
El sistema puede variar el **estilo** del inglés:

1. **Inglés neutral** (base)
   - "The darkness grows stronger."

2. **Inglés arcaico** (opcional)
   - "The darkness doth grow stronger."
   - "Thy path leads to shadow."

3. **Inglés eclesiástico** (opcional)
   - "The darkness waxes in strength."
   - "Thy journey descends into perdition."

Esto vale **más** que traducir a otros idiomas.

---

## 📊 COMPARACIÓN: Multilingüe vs Canónico

| Aspecto | Multilingüe ❌ | Canónico ✅ |
|---------|---------------|-------------|
| **Cache** | Por idioma (fragmentado) | Único (eficiente) |
| **Latencia** | Variable por idioma | Consistente |
| **Tono** | Se pierde en traducción | Auténtico |
| **Complejidad** | Alta (prompts por idioma) | Baja (un solo prompt) |
| **Atmósfera** | Genérica | Misteriosa |
| **Lore** | Inconsistente | Auténtico |
| **Mantenimiento** | Difícil | Simple |

---

## 🔧 IMPLEMENTACIÓN ACTUAL

### Sistema ya configurado correctamente
El sistema actual **ya usa inglés canónico**:

1. **Textos dormidos**: Todos en inglés
   ```cpp
   "The darkness grows stronger with each passing moment."
   "These halls have claimed countless souls."
   ```

2. **Prompt**: En inglés
   ```cpp
   "You are the voice of the dungeon itself from Diablo (1996)."
   ```

3. **Validación**: Palabras prohibidas en inglés
   ```cpp
   g_bannedWords = {"lol", "omg", "cool", ...}
   ```

4. **Respuestas**: Siempre en inglés
   ```
   "The abyss swallows your fear with each step too far."
   ```

### No requiere cambios
✅ El sistema ya está diseñado correctamente desde el inicio.

---

## 🎮 UX DIABLO-FRIENDLY

### Escenario: Juego en Español
```
Jugador: "¿Por qué sigo muriendo?" (chat en español)
Sistema: Detecta pregunta, traduce internamente para contexto
Oráculo: "The abyss swallows your fear with each step too far." (inglés)
```

### El jugador entiende que:
- El Oráculo habla en "lengua antigua"
- No es un bug, es diseño
- Suma misterio y atmósfera
- Puede entender el sentido general (inglés básico)

### Beneficio adicional
Los jugadores que no hablan inglés perfectamente:
- Sienten el misterio más fuerte
- La ambigüedad es **intencional**
- Perfecto para profecías crípticas

---

## 📝 DOCUMENTACIÓN PARA JUGADORES

### En README.md (sección Oracle)
```markdown
## 🔮 The Oracle System

The Oracle speaks in an ancient tongue, unchanged by mortal translations.
Its words echo from a time before the corruption, preserved in their 
original form. The Oracle's cryptic responses are intentionally mysterious,
speaking in riddles and prophecies rather than direct answers.

This is not a limitation - it is by design. The Oracle is the voice of 
the dungeon itself, and its language predates the common tongues of men.
```

### En opciones del juego (si se implementa UI)
```
[ ] Enable Oracle Responses
    The Oracle speaks in the ancient tongue (English).
    Its cryptic words add atmosphere and mystery.
```

---

## 🚫 LO QUE NO HAREMOS

### 1. NO traducción automática
```
❌ Google Translate / DeepL del output
❌ Prompts multilingües
❌ "Detectar idioma del jugador"
```

### 2. NO explicaciones técnicas
```
❌ "Oracle only supports English due to technical limitations"
❌ "Translation coming soon"
❌ "Language not supported"
```

### 3. NO opciones de idioma para el Oráculo
```
❌ "Oracle Language: [English/Spanish/French]"
```

---

## ✅ LO QUE SÍ HAREMOS

### 1. Presentación lore-friendly
```
✅ "The Oracle speaks in the ancient tongue"
✅ "Unchanged by mortal translations"
✅ "Preserved in its original form"
```

### 2. Documentación clara
```
✅ README explica la decisión
✅ Primera respuesta avisa sutilmente
✅ Es una feature, no un bug
```

### 3. Variaciones de estilo (futuro)
```
✅ Inglés neutral (actual)
✅ Inglés arcaico (opcional)
✅ Inglés eclesiástico (opcional)
```

---

## 🎯 VEREDICTO FINAL

### Decisión Firme
**🔮 El Oráculo habla SOLO en inglés canónico**

### Justificación
- 🧠 Lore auténtico
- 📜 Tono correcto
- 🧊 Cache único
- ⚙️ Sin complejidad extra
- 🎭 Suma atmósfera

### Estado
✅ **IMPLEMENTADO CORRECTAMENTE DESDE EL INICIO**

No requiere cambios. El sistema ya está diseñado así.

---

## 📚 REFERENCIAS

### Precedentes en Gaming
Juegos que usan "lenguas antiguas" sin traducir:
- **Dark Souls**: Mensajes en inglés independiente del idioma
- **Elden Ring**: Runas y textos antiguos sin traducir
- **Bloodborne**: Notas en inglés arcaico
- **Diablo I original**: Libros en inglés formal

### Beneficio comprobado
Los jugadores **valoran** el misterio de lenguas antiguas.
No es una barrera, es **inmersión**.

---

**Conclusión**: Esta decisión es correcta técnica, artística y narrativamente.
El Oráculo permanecerá en inglés canónico. Inmutable. Como debe ser.
