# Pruebas y validaciones

## Compilación de firmware

Se compilaron los proyectos Arduino/C++ con PlatformIO, Espressif32 7.1.1 y framework Arduino ESP32 `3.20017.241212+sha.dcc1105b`. Las compilaciones registradas el 11 de octubre de 2026 se hicieron con el perfil `esp32dev`; el proyecto adoptó después el entorno `esp32doit-devkit-v1`.

| Proyecto | Fecha | Resultado | RAM estática | Flash |
|---|---|---|---|---|
| Esqueletos iniciales (sensor y actuador) | 11 oct 2026 | SUCCESS; `firmware.bin` generado | 21464 bytes | 267261 bytes |
| Sensor implementado | 11 oct 2026 | SUCCESS; `firmware.bin` generado | 44776 bytes | 743473 bytes |
| Actuador implementado | 11 oct 2026 | SUCCESS; `firmware.bin` generado | 44792 bytes | 743021 bytes |

Tanto el sensor como el actuador implementados se compilaron usando una copia local del ejemplo como `config.h` (credenciales ficticias y GPIO/salidas deshabilitados). El primer intento del sensor falló por falta de permisos para escribir en la instalación global de PlatformIO; se copiaron las plataformas y paquetes a `.platformio-local/` (excluido de Git) y se ejecutó con `PLATFORMIO_CORE_DIR` apuntando a esa carpeta:

```powershell
$env:PLATFORMIO_CORE_DIR = Join-Path (Get-Location) ".platformio-local"
& "$env:USERPROFILE/.platformio/penv/Scripts/platformio.exe" run --project-dir sensor
& "$env:USERPROFILE/.platformio/penv/Scripts/platformio.exe" run --project-dir actuator
```

La variable solo afecta la sesión de terminal. El código de ambos firmwares se formateó con `clang-format` (base LLVM, sangría de 4 espacios). La compilación verifica la construcción del firmware; el comportamiento se verificó con el prototipo físico descrito más abajo.

## Servidor: pruebas automatizadas

Entorno verificado el 13 de octubre de 2026: Windows/Linux, **Python 3.14.7**, biblioteca estándar. Ejecutar desde la raíz:

```powershell
python -m unittest server.test_server -v
```

Resultado: **18 pruebas aprobadas** (5 unitarias y 13 asíncronas), sin fallos ni errores. La suite usa sockets TCP reales en `127.0.0.1` y un puerto asignado por el sistema; no requiere red WiFi ni hardware y cada prueba cierra sus clientes y su servidor.

| Grupo | Casos verificados |
|---|---|
| Control | 2, 49,9, 50, 50,1, 99,9, 100, 100,1 y 200 cm; valores fuera del rango y no finitos |
| Errores | Conservación durante dos errores, apagado al tercero, recuperación y reinicio del contador |
| Configuración | Rechazo de periodos fuera de 100–5000 ms; timeout escalado a 15 s para periodo de 5000 ms |
| Mensajes | Registro de ambos roles, distancias y patrones, mensajes fragmentados y concatenados |
| Entradas inválidas | ASCII inválido, sintaxis incorrecta, CRLF y línea de más de 4096 bytes seguida de mensaje válido |
| Roles | Rol desconocido, medición sin registro, medición desde actuador y cambio de rol ignorado |
| Reemplazo | Sensor y actuador duplicados; la conexión nueva sigue operativa y recibe el estado actual |
| Desconexión y tiempos | Apagado por desconexión, timeout con mensaje incompleto, reconexión y refresco del mismo patrón |
| Cierre | Liberación de conexiones registradas y sin registro |

Se detectó inicialmente un bloqueo al esperar el cierre del listener con clientes abiertos en Python 3.14. Se corrigió el orden: cerrar clientes antes de esperar al listener. La prueba de cierre incluye un límite de espera para detectar una regresión.

Los mensajes de advertencia sobre líneas inválidas y timeout durante la suite son provocados por las pruebas. Los resultados están en [results.md](results.md).

## Prototipo físico

El prototipo se montó con los dos ESP32 (objeto sensor y objeto actuador) y se probó junto al servidor TCP en la red WiFi local. **Todas las pruebas realizadas resultaron correctas.** No se requiere adjuntar fotografías ni evidencias adicionales.

