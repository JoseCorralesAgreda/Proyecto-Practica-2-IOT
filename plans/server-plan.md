# Subplan del servidor

Pendiente de revisión antes de implementar. Derivado del plan general aprobado el 11 de octubre de 2026. Se mantiene `protocolo.md` por instrucción expresa.

## Archivos y responsabilidades

- `server/main.py`: entrada y ejecución de `asyncio`.
- `server/config.py`: puerto, rangos y tiempos acordados.
- `server/tcp_server.py`: clase `TcpServer`, conexiones por rol, lectura de líneas, envío de órdenes y timeouts.
- `server/distance_controller.py`: clase `DistanceController`, clasificación y contador de lecturas inválidas.
- `server/test_server.py`: pruebas con `unittest` de control y comunicación TCP.

## Pasos

1. Implementar el registro `HELLO` y reemplazo de conexiones duplicadas, cuidando que el cierre de una conexión antigua no elimine la nueva.
2. Procesar el flujo por LF, mensajes fragmentados y concatenados y descarte de líneas largas conforme a `protocolo.md`.
3. Clasificar lecturas válidas de 2 a 200 cm: límites de 50 y 100 cm; emitir los patrones aprobados.
4. Aplicar `PERIOD`, refresco de `LED`, tres errores consecutivos y timeout del sensor. Conservar la última actuación durante los dos primeros errores consecutivos.
5. Verificar límites, roles duplicados, entradas inválidas y desconexiones con clientes simulados. Resolver con la persona cualquier ambigüedad del protocolo antes de codificar la regla afectada.
6. Documentar requerimientos implementados, arquitectura, clases, secuencia, protocolo y pruebas en `docs/`.

Aceptación: pruebas automatizadas satisfactorias y control conforme al protocolo. Las mediciones de extremo a extremo quedan para integración física.
