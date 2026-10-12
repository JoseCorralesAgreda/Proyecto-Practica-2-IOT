"""Unit and loopback TCP tests; run with python -m unittest server.test_server -v."""

import asyncio
import unittest

from server import config
from server.distance_controller import DistanceController
from server.tcp_server import TcpServer

READ_TIMEOUT_SECONDS = 3.0


class DistanceControllerTests(unittest.TestCase):
    """Verify range boundaries and recovery from measurement errors."""

    def test_range_boundaries(self) -> None:
        controller = DistanceController()
        for distance, expected in (
            (2.0, "B00"),
            (49.9, "B00"),
            (50.0, "0B0"),
            (50.1, "0B0"),
            (99.9, "0B0"),
            (100.0, "00S"),
            (100.1, "00S"),
            (200.0, "00S"),
        ):
            with self.subTest(distance=distance):
                self.assertEqual(controller.update(distance), expected)

    def test_invalid_readings_turn_off_on_third_error(self) -> None:
        for invalid in (None, 1.9, 200.1, float("nan"), float("inf")):
            with self.subTest(invalid=invalid):
                controller = DistanceController()
                controller.update(42.5)
                self.assertEqual(controller.update(invalid), "B00")
                self.assertEqual(controller.update(invalid), "B00")
                self.assertEqual(controller.update(invalid), "000")
                self.assertEqual(controller.update(150.0), "00S")
                self.assertEqual(controller.consecutive_errors, 0)

    def test_valid_reading_breaks_error_streak(self) -> None:
        controller = DistanceController()
        controller.update(75.0)
        controller.update(None)
        controller.update(None)
        controller.update(75.0)
        self.assertEqual(controller.update(None), "0B0")

    def test_reset_clears_state(self) -> None:
        controller = DistanceController()
        controller.update(42.5)
        controller.update(None)
        controller.reset()
        self.assertEqual(controller.pattern, "000")
        self.assertEqual(controller.consecutive_errors, 0)

    def test_period_validation_and_scaled_timeout(self) -> None:
        for period in (99, 5001):
            with self.subTest(period=period), self.assertRaises(ValueError):
                TcpServer(period_ms=period)
        self.assertEqual(TcpServer(period_ms=100).sensor_timeout, 2.0)
        self.assertEqual(TcpServer(period_ms=5000).sensor_timeout, 15.0)


