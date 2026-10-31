# CLAUDE.md — Manhunt 2 PC Decomp / PC Port

Port nativo de **Manhunt 2 (PC, Rockstar 2009)** a Windows 10/11, en **32 y 64 bits**,
a partir de la decompilación del ejecutable original. Copia obtenida legalmente.

Repo: **https://github.com/MaryNCRT/MANHUNT-2-DESCOMP**

---

## 0. Método: DECOMPILACIÓN (igual que UMK3)

No tenemos el código fuente. Se **decompila el binario a C/C++ legible**,
función por función, con Ghidra (headless + scripts) y **REA** como acelerador
(inventario, dossiers de función, trazado de valores, búsqueda).

El objetivo no es un parche sobre el exe: es **código fuente propio, compilable**,
que sustituya al ejecutable entero, para que los mods no tengan límites.

### Objetivos finales
- Ejecutable nativo **x86 (32 bits)** y **x64 (64 bits)** desde el mismo código
- **Sin límite de 2 GB** (x64 nativo; en x86, LARGE_ADDRESS_AWARE desde el inicio)
- **Sin límites de texturas, pools, modelos, entidades ni buffers** fijos del original
- **Modelos y texturas HD**, carga de mods desde carpetas (sin tocar los archivos originales)
- Funciona en **Windows 10 / 11** sin parches de terceros
- Después: widescreen y FOV correctos, framerate desbloqueado con la lógica a paso fijo,
  mando XInput, ratón sin aceleración, renderer moderno

### Modelo legal (el de UMK3 / BattleShip)
**Cero assets y cero binarios en el repo.** El port es solo código. Cada usuario
aporta su propia copia del juego. Ver `.gitignore`.

**DRM:** el exe retail lleva SecuROM. Este proyecto **no incluye, documenta ni
automatiza la eliminación del DRM.** El análisis usa el ejecutable sin protección
que el usuario ya tenía (ver PROGRESS.md §2). El port terminado no necesita el exe
original para nada: solo los datos del juego.

---

## 1. Autonomía

**Trabaja de forma continua. No pidas aprobación entre pasos.** Si una decisión
tiene una opción razonable por defecto, tómala, anótala en `PROGRESS.md` y sigue.

Detente solo si: (a) algo bloquea de verdad y no hay alternativa, (b) una acción
es destructiva o irreversible, (c) un supuesto verificado resulta ser falso.

Mantén **`PROGRESS.md`** en la raíz, actualizado al terminar cada bloque de
trabajo: qué se hizo, qué se verificó, qué falta, decisiones tomadas y por qué.
Ese archivo es el estado del proyecto. Escríbelo pensando en quien retome esto
sin contexto.

---

## 2. Reglas del repo

- **No modificar** `MH2GAME\` ni `ORIGINAL\`: son las fuentes originales.
- Código decompilado legible → `decomp/`
- Código nativo escrito a mano (plataforma, render, input, audio) → `runtime/`
- Stubs de lo que se elimina (SecuROM, telemetría, etc.) → `runtime/stubs/`
- Tests y comparadores → `tests/` · Análisis y volcados → `OUTPUT/` (no se versiona)
- Herramientas propias (extractores, scripts) → `tools/` y `OUTPUT/tools/`
- **Todo el código debe compilar en x86 y x64.** Prohibido guardar punteros en
  `int`/`DWORD`; usar `intptr_t`/`uintptr_t`. Structs de archivo con tamaños fijos
  (`uint32_t` etc.) y `static_assert` del tamaño, nunca punteros dentro.

---

## 3. Herramientas

| Herramienta | Ubicación | Uso |
|---|---|---|
| Ghidra 12.1.2 + JDK 21 | `TOOLS\ghidra_12.1.2_PUBLIC` · `C:\Program Files\Microsoft\jdk-21.0.12.8-hotspot` | Decompilador |
| GhidraMCP | `TOOLS\GhidraMCP` | Ghidra por MCP (GUI) |
| REA 6.3.0 | `E:\rea-rea-agents-6.3.0` (fuente) · `TOOLS\rea` (paquete con addon nativo) | MCP/CLI de RE |
| `peinfo.py` | `OUTPUT\tools` | Cabeceras, secciones, imports, marcadores DRM |
| `cmp_text.py` | `OUTPUT\tools` | Diferencias por sección entre dos PE |
| `ExportInventory.java` | `OUTPUT\tools` | Ghidra → `functions.tsv` / `strings.tsv` |
| `DecompileList.java` | `OUTPUT\tools` | Ghidra → C de una lista de funciones (con firmas/structs) |
| Proyecto Ghidra | `OUTPUT\ghidra\MH2.gpr` | Binario importado y analizado |

---

## 4. Hechos verificados (NO re-descubrir)

Ver `PROGRESS.md §2`. Resumen:
- PE32 i386, MSVC 8.0 (VS2005), ImageBase `0x400000`, TimeDateStamp `0x4992F67C` (2009-02-11)
- PDB original `W:\Manhunt2_PC\Assets\DvdRoot\Manhunt2.pdb`: **no disponible**, binario sin símbolos
- Render **Direct3D 9** (d3dx9_35 + efectos .fx), audio **FMOD Ex + FMOD Event**, vídeo **Bink**,
  input **DirectInput 8**
- `.text` = 0x23649A bytes de código

---

## 5. Estructura

```
decomp/          C/C++ legible decompilado  <- el producto principal
runtime/         capa nativa PC escrita a mano
  platform/      ventana, input (XInput/DInput), timing, ficheros, VFS de mods
  stubs/         lo eliminado (SecuROM, etc.)
tests/           comparación decomp vs original
tools/           extractores y utilidades de assets
OUTPUT/          análisis y volcados (no versionado salvo OUTPUT/tools)
docs/            formatos, límites, notas de arquitectura
```

---

## 6. Fases

Ver la tabla en `PROGRESS.md §1`.
