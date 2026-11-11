#!/usr/bin/env python3
"""Validate versioned workload configuration, then invoke the C++ slot simulator."""
import json
import subprocess
import sys
from pathlib import Path

def main():
    if len(sys.argv) != 3:
        raise ValueError('usage: run_scenario.py CONFIG EXECUTABLE')
    c = json.loads(Path(sys.argv[1]).read_text())
    expected = {'version', 'policy', 'ues', 'slots', 'prbs', 'bytes_per_slot', 'seed', 'invariants'}
    if set(c) != expected or c['version'] != 1 or c['policy'] not in ('rr', 'pf', 'qos'):
        raise ValueError('unsupported scenario schema')
    for key, maximum in [('ues', 10000), ('slots', 10000000), ('prbs', 275), ('bytes_per_slot', 4294967295), ('seed', 4294967295)]:
        if type(c[key]) is not int or not 0 <= c[key] <= maximum:
            raise ValueError(f'invalid {key}')
    # Seed retained for schema evolution; current arrivals and channels are constant.
    return subprocess.run([sys.argv[2], c['policy'], *[str(c[k]) for k in ('ues', 'slots', 'prbs', 'bytes_per_slot')]], check=False).returncode

if __name__ == '__main__':
    try:
        sys.exit(main())
    except (ValueError, OSError) as exc:
        print(str(exc), file=sys.stderr)
        sys.exit(2)