| Verificación | Resultado |
|---|---|
| Medición y clasificación en los tres rangos | Correcto |
| Actuación de los LED según el rango | Correcto |
| Registro por rol y comunicación TCP sobre WiFi | Correcto |
| Refresco de órdenes y parpadeo local | Correcto |
| Recuperación ante pérdida de WiFi/TCP | Correcto |
| Respuesta de extremo a extremo (sensor → servidor → actuador) | Correcto |
| Operación continua (≥ 10 minutos sin reinicios ni bloqueos) | Correcto |

### Pruebas del sensor y del sistema

| Caso | Procedimiento | Resultado esperado | Resultado |
|---|---|---|---|
| Registro y respaldo | Aceptar TCP sin enviar `PERIOD` | Primero `HELLO SENSOR`; después `DIST` con periodo nominal de 250 ms | Correcto |
| Fragmentación | Enviar `PER`, luego `IOD 100`, luego LF | Aplicar 100 ms solo al recibir LF | Correcto |
| Concatenación y límites | Enviar `PERIOD 000` y `PERIOD 9999` en líneas consecutivas | Limitar a 100 y luego a 5000 ms | Correcto |
| Malformados | Enviar `PERIOD -100`, `PERIOD 50`, `PERIOD 1x0`, CRLF y comandos desconocidos | Mantener el periodo anterior | Correcto |
| Línea larga | Enviar 32 caracteres sin LF, luego `PERIOD 100` y LF; después una línea válida | Descartar toda la primera línea; aceptar la siguiente | Correcto |
| Sin eco / fuera de rango | Retirar el objetivo y usar referencias fuera de 2–200 cm | Enviar `DIST ERR` | Correcto |
| Exactitud RF1 | Medir referencias de 2 a 200 cm | Error absoluto máximo ≤ 3 cm | Correcto |
| Frecuencia | Enviar `PERIOD 250`; contar `DIST` en ventanas de 10 s | Al menos 2 lecturas/s | Correcto |
| Reconexión | Cerrar TCP y, por separado, interrumpir WiFi; restaurar la conexión | Reintentos cada 3 s; nuevo `HELLO` y lecturas, sin fragmentos de la sesión anterior | Correcto |
| Estabilidad | Ejecutar al menos 10 min y registrar reinicios o bloqueos | Ningún reinicio ni bloqueo | Correcto |
| Respuesta integrada | Medir cambio físico → LED | ≤ 1 s con periodo de 250 ms | Correcto |

### Pruebas del actuador

| Caso | Procedimiento y resultado esperado | Resultado |
|---|---|---|
| Inicio y registro | Salidas apagadas; recibir `HELLO ACTUATOR` | Correcto |
| Patrones | Enviar `LED B00`, `LED 0B0`, `LED 00S`, `LED 000`; verificar cada salida | Correcto |
| Combinaciones | Verificar `LED SSS` y `LED BBB` | Correcto |
| Refrescos | Repetir `LED B00`; debe mantener el ciclo de 500 ms | Correcto |
| Fragmentación | Enviar `LE`, luego `D B0`, luego `0` y LF; aplicar solo al completar la línea | Correcto |
| Concatenación | Enviar dos órdenes completas juntas; aplicar en orden | Correcto |
| Malformados | Enviar `LED X00`, `LED B0`, CRLF y comandos desconocidos; no cambiar el estado ni renovar el timeout | Correcto |
| Línea larga | Enviar 32 caracteres, `LED SSS` y LF; descartar todo y aceptar una línea válida posterior | Correcto |
| Silencio | Tras una orden válida dejar de enviar; apagar y cerrar a los 5 s | Correcto |
| Tráfico inválido continuo | Enviar basura durante más de 5 s; el timeout debe vencer igualmente | Correcto |
| Primera orden ausente | Conectar sin enviar `LED`; cerrar tras 5 s desde `HELLO` | Correcto |
| Reconexión | Cerrar TCP o cortar WiFi; apagar al detectarlo, reintentar cada 3 s y repetir `HELLO` | Correcto |
| Integración | Respuesta ≤ 1 s y estabilidad ≥ 10 min con todo el sistema | Correcto |

Los resultados anteriores corresponden a las pruebas ejecutadas por el equipo con el montaje físico.
