# Protocolo de aplicación

Especificación de referencia del grupo: [protocolo.md](../protocolo.md). Este documento describe el protocolo **tal como lo implementan** el servidor y ambos firmware, con las precisiones acordadas en el plan general. Los tres componentes son coherentes entre sí.

## Formato general

- Texto ASCII sobre TCP persistente, un mensaje por línea terminada en LF (`\n`).
- Forma general `COMANDO [argumento]`, con un único espacio separador.
- Longitud máxima de línea: 32 bytes incluido LF (hasta 31 bytes de contenido). Una línea que alcanza el límite se descarta completa hasta el siguiente LF; la línea siguiente se procesa normalmente.
- TCP no conserva los límites de mensaje: el LF es el delimitador. Se reconstruyen mensajes fragmentados y se separan los concatenados.
- CRLF no forma parte de la gramática. Las líneas desconocidas o malformadas se ignoran y se registran; un `HELLO` con un rol distinto de `SENSOR` o `ACTUATOR` cierra la conexión.

## Mensajes

| Dirección | Mensaje | Argumento | Ejemplo |
|---|---|---|---|
| ESP → servidor | `HELLO` | Rol: `SENSOR` o `ACTUATOR` | `HELLO SENSOR` |
| Sensor → servidor | `DIST` | Distancia en cm con un decimal, o `ERR` | `DIST 42.5` / `DIST ERR` |
| Servidor → sensor | `PERIOD` | Periodo de medición en ms (100–5000) | `PERIOD 250` |
| Servidor → actuador | `LED` | Tres estados, uno por LED (rojo, naranja, verde) | `LED B00` |

Estados de cada LED:

| Carácter | Significado |
|---|---|
| `0` | Apagado |
| `S` | Encendido fijo (*solid*) |
| `B` | Parpadeando (*blink*) |

Ejemplos: `LED B00` rojo parpadeando · `LED 0B0` naranja parpadeando · `LED 00S` verde fijo · `LED 000` todos apagados.

Gramática resumida:

```text
mensaje = hello / dist / period / led
hello   = "HELLO" SP ( "SENSOR" / "ACTUATOR" ) LF
dist    = "DIST" SP ( 1*3DIGIT "." DIGIT / "ERR" ) LF
period  = "PERIOD" SP 3*4DIGIT LF
led     = "LED" SP 3estado LF
estado  = "0" / "S" / "B"
```

## Reglas de comportamiento

1. Cada cliente abre la conexión TCP y envía `HELLO` con su rol. Un rol inválido cierra el socket.
2. Tras `HELLO SENSOR` el servidor responde `PERIOD` con el periodo configurado (250 ms por defecto). Tras `HELLO ACTUATOR` envía el `LED` del patrón vigente.
3. Si ya existe un cliente registrado con ese rol, el servidor cierra el anterior y conserva el nuevo; el cierre del anterior no elimina el registro nuevo. Repetir `HELLO` del mismo rol en el mismo socket devuelve la respuesta y no prolonga el timeout del sensor.
4. Solo el socket registrado como `SENSOR` puede enviar `DIST`. El sensor envía una lectura cada `PERIOD`; si la medición falla o queda fuera de 2–200 cm, envía `DIST ERR`.
5. El servidor clasifica y envía `LED` al actuador cuando el patrón cambia y, además, como refresco cada 1 s.
6. El sensor limita el `PERIOD` recibido al intervalo 100–5000 ms y lo aplica en la siguiente comprobación; sin `PERIOD` usa 250 ms. Un valor como `PERIOD 50` (2 dígitos) se ignora por gramática.
7. El actuador genera el parpadeo localmente con un ciclo de 500 ms (250 ms encendido y 250 ms apagado); los refrescos del mismo patrón no reinician el parpadeo.
8. Un cliente que pierde WiFi o TCP apaga sus salidas, cierra el socket y reintenta cada 3 s, repitiendo el `HELLO`. Una nueva sesión del sensor vuelve al periodo inicial de 250 ms.

## Tiempos

| Parámetro | Valor | Aplica en |
|---|---|---|
| `PERIOD` por defecto | 250 ms | Servidor (lo envía) y sensor (respaldo) |
| Rango de `PERIOD` | 100–5000 ms | Servidor y sensor |
| Timeout de silencio del sensor | `max(2 s, 3 × PERIOD)` | Servidor |
| Errores `DIST ERR` consecutivos tolerados | 2 (al tercero `000`) | Servidor |
| Refresco de `LED` | 1 s | Servidor |
| Timeout de silencio del servidor | 5 s | Actuador |
| Ciclo de parpadeo | 500 ms | Actuador |
| Reintento de conexión | 3 s | Ambos ESP32 |
| Timeout de conexión TCP | 1 s | Ambos ESP32 |
| Timeout de escritura del servidor | 1 s | Servidor |
| Revisión de plazos del servidor | 50 ms | Servidor |

## Validación por componente

- **Servidor:** valida la gramática completa; los valores numéricos fuera de 2–200 cm y los no finitos cuentan como lectura inválida. Una escritura bloqueada se limita a 1 s antes de cerrar la conexión afectada.
- **Sensor:** valida `PERIOD` con 3 o 4 dígitos y lo limita a 100–5000 ms; descarta líneas desconocidas, vacías, con CR, signos, bytes nulos o espacios extra. Un envío incompleto cierra la conexión.
- **Actuador:** exige una línea exacta `LED` con tres caracteres de estado válidos; cualquier otra se ignora. La recepción por iteración se acota para que el tráfico malformado no retrase el watchdog ni el parpadeo.
