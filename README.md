# Práctica 2 — Objetos inteligentes con TCP/IP

Proyectos PlatformIO independientes para dos ESP32-WROOM-32: [sensor](sensor/platformio.ini) y [actuador](actuator/platformio.ini). Ambos usan Arduino/C++, perfil genérico `esp32dev` y plataforma Espressif32 7.1.1, disponible en el entorno de desarrollo.

## Estado

El plan general fue aprobado en la conversación el 11 de octubre de 2026. El archivo del plan se conserva como documento de un solo uso. Se mantiene [protocolo.md](protocolo.md) sin modificaciones. Los tres subplanes están preparados para revisión.

Los programas iniciales solo imprimen un mensaje por puerto serie a 115200 baudios. Medición, WiFi, TCP, LEDs y servidor quedan pendientes de implementación.

## Compilar y cargar

Abrir `sensor/` o `actuator/` como proyecto con PlatformIO, o ejecutar desde la raíz en una terminal con `pio` disponible:

```powershell
pio run --project-dir sensor
pio run --project-dir actuator
```

Conectar únicamente la placa que se desea programar y usar:

```powershell
pio run --project-dir sensor --target upload
pio device monitor --project-dir sensor
```

Para el segundo ESP sustituir `sensor` por `actuator`. Si hay varios puertos, especificar `--upload-port COMx` al cargar y `--port COMx` al abrir el monitor.

Si `pio` no está en el PATH, en esta máquina se puede invocar:

```powershell
& "$env:USERPROFILE/.platformio/penv/Scripts/platformio.exe" run --project-dir sensor
```

El esqueleto compila sin credenciales. Para la implementación de red se copiará `include/config.h.example` a `include/config.h` dentro de cada proyecto y se completarán SSID, contraseña e IP del servidor. Los archivos reales están excluidos de Git.

## Hardware acordado

Dos ESP32-WROOM-32; tres LEDs con una resistencia individual de 220 Ω. El perfil `esp32dev` configura flash de 4 MB; verificar el modelo de placa portadora y puerto al cargar.

La conexión del ultrasónico sin resistencias está pendiente de verificación eléctrica: no conectar un ECHO de 5 V directamente a un GPIO del ESP32. Espressif especifica tolerancia de 3,6 V para los GPIO ([FAQ oficial](https://docs.espressif.com/projects/esp-faq/en/latest/esp-faq-en-master.pdf)). Confirmar una variante compatible con 3,3 V o usar adaptación de nivel antes de cablear.
