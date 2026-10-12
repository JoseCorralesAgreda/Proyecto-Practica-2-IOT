# Pruebas y validaciones

## Compilación inicial — 11 de octubre de 2026

Se compilaron los esqueletos Arduino/C++ con PlatformIO, perfil `esp32dev`, Espressif32 7.1.1 y framework Arduino ESP32 `3.20017.241212+sha.dcc1105b`.

| Proyecto | Resultado | RAM estática informada | Flash informada |
|---|---|---|---|
| Sensor | SUCCESS; firmware.bin generado | 21464 bytes | 267261 bytes |
| Actuador | SUCCESS; firmware.bin generado | 21464 bytes | 267261 bytes |

El primer intento del sensor falló por falta de permiso para escribir `platforms.lock` en la instalación global. Se copiaron las plataformas y paquetes ya instalados a `.platformio-local/`, excluido de Git, y se ejecutaron ambas compilaciones con:

```powershell
$env:PLATFORMIO_CORE_DIR = Join-Path (Get-Location) ".platformio-local"
& "$env:USERPROFILE/.platformio/penv/Scripts/platformio.exe" run --project-dir sensor
& "$env:USERPROFILE/.platformio/penv/Scripts/platformio.exe" run --project-dir actuator
```

La variable solo afecta la sesión de terminal. No se modificó la configuración global. Esta copia permite verificar dentro del entorno restringido; no es necesaria para usar PlatformIO normalmente.

## Compilación del sensor implementado — 11 de octubre de 2026

Se compiló con el mismo perfil y framework, usando una copia local del ejemplo como `config.h`: credenciales ficticias y GPIO deshabilitados. Resultado: **SUCCESS**, `sensor/.pio/build/esp32dev/firmware.bin` generado, RAM estática de **44776 bytes** y flash de **743473 bytes**. Se utilizó el directorio local de PlatformIO con el comando anterior. El código se formateó con `clang-format` (base LLVM, sangría de 4 espacios).

## Alcance y pruebas pendientes

La compilación actual verifica la construcción del firmware del sensor; no ejecuta su comportamiento. No se cargó firmware ni se comprobó salida serie física. La persona confirmó que no hay placas conectadas y solicitó continuar solo con código. No hay pruebas automatizadas de ejecución del firmware.

| Verificación | Estado |
|---|---|
| Carga y salida serie en ambos ESP | PENDIENTE |
| Circuito y compatibilidad eléctrica de ECHO | PENDIENTE |
| Medición y actuación física | PENDIENTE: firmwares funcionales sin implementar |
| Clasificación y comunicación TCP del servidor | Verificadas con clientes simulados; ver sección siguiente |
| Comunicación y recuperación con ESP por WiFi | PENDIENTE: requiere firmwares funcionales |
| Estabilidad, exactitud, respuesta y muestreo | PENDIENTE: requieren sistema integrado y mediciones físicas |

## Servidor: pruebas automatizadas

Entorno: Windows, Python 3.14.7, biblioteca estándar. Ejecutar desde la raíz:

```powershell
python -m unittest server.test_server -v
```

La suite usa cinco pruebas unitarias y trece pruebas asíncronas con sockets TCP reales en `127.0.0.1` y puerto asignado por el sistema. No requiere red WiFi ni hardware. Cada prueba cierra sus clientes y servidor.

| Grupo | Casos verificados |
|---|---|
| Control | 2, 49,9, 50, 50,1, 99,9, 100, 100,1 y 200 cm; valores fuera del rango y no finitos |
| Errores | Conservación durante dos errores, apagado al tercero, recuperación y reinicio del contador |
| Configuración | Rechazo de periodos fuera de 100–5000 ms; timeout escalado a 15 s para periodo de 5000 ms |
| Mensajes | Registro de ambos roles, distancias y patrones, mensajes fragmentados y concatenados |
| Entradas inválidas | ASCII inválido, sintaxis incorrecta, CRLF, línea de más de 4096 bytes seguida de mensaje válido |
| Roles | Rol desconocido, medición sin registro, medición desde actuador y cambio de rol ignorado |
| Reemplazo | Sensor duplicado y actuador duplicado; conexión nueva sigue operativa y recibe estado actual |
| Desconexión y tiempos | Apagado por desconexión, timeout con mensaje incompleto, reconexión y refresco del mismo patrón |
| Cierre | Liberación de conexiones registradas y sin registro |

