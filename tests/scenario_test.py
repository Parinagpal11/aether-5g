import subprocess
import sys
import csv
import io
from pathlib import Path
root=Path(__file__).resolve().parents[1]
exe=sys.argv[1]
for name in ['idle','single','balanced','saturation']:
    cmd=[sys.executable,str(root/'tools/run_scenario.py'),str(root/f'scenarios/{name}.json'),exe]
    a=subprocess.run(cmd,capture_output=True,check=True)
    b=subprocess.run(cmd,capture_output=True,check=True)
    assert a.stdout==b.stdout and a.stderr==b.stderr
    rows=list(csv.DictReader(io.StringIO(a.stdout.decode())))
    if name=='idle': assert not rows
    if name=='single': assert sum(int(x['bytes']) for x in rows)==2000
for args in [('rr','-1','10','50','100'),('rr','1','10','276','100'),('bad','1','10','50','100')]:
    assert subprocess.run([exe,*args],capture_output=True).returncode!=0
print('4 scenario replay checks and 3 invalid CLI checks passed')
