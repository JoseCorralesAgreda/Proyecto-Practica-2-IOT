"""Persistent TCP connections for one sensor and one actuator."""

import asyncio
import logging
import re
import time

from server import config
from server.distance_controller import DistanceController

LOGGER = logging.getLogger(__name__)
DISTANCE_MESSAGE = re.compile(r"DIST ([0-9]{1,3}\.[0-9]|ERR)")
HELLO_MESSAGE = re.compile(r"HELLO ([^ ]+)")


class TcpServer:
    """Coordinate client roles, distance updates, refreshes and sensor loss."""

    def __init__(
        self,
        host: str = config.HOST,
        port: int = config.PORT,
        period_ms: int = config.PERIOD_MS,
    ) -> None:
        if not config.MIN_PERIOD_MS <= period_ms <= config.MAX_PERIOD_MS:
            raise ValueError("Sampling period must be between 100 and 5000 ms")
        self.host = host
        self.port = port
        self.period_ms = period_ms
        self.sensor_timeout = max(
            config.SENSOR_MIN_TIMEOUT_SECONDS,
            config.SENSOR_TIMEOUT_PERIODS * period_ms / 1000,
        )
        self.controller = DistanceController()
        self.clients: dict[str, asyncio.StreamWriter] = {}
        self.last_sensor_reading = 0.0
        self.last_led_send = 0.0
        self._listener: asyncio.Server | None = None
        self._maintenance_task: asyncio.Task | None = None
        self._connections: set[asyncio.StreamWriter] = set()
        self._handlers: set[asyncio.Task] = set()
        self._closing = False

    async def start(self) -> None:
        """Start listening; port zero selects a free port for local tests."""
        self._listener = await asyncio.start_server(
            self._handle_client, self.host, self.port
        )
        self.port = self._listener.sockets[0].getsockname()[1]
        self._maintenance_task = asyncio.create_task(self._maintain_clients())
        LOGGER.info("Listening on %s:%s", self.host, self.port)

    async def serve_forever(self) -> None:
        """Serve connections until cancelled, then release sockets and tasks."""
        await self.start()
        try:
            await self._listener.serve_forever()
        finally:
            await self.close()

    async def close(self) -> None:
        """Close all connections, including clients that never registered."""
        self._closing = True
        if self._listener is not None:
            self._listener.close()
        if self._maintenance_task is not None:
            self._maintenance_task.cancel()
            await asyncio.gather(self._maintenance_task, return_exceptions=True)
        for writer in list(self._connections):
            writer.close()
        handlers = list(self._handlers)
        for task in handlers:
            task.cancel()
        await asyncio.gather(*handlers, return_exceptions=True)
        if self._listener is not None:
            await self._listener.wait_closed()
        self.clients.clear()
        self.controller.reset()

    async def _handle_client(
        self, reader: asyncio.StreamReader, writer: asyncio.StreamWriter
    ) -> None:
        task = asyncio.current_task()
        self._handlers.add(task)
        self._connections.add(writer)
        role = None
        line = bytearray()
        discarding = False
        LOGGER.info("Client connected: %s", writer.get_extra_info("peername"))
        try:
            while not self._closing and not writer.is_closing():
                data = await reader.read(config.READ_CHUNK_BYTES)
                if not data:
                    break
                for byte in data:
                    if writer.is_closing():
                        break
                    if byte == ord("\n"):
                        if not discarding:
                            role = await self._handle_line(bytes(line), role, writer)
                        line.clear()
                        discarding = False
                    elif not discarding:
                        line.append(byte)
                        if len(line) >= config.MAX_LINE_BYTES:
                            LOGGER.warning("Ignoring oversized line")
                            line.clear()
                            discarding = True
        except (ConnectionError, OSError) as error:
            LOGGER.info("Client connection lost: %s", error)
        finally:
            # An old handler must never remove its replacement's registration.
            if role is not None and self.clients.get(role) is writer:
                del self.clients[role]
                if role == "SENSOR":
                    self.controller.reset()
                    if not self._closing:
                        await self._send_led()
            writer.close()
            try:
                await writer.wait_closed()
            except (ConnectionError, OSError):
                pass
            self._connections.discard(writer)
            self._handlers.discard(task)

    async def _handle_line(
        self, raw_line: bytes, role: str | None, writer: asyncio.StreamWriter
    ) -> str | None:
        try:
            message = raw_line.decode("ascii")
        except UnicodeDecodeError:
            LOGGER.warning("Ignoring non-ASCII line")
            return role

        hello = HELLO_MESSAGE.fullmatch(message)
        if hello is not None:
            requested_role = hello.group(1)
            if requested_role not in ("SENSOR", "ACTUATOR"):
                LOGGER.warning("Closing connection with invalid role: %r", message)
                writer.close()
                return role
            if role is not None and role != requested_role:
                LOGGER.warning("Ignoring role change on registered connection")
                return role
            if self.clients.get(requested_role) is not writer:
                previous = self.clients.get(requested_role)
                self.clients[requested_role] = writer
                if previous is not None:
                    previous.close()
                if requested_role == "SENSOR":
                    self.last_sensor_reading = time.monotonic()
                LOGGER.info("Registered %s", requested_role)
            if requested_role == "SENSOR":
                await self._send(writer, f"PERIOD {self.period_ms}")
            else:
                await self._send_led()
            return requested_role

        distance = DISTANCE_MESSAGE.fullmatch(message)
        if (
            distance is not None
            and role == "SENSOR"
            and self.clients.get("SENSOR") is writer
        ):
            value = distance.group(1)
            distance_cm = None if value == "ERR" else float(value)
            self.last_sensor_reading = time.monotonic()
            previous_pattern = self.controller.pattern
            pattern = self.controller.update(distance_cm)
            LOGGER.info("Received %s; LED %s", message, pattern)
            if pattern != previous_pattern:
                await self._send_led()
        else:
            LOGGER.warning("Ignoring unexpected or malformed line: %r", message)
        return role

    async def _send(self, writer: asyncio.StreamWriter, message: str) -> None:
        try:
            writer.write((message + "\n").encode("ascii"))
            await asyncio.wait_for(writer.drain(), config.WRITE_TIMEOUT_SECONDS)
        except (ConnectionError, OSError, TimeoutError) as error:
            LOGGER.info("Closing failed output connection: %s", error)
            writer.close()

    async def _send_led(self) -> None:
        writer = self.clients.get("ACTUATOR")
        if writer is not None and not writer.is_closing():
            # Record before awaiting I/O so refreshes do not race this send.
            self.last_led_send = time.monotonic()
            await self._send(writer, f"LED {self.controller.pattern}")

    async def _maintain_clients(self) -> None:
        while True:
            await asyncio.sleep(config.MAINTENANCE_INTERVAL_SECONDS)
            now = time.monotonic()
            sensor = self.clients.get("SENSOR")
            if (
                sensor is not None
                and now - self.last_sensor_reading >= self.sensor_timeout
            ):
                LOGGER.warning("Sensor timed out")
                del self.clients["SENSOR"]
                sensor.close()
                self.controller.reset()
                await self._send_led()
            if now - self.last_led_send >= config.LED_REFRESH_SECONDS:
                await self._send_led()
