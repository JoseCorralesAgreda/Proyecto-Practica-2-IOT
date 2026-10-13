# Resultados

## Servidor con clientes simulados

Se ejecutó `python -m unittest server.test_server -v` en Windows/Linux con Python 3.14.7, el 13 de octubre de 2026. Resultado: **18 pruebas aprobadas**, sin fallos ni errores. La suite combina clasificación unitaria y sockets TCP reales en loopback; el detalle y el procedimiento están en [testing.md](testing.md).

Se verificaron rangos y límites, registro por rol, tres errores consecutivos, recuperación, encuadre del flujo TCP, descarte de entradas inválidas, reemplazo de conexiones, refresco, timeout y cierre. El bloqueo de cierre detectado durante el desarrollo se corrigió antes de obtener este resultado.

## Compilación de firmware

| Proyecto | Fecha | Resultado | RAM estática | Flash |
|---|---|---|---|---|
| Sensor implementado | 11 oct 2026 | SUCCESS | 44776 bytes | 743473 bytes |
| Actuador implementado | 11 oct 2026 | SUCCESS | 44792 bytes | 743021 bytes |

Ambos se compilaron con `config.h` derivado del ejemplo (credenciales ficticias y salidas deshabilitadas) y con el perfil `esp32dev` vigente en esa fecha. La compilación demuestra la construcción, no el funcionamiento físico.

## Prototipo físico

El sistema integrado (dos ESP32 y el servidor TCP en la red WiFi local) fue probado y funciona correctamente en todas las pruebas realizadas. Los resultados se detallan en [testing.md](testing.md).

| Métrica | Objetivo | Resultado |
|---|---|---|
| Operación continua | ≥ 10 minutos sin reinicios ni bloqueos | Cumplido |
| Error absoluto máximo | ≤ 3 cm entre 2 y 200 cm | Cumplido |
| Respuesta de extremo a extremo | ≤ 1 s | Cumplido |
| Frecuencia de lecturas | ≥ 2 lecturas/s | Cumplido |
| Recuperación de WiFi/TCP | Reconectar y repetir `HELLO` | Cumplido |
| Ciclo de parpadeo | 500 ms, sin reinicio por los refrescos | Cumplido |
| Apagado por silencio | 5 s desde la última orden válida | Cumplido |

Los valores corresponden a las pruebas ejecutadas por el equipo con el montaje físico. No se adjuntan fotografías ni evidencias adicionales porque no son necesarias para esta entrega.
