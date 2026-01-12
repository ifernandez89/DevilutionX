# 🔥 NIGHTMARE UI ARCHITECTURE - FASE 2
## FADE INFERNAL Y FONDO ANIMADO

### ✅ FASE 1 COMPLETADA:
- ✅ Estructura base del sistema de capas
- ✅ Compilación exitosa
- ✅ Ejecutable funcionando perfectamente
- ✅ Sistema de logging implementado
- ✅ Estado global Nightmare UI

### 🎯 FASE 2 OBJETIVOS:
1. **Fade Infernal**: Efectos de transición suaves
2. **Fondo Animado**: Imagen personalizada del usuario en menú principal
3. **Integración con Title Screen**: Reemplazar imagen de fondo
4. **Alpha Blending Real**: Efectos visuales modernos

### 🔧 IMPLEMENTACIÓN TÉCNICA:

#### 1. FADE INFERNAL
```cpp
// Fade suave para transiciones de pantalla
void StartNightmareFade(bool fadeIn, float duration = 1.0f);
void UpdateFadeEffect(float deltaTime);
void RenderFadeOverlay(const Surface &out);
```

#### 2. FONDO ANIMADO PERSONALIZADO
```cpp
// Sistema de fondo animado con la imagen del usuario
void LoadCustomTitleBackground();
void UpdateBackgroundAnimation();
void RenderCustomBackground(const Surface &out);
```

#### 3. INTEGRACIÓN CON TITLE SCREEN
- Modificar `Source/DiabloUI/title.cpp`
- Usar imagen PNG personalizada del usuario
- Mantener compatibilidad con sistema original

### 🎨 MEJORAS VISUALES NIGHTMARE:
- **Fade Duration**: 1.5 segundos (atmósfera lenta)
- **Background Animation**: 16 frames, 120ms por frame
- **Alpha Blending**: Transiciones suaves
- **Color Palette**: Tonos más oscuros para atmósfera Nightmare

### 📁 ARCHIVOS A MODIFICAR:
- `Source/ui_nightmare.cpp` - Implementación de efectos
- `Source/DiabloUI/title.cpp` - Integración con menú principal
- `assets/ui_art/custom_title.png` - Imagen personalizada del usuario

### 🧪 TESTING:
- Verificar fade effects en transiciones
- Probar imagen personalizada en menú principal
- Confirmar compatibilidad con sistema original
- Validar rendimiento sin impacto en gameplay