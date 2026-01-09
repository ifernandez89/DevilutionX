# 🎮 GUÍA DE TESTING COMPLETO - ENERO 2026

## 🎯 OBJETIVO DEL TESTING

Verificar que **todas las 15 features implementadas** funcionan correctamente y que el juego mantiene estabilidad y performance óptima.

---

## 📋 CHECKLIST DE TESTING

### ✅ **PREPARACIÓN**
- [ ] Ejecutable funciona: `build_NOW/devilutionx.exe`
- [ ] Audio funciona correctamente
- [ ] No crashes al iniciar
- [ ] Menús principales responden

---

## 🎨 **FASE V2 - PALETA CONTEXTUAL** (PRIORIDAD MÁXIMA)

### **🏠 V2.1 - TESTING DE TINTE POR BIOMA**

**Qué probar**: Cada bioma debe tener su propia personalidad visual

#### **Test 1: Town (Pueblo)**
- [ ] **Iniciar en Town**
- [ ] **Observar**: Paleta debe ser **cálida y acogedora**
  - Rojos ligeramente más intensos (+10%)
  - Verdes ligeramente más intensos (+5%)  
  - Azules ligeramente reducidos (-10%)
- [ ] **Sensación esperada**: Hogar, seguridad, calidez

#### **Test 2: Cathedral (Niveles 1-4)**
- [ ] **Entrar a Cathedral** (cualquier nivel 1-4)
- [ ] **Observar**: Paleta debe ser **fría con corrupción roja**
  - Rojos intensificados (+20%) - corrupción
  - Verdes reducidos (-20%)
  - Azules reducidos (-30%)
- [ ] **Sensación esperada**: Corrupción religiosa, frialdad, peligro

#### **Test 3: Catacombs (Niveles 5-8)**
- [ ] **Bajar a Catacombs** (niveles 5-8)
- [ ] **Observar**: Paleta debe ser **azul-gris húmeda**
  - Rojos reducidos (-30%)
  - Verdes reducidos (-20%)
  - Azules intensificados (+30%)
  - Ligera desaturación
- [ ] **Sensación esperada**: Muerte, frialdad subterránea, humedad

#### **Test 4: Caves (Niveles 9-12)**
- [ ] **Bajar a Caves** (niveles 9-12)
- [ ] **Observar**: Paleta debe ser **verde-amarilla corrupta**
  - Rojos ligeramente reducidos (-10%)
  - Verdes intensificados (+20%)
  - Azules muy reducidos (-40%)
  - Alta saturación
- [ ] **Sensación esperada**: Naturaleza tóxica, corrupción orgánica

#### **Test 5: Hell (Niveles 13-16)**
- [ ] **Bajar a Hell** (niveles 13-16)
- [ ] **Observar**: Paleta debe ser **roja intensa infernal**
  - Rojos muy intensificados (+50%)
  - Verdes muy reducidos (-40%)
  - Azules muy reducidos (-60%)
- [ ] **Sensación esperada**: Fuego infernal, calor extremo, peligro máximo

### **🌊 V2.4 - TESTING DE TRANSICIONES SUAVES**

**Qué probar**: Los cambios entre biomas deben ser graduales y naturales

#### **Test 6: Transiciones Town ↔ Cathedral**
- [ ] **Salir y entrar de Town varias veces**
- [ ] **Observar**: Transición debe durar ~2 segundos
- [ ] **Verificar**: No hay "saltos" bruscos de color
- [ ] **Curva**: Debe ser suave (ease-in-out)

#### **Test 7: Transiciones entre Dungeons**
- [ ] **Cambiar entre niveles de diferentes biomas**
- [ ] **Observar**: Cathedral → Catacombs → Caves → Hell
- [ ] **Verificar**: Cada transición es suave y natural

### **🌑 V2.2 - TESTING DE OSCURECIMIENTO POR PROFUNDIDAD**

**Qué probar**: Niveles más profundos deben verse progresivamente más oscuros

