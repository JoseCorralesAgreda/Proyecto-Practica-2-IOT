# Desarrollo e implementación

Los tres componentes están implementados: el servidor TCP en Python 3 y los dos clientes ESP32 en Arduino/C++. El código sigue un estilo orientado a objetos con pocas clases por componente, constantes centralizadas y nombres, comentarios y mensajes de protocolo en inglés.

## Estructura general

| Componente | Lenguaje | Entorno | Entrada |
|---|---|---|---|
| Servidor | Python 3 + `asyncio` | Biblioteca estándar | [server/main.py](../server/main.py) |
| Sensor | Arduino/C++ | PlatformIO `esp32doit-devkit-v1` | [sensor/src/main.cpp](../sensor/src/main.cpp) |
| Actuador | Arduino/C++ | PlatformIO `esp32doit-devkit-v1` | [actuator/src/main.cpp](../actuator/src/main.cpp) |

Ambos proyectos PlatformIO usan la plataforma Espressif32 7.1.1 y el framework Arduino. Cada uno tiene su propio `include/config.h`, copiado localmente desde `include/config.h.example`; contiene credenciales, IPv4, GPIO y constantes. `.gitignore` excluye las configuraciones reales y las salidas de compilación.

## Servidor TCP

Se ejecuta con `python -m server.main` desde la raíz del repositorio. Los imports usan `server` como paquete de espacio de nombres, sin dependencias externas.

| Archivo | Responsabilidad |
|---|---|
| [main.py](../server/main.py) | Logs de consola, bucle `asyncio`, salida por Ctrl+C y error de arranque |
| [config.py](../server/config.py) | Red, rangos, patrones, periodo y tiempos |
| [distance_controller.py](../server/distance_controller.py) | `DistanceController`: clasificación y tolerancia de dos errores consecutivos |
| [tcp_server.py](../server/tcp_server.py) | `TcpServer`: sockets, registro, protocolo, refrescos y pérdidas de conexión |
| [test_server.py](../server/test_server.py) | Pruebas unitarias y clientes TCP simulados |

### Fragmentos clave

- `DistanceController.update()` aplica intervalos contiguos de 2 a 200 cm con umbrales de 50 y 100 cm. `None`, valores no finitos y valores fuera del rango cuentan como lecturas inválidas; el tercer error consecutivo apaga el patrón. Una lectura válida reinicia el contador.
- `TcpServer._handle_client()` lee bloques y reconstruye líneas por LF. Mantiene hasta 31 bytes de contenido por línea; cuando se supera ese límite descarta hasta LF y luego reanuda la lectura, de modo que no confunde los límites de los bloques TCP con los de los mensajes.
- `_handle_line()` valida ASCII y la gramática completa. Solo el socket registrado como sensor puede modificar el control. Un `HELLO` duplicado sustituye el socket anterior; el bloque `finally` elimina un rol únicamente si todavía corresponde a ese mismo socket.
- `_maintain_clients()` usa reloj monotónico y revisa cada 50 ms el silencio del sensor y el refresco del actuador. El timeout se calcula como `max(2 s, 3 × PERIOD)` y el refresco es de 1 s. Las escrituras esperan como máximo 1 s antes de cerrar el socket que falla.
- `close()` deja de aceptar conexiones, cancela la supervisión, cierra los clientes registrados y sin registrar, cancela sus tareas y finalmente espera al cierre del listener. Este orden evita esperar indefinidamente a clientes abiertos.

La comunicación de prueba se realizó en loopback. La temporización depende del sistema operativo y del bucle de eventos; la latencia física del sistema completo se verificó, además, con el prototipo integrado.

## Sensor