Se detectó inicialmente un bloqueo al esperar el cierre del listener con clientes abiertos en Python 3.14. Se corrigió el orden: cerrar clientes antes de esperar al listener. La prueba de cierre incluye un límite de espera para detectar una regresión.

Los mensajes de advertencia sobre líneas inválidas y timeout durante la suite son provocados por las pruebas. Los resultados obtenidos están en [results.md](results.md). Estas pruebas no acreditan exactitud del sensor ni respuesta de los LEDs físicos.

## Procedimiento pendiente de aceptación física

Con ambos firmwares implementados: medir posiciones con cinta métrica dentro de 2–200 cm, registrar lecturas repetidas y errores absolutos; comprobar cambios de rango alrededor de 50 y 100 cm; medir del cambio físico a la respuesta visible; contar lecturas por segundo y mantener el sistema al menos diez minutos. Desconectar y reconectar WiFi, sensor y servidor para registrar recuperación. Los datos y condiciones reales se incorporarán al ejecutarse estas pruebas.
| Medición | PENDIENTE: firmware implementado, sin validación física |
| Clasificación y actuación | PENDIENTE: componentes sin implementar |
| Comunicación TCP y recuperación de fallas | PENDIENTE: cliente sensor implementado, sin ejecución en placa |
| Estabilidad, exactitud, respuesta y muestreo | PENDIENTE: requieren sistema integrado y mediciones físicas |

## Procedimientos reproducibles pendientes

Con montaje confirmado y un servidor TCP en la IPv4 configurada, registrar los mensajes y sus tiempos de recepción. Conservar los datos sin reemplazarlos por valores esperados.

| Caso | Procedimiento | Resultado esperado | Resultado obtenido |
|---|---|---|---|
| Registro y respaldo | Aceptar TCP sin enviar `PERIOD` | Primero `HELLO SENSOR`; después `DIST` con periodo nominal de 250 ms | PENDIENTE |
| Fragmentación | Enviar `PER`, luego `IOD 100`, luego LF | Solo aplicar 100 ms al recibir LF | PENDIENTE |
| Concatenación y límites | Enviar `PERIOD 000` y `PERIOD 9999` en líneas consecutivas | Limitar a 100 y luego 5000 ms | PENDIENTE |
| Malformados | Enviar `PERIOD -100`, `PERIOD 50`, `PERIOD 1x0`, CRLF y comandos desconocidos | Mantener periodo anterior | PENDIENTE |
| Línea larga | Enviar 32 caracteres sin LF, luego `PERIOD 100` y LF; después una línea válida | Descartar toda la primera línea; aceptar la siguiente | PENDIENTE |
| Sin eco / fuera de rango | Retirar objetivo y comprobar referencias fuera de 2–200 cm | `DIST ERR` | PENDIENTE |
| Exactitud RF1 | Medir referencias a 2, 10, 49, 50, 99, 100 y 200 cm; tomar 10 lecturas por posición | Error absoluto máximo ≤ 3 cm; investigar errores y lecturas inválidas | PENDIENTE |
| Frecuencia | Enviar `PERIOD 250`; contar DIST en ventanas de 10 s durante 1 min | Al menos 20 lecturas por ventana | PENDIENTE |
| Reconexión | Cerrar TCP y, por separado, interrumpir WiFi; restaurar conexión | Reintentos cada 3 s; nuevo HELLO y lecturas, sin fragmentos de la sesión anterior | PENDIENTE |
| Estabilidad | Ejecutar 10 min y registrar reinicios o bloqueos | Ningún reinicio ni bloqueo | PENDIENTE |
| Respuesta integrada | Una vez implementados servidor y actuador, medir cambio físico → LED | ≤ 1 s a 250 ms | PENDIENTE |

Al disponer del hardware, completar [resultados](results.md) con las mediciones y evidencias de estos procedimientos.
