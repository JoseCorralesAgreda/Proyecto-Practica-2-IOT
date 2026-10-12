# Implementación actual

## Inicialización de los ESP

Se crearon dos proyectos independientes para ESP32-WROOM-32 con Arduino/C++ y PlatformIO. La configuración usa `esp32dev` y fija Espressif32 7.1.1, versión instalada al inicializar.

| Proyecto | Configuración | Entrada |
|---|---|---|
| Sensor | [platformio.ini](../sensor/platformio.ini) | [main.cpp](../sensor/src/main.cpp) |
| Actuador | [platformio.ini](../actuator/platformio.ini) | [main.cpp](../actuator/src/main.cpp) |

El actuador conserva el esqueleto: inicializa Serial a 115200 baudios y cede ejecución cada 10 ms. El servidor está pendiente. El sensor ya implementa el subplan, sin dependencias externas nuevas.

El sensor incluye `config.h`, copia local de `include/config.h.example`, con credenciales, IPv4, GPIO y constantes. `.gitignore` excluye las configuraciones reales, las salidas de compilación y la copia local de herramientas utilizada para validación. El actuador todavía no incluye su configuración.

El protocolo existente permanece sin cambios. Las resistencias de 220 Ω por LED son una decisión de montaje registrada; todavía no se ha construido ni validado el circuito mediante este trabajo.

## Servidor TCP

Implementado en Python con dos clases y biblioteca estándar; se ejecuta mediante `python -m server.main` desde la raíz. Los imports usan el directorio `server` como paquete de espacio de nombres, sin dependencias externas.

| Archivo | Responsabilidad |
|---|---|
| [main.py](../server/main.py) | Logs de consola, bucle `asyncio`, salida por Ctrl+C y error de arranque |
| [config.py](../server/config.py) | Red, rangos, patrones, periodo y tiempos |
| [distance_controller.py](../server/distance_controller.py) | `DistanceController`: clasificación y tolerancia de dos errores consecutivos |
| [tcp_server.py](../server/tcp_server.py) | `TcpServer`: sockets, registro, protocolo, refrescos y pérdidas de conexión |
| [test_server.py](../server/test_server.py) | Pruebas unitarias y clientes TCP simulados |

### Fragmentos clave

- `DistanceController.update()` aplica intervalos contiguos de 2 a 200 cm con umbrales de 50 y 100 cm. `None`, valores no finitos y valores fuera del rango cuentan como lecturas inválidas; el tercer error consecutivo apaga el patrón. Una lectura válida reinicia el contador.
- `TcpServer._handle_client()` lee bloques y reconstruye líneas por LF. Mantiene hasta 31 bytes de contenido por línea; cuando se supera ese límite descarta hasta LF y luego reanuda la lectura. No confunde bloques TCP con mensajes.
- `_handle_line()` valida ASCII y gramática completa. Solo el socket registrado como sensor puede modificar el control. Un `HELLO` duplicado sustituye el socket anterior; el bloque `finally` elimina un rol únicamente si todavía corresponde a ese mismo socket.
- `_maintain_clients()` usa reloj monotónico y revisa cada 50 ms el silencio del sensor y el refresco del actuador. El timeout se calcula como `max(2 s, 3 × PERIOD)` y el refresco es de 1 s. Las escrituras esperan como máximo 1 s antes de cerrar el socket que falla.
- `close()` deja de aceptar conexiones, cancela el mantenimiento, cierra clientes registrados y sin registrar, cancela sus tareas y finalmente espera al cierre del listener. Este orden evita esperar indefinidamente a clientes abiertos.

La comunicación de prueba se realizó en loopback. La temporización depende del sistema operativo y del bucle de eventos; no constituye una garantía de tiempo real ni acredita la latencia física del sistema completo.
## Sensor

| Archivo | Responsabilidad |
|---|---|
| [main.cpp](../sensor/src/main.cpp) | Compone las clases; inicia Serial, sensor y cliente; atiende el cliente y cede 1 ms por iteración |
| [ultrasonic_sensor.h](../sensor/include/ultrasonic_sensor.h) / [ultrasonic_sensor.cpp](../sensor/src/ultrasonic_sensor.cpp) | GPIO, disparo ultrasónico, timeout y conversión |
| [sensor_client.h](../sensor/include/sensor_client.h) / [sensor_client.cpp](../sensor/src/sensor_client.cpp) | WiFi/TCP, registro, parser y envío periódico |
| [config.h.example](../sensor/include/config.h.example) | Parámetros centralizados; copiar a `config.h` local |

`UltrasonicSensor::readDistance()` mantiene TRIG bajo 2 µs y alto 10 µs. `pulseIn()` espera como máximo 30000 µs. La conversión es `eco_us × 0,0343 / 2`; el divisor corresponde al trayecto de ida y vuelta. Se aceptan 2–200 cm inclusive antes de redondear a un decimal. Sin eco, fuera de rango o con montaje deshabilitado, retorna `false` y se transmite `DIST ERR`.

El ejemplo deja los GPIO en `-1` y la confirmación eléctrica en `false`. `begin()` exige confirmación, un GPIO de salida válido para TRIG, un GPIO válido para ECHO y pines distintos; no configura pines si falla esta comprobación. Esto no verifica eléctricamente el montaje ni sustituye la selección de GPIO adecuados para la placa portadora.

`SensorClient::update()` inicia WiFi sin bucles de espera. Reintenta cada 3 s; conecta TCP a una IPv4 literal con timeout de 1 s y envía `HELLO SENSOR`. Cada sesión vuelve al periodo inicial de 250 ms. No necesita esperar `PERIOD` para enviar lecturas.

El muestreo se programa desde el inicio de la última medición mediante resta sin signo de `millis()`, compatible con su desbordamiento. El periodo recibido se aplica en la siguiente comprobación; no se generan ráfagas para recuperar muestras atrasadas. Un envío incompleto cierra la conexión para evitar continuar una línea truncada y una nueva sesión repite el registro.

La recepción conserva fragmentos entre iteraciones, procesa varias líneas y descarta líneas demasiado largas hasta LF. Lee como máximo 128 bytes por iteración para que un flujo continuo no monopolice el programa. Ver la gramática en [protocolo](protocol.md) y los diagramas de [clases](classes.md) y [secuencia](sequence.md).

La medición y la conexión TCP tienen esperas acotadas. La escritura usa los reintentos internos de `WiFiClient` del framework; bajo congestión puede demorar más que el periodo nominal. La compilación no demuestra frecuencia ni latencia de red. No se añadió timeout por silencio del servidor: el protocolo permite que solo envíe `PERIOD` al registrar al sensor.