#### **Test 8: Progresión de Oscurecimiento**
- [ ] **Nivel 1**: Oscurecimiento mínimo
- [ ] **Nivel 5**: Notablemente más oscuro
- [ ] **Nivel 10**: Significativamente más oscuro
- [ ] **Nivel 15**: Máximo oscurecimiento (pero aún jugable)

#### **Test 9: Comparación Directa**
- [ ] **Tomar screenshot en Nivel 1**
- [ ] **Tomar screenshot en Nivel 15**
- [ ] **Comparar**: Diferencia debe ser clara pero no extrema

---

## 🎮 **FASE V3 - VISUAL FEEDBACK SYSTEM**

### **💥 V3.1 - TESTING DE FLASH DE DAÑO**

#### **Test 10: Daño por Tipo**
- [ ] **Recibir daño físico** (monstruo melee)
  - **Observar**: Flash **rojo** (255, 100, 100)
- [ ] **Recibir daño de fuego** (Fireball, lava)
  - **Observar**: Flash **naranja** (255, 150, 50)
- [ ] **Recibir daño de rayo** (Lightning)
  - **Observar**: Flash **azul** (150, 150, 255)
- [ ] **Recibir daño mágico** (hechizos enemigos)
  - **Observar**: Flash **púrpura** (200, 100, 255)

#### **Test 11: Intensidad por Daño**
- [ ] **Recibir poco daño** (1-20 HP)
  - **Observar**: Flash sutil
- [ ] **Recibir mucho daño** (50+ HP)
  - **Observar**: Flash intenso y más duradero

### **✨ V3.2 - TESTING DE PULSE ELITE**

#### **Test 12: Monstruos Elite**
- [ ] **Buscar monstruos con nombre "Corrupted"**
- [ ] **Observar**: Pulse **dorado** continuo (255, 215, 0)
- [ ] **Verificar**: Pulse dura ~3 segundos, se repite
- [ ] **Confirmar**: Solo monstruos elite tienen este efecto

### **🔮 V3.6 - TESTING DE BRILLO DE HECHIZO**

#### **Test 13: Hechizos por Tipo**
- [ ] **Lanzar Firebolt/Fireball/Inferno**
  - **Observar**: Brillo **rojo-naranja** (255, 100, 50)
- [ ] **Lanzar Lightning/Chain Lightning**
  - **Observar**: Brillo **azul** (150, 150, 255)
- [ ] **Lanzar Heal/HealOther**
  - **Observar**: Brillo **verde** (100, 255, 100)
- [ ] **Otros hechizos**
  - **Observar**: Brillo **púrpura mágico** (200, 150, 255)

### **❤️ V3.4 - TESTING DE PULSE DE VIDA BAJA**

#### **Test 14: Advertencia de Vida Baja**
- [ ] **Reducir vida a menos del 25%**
- [ ] **Observar**: Pulse **rojo de advertencia** persistente (255, 50, 50)
- [ ] **Verificar**: Pulse continúa mientras vida esté baja
- [ ] **Curar**: Pulse debe desaparecer al superar 25% de vida

### **💀 V3.7 - TESTING DE FLASH DE MUERTE**

#### **Test 15: Muerte del Jugador**
- [ ] **Morir intencionalmente**
- [ ] **Observar**: Flash **rojo oscuro dramático** (150, 0, 0)
- [ ] **Verificar**: Efecto dura ~2 segundos
- [ ] **Confirmar**: Es el efecto más intenso y duradero

### **⚡ V3.5 - TESTING DE GOLPE CRÍTICO**

#### **Test 16: Golpes Críticos**
- [ ] **Realizar ataques hasta conseguir críticos**
- [ ] **Observar**: Flash **amarillo brillante** (255, 255, 100)
- [ ] **Verificar**: Flash rápido (~400ms)
- [ ] **Confirmar**: Solo aparece en golpes críticos

---

## 🛡️ **TESTING DE ESTABILIDAD**

