# Protocolo implementado por el servidor

La especificación de referencia se conserva en [protocolo.md](../protocolo.md). Esta sección describe el comportamiento del servidor actual y las precisiones del plan general aprobado.

## Formato y mensajes

TCP persistente, ASCII estricto, separador de un espacio y terminador LF (`\n`). Límite de 32 bytes incluyendo LF: hasta 31 bytes de contenido. Se ignora una línea excesiva completa hasta LF; la siguiente línea se procesa normalmente. Se reconstruyen mensajes fragmentados y se separan mensajes concatenados. CRLF no forma parte de la gramática.

| Dirección | Mensaje | Validación / respuesta |
|---|---|---|
| Cliente → servidor | `HELLO SENSOR\n` | Registra sensor; responde `PERIOD 250\n` por defecto |
| Cliente → servidor | `HELLO ACTUATOR\n` | Registra actuador; responde `LED` con el patrón actual |
| Sensor → servidor | `DIST 42.5\n` | De uno a tres dígitos, punto y exactamente un decimal |
| Sensor → servidor | `DIST ERR\n` | Lectura inválida, contabilizada sin cerrar la conexión |
| Servidor → sensor | `PERIOD 250\n` | Periodo configurado de 100 a 5000 ms |
| Servidor → actuador | `LED B00\n` | Tres estados en orden rojo, naranja y verde: `0`, `S`, `B` |

La gramática se valida completa. Valores como `nan`, enteros sin decimal, negativos y espacios adicionales se ignoran y registran en el log. Los valores numéricos sintácticamente válidos fuera de 2 a 200 cm se contabilizan como lecturas inválidas. El formato `DIST ERR` debe ser usado por el futuro firmware para esos casos.

## Registro y control

- Sin registro no se aceptan distancias. El actuador tampoco puede enviar mediciones ni cambiar de rol dentro del mismo socket.
- Un `HELLO` con un único argumento de rol inválido cierra el socket, según la regla específica de registro. Otras líneas malformadas se ignoran sin respuesta.
- Un nuevo socket del mismo rol reemplaza al anterior. La limpieza del anterior no elimina el nuevo. Se conserva el patrón al reemplazar el sensor hasta recibir nuevas lecturas o cumplirse su timeout.
- Repetir `HELLO` del mismo rol en el mismo socket devuelve la respuesta correspondiente; no prolonga el timeout del sensor.
- Dos lecturas inválidas consecutivas conservan el patrón; la tercera produce `000`. Una lectura válida reinicia el contador.
- Se envía `LED` cuando cambia el patrón y como refresco cada segundo. El actuador recibe el estado actual al registrarse, aunque antes no estuviera conectado.
- Desconexión explícita o silencio del sensor durante `max(2 s, 3 × PERIOD)` produce apagado y cierre del socket. Se admiten nuevos registros después de la falla.

## Tiempos y alcance

El periodo inicial es 250 ms. El servidor revisa los plazos con reloj monotónico cada 50 ms; la planificación del sistema y las escrituras pueden añadir demora. Una escritura bloqueada se limita a 1 s antes de cerrar esa conexión.

El parpadeo de 500 ms, el timeout de 5 s del actuador, la limitación de `PERIOD` y los reintentos cada 3 s corresponden a los firmwares pendientes. El servidor emite el periodo configurado y rechaza su propia configuración si sale de 100 a 5000 ms. No se agregaron comandos al protocolo.
# Protocolo implementado en el sensor

Referencia del sistema: [protocolo.md](../protocolo.md), conservado sin cambios. Esta sección describe únicamente el cliente sensor implementado.

| Sentido | Formato con terminador | Comportamiento |
|---|---|---|
| Sensor → servidor | `HELLO SENSOR\n` | Primer mensaje en cada conexión TCP |
| Servidor → sensor | `PERIOD 250\n` | Un espacio y 3–4 dígitos; valor limitado a 100–5000 ms |
| Sensor → servidor | `DIST 42.5\n` | Centímetros con un decimal; rango físico válido de 2 a 200 cm |
| Sensor → servidor | `DIST ERR\n` | Sin eco, fuera de rango o configuración de hardware sin confirmar |

ASCII sobre TCP persistente al puerto configurable 5000. Se utiliza IPv4 literal. El periodo inicial y de respaldo de cada sesión es 250 ms; no hay ACK de lectura.

El buffer admite 31 bytes de contenido más LF, siguiendo el cómputo de bytes de los ejemplos de la referencia. Ningún `PERIOD` válido alcanza ese límite. Las líneas más largas se descartan completas hasta LF; los fragmentos permanecen entre iteraciones. Líneas desconocidas, vacías, con CR, signos, caracteres no numéricos, bytes nulos, espacios adicionales o cantidad incorrecta de dígitos se ignoran. Por ejemplo, `PERIOD 000` se limita a 100 y `PERIOD 9999` a 5000; `PERIOD 50` se ignora por no cumplir la gramática de 3–4 dígitos.

Una pérdida detectada de WiFi o TCP inicia recuperación; el reintento se programa cada 3 s y repite `HELLO`. Los envíos incompletos cierran el socket. El estado del parser se limpia entre sesiones. El cliente no clasifica distancias ni controla actuadores.
