"""Run the portable package's offline behavioral regressions. Python 3.10+."""
import argparse
import io
import json
from pathlib import Path
import sys
import unittest

if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--output');a=p.parse_args()
    root=Path(__file__).resolve().parent
    stream=io.StringIO()
    suite=unittest.defaultTestLoader.discover(str(root/'tests'),pattern='test_*.py')
    result=unittest.TextTestRunner(stream=stream,verbosity=2).run(suite)
    print(stream.getvalue())
    receipt={'passed':result.wasSuccessful(),'tests_run':result.testsRun,'failures':len(result.failures),'errors':len(result.errors),'skipped':len(result.skipped),'hardware_access':False,'python':sys.version.split()[0],'scope':'portable scripts and recorded evidence replay; no new Vivado or board run'}
    if a.output:
        output=Path(a.output);output.parent.mkdir(parents=True,exist_ok=True)
        output.write_text(json.dumps(receipt,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print(json.dumps(receipt,ensure_ascii=False))
    raise SystemExit(0 if result.wasSuccessful() else 1)