| Archivo | Responsabilidad |
|---|---|
| [main.cpp](../sensor/src/main.cpp) | Compone las clases; inicia Serial, sensor y cliente; atiende el cliente y cede 1 ms por iteración |
| [ultrasonic_sensor.h](../sensor/include/ultrasonic_sensor.h) / [ultrasonic_sensor.cpp](../sensor/src/ultrasonic_sensor.cpp) | GPIO, disparo ultrasónico, timeout y conversión |
| [sensor_client.h](../sensor/include/sensor_client.h) / [sensor_client.cpp](../sensor/src/sensor_client.cpp) | WiFi/TCP, registro, parser y envío periódico |
| [config.h.example](../sensor/include/config.h.example) | Parámetros centralizados; copiar a `config.h` local |

- `UltrasonicSensor::readDistance()` mantiene TRIG bajo 2 µs y alto 10 µs. `pulseIn()` espera como máximo 30000 µs. La conversión es `eco_us × 0,0343 / 2`; el divisor corresponde al trayecto de ida y vuelta. Se aceptan 2–200 cm inclusive antes de redondear a un decimal. Sin eco, fuera de rango o con montaje deshabilitado, retorna `false` y se transmite `DIST ERR`.
- `UltrasonicSensor::begin()` exige confirmación de montaje, un GPIO de salida válido para TRIG, un GPIO válido para ECHO y pines distintos; no configura ningún pin si falla la comprobación. El ejemplo deja los GPIO en `-1` y la confirmación en `false`.
- `SensorClient::update()` inicia WiFi sin bucles de espera y reintenta cada 3 s. Conecta TCP a una IPv4 literal con timeout de 1 s y envía `HELLO SENSOR`; cada sesión vuelve al periodo inicial de 250 ms. `applyCommand()` limita el `PERIOD` recibido a 100–5000 ms.
- El muestreo se programa desde el inicio de la última medición mediante resta sin signo de `millis()`, compatible con su desbordamiento. El periodo recibido se aplica en la siguiente comprobación; no se generan ráfagas para recuperar muestras atrasadas.
- La recepción conserva fragmentos entre iteraciones, procesa varias líneas y descarta las demasiado largas hasta LF. Lee como máximo 128 bytes por iteración para que un flujo continuo no monopolice el programa. Un envío incompleto cierra la conexión para no continuar una línea truncada.

## Actuador

| Archivo | Responsabilidad |
|---|---|
| [main.cpp](../actuator/src/main.cpp) | Inicia Serial, los LED y el cliente; atiende comandos y parpadeo |
| [led_controller.h](../actuator/include/led_controller.h) / [led_controller.cpp](../actuator/src/led_controller.cpp) | Validación de GPIO, aplicación de patrones y parpadeo local no bloqueante |
| [actuator_client.h](../actuator/include/actuator_client.h) / [actuator_client.cpp](../actuator/src/actuator_client.cpp) | WiFi/TCP, registro, recepción de comandos y watchdog |
| [config.h.example](../actuator/include/config.h.example) | Parámetros centralizados; copiar a `config.h` local |

- `LedController::begin()` no configura las salidas si `LED_WIRING_CONFIRMED` es `false` o si los GPIO no son válidos y distintos; con el ejemplo la salida serie avisa y los LED quedan apagados.
- `LedController::update()` calcula la fase del parpadeo como `(now - inicio) % 500 ms` y enciende en `S` o en la primera mitad del ciclo de `B`. Avanza ciclos completos para preservar la fase ante el desbordamiento de `millis()`. Los refrescos de un mismo patrón no reinician el parpadeo.
- `ActuatorClient` inicia las salidas apagadas, se registra con `HELLO ACTUATOR` y aplica líneas `LED` exactas de tres estados. Si se pierde WiFi o TCP, o se superan 5 s sin órdenes, apaga las salidas y espera la reconexión. El monitor serie muestra conexión, fallos y reintentos, además de los cambios de patrón; no repite el mismo patrón en cada refresco del servidor.
- La recepción por iteración también se acota a 128 bytes para que el tráfico malformado no retrase el watchdog.

El sistema se verificó con el prototipo físico: los GPIO, el montaje de los circuitos y la salida serie funcionaron correctamente (ver [testing.md](testing.md)).
