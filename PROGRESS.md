# PROGRESS.md — Manhunt 2 PC Decomp

Estado del proyecto. Escrito para retomarlo sin contexto previo.

**Última actualización:** 2026-10-31

---

## 0. Dónde está todo

| Ruta | Qué es |
|---|---|
| `E:\MANHUNT 2 DESCOMP\` | Repo y entorno de trabajo (remote `MaryNCRT/MANHUNT-2-DESCOMP`). El `.gitignore` bloquea binarios, assets y análisis. |
| `MH2GAME\Manhunt 2\` | Instalación retail del juego. **No modificar.** |
| `ORIGINAL\` | Copias de referencia de los exe (no versionado). |
| `TOOLS\` | Ghidra 12.1.2 y GhidraMCP copiados de `E:\MK3 PROJECT\TOOLS` (no versionado). |
| `E:\rea-rea-agents-6.3.0` | Fuente de REA. |

---

## 1. Estado en una tabla

| Fase | Estado |
|---|---|
| 0 — Análisis del binario y mapeo | 🔄 EN CURSO |
| 1 — Inventario de funciones + identificar CRT/librerías (FID) | 🔄 EN CURSO |
| 2 — Mapa de subsistemas (strings, imports, vtables, RTTI) | ⬜ |
| 3 — Catálogo de límites del original (pools, arrays, texturas, memoria) | ⬜ |
| 4 — Esqueleto de build x86 + x64 (CMake), arranque con stubs | ⬜ |
| 5 — Decompilar núcleo: memoria, ficheros, carga de niveles | ⬜ |
| 6 — Render D3D9 (y texturas/modelos sin límites, HD) | ⬜ |
| 7 — Audio FMOD, vídeo Bink, input | ⬜ |
| 8 — Lógica de juego, IA, scripts de nivel | ⬜ |
| 9 — Mejoras: VFS de mods, widescreen, FPS, XInput, renderer moderno | ⬜ |

**Global: ~1 %.** Nada compilable todavía.

---

## 2. Hechos verificados

### El ejecutable retail lleva SecuROM
`MH2GAME\Manhunt 2\Manhunt2.exe` (9.748.480 B) tiene secciones `.securom`,
`Stext`, `Sdata`… y `.text` con entropía 7,99: **está cifrado**. No se puede
analizar ni decompilar tal cual.

**Decisión:** el proyecto **no rompe ni documenta la eliminación del DRM**. Como
referencia se usa el exe sin protección que el usuario ya tenía en otras
instalaciones (es el que usa toda la escena de mods):

| Copia | SHA-1 | Notas |
|---|---|---|
| `E:\Manhunt 2 remaster\Manhunt 2\Manhunt2.exe` → `ORIGINAL\Manhunt2_nosecu.exe` | `9a7053ada233b3aeb2433509dbeba9c45b086cbc` | **Referencia.** 3.231.744 B, sin LAA, sin parches |
| `E:\mh2\game\Manhunt 2\Manhunt2.exe` | `e272e836d3183a5fd5c8713b6c69b61dfba9dc02` | Igual + flag LAA + 85 B en `.text` y 14 B en `.rdata` parcheados por mods |
| `E:\Manhunt 2\Manhunt2.exe` | `8e6bf178637d8a0c869eb5e431b459126006965e` | Variante parcheada |

Mismo `TimeDateStamp` (`0x4992F67C`) y mismo layout de `.text/.rdata/.data`
que el retail: es el mismo build.

### Cabecera
- PE32 i386, GUI, MSVC **8.0 (VS2005)**, ImageBase `0x400000`, entry `0x61C2A1`
- **`LARGE_ADDRESS_AWARE = False`** → origen del límite de 2 GB
- `DllCharacteristics = 0` (sin ASLR/DEP/NX-compat)
- Secciones: `.text` 0x23649A · `.rdata` 0x4CD38 · `.data` 0x11C024 (VS) / 0x59000 (raw:
  ~1,1 MB de BSS, ahí estarán los pools estáticos) · `.rsrc` · `.secu` (resto de SecuROM)
- PDB: `W:\Manhunt2_PC\Assets\DvdRoot\Manhunt2.pdb` — no disponible. **Binario sin símbolos.**
- Una ruta de fuente sobrevive: `W:\Manhunt2_PC\Code\src\ai\graph.cpp`

### Dependencias (imports)
| DLL | Uso |
|---|---|
| `d3dx9_35.dll` | Solo 6 funciones: efectos (`D3DXCreateEffectEx`/`Pool`) y matrices. `d3d9` se carga dinámicamente |
| `fmodex.dll` (29) + `fmod_event.dll` (14) | Audio: FMOD Ex + Event System |
| `binkw32.dll` (12) | Vídeo |
| `DINPUT8.dll` | Input (solo DirectInput, **sin XInput**) |
| `WSOCK32.dll` | `send/recv` (restos de debug/red; a investigar) |
| `ADVAPI32` (servicios, SID, registro) / `VERSION` | Sobre todo restos de SecuROM; a confirmar |

Todas existen en x64 (D3D9, FMOD Ex, Bink) o se sustituyen; no hay bloqueo para el build de 64 bits.

---

## 3. Decisiones

1. **Ruta de 64 bits:** el mismo código fuente compila x86 y x64. Nada de parches
   sobre el exe para x64. Las reglas de portabilidad están en `CLAUDE.md §2`.
2. **REA:** la build desde fuente (`E:\rea-rea-agents-6.3.0`) no compila su addon
   nativo en Windows (`build:windows-native` exige Linux + MinGW), así que
   `rea doctor` falla en `ghidra-native_authority`. Solución: usar el paquete npm
   `rea-agents@6.3.0`, que trae `rea-windows-x64.node`, instalado en `TOOLS\rea`.
   **Pendiente de permiso del usuario.** Mientras tanto: Ghidra headless.
3. Mods de la comunidad presentes en las otras instalaciones
   (`Manhunt2.NoTexureLimit.asi`, `Mh2Modern.asi`, `Manhunt2.DebugMenu.asi`,
   `Manhunt2.UM2.asi`…) sirven como **pista de qué límites existen y dónde**.
   No se copia su código.

---

## 4. Siguiente trabajo

1. Terminar análisis Ghidra → `OUTPUT\inventory\functions.tsv` / `strings.tsv`
2. Identificar CRT de VS2005, STL y librerías estáticas (no se decompilan)
3. Agrupar funciones por subsistema a partir de cadenas y tablas virtuales
4. Catálogo de límites → `docs/LIMITS.md`
5. Esqueleto CMake x86/x64 → `runtime/`

---

## 5. Reproducir el análisis

```powershell
$env:JAVA_HOME = "C:\Program Files\Microsoft\jdk-21.0.12.8-hotspot"
TOOLS\ghidra_12.1.2_PUBLIC\support\analyzeHeadless.bat OUTPUT\ghidra MH2 `
    -import ORIGINAL\Manhunt2_nosecu.exe -overwrite `
    -scriptPath OUTPUT\tools -postScript ExportInventory.java OUTPUT\inventory
python OUTPUT\tools\peinfo.py ORIGINAL\Manhunt2_nosecu.exe
```
