"""Distance classification independent of networking."""

import math

from server import config


class DistanceController:
    """Keep the LED pattern and tolerate two consecutive invalid readings."""

    def __init__(self) -> None:
        self.pattern = config.OFF_PATTERN
        self.consecutive_errors = 0

    def update(self, distance_cm: float | None) -> str:
        """Classify a reading; None represents DIST ERR."""
        if (
            distance_cm is None
            or not math.isfinite(distance_cm)
            or not config.MIN_DISTANCE_CM <= distance_cm <= config.MAX_DISTANCE_CM
        ):
            self.consecutive_errors += 1
            if self.consecutive_errors >= config.MAX_CONSECUTIVE_ERRORS:
                self.pattern = config.OFF_PATTERN
            return self.pattern

        self.consecutive_errors = 0
        if distance_cm < config.NEAR_LIMIT_CM:
            self.pattern = config.NEAR_PATTERN
        elif distance_cm < config.FAR_LIMIT_CM:
            self.pattern = config.MIDDLE_PATTERN
        else:
            self.pattern = config.FAR_PATTERN
        return self.pattern

    def reset(self) -> None:
        """Turn the LEDs off and clear errors after sensor loss."""
        self.pattern = config.OFF_PATTERN
        self.consecutive_errors = 0
