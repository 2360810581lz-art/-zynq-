"""Verify file hashes and declared dependency edges, without programming hardware."""
import argparse
import hashlib
import json
from pathlib import Path, PureWindowsPath
import re


def verify(root, manifest):
    root = Path(root).resolve()
    issues, files = [], manifest.get('artifacts')
    if manifest.get('schema_version') != 1 or not isinstance(files, list) or not files:
        return {'passed':False, 'issues':['unsupported schema or empty artifacts']}
    by_id = {}
    for item in files:
        ident = item.get('id')
        if not isinstance(ident,str) or not ident or ident in by_id:
            issues.append('missing or duplicate artifact id'); continue
        by_id[ident] = item
        raw = item.get('path','')
        if not isinstance(raw,str) or not raw or PureWindowsPath(raw).drive or PureWindowsPath(raw).is_absolute() or Path(raw).is_absolute():
            issues.append(f'{ident}: path must be relative'); continue
        target = (root / raw.replace('\\','/')).resolve()
        if not target.is_relative_to(root): issues.append(f'{ident}: path escapes root'); continue
        digest = item.get('sha256','')
        if not isinstance(digest,str) or not re.fullmatch(r'[0-9a-f]{64}',digest): issues.append(f'{ident}: invalid sha256'); continue
        if not target.is_file(): issues.append(f'{ident}: file missing'); continue
        if hashlib.sha256(target.read_bytes()).hexdigest() != digest: issues.append(f'{ident}: sha256 mismatch')
    required = manifest.get('required_roles',[])
    for role in required:
        if not any(x.get('role') == role for x in files): issues.append(f'missing role: {role}')
    visiting, visited = set(), set()
    def visit(ident):
        if ident in visiting: issues.append(f'dependency cycle at {ident}'); return
        if ident in visited: return
        visiting.add(ident)
        deps = by_id[ident].get('depends_on',{})
        if not isinstance(deps,dict): issues.append(f'{ident}: depends_on must map id to digest'); deps={}
        for parent, digest in deps.items():
            if parent not in by_id: issues.append(f'{ident}: missing parent {parent}'); continue
            if by_id[parent].get('sha256') != digest: issues.append(f'{ident}: parent digest mismatch {parent}')
            visit(parent)
        visiting.remove(ident); visited.add(ident)
    for ident in by_id: visit(ident)
    return {'passed':not issues, 'artifact_count':len(files), 'issues':issues, 'scope':'file integrity and declared ancestry; does not prove build correctness, timing or hardware behavior'}


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--root',required=True);p.add_argument('--manifest',required=True);p.add_argument('--output')
    a=p.parse_args()
    try: result=verify(a.root,json.loads(Path(a.manifest).read_text(encoding='utf-8-sig')))
    except (OSError,ValueError,TypeError,KeyError,AttributeError,RecursionError) as e: result={'passed':False,'error':str(e)}
    text=json.dumps(result,ensure_ascii=False,indent=2)
    if a.output:Path(a.output).write_text(text+'\n',encoding='utf-8')
    print(text)
    return 0 if result['passed'] else 1


if __name__=='__main__':raise SystemExit(main())
