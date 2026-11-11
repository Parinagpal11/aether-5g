"""Replay full grant traces and check the public command-line contract."""
import csv
import io
import json
from pathlib import Path
import subprocess
import sys

root = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(root / "tools"))
from run_bidirectional import command

exe = sys.argv[1]
config = json.loads((root / "scenarios/bidirectional.json").read_text())
for policy in ("rr", "pf", "qos"):
    config["policy"] = policy
    args = command(config, exe) + ["--trace"]
    first = subprocess.run(args, capture_output=True, check=True)
    second = subprocess.run(args, capture_output=True, check=True)
    if (first.stdout, first.stderr) != (second.stdout, second.stderr):
        raise AssertionError("replay mismatch")
    summary = json.loads(first.stderr)
    for direction in ("ul", "dl"):
        a = summary[direction]
        if a["queued"] or a["in_flight"] or a["admitted"] != a["delivered"] + a["failed"]:
            raise AssertionError("non-conserved or undrained bytes")
        if not a["delivered"]:
            raise AssertionError("no successful delivery")
    allocations = {}
    for row in csv.DictReader(io.StringIO(first.stdout.decode())):
        key = (row["slot"], row["direction"])
        used, ids = allocations.setdefault(key, (0, set()))
        start, count = int(row["first_prb"]), int(row["prbs"])
        if start != used or count <= 0 or used + count > 100 or row["ue"] in ids:
            raise AssertionError("invalid PRB allocation")
        ids.add(row["ue"])
        allocations[key] = (used + count, ids)
    if not summary["retries"]:
        raise AssertionError("retransmission path not exercised")

for key, value in (("ues", 0), ("traffic_slots", -1), ("seed", True),
                   ("nack_per_mille", 1001), ("policy", "bad"), ("version", 1)):
    invalid = dict(config, **{key: value})
    try:
        command(invalid, exe)
    except ValueError:
        pass
    else:
        raise AssertionError(f"accepted invalid {key}")
for args in ([], ["rr","0","10","42","0","0"], ["rr","1","10","42","1001","0"],
             ["rr","1","-1","42","0","0"], ["bad","1","10","42","0","0"],
             ["rr","1","10","42","0","0","--bogus"]):
    if subprocess.run([exe, *args], capture_output=True).returncode == 0:
        raise AssertionError("accepted invalid CLI")
print("Three 100-UE full-trace replays, conservation, PRB bounds and invalid inputs passed")
