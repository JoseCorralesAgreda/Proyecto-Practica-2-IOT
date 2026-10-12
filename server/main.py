"""Run with python -m server.main from the repository root."""

import asyncio
import logging

from server.tcp_server import TcpServer


def main() -> None:
    """Configure console logging and run the TCP server until Ctrl+C."""
    logging.basicConfig(
        level=logging.INFO,
        format="%(asctime)s %(levelname)s %(message)s",
    )
    try:
        asyncio.run(TcpServer().serve_forever())
    except KeyboardInterrupt:
        logging.info("Server stopped")
    except OSError as error:
        logging.error("Cannot run TCP server: %s", error)
        raise SystemExit(1) from error


if __name__ == "__main__":
    main()
