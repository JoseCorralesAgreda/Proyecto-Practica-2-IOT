# Circuitos

## Sensor

Montaje real: **PENDIENTE**. No hay placa conectada; la persona indicó continuar únicamente con el código. No se han confirmado GPIO, variante del ultrasónico ni adaptación de ECHO. No existe todavía un esquema del circuito real ni fotografías verificadas.

| Configuración | Estado |
|---|---|
| ESP32-WROOM-32, perfil de compilación `esp32dev` | Configurado en PlatformIO |
| Modelo exacto de ultrasónico y alimentación | PENDIENTE |
| GPIO de TRIG | PENDIENTE; ejemplo `-1` |
| GPIO de ECHO | PENDIENTE; ejemplo `-1` |
| Compatibilidad ECHO con 3,3 V o adaptación sin divisor resistivo | PENDIENTE |
| Confirmación del montaje | `SENSOR_WIRING_CONFIRMED = false` |

Antes de habilitar el sensor hay que confirmar estos datos y documentar el circuito real, incluida la masa común. Se mantiene la condición del subplan: no validar una conexión directa de ECHO de 5 V al ESP32. Con el ejemplo no se activa ningún GPIO del sensor y las lecturas se representan como `DIST ERR` si existe conexión TCP.

## Actuador

Circuito real y verificación: **PENDIENTE**, fuera de esta implementación del sensor.
