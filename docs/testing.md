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
| Medición, clasificación y actuación | PENDIENTE: funcionalidad sin implementar |
| Comunicación TCP y recuperación de fallas | PENDIENTE: funcionalidad sin implementar |
| Estabilidad, exactitud, respuesta y muestreo | PENDIENTE: requieren sistema integrado y mediciones físicas |
