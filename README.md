# Práctica 2 — Objetos inteligentes con TCP/IP

Proyectos PlatformIO independientes para dos ESP32-WROOM-32: [sensor](sensor/platformio.ini) y [actuador](actuator/platformio.ini). Ambos usan Arduino/C++, perfil genérico `esp32dev` y plataforma Espressif32 7.1.1, disponible en el entorno de desarrollo.

## Estado

El plan general y la ejecución del subplan del servidor fueron aprobados en la conversación. Los planes se conservan como documentos de un solo uso. Se mantiene [protocolo.md](protocolo.md) como especificación de referencia; [docs/protocol.md](docs/protocol.md) describe su implementación actual.

El servidor TCP está implementado y probado con clientes simulados. Los programas de los ESP solo imprimen un mensaje por puerto serie a 115200 baudios; medición, WiFi, TCP y LEDs en los dispositivos quedan pendientes de sus subplanes.

## Ejecutar el servidor

Requiere Python 3.11 o posterior y utiliza exclusivamente la biblioteca estándar. Desde la raíz del repositorio:

```powershell
python -m server.main
```

Escucha en `0.0.0.0:5000`. Configurar en los ESP la IPv4 de la PC dentro de la misma red WiFi, permitir TCP entrante al puerto 5000 en el firewall y evitar redes que aíslen los clientes entre sí. `0.0.0.0` es la dirección de escucha, no la dirección que deben usar los ESP. Detener con Ctrl+C.

Los rangos y tiempos se concentran en [server/config.py](server/config.py). El periodo nominal es 250 ms; cambiarlo por encima de 500 ms no satisface el objetivo de al menos dos lecturas por segundo. Si el puerto está ocupado, cerrar la otra instancia o configurar el mismo puerto en servidor y clientes.

Para repetir las pruebas automatizadas del servidor, sin placas conectadas:

```powershell
python -m unittest server.test_server -v
```

Consultar [pruebas](docs/testing.md), [arquitectura](docs/architecture.md) e [implementación](docs/implementation.md).

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
