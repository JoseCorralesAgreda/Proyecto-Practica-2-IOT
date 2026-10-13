# Subplan del actuador

Pendiente de revisión antes de implementar. El proyecto PlatformIO inicial ya usa Arduino/C++ y `esp32dev` para ESP32-WROOM-32. Se mantiene `protocolo.md`. Se utilizará un resistor de 220 Ω en serie con cada LED, según instrucción de la persona.

## Archivos y responsabilidades

- `actuator/src/main.cpp`: composición, `setup()` y `loop()`.
- `actuator/include/config.h.example`: ejemplo de credenciales y parámetros; el archivo real se excluye de Git.
- `actuator/include/led_controller.h` y `actuator/src/led_controller.cpp`: clase `LedController`, estados apagado, fijo y parpadeo.
- `actuator/include/actuator_client.h` y `actuator/src/actuator_client.cpp`: clase `ActuatorClient`, WiFi, TCP, registro y recepción de órdenes.

## Pasos

1. Acordar GPIO para rojo, naranja y verde; verificar polaridad y resistencia individual de 220 Ω. Iniciar los LEDs apagados.
2. Implementar patrones `0`, `S` y `B` con temporización no bloqueante, según el periodo indicado en `protocolo.md`. Los refrescos de un mismo patrón no reiniciarán el parpadeo.
3. Conectar y enviar `HELLO ACTUATOR`; interpretar `LED` por líneas LF, incluyendo recepción fragmentada y descarte de líneas largas.
4. Apagar y cerrar la conexión al cumplirse 5 s sin órdenes válidas; reconectar cada 3 s y repetir el registro.
5. Compilar y comprobar los tres patrones, refrescos, mensajes inválidos y caída del servidor con el hardware disponible.
6. Documentar circuito real, clases, implementación y pruebas en `docs/`.

Aceptación: patrones y apagado por timeout correctos. Verificar con el sistema integrado respuesta máxima de 1 s y estabilidad mínima de 10 minutos.
