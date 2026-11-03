"""Portable offline PYNQ .bit/.hwh metadata check. Never loads an overlay."""
import argparse
import hashlib
import json
from pathlib import Path
import xml.etree.ElementTree as ET


def inspect(bit, hwh, contract):
    bit,hwh=Path(bit),Path(hwh)
    issues,ips=[],{}
    if bit.suffix.lower()!='.bit' or hwh.suffix.lower()!='.hwh' or bit.stem!=hwh.stem: issues.append('bit/hwh must have the same stem and expected extensions')
    if bit.parent.resolve()!=hwh.parent.resolve():issues.append('bit/hwh must be in the same directory for the normal PYNQ lookup')
    if not bit.is_file() or not hwh.is_file():raise ValueError('bit/hwh pair missing')
    if not bit.stat().st_size:issues.append('empty bitstream')
    root=ET.parse(hwh).getroot()
    for element in root.iter():element.tag=element.tag.rsplit('}',1)[-1]
    for module in root.findall('.//MODULE'):
        name=module.get('FULLNAME') or module.get('INSTANCE')
        if not name:continue
        name=name.lstrip('/')
        if name in ips:issues.append(f'duplicate HWH IP: {name}');continue
        registers={}
        for register in module.findall('.//REGISTER'):
            rname=register.get('NAME')
            if not rname:continue
            props={p.get('NAME'):p.get('VALUE') for p in register.findall('.//PROPERTY')}
            raw=register.get('ADDRESS_OFFSET') or props.get('ADDRESS_OFFSET')
            if raw is not None:registers[rname]=int(raw,0)
        ips[name]={'vlnv':module.get('VLNV',''),'registers':registers}
    required=contract.get('required_ips')
    if not isinstance(required,dict) or not required:raise ValueError('nonempty required_ips contract required')
    for name,spec in required.items():
        if name not in ips:issues.append(f'missing IP: {name}');continue
        if spec.get('vlnv') and spec['vlnv']!=ips[name]['vlnv']:issues.append(f'VLNV mismatch: {name}')
        for reg,offset in spec.get('registers',{}).items():
            expected=int(offset,0) if isinstance(offset,str) else offset
            if type(expected) is not int or expected<0 or expected%4:raise ValueError('contract register offsets must be aligned nonnegative integers')
            if ips[name]['registers'].get(reg)!=expected:issues.append(f'register offset mismatch: {name}/{reg}')
    hashes={'bit':hashlib.sha256(bit.read_bytes()).hexdigest(),'hwh':hashlib.sha256(hwh.read_bytes()).hexdigest()}
    for name,digest in contract.get('expected_sha256',{}).items():
        if name not in hashes or hashes[name]!=digest:issues.append(f'{name}: expected sha256 mismatch')
    return {'passed':not issues,'issues':issues,'sha256':hashes,'ips':ips,'hardware_access':False,'board_verified':False,'scope':'metadata contract only; matching names do not prove paired build, device fit, clocking, DMA or electrical compatibility'}


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--bit',required=True);p.add_argument('--hwh',required=True);p.add_argument('--contract',required=True);p.add_argument('--output')
    a=p.parse_args()
    try:out=inspect(a.bit,a.hwh,json.loads(Path(a.contract).read_text(encoding='utf-8-sig')))
    except (OSError,ValueError,TypeError,AttributeError,ET.ParseError) as e:out={'passed':False,'error':str(e),'hardware_access':False,'board_verified':False}
    text=json.dumps(out,ensure_ascii=False,indent=2)
    if a.output:Path(a.output).write_text(text+'\n',encoding='utf-8')
    print(text)
    return 0 if out['passed'] else 1


if __name__=='__main__':raise SystemExit(main())
