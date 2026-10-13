# Requerimientos

Fuente: `practica.md` (§3.1 y §3.2) y el plan general aprobado. Estado a 13 de octubre de 2026: los tres componentes están **implementados y verificados** con el prototipo físico.

## Requerimientos funcionales

| ID | Comportamiento definido | Estado |
|---|---|---|
| RF1 | Medir la distancia al objeto con un HC-SR04 en el rango de trabajo de 2 a 200 cm | Implementado (`UltrasonicSensor`) y verificado con el prototipo físico |
| RF2 | Clasificar la distancia en tres rangos contiguos y sin solape: 2 ≤ d < 50 (cerca), 50 ≤ d < 100 (intermedia), 100 ≤ d ≤ 200 (lejos) | Implementado (`DistanceController` en el servidor y comprobación de rango en el sensor); verificado con pruebas y con el prototipo físico |
| RF3 | Actuar de forma distinta según el rango: cerca `LED B00`, intermedia `LED 0B0`, lejos `LED 00S` | Implementado en el actuador y verificado físicamente con los LED |
| RF4 | Intercambiar datos mediante sockets TCP sobre WiFi (IEEE 802.11) con un protocolo de aplicación propio | Implementado y verificado de extremo a extremo con los tres componentes en la red WiFi |

### Lógica de control

| Distancia válida | Clasificación | Patrón `LED` |
|---|---|---|
| 2 ≤ d < 50 cm | Cerca | `B00` (rojo parpadeando) |
| 50 ≤ d < 100 cm | Intermedia | `0B0` (naranja parpadeando) |
| 100 ≤ d ≤ 200 cm | Lejos | `00S` (verde fijo) |

Los intervalos cubren todo el rango de trabajo sin solaparse. Una lectura menor que 2 cm, mayor que 200 cm, no numérica (`DIST ERR`) o no finita se considera inválida.

### Manejo de errores

- Dos lecturas inválidas consecutivas conservan el patrón vigente; la tercera apaga los LED (`000`). Una lectura válida reinicia el contador.
- La pérdida o el silencio del sensor durante `max(2 s, 3 × PERIOD)` apaga los LED y cierra la conexión; el sensor puede registrarse de nuevo.
- El actuador apaga los LED y cierra la conexión tras 5 s sin órdenes válidas, y reintenta cada 3 s.
- Una línea malformada o desconocida se ignora y se registra; un `HELLO` con rol inválido cierra la conexión.

## Requerimientos no funcionales

| Atributo | Objetivo aprobado | Estado |
|---|---|---|
| Estabilidad | ≥ 10 minutos sin reinicios ni bloqueos | Verificado con el prototipo físico |
| Exactitud | Error absoluto ≤ 3 cm dentro de 2–200 cm | Verificado con el prototipo físico |
| Respuesta | ≤ 1 s del cambio físico a la actuación, extremo a extremo | Verificado con el prototipo físico |
| Muestreo | ≥ 2 lecturas/s; periodo nominal 250 ms | El servidor envía `PERIOD 250` y el sensor lo aplica; frecuencia verificada con el prototipo físico |

Los resultados de las pruebas están en [testing.md](testing.md) y [results.md](results.md).

## Convenciones de código

Código orientado a objetos con clases de responsabilidad única, constantes centralizadas en `server/config.py` (servidor) y `config.h` (cada ESP32), y nombres, comentarios y mensajes del protocolo en inglés. No se requieren bibliotecas externas. Las credenciales WiFi y la IPv4 del servidor quedan fuera de Git, en archivos `*.example`.
