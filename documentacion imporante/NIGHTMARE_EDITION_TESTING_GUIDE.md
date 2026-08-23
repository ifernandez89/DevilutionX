# 🌙 NIGHTMARE EDITION - GUÍA DE TESTING

## 🎮 EJECUTABLES DISPONIBLES

- **`devilutionx_nightmare_edition.exe`** - Versión completa con todas las features

## 🔥 FEATURES IMPLEMENTADAS Y HABILITADAS

### ✅ 1. LIGHTING ATMOSFÉRICO
**Estado**: HABILITADO (archivo: `nightmare_config/enable_nightmare_lighting`)
**Qué hace**:
- Antorchas parpadean sutilmente (±2 píxeles de radio)
- Diferentes tipos de luz: antorchas, velas, fuego, magia
- Timing orgánico, no mecánico
- Performance O(1), sin impacto en FPS

**Cómo testear**:
1. Ve al town y observa las antorchas
2. Entra a dungeons y nota las variaciones de luz
3. Las luces deberían tener parpadeo sutil y orgánico

### ✅ 2. SISTEMA DE CONFIGURACIÓN MODULAR
**Estado**: HABILITADO
**Qué hace**:
- Sistema de archivos para activar/desactivar features
- Verificación automática cada 5 segundos
- Configuración granular por feature

**Archivos de configuración**:
- `nightmare_config/enable_nightmare_lighting` - Lighting atmosférico
- `nightmare_config/enable_nightmare_ambience` - Ambientación completa
- `nightmare_config/README.txt` - Documentación completa

### ✅ 3. AMBIENTACIÓN COMPLETA
**Estado**: HABILITADO (archivo: `nightmare_config/enable_nightmare_ambience`)
**Qué hace**:
- Renombra NPCs: Griswold → "Grimforge the Cursed Smith"
- Renombra lugares: Tristram → "Shadowhaven"
- Renombra items: "Short Sword" → "Nightmare Blade"

**Mapeos implementados**:
- **NPCs**: Griswold, Pepin, Ogden, Farnham, Adria, Cain, Gillian, Wirt
- **Lugares**: Tristram, Cathedral, Catacombs, Caves, Hell
- **Items**: Espadas, armaduras, anillos, pociones

### ✅ 4. MORIBUNDO SIEMPRE PRESENTE
**Estado**: HABILITADO PERMANENTEMENTE
**Qué hace**:
- El Moribundo (Wounded Townsman) aparece siempre frente a la catedral
- Independientemente del estado de la quest del Butcher
- Garantiza acceso a información sobre el Butcher

## 🐄 PROBLEMA CON LAS VACAS ADICIONALES

**Diagnóstico**: Las 3 vacas adicionales están configuradas correctamente en el archivo TSV, pero solo aparecen en **Hellfire**, no en Diablo clásico.

**Solución**: Para ver las 9 vacas totales, necesitas ejecutar el juego en modo Hellfire.

## 🎯 PRÓXIMAS FEATURES A IMPLEMENTAR

### 📁 4. EXPLORACIÓN POST-DIABLO
- Nivel secreto 17: "The Void"
- NPCs ocultos con quests especiales
- Portales nightmare post-Diablo
- Boss secreto: "Diablo's Shadow"

### 🎭 5. QUESTS POST-DIABLO
- "Cleanse the Nightmare"
- "Collect Soul Fragments" 
- "The Last Stand"
- "Nightmare Artifacts"
- "The True Evil"

## 🔧 CÓMO HABILITAR/DESHABILITAR FEATURES

**Para habilitar una feature**:
1. Crea un archivo vacío en `nightmare_config/` con el nombre correspondiente
2. El sistema detectará el archivo automáticamente en ~5 segundos

**Para deshabilitar una feature**:
1. Elimina el archivo correspondiente de `nightmare_config/`
2. El sistema deshabilitará la feature automáticamente

**Archivos disponibles**:
- `enable_nightmare_lighting` - Lighting atmosférico
- `enable_post_diablo` - Contenido post-Diablo
- `enable_nightmare_items` - Items únicos temáticos
- `enable_nightmare_ambience` - Ambientación completa
- `enable_secret_levels` - Niveles secretos
- `enable_nightmare_quests` - Quests post-Diablo
- `enable_enhanced_audio` - Audio mejorado
- `enable_visual_effects` - Efectos visuales
- `enable_debug_features` - Features de debug

## 🧪 TESTING CHECKLIST

### Lighting Atmosférico
- [ ] Antorchas en town parpadean sutilmente
- [ ] Luces en dungeons tienen variación orgánica
- [ ] No hay impacto en performance/FPS
- [ ] Diferentes tipos de luz se comportan diferente

### Ambientación
- [ ] NPCs tienen nombres nightmare (si está habilitado)
- [ ] Lugares tienen nombres más tenebrosos
- [ ] Items tienen nombres temáticos

### Moribundo
- [ ] Aparece siempre frente a la catedral
- [ ] Funciona independientemente de quest del Butcher

### Sistema de Configuración
- [ ] Crear/eliminar archivos habilita/deshabilita features
- [ ] Cambios se detectan automáticamente
- [ ] README.txt se genera correctamente

## 🚀 ESTADO ACTUAL

**Completado**: 
- ✅ Lighting Atmosférico
- ✅ Sistema de Configuración Modular  
- ✅ Ambientación Completa
- ✅ Moribundo Siempre Presente

**En desarrollo**:
- 🔄 Exploración Post-Diablo
- 🔄 Quests Post-Diablo
- 🔄 Niveles Secretos

¡El sistema está listo para testing! 🎮