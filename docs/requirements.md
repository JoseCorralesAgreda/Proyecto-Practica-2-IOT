# Requerimientos y estado

Fuente: `practica.md` y plan general aprobado. El alcance implementado en esta etapa es el servidor; los ESP mantienen sus programas mínimos.

| Requisito | Comportamiento definido | Estado y evidencia |
|---|---|---|
| RF1 | Medir distancia con HC-SR04 entre 2 y 200 cm | Firmware y verificación física PENDIENTES |
| RF2 | 2 ≤ d < 50: cerca; 50 ≤ d < 100: intermedia; 100 ≤ d ≤ 200: lejos | Implementado en `DistanceController`; pruebas de límites |
| RF3 | Cerca: `B00`; intermedia: `0B0`; lejos: `00S` | Órdenes del servidor comprobadas; ejecución física PENDIENTE |
| RF4 | Clientes persistentes TCP sobre WiFi, protocolo ASCII de `protocolo.md` | Servidor probado por TCP en loopback; WiFi y ESP PENDIENTES |

Lecturas inválidas: conservar el estado durante dos errores consecutivos; al tercero, `000`. Reiniciar el contador con una lectura válida. Ante desconexión o silencio del sensor, apagar y esperar un nuevo registro. Sin sensor inicial, el estado es `000`.

| Atributo | Objetivo aprobado | Estado |
|---|---|---|
| Estabilidad | ≥ 10 minutos sin reinicios ni bloqueos | PENDIENTE con sistema integrado |
| Exactitud | Error absoluto ≤ 3 cm entre 2 y 200 cm | PENDIENTE con referencia física |
| Respuesta | ≤ 1 s desde cambio físico hasta actuación | PENDIENTE con ambos ESP |
| Muestreo | ≥ 2 lecturas/s; periodo nominal de 250 ms | Servidor envía `PERIOD 250`; frecuencia real PENDIENTE |

El código usa clases con responsabilidades separadas, constantes en `server/config.py`, mensajes y comentarios en inglés. No requiere bibliotecas externas. Las credenciales de los futuros clientes se mantienen fuera de Git.
