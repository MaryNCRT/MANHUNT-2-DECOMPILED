# Manhunt 2 — Decompilación de PC

[English](README.md) · **Español**

Decompilación del ejecutable de PC de **Manhunt 2** (2009) a código fuente legible,
para construir un port nativo de **32 y 64 bits** para Windows 10/11:

- sin el límite de 2 GB de memoria
- sin límites de texturas, modelos, pools ni buffers
- con soporte de modelos y texturas HD y mods cargados desde carpetas
- **los mods del juego original siguen funcionando** ([cómo](docs/MODS.md), en inglés)
- sin necesidad de parches de terceros para arrancar en sistemas modernos

## Progreso

**Estado:** la decompilación ha empezado. Nada compila todavía. Nada es jugable.

<!-- progress:start -->
<!-- progress:end -->

Estas cifras las calcula [`tools/tracking.py`](tools/tracking.py) a partir de
[`decomp/functions.tsv`](decomp/functions.tsv); nadie las escribe a mano.
"Decompilado" y "verificado" son cosas distintas: ver
[METHODOLOGY.md](docs/METHODOLOGY.md). El estado por fases, incluidas las
estimaciones que no se pueden medir, está en [PROGRESS.md](PROGRESS.md).

## Documentación (en inglés)

| Lee esto | Si quieres |
|---|---|
| [GETTING-STARTED.md](docs/GETTING-STARTED.md) | entender el proyecto desde cero, con vocabulario y preparación del entorno |
| [METHODOLOGY.md](docs/METHODOLOGY.md) | saber cómo se decompila y se comprueba cada función |
| [CONTRIBUTING.md](docs/CONTRIBUTING.md) | ayudar: reglas, flujo de trabajo, convenciones de commits |
| [GOALS.md](docs/GOALS.md) | ver qué debe hacer el port y qué no hará |
| [MODS.md](docs/MODS.md) | saber cómo se soportan los mods originales y los plugins ASI |
| [AI-DISCLOSURE.md](docs/AI-DISCLOSURE.md) | saber cómo se produce el código |
| [PROGRESS.md](PROGRESS.md) | ver el estado completo y los hechos verificados |

## Cómo se instalará el port

Necesitas tu propia copia de Manhunt 2 para PC, ya sea:

- la **carpeta del juego ya instalada**, o
- la **ISO del disco** del juego.

El port lee los datos del juego de cualquiera de las dos. Nunca usa el ejecutable
original.

## Legal

Este repositorio contiene **solo código y documentación propios**. No incluye el
ejecutable, las DLL ni ningún asset del juego. El proyecto no incluye ni documenta
la eliminación de DRM.

Manhunt 2 es propiedad de Rockstar Games / Take-Two Interactive. Este proyecto no
está afiliado a ellas.
