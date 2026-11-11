#!/usr/bin/env python3
"""Validate the Sprint 2 scenario schema before invoking the deterministic CLI."""
import json
import subprocess
import sys
from pathlib import Path


def command(config, executable):
    fields = {"version", "policy", "ues", "traffic_slots", "seed",
              "nack_per_mille", "silent_per_mille"}
    if set(config) != fields or type(config["version"]) is not int or config["version"] != 2:
        raise ValueError("unsupported bidirectional scenario schema")
    if config["policy"] not in ("rr", "pf", "qos"):
        raise ValueError("invalid policy")
    limits = {"ues": (1, 100), "traffic_slots": (1, 1000),
              "seed": (0, 4294967295), "nack_per_mille": (0, 1000),
              "silent_per_mille": (0, 1000)}
    for key, (low, high) in limits.items():
        if type(config[key]) is not int or not low <= config[key] <= high:
            raise ValueError(f"invalid {key}")
    return [str(executable), config["policy"],
            *[str(config[key]) for key in limits]]


def main():
    if len(sys.argv) != 3:
        raise ValueError("usage: run_bidirectional.py CONFIG EXECUTABLE")
    config = json.loads(Path(sys.argv[1]).read_text())
    if not isinstance(config, dict):
        raise ValueError("scenario must be an object")
    return subprocess.run(command(config, sys.argv[2]), check=False).returncode


if __name__ == "__main__":
    try:
        sys.exit(main())
    except (ValueError, OSError) as error:
        print(error, file=sys.stderr)
        sys.exit(2)
