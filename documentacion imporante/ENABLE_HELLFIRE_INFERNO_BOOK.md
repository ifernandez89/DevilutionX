# 🔥 SOLUCIÓN: Habilitar Libro Inferno de Hellfire

## Problema Identificado
El libro Inferno está completamente implementado en DevilutionX Enhanced, pero **Hellfire no está habilitado** por defecto. Por eso el libro no aparece en tu inventario cuando cargas el save de Hellfire.

## ✅ Solución 1: Habilitar Hellfire (Recomendado)

### Método A: Línea de comandos
Ejecuta DevilutionX Enhanced con el parámetro `--hellfire`:

```bash
devilutionx_enhanced.exe --hellfire
```

### Método B: Crear acceso directo
1. Crea un acceso directo al ejecutable
2. En las propiedades, añade `--hellfire` al final del destino:
   ```
   "C:\ruta\a\devilutionx_enhanced.exe" --hellfire
   ```

### Método C: Modificar configuración
Crea/edita el archivo `diablo.ini` en tu carpeta de configuración y añade:

```ini
[Mods]
Hellfire=1
```

## ✅ Solución 2: Forzar habilitación de Hellfire en código

Si quieres que Hellfire esté siempre habilitado, podemos modificar el código:

### Archivo: Source/diablo.cpp
En la función de inicialización, forzar `gbIsHellfire = true`:

```cpp
// En la línea ~1261, después de:
bool enableHellfire = forceHellfire || wasHellfireDiscovered;

// Añadir:
enableHellfire = true; // 🔥 FORZAR HELLFIRE SIEMPRE HABILITADO
```

## ✅ Verificación

Una vez habilitado Hellfire, deberías ver:
- ✅ El libro Inferno aparece en tu inventario
- ✅ Spells de Hellfire disponibles (Immolation, Lightning Wall, etc.)
- ✅ Items de Hellfire funcionando
- ✅ NPCs de Hellfire (Farmer, Girl, etc.)

## 📋 Datos del Spell Inferno

Según la configuración encontrada:
- **Nivel de libro**: 3 (aparece en libros de nivel 3+)
- **Nivel de staff**: 2 (aparece en staffs de nivel 2+)
- **Inteligencia mínima**: 20
- **Costo de mana**: 11
- **Tipo**: Fire, Targeted
- **Missile**: InfernoControl

## 🎯 Recomendación

**Usa el Método A** (línea de comandos con `--hellfire`) ya que es la forma más limpia y no requiere modificar código. Una vez que ejecutes el juego con Hellfire habilitado, tu save debería cargar correctamente con el libro Inferno visible en el inventario.