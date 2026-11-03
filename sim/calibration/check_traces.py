"""Check frozen motion traces, simulation assertions and source identities offline."""
from pathlib import Path
import argparse,hashlib,json,re

VARIANTS=('original_calibrated','known_good_integrated','current_integrated')
HERE=Path(__file__).resolve().parent

def inspect(root,check_sources=True):
    issues=[];rows={};traces={}
    receipt=json.loads((root/'status.json').read_text(encoding='utf-8')) if check_sources else None
    for name in VARIANTS:
        directory=root/name
        log=(directory/'trace.log').read_text(encoding='utf-8',errors='replace')
        events=[s.strip() for s in log.splitlines() if s.startswith('TRACE ')]
        frozen=(directory/'motion_trace.txt').read_text(encoding='utf-8').splitlines()
        matches=re.findall(r'PASS original_motion_trace assertions=(\d+)',log)
        if re.search(r'Fatal:|Error:',log):issues.append(name+': simulation error')
        if len(matches)!=1:issues.append(name+': expected one PASS marker')
        if not events or frozen!=events:issues.append(name+': decoded trace mismatch')
        row=dict(assertions=int(matches[0]) if len(matches)==1 else None,
                 motor_packets=sum(s.startswith('TRACE MOTOR ') for s in events),
                 servo_targets=sum(s.startswith('TRACE SERVO ') for s in events),
                 trace_events=len(events))
        rows[name]=row;traces[name]=events
        if check_sources:
            expected=receipt['variants'][name]
            for key,value in row.items():
                if value!=expected[key]:issues.append(name+': '+key+' count changed')
            for filename,expected_hash in expected['rtl_sha256'].items():
                p=directory/'inputs'/filename
                if not p.is_file() or hashlib.sha256(p.read_bytes()).hexdigest()!=expected_hash:
                    issues.append(name+': input changed '+filename)
        elif row['assertions']!=354:
            issues.append(name+': expected 354 assertions')
    equal=bool(traces[VARIANTS[0]]) and all(traces[n]==traces[VARIANTS[0]] for n in VARIANTS)
    if not equal:issues.append('motion traces differ')
    return dict(passed=not issues,motion_traces_equal=equal,variants=rows,issues=issues,
                hardware_access=False,scope='decoded commands and servo targets; timestamps ignored')

if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--root',type=Path,default=HERE/'comparison')
    p.add_argument('--output',type=Path)
    a=p.parse_args();out=inspect(a.root)
    if a.output:
        a.output.parent.mkdir(parents=True,exist_ok=True)
        a.output.write_text(json.dumps(out,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print(json.dumps(out,ensure_ascii=False,indent=2))
    raise SystemExit(0 if out['passed'] else 1)
