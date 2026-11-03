"""Offline vision-task contract validation. No serial or hardware dependencies."""
import argparse
from decimal import Decimal, ROUND_HALF_UP
import json
import math
from pathlib import Path


def number(value):
    return isinstance(value,(int,float)) and not isinstance(value,bool) and math.isfinite(value)


def preview(result, config):
    issues, tasks = [], []
    if config.get('units') != 'cm' or result.get('units') != 'cm': raise ValueError('explicit cm units required')
    origin, signs, limits = (config[k] for k in ('origin_cm','axis_signs','limits_cm'))
    if len(origin)!=2 or len(signs)!=2 or len(limits)!=2 or not all(number(x) for x in origin) or any(x not in (-1,1) or isinstance(x,bool) for x in signs): raise ValueError('invalid coordinate transform')
    if any(len(v)!=2 or not all(number(x) for x in v) or v[0]>=v[1] for v in limits): raise ValueError('invalid limits')
    generation=result.get('generation')
    if type(generation) is not int or generation<=0: issues.append('invalid frame generation')
    if result.get('roi_valid') is not True or result.get('status')!=0 or type(result.get('status')) is not int or result.get('overflow') is not False: issues.append('invalid ROI, status or overflow')
    age, max_age = result.get('age_ms'),config.get('max_age_ms')
    if not number(max_age) or max_age<=0: raise ValueError('positive max_age_ms required')
    if not number(age) or age<0 or age>max_age: issues.append('stale or invalid frame age')
    objects=result.get('objects')
    if not isinstance(objects,list): raise ValueError('objects must be an array')
    if len(objects)>config.get('max_objects',8): issues.append('object capacity exceeded')
    for i,obj in enumerate(objects):
        if obj.get('valid') is not True or obj.get('clipped') is not False or obj.get('generation')!=generation:
            issues.append(f'object {i}: invalid, clipped or mixed generation');continue
        center=obj.get('center_cm')
        if not isinstance(center,list) or len(center)!=2 or not all(number(x) for x in center): issues.append(f'object {i}: invalid center_cm');continue
        if not isinstance(obj.get('color'),str) or not isinstance(obj.get('shape'),str): issues.append(f'object {i}: invalid class');continue
        route=config.get('routes',{}).get(obj['color']+'/'+obj['shape'])
        if type(route) is not int or not 1<=route<=config.get('box_count',6): issues.append(f'object {i}: unmapped class');continue
        xy=[signs[k]*(center[k]-origin[k]) for k in range(2)]
        # Reject before rounding; never round an unreachable point back inside.
        if any(not limits[k][0]<=xy[k]<=limits[k][1] for k in range(2)): issues.append(f'object {i}: coordinate out of calibrated range');continue
        encoded=[str(Decimal(str(x)).quantize(Decimal('0.01'),rounding=ROUND_HALF_UP)) for x in xy]
        tasks.append({'object_index':i,'generation':generation,'x_cm':encoded[0],'y_cm':encoded[1],'box':route,'consumes_frame':True})
    return {'passed':not issues,'mode':'offline_preview','issues':issues,'candidates':tasks if not issues else [],'execution_policy':'select one candidate; after completed pick/drop obtain a new frame; never execute this snapshot as a moving-scene batch','hardware_access':False,'encoding_resolution_cm':0.01,'position_accuracy':'not measured'}


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--result',required=True);p.add_argument('--config',required=True);p.add_argument('--output')
    a=p.parse_args()
    try: out=preview(json.loads(Path(a.result).read_text(encoding='utf-8-sig')),json.loads(Path(a.config).read_text(encoding='utf-8-sig')))
    except (OSError,ValueError,TypeError,KeyError,AttributeError,OverflowError) as e:out={'passed':False,'error':str(e),'hardware_access':False}
    text=json.dumps(out,ensure_ascii=False,indent=2)
    if a.output:Path(a.output).write_text(text+'\n',encoding='utf-8')
    print(text)
    return 0 if out['passed'] else 1


if __name__=='__main__':raise SystemExit(main())
