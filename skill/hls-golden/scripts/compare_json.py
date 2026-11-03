"""Compare structured golden outputs, including shape, keys and finite numbers."""
import argparse
import json
import math
from pathlib import Path


def compare(golden, actual, atol=0.0, rtol=0.0):
    if not all(math.isfinite(x) and x >= 0 for x in (atol, rtol)):
        raise ValueError('tolerances must be finite and nonnegative')
    errors, leaves = [], 0

    def visit(a, b, path='$'):
        nonlocal leaves
        numeric = lambda x: isinstance(x, (int, float)) and not isinstance(x, bool)
        if numeric(a) and numeric(b):
            leaves += 1
            finite = lambda x: isinstance(x, int) or math.isfinite(x)
            if not finite(a) or not finite(b):
                errors.append({'path': path, 'reason': 'nonfinite_number'})
            elif (atol == rtol == 0 and a != b) or (atol != 0 or rtol != 0) and abs(a-b) > atol + rtol * abs(a):
                errors.append({'path': path, 'reason': 'numeric_mismatch', 'golden': a, 'actual': b})
        elif type(a) is not type(b):
            errors.append({'path': path, 'reason': 'type_mismatch'})
        elif isinstance(a, dict):
            if set(a) != set(b):
                errors.append({'path': path, 'reason': 'key_mismatch', 'missing': sorted(set(a)-set(b)), 'extra': sorted(set(b)-set(a))})
            for key in sorted(set(a) & set(b)): visit(a[key], b[key], f'{path}.{key}')
        elif isinstance(a, list):
            if len(a) != len(b): errors.append({'path': path, 'reason': 'length_mismatch', 'golden': len(a), 'actual': len(b)})
            for i, (x, y) in enumerate(zip(a, b)): visit(x, y, f'{path}[{i}]')
        else:
            leaves += 1
            if a != b: errors.append({'path': path, 'reason': 'value_mismatch', 'golden': a, 'actual': b})
    visit(golden, actual)
    if not leaves: errors.append({'path': '$', 'reason': 'no_compared_values'})
    return {'passed': not errors, 'compared_values': leaves, 'mismatch_count': len(errors), 'mismatches': errors[:100], 'atol': atol, 'rtol': rtol, 'scope': 'output equivalence only; not recognition accuracy or board performance'}


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--golden', required=True); p.add_argument('--actual', required=True)
    p.add_argument('--atol', type=float, default=0); p.add_argument('--rtol', type=float, default=0)
    p.add_argument('--output')
    a = p.parse_args()
    try:
        result = compare(json.loads(Path(a.golden).read_text(encoding='utf-8-sig')), json.loads(Path(a.actual).read_text(encoding='utf-8-sig')), a.atol, a.rtol)
    except (OSError, ValueError, TypeError, OverflowError) as e:
        result = {'passed': False, 'error': str(e)}
    text = json.dumps(result, ensure_ascii=False, indent=2, allow_nan=False)
    if a.output: Path(a.output).write_text(text+'\n', encoding='utf-8')
    print(text)
    return 0 if result['passed'] else 1


if __name__ == '__main__': raise SystemExit(main())
