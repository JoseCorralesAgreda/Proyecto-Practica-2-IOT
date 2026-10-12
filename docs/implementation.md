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
