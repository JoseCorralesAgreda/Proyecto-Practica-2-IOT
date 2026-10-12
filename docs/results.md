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
