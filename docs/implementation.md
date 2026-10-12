# Implementación actual

## Inicialización de los ESP

Se crearon dos proyectos independientes para ESP32-WROOM-32 con Arduino/C++ y PlatformIO. La configuración usa `esp32dev` y fija Espressif32 7.1.1, versión instalada al inicializar.

| Proyecto | Configuración | Entrada |
|---|---|---|
| Sensor | [platformio.ini](../sensor/platformio.ini) | [main.cpp](../sensor/src/main.cpp) |
| Actuador | [platformio.ini](../actuator/platformio.ini) | [main.cpp](../actuator/src/main.cpp) |

`setup()` inicializa el puerto serie a 115200 baudios e imprime la identificación del proyecto. `loop()` cede ejecución con una espera de 10 ms. No hay todavía clases de aplicación, uso de GPIO, medición ni comunicación de red.

Los archivos `include/config.h.example` contienen valores ficticios para la futura conexión WiFi/TCP. No se incluyen desde el esqueleto actual. `.gitignore` excluye las configuraciones reales, las salidas de compilación y la copia local de herramientas utilizada para validación.

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
