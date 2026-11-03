"""Replay only offline checks from this package. Expected rejection is success."""
from pathlib import Path
import json
import subprocess
import sys
from datetime import datetime, timezone

ROOT=Path(__file__).resolve().parents[2]
OUT=ROOT/'sim/skill_validation/results'
OUT.mkdir(parents=True,exist_ok=True)
cases=[
('golden_match',0,'skill/hls-golden/scripts/compare_json.py',['--golden','skill/hls-golden/examples/golden.json','--actual','skill/hls-golden/examples/actual.json']),
('color_hls',0,'skill/hls-golden/scripts/inspect_hls.py',['--synthesis','skill/evidence/e03_color_csynth.xml','--cosim','skill/evidence/e04_color_cosim.rpt']),
('historical_receipt_conflict',1,'skill/hls-golden/scripts/inspect_hls.py',['--synthesis','skill/evidence/e08_components_csynth.xml','--cosim','skill/evidence/e07_components_cosim.rpt','--receipt','skill/evidence/e06_components_old_receipt.json']),
('current_timing_rejected',1,'skill/vivado-report-gate/scripts/check_reports.py',['--timing','skill/evidence/e12_timing.rpt','--check-timing','skill/evidence/e13_check_timing.rpt','--drc','skill/evidence/e14_drc.rpt','--utilization','skill/evidence/e15_utilization.rpt']),
('manifest_fixture',0,'skill/release-provenance/scripts/verify_manifest.py',['--root','skill/release-provenance/examples','--manifest','skill/release-provenance/examples/manifest.json']),
('coordinate_preview',0,'skill/vision-motion-contract/scripts/preview_task.py',['--result','skill/vision-motion-contract/examples/frame.json','--config','skill/vision-motion-contract/examples/config.json']),
('pynq_metadata_fixture',0,'skill/pynq-overlay-portability/scripts/preflight.py',['--bit','skill/pynq-overlay-portability/examples/demo.bit','--hwh','skill/pynq-overlay-portability/examples/demo.hwh','--contract','skill/pynq-overlay-portability/examples/contract.json']),
]
log=[]
for name,expected,script,args in cases:
    command=[sys.executable,'-X','utf8',script,*args]
    result=subprocess.run(command,cwd=ROOT,capture_output=True,text=True,encoding='utf-8',timeout=30)
    (OUT/(name+'.json')).write_text(result.stdout,encoding='utf-8')
    record={'name':name,'command':['python','-X','utf8',script,*args],'expected_exit':expected,'actual_exit':result.returncode,'expectation_met':result.returncode==expected,'result_file':'sim/skill_validation/results/'+name+'.json','stderr':result.stderr}
    log.append(record)
    print(name+': '+('OK' if record['expectation_met'] else 'UNEXPECTED'))
receipt={'timestamp_utc':datetime.now(timezone.utc).isoformat(),'passed':all(c['expectation_met'] for c in log),'hardware_access':False,'cases':log,'scope':'offline report and fixture replay; expected rejection is preserved'}
(ROOT/'sim/skill_validation/replay_log.json').write_text(json.dumps(receipt,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
raise SystemExit(0 if receipt['passed'] else 1)
