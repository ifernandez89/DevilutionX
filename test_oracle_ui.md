# 🧪 ORACLE UI - Quick Testing Guide

## 🚀 Compilación
```bash
cmake --build build_NOW -j 4
```

## ✅ Tests Rápidos (5 minutos)

### Test 1: Mensaje de Bienvenida (30 segundos)
1. Iniciar juego
2. **Buscar**: Mensaje en esquina superior izquierda
3. **Texto esperado**: "The Oracle awakens..."
4. **Duración**: 4 segundos con fade

### Test 2: Canal Debug (1 minuto)
1. En juego, NO abrir chat
2. Hacer pregunta al Oráculo (evento de muerte, libro, etc.)
3. **Buscar**: Mensaje en x=10, y=50 (esquina superior izquierda)
4. **Duración**: 4 segundos

### Test 3: Canal Chat (1 minuto)
1. Presionar Enter (abrir chat)
2. Hacer pregunta al Oráculo
3. **Buscar**: Mensaje centrado arriba del panel principal
4. **Duración**: 5 segundos

### Test 4: Fade Visual (1 minuto)
1. Observar cualquier mensaje
2. **Verificar**:
   - Aparece gradualmente (1 segundo)
   - Permanece visible (2-3 segundos)
   - Desaparece gradualmente (1 segundo)

### Test 5: Solo 1 Mensaje (30 segundos)
1. Hacer múltiples preguntas rápido
2. **Verificar**: Solo se ve el último mensaje

## 🐛 Si Algo Falla

### Texto No Visible
- Revisar logs: Buscar "Oracle UI:" en console
- Verificar que `OracleUI::Update()` se llama
- Verificar que `RenderChatMessage/RenderDebugMessage` se llaman

### Posición Incorrecta
- Ajustar coordenadas en `oracle_ui.cpp`:
  - Chat: línea ~127-128
  - Debug: línea ~158-159

### Sin Fade
- Verificar que `Update()` se llama cada frame
- Revisar `alpha` en logs de debug

## 📝 Reportar Resultados
Crear archivo: `ORACLE_UI_TEST_RESULTS_ENERO_16_2026.md`

Incluir:
- ✅/❌ para cada test
- Screenshots si es posible
- Logs de errores si hay
- Ajustes necesarios
