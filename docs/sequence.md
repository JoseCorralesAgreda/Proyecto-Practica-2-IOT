# Diagramas de secuencia

El servidor intercambia mensajes con ambos clientes. La operación del servidor está

**verificada con clientes simulados** por TCP en loopback y con el prototipo físico integrado.

## Operación normal

```mermaid
sequenceDiagram
    autonumber
    participant S as Sensor (ESP32)
    participant SRV as Servidor (PC)
    participant A as Actuador (ESP32)

    Note over S,A: Fase 1 — registro
    A->>SRV: connect (TCP)
    A->>SRV: HELLO ACTUATOR
    SRV->>A: LED 000
    S->>SRV: connect (TCP)
    S->>SRV: HELLO SENSOR
    SRV->>S: PERIOD 250

    Note over S,A: Fase 2 — operación
    loop cada PERIOD ms
        S->>SRV: DIST 42.5
        SRV->>SRV: clasificar → cerca
        opt el patrón cambió
            SRV->>A: LED B00
        end
    end
    loop cada 1 s
        SRV->>A: LED B00 (refresco)
    end
    Note over A: parpadeo local, ciclo de 500 ms
```

## Errores y pérdida de conexión

```mermaid
sequenceDiagram
    autonumber
    participant S as Sensor (ESP32)
    participant SRV as Servidor (PC)
    participant A as Actuador (ESP32)

    Note over S,SRV: Errores de medición
    S->>SRV: DIST ERR (1.º y 2.º)
    Note over SRV: conserva el patrón
    S->>SRV: DIST ERR (3.º)
    SRV->>A: LED 000

    Note over S,SRV: Pérdida del sensor
    S--xSRV: deja de enviar DIST
    Note over SRV: silencio ≥ max(2 s, 3 × PERIOD)
    SRV->>SRV: cierra el socket y reinicia el control
    SRV->>A: LED 000
    S->>SRV: connect + HELLO SENSOR (cada 3 s)
    SRV->>S: PERIOD 250

    Note over SRV,A: Pérdida del servidor o de la red del actuador
    SRV--xA: deja de enviar LED
    Note over A: 5 s sin órdenes válidas
    A->>A: apaga los LED y cierra el socket
    A->>SRV: connect + HELLO ACTUATOR (cada 3 s)
    SRV->>A: LED (patrón actual)
```

## Notas de comportamiento

- `DIST ERR` es un mensaje válido y renueva el plazo de recepción del sensor, aunque cuente como fallo de medición. Los mensajes malformados y las líneas parciales no renuevan ese plazo.
- Una desconexión explícita del sensor apaga el estado sin esperar el timeout.
- Un actuador que se registra recibe de inmediato el patrón vigente, aunque antes no estuviera conectado.
- La secuencia completa prevista del sistema, incluida la variante con servomotor, se conserva en [protocolo.md](../protocolo.md).
