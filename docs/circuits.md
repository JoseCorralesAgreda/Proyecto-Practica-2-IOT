# Circuitos

## Sensor

Montaje real: **PENDIENTE**. No hay placa conectada; la persona indicó continuar únicamente con el código. No se han confirmado GPIO, variante del ultrasónico ni adaptación de ECHO. No existe todavía un esquema del circuito real ni fotografías verificadas.

| Configuración | Estado |
|---|---|
| ESP32-WROOM-32, perfil de compilación `esp32dev` | Configurado en PlatformIO |
| Modelo exacto de ultrasónico y alimentación | PENDIENTE |
| GPIO de TRIG | Asignado: GPIO 18; montaje físico pendiente |
| GPIO de ECHO | Asignado: GPIO 19; montaje físico pendiente |
| Compatibilidad ECHO con 3,3 V o adaptación sin divisor resistivo | PENDIENTE |
| Confirmación del montaje | `SENSOR_WIRING_CONFIRMED = false` |

Antes de habilitar el sensor hay que confirmar estos datos y documentar el circuito real, incluida la masa común. Se mantiene la condición del subplan: no validar una conexión directa de ECHO de 5 V al ESP32. Con el ejemplo no se activa ningún GPIO del sensor y las lecturas se representan como `DIST ERR` si existe conexión TCP.

## Actuador

Configuración prevista para ESP32-WROOM-32 (`esp32doit-devkit-v1`):

| LED | GPIO |
|---|---:|
| Rojo | 25 |
| Naranja | 26 |
| Verde | 27 |

Conectar cada LED con una resistencia de 220 Ω en serie y masa común. El circuito real y su verificación física están **PENDIENTES**.
