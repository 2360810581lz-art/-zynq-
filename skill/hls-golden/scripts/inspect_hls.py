"""Read actual HLS XML/cosim reports and expose stale-receipt contradictions."""
import argparse
import json
import math
from pathlib import Path
import re
import xml.etree.ElementTree as ET


def inspect(xml_text, cosim_text, receipt=None):
    root=ET.fromstring(xml_text)
    for e in root.iter():e.tag=e.tag.rsplit('}',1)[-1]
    def value(path):
        text=root.findtext(path)
        if text is None:raise ValueError(f'HLS field missing: {path}')
        return text.strip()
    period=float(value('PerformanceEstimates/SummaryOfTimingAnalysis/EstimatedClockPeriod'))
    target=float(value('UserAssignments/TargetClockPeriod'))
    if not all(math.isfinite(v) and v>0 for v in (period,target)):raise ValueError('invalid HLS clock estimate')
    resources={k:int(value('AreaEstimates/Resources/'+k)) for k in ('LUT','FF','DSP48E','BRAM_18K')}
    if any(v<0 for v in resources.values()):raise ValueError('negative HLS resource count')
    rows=re.findall(r'^\|\s*(Verilog|VHDL)\s*\|\s*(Pass|Fail|NA)\s*\|',cosim_text,re.M)
    active=[status for _,status in rows if status!='NA']
    if not active:raise ValueError('no actual RTL cosimulation result row')
    rtl_pass=all(x=='Pass' for x in active)
    conflicts=[]
    if receipt is not None and receipt.get('rtl_cosim_pass') is not rtl_pass:conflicts.append('receipt rtl_cosim_pass disagrees with actual cosim report')
    return {'passed':rtl_pass and not conflicts,'rtl_report_pass':rtl_pass,'receipt_conflicts':conflicts,'hls_estimates':{'clock_period_ns':period,'target_period_ns':target,'resources':resources},'board_verified':False,'scope':'HLS estimates and cosimulation report only; no whole-system FPS inference'}


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--synthesis',required=True);p.add_argument('--cosim',required=True);p.add_argument('--receipt');p.add_argument('--output')
    a=p.parse_args()
    try:
        receipt=json.loads(Path(a.receipt).read_text(encoding='utf-8-sig')) if a.receipt else None
        result=inspect(Path(a.synthesis).read_text(encoding='utf-8-sig'),Path(a.cosim).read_text(encoding='utf-8-sig'),receipt)
    except (OSError,ValueError,TypeError,AttributeError,ET.ParseError) as e:result={'passed':False,'error':str(e)}
    text=json.dumps(result,ensure_ascii=False,indent=2)
    if a.output:Path(a.output).write_text(text+'\n',encoding='utf-8')
    print(text)
    return 0 if result['passed'] else 1


if __name__=='__main__':raise SystemExit(main())
