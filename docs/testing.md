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

## Alcance y pruebas pendientes

La compilación verifica los programas mínimos, que únicamente inicializan Serial. No se cargó firmware en placas ni se comprobó salida serie física.

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
