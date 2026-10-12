# AGENTS.md

Guía de trabajo para el agente de IA en la **Práctica 2 — Integración de Objetos Inteligentes con TCP/IP** (SIS-234).

## 0. Documento de referencia: `practica.md`

El archivo **`practica.md`** (en la raíz del repositorio) es el enunciado oficial de la práctica. Es la fuente de verdad sobre lo que se debe construir y entregar: descripción del sistema, requerimientos funcionales y no funcionales, estructura del informe, entregables, rúbricas de evaluación y condiciones de entrega.

- **Léelo completo antes de empezar** cualquier tarea y vuelve a consultarlo cuando tengas dudas sobre qué se pide.
- Este `AGENTS.md` **no repite** lo que está en `practica.md`; solo define cómo trabajar. Si algo parece contradecirse, `practica.md` manda en lo que se exige, y este archivo manda en cómo se trabaja.
- Si `practica.md` deja algo abierto, no lo asumas: consulta (ver §4).

---

## 1. Idioma

| Qué | Idioma |
|---|---|
| Código (nombres de clases, funciones, variables, constantes, comentarios, docstrings, mensajes de log) | **Inglés** |
| Nombres de archivos y carpetas | **Inglés** |
| Mensajes del protocolo de aplicación (comandos, campos) | **Inglés** |
| Documentación, planes, informe e interacción con la persona | **Español** |

---

## 2. Principios para el código

El proyecto es pequeño, tiene resultados acotados y **no se va a extender**. Todo el código se juzga por una pregunta: *¿un compañero lo entiende a la primera lectura?*

- **Legibilidad por encima de todo.** Nombres claros, funciones cortas, flujo directo.
- **Nada ingenioso.** Sin trucos, sin one-liners densos, sin metaprogramación ni patrones innecesarios. Si hay una forma obvia y otra "elegante", usa la obvia.
- **Sin overengineering.** No agregues abstracciones, interfaces, fábricas, configuraciones o capas "por si acaso". No diseñes para extensibilidad futura. Si algo solo se usa en un lugar, no lo generalices.
- **Orientado a objetos y modular**, como exige `practica.md`: pocas clases, cada una con una responsabilidad evidente. Sin jerarquías de herencia salvo que haya una razón clara.
- **Buenas prácticas del lenguaje elegido.** Sigue su guía de estilo oficial y sus convenciones de nombres, formato y estructura. Usa el formateador/linter estándar del lenguaje si existe.
- **Documentado.** Docstrings o comentarios en clases y métodos públicos que expliquen *qué hace* y *por qué*, no cada línea. Comenta lo no obvio (unidades, rangos, temporización).
- **Manejo de errores simple pero presente.** Desconexiones, timeouts, mensajes mal formados y lecturas inválidas del sensor deben tener un comportamiento definido y documentado.
- **Constantes con nombre.** Rangos, puertos, pines e intervalos en un solo lugar visible, sin números mágicos.
- **Sin secretos en el repositorio.** Credenciales WiFi en un archivo de ejemplo (`*.example`); el real queda fuera de git.

### Organización de archivos

Misma regla: lo más simple posible. Una carpeta por componente, una de documentación y una de planes. Sin estructuras profundas ni carpetas vacías "para el futuro". La estructura concreta se propone en el plan general y la aprueba la persona.

```
practica.md      enunciado (referencia, no se modifica)
AGENTS.md        este archivo
README.md        cómo ejecutar el proyecto (breve)
report.md        informe técnico final
server/          código del servidor TCP
sensor/          código del objeto con sensor ultrasónico
actuator/        código del objeto con actuadores
docs/            documentación viva del proyecto (ver §5)
plans/           planes de trabajo de un solo uso (ver §3)
```

---

## 3. Forma de trabajo: planes

Se trabaja con **un plan general y exactamente tres subplanes**. No se crean más.

1. **Plan general** (`plans/general-plan.md`), de alto nivel: arquitectura, decisiones clave, estructura de carpetas y orden de trabajo. Debe ser aprobado por la persona antes de continuar.
2. **Tres subplanes de implementación**, derivados del plan general, uno por componente:
   - `plans/server-plan.md`
   - `plans/sensor-plan.md`
   - `plans/actuator-plan.md`

