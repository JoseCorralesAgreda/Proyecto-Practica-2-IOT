# Subplan del sensor

Pendiente de revisión antes de implementar. El proyecto PlatformIO inicial ya usa Arduino/C++ y `esp32dev` para ESP32-WROOM-32. Se mantiene `protocolo.md`.

## Archivos y responsabilidades

- `sensor/src/main.cpp`: composición, `setup()` y `loop()`.
- `sensor/include/config.h.example`: ejemplo de credenciales y parámetros; el archivo real se excluye de Git.
- `sensor/include/ultrasonic_sensor.h` y `sensor/src/ultrasonic_sensor.cpp`: clase `UltrasonicSensor`, disparo, espera acotada del eco y conversión a centímetros.
- `sensor/include/sensor_client.h` y `sensor/src/sensor_client.cpp`: clase `SensorClient`, WiFi, TCP, registro, periodo y envío de lecturas.

## Pasos

1. Confirmar GPIO y compatibilidad eléctrica del sensor real antes de cablear. La persona solicita no usar resistencias en la conexión del sensor: queda pendiente confirmar una variante con ECHO compatible con 3,3 V o adaptación de nivel sin divisor resistivo. No considerar validada una conexión directa de ECHO a 5 V.
2. Implementar medición con timeout de eco y rango válido de 2 a 200 cm; representar lecturas inválidas como `DIST ERR`.
3. Conectar WiFi/TCP, enviar `HELLO SENSOR`, recibir `PERIOD` y limitarlo al intervalo de 100 a 5000 ms. Usar 250 ms inicialmente.
4. Enviar `DIST` con un decimal y LF; procesar líneas parciales y descartar líneas largas según el protocolo.
5. Reintentar conexión cada 3 s sin bucles de espera indefinidos y repetir el registro.
6. Compilar, cargar con la placa conectada y medir distancias de referencia, frecuencia y recuperación de desconexión.
7. Documentar circuito real, clases, implementación y evidencias obtenidas en `docs/`.

Aceptación: medición y envío conforme al protocolo, al menos 2 lecturas/s a 250 ms y error absoluto máximo de 3 cm en el rango aprobado. Sin datos físicos, resultados `PENDIENTE`.
