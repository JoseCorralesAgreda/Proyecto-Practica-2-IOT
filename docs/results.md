# Resultados

## Servidor con clientes simulados

Se ejecutó `python -m unittest server.test_server -v` en Windows con Python 3.14.7. Resultado: **18 pruebas aprobadas**, sin fallos ni errores. La suite combina clasificación unitaria y sockets TCP reales en loopback; el detalle y procedimiento están en [testing.md](testing.md).

Se verificaron rangos y límites, registro por rol, tres errores consecutivos, recuperación, encuadre del flujo TCP, descarte de entradas inválidas, reemplazo de conexiones, refresco, timeout y cierre. El bloqueo de cierre detectado durante el desarrollo se corrigió antes de obtener este resultado.

El resultado acredita el comportamiento probado del servidor. No representa una prueba de estabilidad durante diez minutos ni demuestra comunicación por WiFi o actuación física.

## Sistema físico

| Métrica | Objetivo | Resultado medido |
|---|---|---|
| Operación continua | ≥ 10 minutos sin reinicios ni bloqueos | PENDIENTE |
| Error absoluto máximo | ≤ 3 cm entre 2 y 200 cm | PENDIENTE |
| Respuesta de extremo a extremo | ≤ 1 s | PENDIENTE |
| Frecuencia de lecturas | ≥ 2 lecturas/s | PENDIENTE |

Estas mediciones requieren ambos ESP, sensor y actuador montados y sus firmwares funcionales. Se deben obtener durante la integración con la persona, registrando condiciones y evidencia real.
# Resultados del sensor

La compilación del firmware terminó con **SUCCESS** el 11 de octubre de 2026: RAM estática de 44776 bytes y flash de 743473 bytes, con configuración de ejemplo. Es evidencia de construcción, no de funcionamiento físico. Ver [pruebas](testing.md).

| Magnitud | Objetivo | Medición |
|---|---|---|
| Error absoluto de distancia | ≤ 3 cm dentro de 2–200 cm | PENDIENTE |
| Frecuencia a periodo de 250 ms | ≥ 2 lecturas/s | PENDIENTE |
| Estabilidad | ≥ 10 min sin reinicios ni bloqueos | PENDIENTE |
| Respuesta extremo a extremo | ≤ 1 s | PENDIENTE |
| Recuperación de WiFi/TCP | Reconectar y repetir HELLO | PENDIENTE |

## Datos físicos por completar

Repetir cada referencia 10 veces; registrar aparte `DIST ERR` y no incluirlo como cero al calcular el error absoluto `|lectura − referencia|`.

| Referencia (cm) | Lecturas (cm) | Máximo error absoluto (cm) | Lecturas inválidas |
|---|---|---|---|
| 2 | PENDIENTE | PENDIENTE | PENDIENTE |
| 10 | PENDIENTE | PENDIENTE | PENDIENTE |
| 49 | PENDIENTE | PENDIENTE | PENDIENTE |
| 50 | PENDIENTE | PENDIENTE | PENDIENTE |
| 99 | PENDIENTE | PENDIENTE | PENDIENTE |
| 100 | PENDIENTE | PENDIENTE | PENDIENTE |
| 200 | PENDIENTE | PENDIENTE | PENDIENTE |

No hay datos para concluir cumplimiento de los objetivos físicos. Su medición queda pendiente hasta disponer del montaje, por indicación de la persona.