Reglas:

- Los subplanes son cortos y se enfocan en implementar: clases, archivos y pasos. No hay subplanes de segundo nivel ni planes adicionales por tarea.
- Los planes son **de un solo uso**: guían la implementación y no se mantienen actualizados después. Si algo cambia durante la implementación, avisa a la persona; no reescribas el plan.
- **Los planes no son documentación** y no sustituyen a `docs/`.
- Un plan no se ejecuta hasta que la persona lo haya revisado.

---

## 4. Cuándo preguntar

**Consulta antes de tomar cualquier decisión significativa que no esté definida** en `practica.md`, en este archivo o en un plan aprobado. Por ejemplo:

- Elegir lenguaje, librerías o plataforma de hardware de cada componente.
- Elegir la variante del actuador (LEDs o servomotor).
- Definir rangos de distancia, comportamientos de actuación y valores objetivo de los requerimientos no funcionales.
- Definir o cambiar el formato del protocolo de aplicación.
- Cambiar la estructura de carpetas o la arquitectura.
- Agregar una dependencia nueva.
- Desviarte de un plan aprobado.

Al preguntar, ofrece dos o tres opciones concretas con una recomendación breve y espera la respuesta. Las decisiones menores y obvias (nombre de una variable, orden de métodos) no requieren consulta.

---

## 5. Documentación

La documentación es **distinta de los planes**:

- Se genera **después de implementar** el código correspondiente (no antes, no como plan).
- Vive en `docs/` y **se actualiza** cuando el código o el diseño cambian. Los planes no se actualizan; la documentación sí.
- Se escribe **a medida que avanza el proyecto**, pieza por pieza, no al final.
- Cubre todo lo que `practica.md` exige para el informe (§4.2), de modo que el informe final sea solo la síntesis de lo ya documentado. Un archivo por tema:

```
docs/requirements.md      requerimientos funcionales y no funcionales
docs/architecture.md      diagrama de arquitectura
docs/circuits.md          circuito de cada objeto inteligente
docs/classes.md           diagrama de clases
docs/sequence.md          diagrama de secuencia
docs/protocol.md          especificación del protocolo de aplicación
docs/implementation.md    estructura del código y fragmentos clave, con enlaces al código fuente
docs/testing.md           plan de pruebas, procedimiento, datos y análisis de errores
docs/results.md           resultados cuantificables y análisis
```

Reglas de documentación:

- Diagramas en **Mermaid** dentro de Markdown cuando sea posible. Los diagramas de circuito pueden ser imagen (esquema o fotografía) guardada en `docs/` y referenciada.
- **No inventes datos.** Nunca escribas mediciones, tiempos, errores o resultados de pruebas que no se hayan obtenido realmente. Si faltan, deja la tabla preparada con huecos marcados como `PENDIENTE` y pide a la persona que los mida.
- La documentación debe ser coherente con el código real: si cambias el código, actualiza el documento afectado en el mismo trabajo.
- Tono técnico, claro y breve.

### Informe final (`report.md`)

Se arma **al final**, condensando y enlazando lo ya documentado en `docs/`, con la estructura que pide `practica.md`. No incluyas contenido nuevo que no exista antes en la documentación, salvo conclusiones y recomendaciones, que se redactan con la persona.

---

## 6. Git

- **No hagas commits ni push por tu cuenta** salvo que la persona lo pida explícitamente. Cada integrante debe commitear con su propia cuenta (ver `practica.md`).
- Si te piden un commit: mensaje en inglés, claro, un cambio lógico por commit.

---

## 7. Resumen rápido

1. Lee `practica.md`; es la fuente de verdad de lo exigido.
2. Código, archivos y protocolo en inglés; documentación, planes e interacción en español.
3. Plan general → aprobación → 3 subplanes (server, sensor, actuator).
4. Código simple, legible, orientado a objetos, sin overengineering.
5. Documentar en `docs/` después de implementar cada pieza.
6. Preguntar antes de decidir lo que no esté definido.
7. Nunca inventar datos de pruebas.
8. El informe final es solo la síntesis de la documentación.
