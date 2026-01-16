# 🌙 NIGHTMARE EDITION - Cambios Implementados
## Fecha: Enero 16, 2026

---

## ✅ CAMBIOS REALIZADOS

### 1. 🎬 **CINEMÁTICA DE LA ESPADA - HABILITADA PERMANENTEMENTE**
**Archivo:** `Source/diablo.cpp` - Función `DiabloSplash()`

**Cambio:**
- ❌ **ANTES:** La intro se mostraba solo una vez (configuración "Once")
- ✅ **AHORA:** La intro se muestra SIEMPRE al iniciar el juego

**Código modificado:**
```cpp
void DiabloSplash()
{
	if (!gbShowIntro)
		return;

	if (*GetOptions().StartUp.splash == StartUpSplash::LogoAndTitleDialog)
		play_movie("gendata\\logo.smk", true);

	// NIGHTMARE EDITION: Always show the legendary sword intro!
	// Force intro to play every time for the epic experience
	if (gbIsHellfire)
		play_movie("gendata\\Hellfire.smk", true);
	else
		play_movie("gendata\\diablo1.smk", true);
	
	// Note: Removed the "Once" check to make intro permanent
```

**Resultado:**
- 🎥 La mítica cinemática de la espada se reproduce SIEMPRE
- 🔥 Experiencia épica garantizada en cada inicio
- ⚔️ No se puede deshabilitar (excepto con flag `-n` en línea de comandos)

---

### 2. 💬 **CHAT EN SINGLE PLAYER - HABILITADO**
**Archivo:** `Source/control/control_chat.cpp` - Función `IsChatAvailable()`

**Cambio:**
- ❌ **ANTES:** Chat solo disponible en multiplayer (`return gbIsMultiplayer`)
- ✅ **AHORA:** Chat disponible SIEMPRE (single y multiplayer)

**Código modificado:**
```cpp
bool IsChatAvailable()
{
	// NIGHTMARE EDITION: Enable chat in single player for notes and commands
	return true; // Previously: gbIsMultiplayer
}
```

**Resultado:**
- 💬 Chat funcional en modo single player
- 📝 Permite tomar notas durante el juego
- 🎮 Comandos de chat disponibles en todo momento
- 🔧 Útil para testing y debugging

---

### 3. 🌀 **PORTAL ROJO - VERIFICADO Y DOCUMENTADO**
**Archivo:** `Source/interfac.cpp` - Función `LoadCutsceneBackground()`

**Estado:**
- ✅ **YA IMPLEMENTADO** correctamente
- 🎨 Cinemática: `gendata\\cutportr.pal` y `gendata\\cutportrw.clx`
- 🔴 Se activa en `SL_VILEBETRAYER` (Vile Betrayer level)

**Código verificado:**
```cpp
case CutPortalRed:
	// NIGHTMARE EDITION: Red Portal Cinematic
	ArtCutsceneWidescreen = LoadOptionalClx("gendata\\cutportrw.clx");
	celPath = "gendata\\cutportr";
	palPath = "gendata\\cutportr.pal";
	progress_id = 1;
	break;
```

**Resultado:**
- 🌀 Portal rojo funcional en el código
- ⚠️ **NOTA:** Requiere archivos de assets en `gendata/` para visualizarse
- 🎮 Se activa automáticamente en el nivel correspondiente

---

## 📋 RESUMEN DE FEATURES

| Feature | Estado | Archivo | Línea |
|---------|--------|---------|-------|
| 🎬 Cinemática Espada | ✅ PERMANENTE | `diablo.cpp` | ~1431 |
| 💬 Chat Single Player | ✅ HABILITADO | `control_chat.cpp` | ~309 |
| 🌀 Portal Rojo | ✅ IMPLEMENTADO | `interfac.cpp` | ~196 |
| 📚 CMakeLists.txt | ✅ ACTUALIZADO | `CMakeLists.txt` | ~20-21 |

---

## 🔧 ARCHIVOS MODIFICADOS

1. **Source/diablo.cpp**
   - Función `DiabloSplash()` - Intro permanente

2. **Source/control/control_chat.cpp**
   - Función `IsChatAvailable()` - Chat en single player

3. **Source/interfac.cpp**
   - Función `LoadCutsceneBackground()` - Portal rojo documentado

4. **Source/CMakeLists.txt**
   - Agregados: `guarantee_inferno_book.cpp` y `hellfire_book_fix.cpp`

---

## ⚠️ NOTAS IMPORTANTES

### Sobre la Cinemática de la Espada:
- La intro se reproduce desde `gendata\diablo1.smk` (Diablo) o `gendata\Hellfire.smk` (Hellfire)
- Si los archivos `.smk` no existen, la intro se saltará automáticamente
- El flag `-n` en línea de comandos aún puede deshabilitar la intro

### Sobre el Portal Rojo:
- El código está implementado correctamente
- Requiere archivos de assets: `cutportr.cel` y `cutportr.pal` en carpeta `gendata/`
- Si los archivos no existen, usará fallback a cinemática estándar

### Sobre el Chat:
- Ahora funciona en single player
- Útil para comandos de debug y notas personales
- No afecta el gameplay normal

---

## 🎮 PRÓXIMOS PASOS

1. **Recompilar el proyecto:**
   ```bash
   cmake -B build_NOW -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DNONET=ON
   cmake --build build_NOW -j 4
   ```

2. **Verificar archivos de assets:**
   - Confirmar que existen `gendata/diablo1.smk` o `gendata/Hellfire.smk`
   - Confirmar que existen `gendata/cutportr.cel` y `gendata/cutportr.pal`

3. **Testing:**
   - Iniciar el juego y verificar que la intro se reproduce
   - Probar el chat en single player (tecla Enter)
   - Llegar al nivel Vile Betrayer para ver el portal rojo

---

## 🏆 NIGHTMARE EDITION - READY FOR EPIC GAMING!

**Versión:** Enero 16, 2026  
**Estado:** ✅ Cambios aplicados, pendiente compilación  
**Próximo:** Compilar y testear
