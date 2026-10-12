# Secuencias del servidor

Intercambio implementado y probado con clientes simulados. El parpadeo físico del actuador queda pendiente.

```mermaid
sequenceDiagram
    participant S as Cliente sensor
    participant T as TcpServer
    participant C as DistanceController
    participant A as Cliente actuador
    A->>T: TCP connect + HELLO ACTUATOR LF
    T->>A: LED 000 LF
    S->>T: TCP connect + HELLO SENSOR LF
    T->>S: PERIOD 250 LF
    S->>T: DIST 42.5 LF
    T->>C: update(42.5)
    C-->>T: B00
    T->>A: LED B00 LF
    Note over T,A: Refresco cada 1 s aunque no cambie el patrón
    S->>T: DIST ERR LF (primero y segundo)
    Note over C: Conserva B00
    S->>T: DIST ERR LF (tercero)
    T->>C: update(None)
    C-->>T: 000
    T->>A: LED 000 LF
```

```mermaid
sequenceDiagram
    participant Old as Sensor anterior
    participant New as Sensor nuevo
    participant T as TcpServer
    participant A as Actuador
    New->>T: TCP connect + HELLO SENSOR LF
    T->>T: Asociar SENSOR al socket nuevo
    T-->>Old: Cerrar socket anterior
    T->>New: PERIOD 250 LF
    Note over T: La limpieza del socket anterior conserva el registro nuevo
    New->>T: DIST 150.0 LF
    T->>A: LED 00S LF
    Note over New,T: Sin DIST válido durante max(2 s, 3 × PERIOD)
    T-->>New: Cerrar socket
    T->>T: Reiniciar control
    T->>A: LED 000 LF
```

`DIST ERR` es un mensaje válido y renueva el plazo de recepción, aunque cuente como fallo de medición. Los mensajes malformados y las líneas parciales no renuevan ese plazo. Una desconexión explícita del sensor apaga el estado sin esperar el timeout.
