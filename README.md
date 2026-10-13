# Práctica 2 — Objetos inteligentes con TCP/IP

Proyectos PlatformIO independientes para dos ESP32-WROOM-32: [sensor](sensor/platformio.ini) y [actuador](actuator/platformio.ini). Ambos usan Arduino/C++, entorno `esp32doit-devkit-v1` y plataforma Espressif32 7.1.1, disponible en el entorno de desarrollo.

## Estado

El plan general y los tres subplanes fueron aprobados y ejecutados. Los planes se conservan como documentos de un solo uso. Se mantiene [protocolo.md](protocolo.md) como especificación de referencia; [docs/protocol.md](docs/protocol.md) describe su implementación actual.

El servidor TCP, el firmware del sensor (medición, WiFi/TCP, registro, envío periódico y reconexión) y el firmware del actuador (tres LED con parpadeo local, registro, watchdog y reconexión) están implementados. El servidor está probado con clientes simulados y el sistema integrado se probó con las placas: todas las pruebas realizadas resultaron correctas.

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

Antes de compilar, copiar `include/config.h.example` a `include/config.h` en `sensor/` y en `actuator/`, y completar SSID, contraseña e IPv4 del servidor. En el sensor, configurar `TRIGGER_PIN`, `ECHO_PIN` y `SENSOR_WIRING_CONFIRMED = true` tras confirmar el montaje; en el actuador, confirmar `LED_PINS` y `LED_WIRING_CONFIRMED = true`. Con los valores de ejemplo ambos compilan, pero los GPIO permanecen deshabilitados. Los archivos reales están excluidos de Git.

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

Ambos proyectos requieren `config.h`; los ejemplos solo contienen valores ficticios. Consultar [implementación](docs/implementation.md), [protocolo](docs/protocol.md) y [pruebas](docs/testing.md).

## Hardware acordado

Dos ESP32-WROOM-32; tres LEDs con una resistencia individual de 220 Ω. El entorno `esp32doit-devkit-v1` configura flash de 4 MB; verificar el modelo de placa portadora y puerto al cargar.

El equipo verificó experimentalmente que la conexión directa del ECHO del ultrasónico a un GPIO del ESP32 no daña componentes y es adecuada para esta práctica: el sensor **no requiere resistencias** ni adaptación de nivel. Ver el esquemático en [circuitos](docs/circuits.md).
