"""Read-only screening of Vivado English text reports. Not a complete signoff."""
import argparse
import hashlib
import json
import math
import re
from pathlib import Path


def parse_timing(text):
    header = re.search(r'^\s*WNS\(ns\).*WHS\(ns\).*WPWS\(ns\).*$', text, re.M)
    if not header: raise ValueError('global timing summary header missing')
    for line in text[header.end():].splitlines()[:5]:
        fields = line.split()
        if len(fields) == 12:
            try: values = [float(x) for x in fields]
            except ValueError: continue
            if not all(math.isfinite(x) for x in values): raise ValueError('nonfinite timing metric')
            names = ['wns_ns','tns_ns','setup_failing','setup_total','whs_ns','ths_ns','hold_failing','hold_total','wpws_ns','tpws_ns','pulse_failing','pulse_total']
            return dict(zip(names, values))
    raise ValueError('global timing summary data missing or unsupported')


def parse_check(text):
    specs = {
      'no_clock': r'There are (\d+) register/latch pins with no clock\.',
      'unconstrained_internal': r'There are (\d+) pins that are not constrained for maximum delay\.',
      'no_input_delay': r'There are (\d+) input ports with no input delay specified\.',
      'no_output_delay': r'There are (\d+) ports with no output delay specified\.',
    }
    out = {}
    for name, pattern in specs.items():
        m = re.search(pattern, text)
        if not m: raise ValueError(f'check_timing metric missing: {name}')
        out[name] = int(m[1])
    return out


def parse_utilization(text):
    keys = {'Slice LUTs':'lut', 'CLB LUTs':'lut', 'Slice Registers':'ff', 'CLB Registers':'ff', 'Block RAM Tile':'bram36_tiles', 'DSPs':'dsp'}
    out = {}
    for line in text.splitlines():
        cols = [x.strip() for x in line.split('|')]
        if len(cols) < 7 or cols[1] not in keys: continue
        name = keys[cols[1]]
        used, available = float(cols[2].replace(',','')), float(cols[4].replace(',',''))
        if not all(math.isfinite(v) for v in (used, available)) or used < 0 or available <= 0: raise ValueError('invalid utilization row')
        if name not in out: out[name] = {'used': used, 'available': available, 'ratio': used/available}
    if set(out) != {'lut','ff','bram36_tiles','dsp'}: raise ValueError('required resource rows missing')
    return out


def parse_drc(text):
    if '1. REPORT SUMMARY' not in text or '2. REPORT DETAILS' not in text: raise ValueError('DRC summary/details missing')
    summary = text.split('1. REPORT SUMMARY')[-1].split('2. REPORT DETAILS')[0]
    m = re.search(r'Violations found:\s*(\d+)', summary)
    if not m: raise ValueError('DRC total missing')
    counts = {'Error':0, 'Critical Warning':0, 'Warning':0, 'Advisory':0}
    rows = re.findall(r'^\|\s*[^|]+\|\s*(Error|Critical Warning|Warning|Advisory)\s*\|[^|]*\|\s*(\d+)\s*\|', summary, re.M)
    for severity, number in rows: counts[severity] += int(number)
    if sum(counts.values()) != int(m[1]): raise ValueError('DRC table incomplete or unsupported severity')
    return counts


def check(timing, coverage, drc, utilization, max_utilization=0.9):
    if not math.isfinite(max_utilization) or not 0 < max_utilization <= 1: raise ValueError('max utilization must be in (0,1]')
    metrics, issues = {}, []
    for name, parser, raw in [('timing',parse_timing,timing),('coverage',parse_check,coverage),('drc',parse_drc,drc),('utilization',parse_utilization,utilization)]:
        try: metrics[name] = parser(raw)
        except (ValueError, OverflowError) as e: issues.append(str(e))
    if not re.search(r'Design State\s*:\s*Fully Routed', drc): issues.append('DRC is not from a fully routed design')
    if 'timing' in metrics:
        for key, value in metrics['timing'].items():
            if (key.endswith('_ns') and value < 0) or (key.endswith('_failing') and value != 0): issues.append(f'{key}={value}')
    if 'coverage' in metrics:
        for key, value in metrics['coverage'].items():
            if value: issues.append(f'{key}={value}; review constraints against hardware')
    if 'drc' in metrics:
        for key in ('Error','Critical Warning'):
            if metrics['drc'][key]: issues.append(f'DRC {key}={metrics["drc"][key]}')
    if 'utilization' in metrics:
        for key, row in metrics['utilization'].items():
            if row['ratio'] > max_utilization: issues.append(f'{key} utilization exceeds {max_utilization}')
    return {'passed': not issues, 'metrics': metrics, 'issues': issues, 'full_signoff': False, 'scope': 'report screening only; CDC, exceptions, I/O timing model and board tests require separate review'}


def main():
    p = argparse.ArgumentParser(description=__doc__)
    for flag in ('timing','check-timing','drc','utilization'): p.add_argument('--'+flag, required=True)
    p.add_argument('--max-utilization', type=float, default=0.9); p.add_argument('--output')
    a = p.parse_args()
    try:
        files = [Path(getattr(a,n)) for n in ('timing','check_timing','drc','utilization')]
        result = check(*(f.read_text(encoding='utf-8-sig',errors='replace') for f in files), a.max_utilization)
        result['input_sha256'] = {f.name:hashlib.sha256(f.read_bytes()).hexdigest() for f in files}
    except (OSError, ValueError) as e: result = {'passed':False, 'error':str(e), 'full_signoff':False}
    text = json.dumps(result, ensure_ascii=False, indent=2, allow_nan=False)
    if a.output: Path(a.output).write_text(text+'\n',encoding='utf-8')
    print(text)
    return 0 if result['passed'] else 1


if __name__ == '__main__': raise SystemExit(main())