class TcpServerTests(unittest.IsolatedAsyncioTestCase):
    """Exercise real TCP sockets on loopback without requiring ESP hardware."""

    async def asyncSetUp(self) -> None:
        self.server = TcpServer(host="127.0.0.1", port=0)
        self.writers = []
        await self.server.start()
        self.addAsyncCleanup(self.server.close)
        self.addAsyncCleanup(self.close_clients)

    async def close_clients(self) -> None:
        """Release client-side sockets even if a test assertion fails."""
        for writer in self.writers:
            writer.close()
        for writer in self.writers:
            try:
                await writer.wait_closed()
            except ConnectionError:
                pass

    async def connect(self, role=None):
        """Connect and optionally complete the role registration handshake."""
        reader, writer = await asyncio.open_connection("127.0.0.1", self.server.port)
        self.writers.append(writer)
        if role is not None:
            writer.write(f"HELLO {role}\n".encode("ascii"))
            await writer.drain()
            expected = b"PERIOD 250\n" if role == "SENSOR" else b"LED 000\n"
            self.assertEqual(await self.read_line(reader), expected)
        return reader, writer

    async def read_line(self, reader):
        """Bound each read so a protocol regression fails instead of hanging."""
        return await asyncio.wait_for(reader.readline(), READ_TIMEOUT_SECONDS)

    async def test_registration_and_all_ranges(self) -> None:
        actuator, _ = await self.connect("ACTUATOR")
        _, sensor = await self.connect("SENSOR")
        for distance, pattern in ((42.5, "B00"), (50.0, "0B0"), (100.0, "00S")):
            sensor.write(f"DIST {distance:.1f}\n".encode("ascii"))
            await sensor.drain()
            self.assertEqual(await self.read_line(actuator), f"LED {pattern}\n".encode())

    async def test_fragmented_and_concatenated_messages(self) -> None:
        actuator, _ = await self.connect("ACTUATOR")
        reader, sensor = await self.connect()
        sensor.write(b"HELLO SEN")
        await sensor.drain()
        sensor.write(b"SOR\nDIST 42.")
        await sensor.drain()
        self.assertEqual(await self.read_line(reader), b"PERIOD 250\n")
        sensor.write(b"5\nDIST 50.0\nDIST 100.0\n")
        await sensor.drain()
        for pattern in (b"B00", b"0B0", b"00S"):
            self.assertEqual(await self.read_line(actuator), b"LED " + pattern + b"\n")

    async def test_invalid_and_oversized_lines_recover_at_lf(self) -> None:
        actuator, _ = await self.connect("ACTUATOR")
        _, sensor = await self.connect("SENSOR")
        sensor.write(
            b"DIST nan\nDIST -1.0\nDIST 2\nDIST 42.50\nDIST  42.5\n"
            b"DIST 42.5\r\n\xff\nUNKNOWN\n"
            + b"X" * 4096
            + b"DIST 150.0\nDIST 75.0\n"
        )
        await sensor.drain()
        self.assertEqual(await self.read_line(actuator), b"LED 0B0\n")
        self.assertEqual(self.server.controller.consecutive_errors, 0)

    async def test_third_error_turns_off_and_valid_reading_recovers(self) -> None:
        actuator, _ = await self.connect("ACTUATOR")
        _, sensor = await self.connect("SENSOR")
        sensor.write(b"DIST 42.5\nDIST ERR\nDIST ERR\nDIST ERR\nDIST 150.0\n")
        await sensor.drain()
        for expected in (b"LED B00\n", b"LED 000\n", b"LED 00S\n"):
            self.assertEqual(await self.read_line(actuator), expected)

    async def test_numeric_out_of_range_counts_as_error(self) -> None:
        actuator, _ = await self.connect("ACTUATOR")
        _, sensor = await self.connect("SENSOR")
        sensor.write(b"DIST 100.0\nDIST 1.9\nDIST 200.1\nDIST 999.9\n")
        await sensor.drain()
        self.assertEqual(await self.read_line(actuator), b"LED 00S\n")
        self.assertEqual(await self.read_line(actuator), b"LED 000\n")

    async def test_unknown_role_closes_connection(self) -> None:
        reader, writer = await self.connect()
        writer.write(b"HELLO UNKNOWN\n")
        await writer.drain()
        self.assertEqual(await self.read_line(reader), b"")

    async def test_unregistered_and_wrong_role_messages_are_ignored(self) -> None:
        reader, writer = await self.connect()
        writer.write(b"DIST 150.0\nHELLO ACTUATOR\nDIST 150.0\nHELLO SENSOR\n")
        await writer.drain()
        self.assertEqual(await self.read_line(reader), b"LED 000\n")
        # A second HELLO ACTUATOR acts as a processing barrier.
        writer.write(b"HELLO ACTUATOR\n")
        await writer.drain()
        self.assertEqual(await self.read_line(reader), b"LED 000\n")
        self.assertNotIn("SENSOR", self.server.clients)

    async def test_duplicate_sensor_keeps_new_connection(self) -> None:
        actuator, _ = await self.connect("ACTUATOR")
        old_reader, _ = await self.connect("SENSOR")
        _, new_sensor = await self.connect("SENSOR")
        self.assertEqual(await self.read_line(old_reader), b"")
        new_sensor.write(b"DIST 75.0\n")
        await new_sensor.drain()
        self.assertEqual(await self.read_line(actuator), b"LED 0B0\n")

    async def test_duplicate_actuator_receives_current_pattern(self) -> None:
        old_reader, _ = await self.connect("ACTUATOR")
        _, sensor = await self.connect("SENSOR")
        sensor.write(b"DIST 150.0\n")
        await sensor.drain()
        self.assertEqual(await self.read_line(old_reader), b"LED 00S\n")
        new_reader, new_writer = await self.connect()
        new_writer.write(b"HELLO ACTUATOR\n")
        await new_writer.drain()
        self.assertEqual(await self.read_line(new_reader), b"LED 00S\n")
        self.assertEqual(await self.read_line(old_reader), b"")
        sensor.write(b"DIST 50.0\n")
        await sensor.drain()
        self.assertEqual(await self.read_line(new_reader), b"LED 0B0\n")

    async def test_sensor_disconnect_turns_off(self) -> None:
        actuator, _ = await self.connect("ACTUATOR")
        _, sensor = await self.connect("SENSOR")
        sensor.write(b"DIST 42.5\n")
        await sensor.drain()
        self.assertEqual(await self.read_line(actuator), b"LED B00\n")
        sensor.close()
        await sensor.wait_closed()
        self.assertEqual(await self.read_line(actuator), b"LED 000\n")

    async def test_silent_sensor_times_out_and_can_reconnect(self) -> None:
        actuator, _ = await self.connect("ACTUATOR")
        reader, sensor = await self.connect("SENSOR")
        sensor.write(b"DIST 42.5\n")
        await sensor.drain()
        self.assertEqual(await self.read_line(actuator), b"LED B00\n")
        # Incomplete data must not extend the reading deadline.
        sensor.write(b"DIST 42.")
        await sensor.drain()
        self.assertEqual(await self.read_line(reader), b"")
        # Discard the periodic refresh queued before the timeout.
        for _ in range(3):
            last_message = await self.read_line(actuator)
            if last_message == b"LED 000\n":
                break
        self.assertEqual(last_message, b"LED 000\n")
        await self.connect("SENSOR")

    async def test_unchanged_pattern_is_refreshed_after_one_second(self) -> None:
        actuator, _ = await self.connect("ACTUATOR")
        _, sensor = await self.connect("SENSOR")
        sensor.write(b"DIST 42.5\n")
        await sensor.drain()
        self.assertEqual(await self.read_line(actuator), b"LED B00\n")
        started = asyncio.get_running_loop().time()
        sensor.write(b"DIST 42.5\n")
        await sensor.drain()
        self.assertEqual(await self.read_line(actuator), b"LED B00\n")
        self.assertGreaterEqual(asyncio.get_running_loop().time() - started, 0.8)

    async def test_server_shutdown_closes_unregistered_connections(self) -> None:
        reader, writer = await self.connect()
        writer.write(b"HELLO ACTUATOR\n")
        await writer.drain()
        self.assertEqual(await self.read_line(reader), b"LED 000\n")
        unregistered_reader, _ = await self.connect()
        await asyncio.wait_for(self.server.close(), READ_TIMEOUT_SECONDS)
        self.assertEqual(await self.read_line(reader), b"")
        self.assertEqual(await self.read_line(unregistered_reader), b"")


if __name__ == "__main__":
    unittest.main()