### **🔥 Test 17: Stress Test con Inferno**
- [ ] **Ir a área con muchos monstruos** (Hell level)
- [ ] **Usar Inferno repetidamente** (spam click)
- [ ] **Verificar**: No crashes
- [ ] **Observar**: Floating numbers limitados (máximo 15)
- [ ] **Confirmar**: Juego mantiene fluidez

### **💬 Test 18: Diálogos con NPCs**
- [ ] **Hablar con Pepin** (el que causaba crashes antes)
- [ ] **Hablar con Griswold**
- [ ] **Hablar con Adria**
- [ ] **Hablar con Cain**
- [ ] **Verificar**: No crashes en ningún diálogo

### **🎯 Test 19: Features de Gameplay**
- [ ] **Verificar todas las quests disponibles** (Guaranteed Quests)
- [ ] **Observar iluminación mejorada** (radio 12 vs 10 original)
- [ ] **Encontrar monstruos "Corrupted"** (Enhanced Elite System)
- [ ] **Verificar escalado por profundidad** (monstruos más fuertes en niveles profundos)

---

## 📊 **TESTING DE PERFORMANCE**

### **Test 20: FPS y Fluidez**
- [ ] **Medir FPS en Town** (baseline)
- [ ] **Medir FPS en Cathedral** (con paleta contextual)
- [ ] **Medir FPS en Hell** (máxima carga visual)
- [ ] **Verificar**: No drops significativos de FPS
- [ ] **Confirmar**: Juego mantiene 60 FPS estables

### **Test 21: Múltiples Efectos Simultáneos**
- [ ] **Activar múltiples efectos a la vez**:
  - Recibir daño (flash)
  - Estar cerca de elite (pulse)
  - Lanzar hechizo (brillo)
  - Vida baja (pulse rojo)
- [ ] **Verificar**: Todos los efectos se muestran correctamente
- [ ] **Confirmar**: No interferencia entre efectos

---

## 🎨 **TESTING DE INTEGRACIÓN VISUAL**

### **Test 22: Combinación V1 + V2 + V3**
- [ ] **Ir a Hell level con vida baja**
- [ ] **Observar**: 
  - Paleta roja intensa (V2)
  - Iluminación inteligente (V1)
  - Pulse de vida baja (V3)
- [ ] **Verificar**: Todos los sistemas trabajan en armonía
- [ ] **Confirmar**: Efecto visual cohesivo y potente

---

## 📝 **REPORTE DE TESTING**

### **Formato de Reporte**
Para cada test, reportar:
- ✅ **PASS**: Funciona como esperado
- ⚠️ **ISSUE**: Funciona pero con problemas menores
- ❌ **FAIL**: No funciona o causa problemas

### **Información Adicional a Reportar**
- **Performance**: ¿Hay drops de FPS?
- **Estabilidad**: ¿Algún crash o freeze?
- **Calidad Visual**: ¿Los efectos se ven bien?
- **Usabilidad**: ¿Mejora o interfiere con el gameplay?

---

## 🎯 **CRITERIOS DE ÉXITO**

### **Mínimo Aceptable**
- ✅ 80% de tests PASS
- ✅ 0 crashes críticos
- ✅ Performance estable (>50 FPS)

### **Éxito Completo**
- ✅ 95% de tests PASS
- ✅ 0 crashes de cualquier tipo
- ✅ Performance óptima (60 FPS estables)
- ✅ Experiencia visual significativamente mejorada

---

## 🚀 **DESPUÉS DEL TESTING**

### **Si Todo Funciona Bien**
- Proceder con **FASE D1 - 3D Simulado**
- Implementar depth cues visuales
- Continuar con el roadmap

### **Si Hay Issues**
- Documentar problemas específicos
- Priorizar fixes críticos
- Ajustar parámetros según feedback
- Re-testing de áreas problemáticas

---

**¡Comienza el testing y reporta los resultados! El sistema está listo para ser probado exhaustivamente.** 🎮✨