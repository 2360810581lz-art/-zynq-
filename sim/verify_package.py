"""Offline inventory, integrity, source identity and dependency audit."""
from pathlib import Path
import argparse,hashlib,json,re,xml.etree.ElementTree as ET,zipfile
from urllib.parse import unquote,urlsplit
ROOT=Path(__file__).resolve().parents[1]
EXCLUDED={'manifest.json','sim/package_validation.json'}
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def verify(root):
    root=root.resolve()
    issues=[];counts={};files=[p for p in root.rglob('*') if p.is_file() and '.git' not in p.relative_to(root).parts]
    for p in files:
        if any(part.lower() in ('__pycache__','.git','.xil','node_modules') for part in p.relative_to(root).parts) or p.suffix.lower() in ('.pyc','.pyo','.jou','.bak','.tmp','.dcp'):
            issues.append('unwanted_file:'+p.relative_to(root).as_posix())
        for part in p.relative_to(root).parts:
            if part in ('README.md','SKILL.md'):continue
            if not re.fullmatch('[a-z0-9_.-]+',part):issues.append('invalid_path:'+p.relative_to(root).as_posix());break
    manifest=json.loads((root/'manifest.json').read_text(encoding='utf-8'))
    declared=set()
    for item in manifest['files']:
        rel=item['path'];p=root/rel
        if rel in declared:issues.append('duplicate_manifest_path:'+rel)
        declared.add(rel)
        if not p.resolve().is_relative_to(root.resolve()):issues.append('outside_root:'+rel);continue
        if not p.is_file():issues.append('missing:'+rel);continue
        if p.stat().st_size!=item['bytes']:issues.append('size:'+rel)
        if sha(p)!=item['sha256']:issues.append('hash:'+rel)
    if set(manifest.get('excluded_paths',[]))!=EXCLUDED:issues.append('manifest_exclusions')
    extra={p.relative_to(root).as_posix() for p in files}-declared-EXCLUDED
    if extra:issues.extend('unlisted:'+x for x in sorted(extra))
    with zipfile.ZipFile(root/'board/release/camera_claw_top.hdf') as z:
        bits=[x for x in z.namelist() if x.endswith('.bit')]
        if len(bits)!=1 or hashlib.sha256(z.read(bits[0])).hexdigest()!=sha(root/'board/release/camera_claw_top.bit'):issues.append('hdf_bit_mismatch')
    restore=json.loads((root/'report/provenance/restore_manifest.json').read_text(encoding='utf-8'))
    for name,expected in restore['original_control_rtl_sha256'].items():
        if sha(root/'src/rtl/control'/name.lower())!=expected:issues.append('original_control_changed:'+name)
    if sha(root/'board/release/boot.bin')!=restore['boot_sha256']:issues.append('restore_boot_changed')
    release=json.loads((root/'board/release/release.json').read_text(encoding='utf-8'))
    for name,expected in release['sha256'].items():
        if sha(root/name)!=expected:issues.append('release_mismatch:'+name)
    ns={'s':'http://www.spiritconsortium.org/XMLSchema/SPIRIT/1685-2009'};refs=0
    for comp in (root/'src/ip').rglob('component.xml'):
        for f in ET.parse(comp).findall('.//s:file/s:name',ns):
            refs+=1
            if not (comp.parent/f.text).is_file():issues.append('ip_missing:'+str(f.text))
    mapping=json.loads((root/'report/provenance/source_map.json').read_text(encoding='utf-8'))['files']
    for row in mapping:
        if not (root/row['path']).is_file() or sha(root/row['path'])!=row['packaged_sha256']:issues.append('provenance:'+row['path'])
    ps=json.loads((root/'build/ps_sources.json').read_text(encoding='utf-8'))['source_sha256']
    if ps!={p.relative_to(root/'src/ps').as_posix():sha(p) for p in (root/'src/ps').rglob('*') if p.is_file()}:issues.append('ps_sources')
    color=root/'src/hls/object_color'
    cases=(color/'cases.txt').read_text(encoding='utf-8').split()
    if len(cases)!=15 or len(set(cases))!=15:issues.append('color_case_count')
    for name in cases:
        if not (color/(name+'.bin')).is_file():issues.append('color_input_missing:'+name)
    if not (color/'hls_export_2019.tcl').is_file():issues.append('color_exporter_missing')
    tb=(root/'src/hls/feature/tb/test_vision_feature_accel.cpp').read_text(encoding='utf-8')
    ppm=set(re.findall(r'"([^"\n]+\.ppm)"',tb))
    actual={p.name for p in (root/'data/feature_ppm').glob('*.ppm')}
    if ppm!=actual or len(ppm)!=18:issues.append('feature_inputs')
    links=0
    for readme in root.rglob('README.md'):
        for target in re.findall(r'\]\(([^\s)]+)\)',readme.read_text(encoding='utf-8')):
            parsed=urlsplit(target.strip('<>'))
            if parsed.scheme or parsed.netloc or not parsed.path:continue
            links+=1;destination=(readme.parent/unquote(parsed.path)).resolve()
            if not destination.is_relative_to(root) or not destination.exists():
                issues.append('readme_link:'+readme.relative_to(root).as_posix()+':'+target)
    counts.update(payload_files=len(manifest['files']),source_records=len(mapping),ip_file_references=refs,locked_control_files=len(restore['original_control_rtl_sha256']),ps_sources=len(ps),color_file_cases=len(cases),feature_test_images=len(ppm),readme_local_links=links)
    return {'passed':not issues,'counts':counts,'issues':issues,'hardware_access':False,'scope':'offline hashes and filesystem dependencies; not timing, synthesis or board acceptance'}
if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--root',type=Path,default=ROOT);p.add_argument('--output',type=Path);a=p.parse_args();out=verify(a.root)
    if a.output:a.output.parent.mkdir(parents=True,exist_ok=True);a.output.write_text(json.dumps(out,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print(json.dumps(out,ensure_ascii=False,indent=2));raise SystemExit(0 if out['passed'] else 1)
