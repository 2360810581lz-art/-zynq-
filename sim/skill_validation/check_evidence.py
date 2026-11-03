"""Check copied engineering files and quoted text against package indexes."""
from pathlib import Path
import hashlib
import json

ROOT=Path(__file__).resolve().parents[2]
index=json.loads((ROOT/'skill/evidence/index.json').read_text(encoding='utf-8'))
issues=[]
for item in index['entries']:
    target=ROOT/'skill/evidence'/item['file']
    if not target.is_file() or hashlib.sha256(target.read_bytes()).hexdigest()!=item['sha256']:issues.append(item['id'])
dialogues=[json.loads(x) for x in (ROOT/'report/collaboration/records/dialogue_excerpts.jsonl').read_text(encoding='utf-8').splitlines() if x.strip()]
for item in dialogues:
    if hashlib.sha256(item['text'].encode('utf-8')).hexdigest()!=item['excerpt_sha256']:issues.append(item['id'])
history=[json.loads(x) for x in (ROOT/'report/collaboration/records/history_transcript.jsonl').read_text(encoding='utf-8').splitlines() if x.strip()]
stats=json.loads((ROOT/'report/collaboration/records/history_index.json').read_text(encoding='utf-8'))
seen=set()
for item in history:
    if item['id'] in seen:issues.append('duplicate '+item['id'])
    seen.add(item['id'])
    if hashlib.sha256(item['text'].encode('utf-8')).hexdigest()!=item['text_sha256']:issues.append(item['id'])
    if item['character_start']!=0 or item['character_end']!=len(item['text']):issues.append('range '+item['id'])
    if item['role'] not in ('user','assistant') or item['source_line']<1:issues.append('metadata '+item['id'])
if len(history)!=stats['historical_messages']:issues.append('history count')
for role in ('user','assistant'):
    if sum(x['role']==role for x in history)!=stats[role+'_messages']:issues.append(role+' count')
out={'passed':not issues,'evidence_files':len(index['entries']),'dialogue_excerpts':len(dialogues),'expanded_historical_messages':len(history),'expanded_cases':stats['expanded_cases'],'issues':issues,'scope':'integrity against package indexes, not external authenticity proof'}
(ROOT/'sim/skill_validation/evidence_integrity.json').write_text(json.dumps(out,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
print(json.dumps(out,ensure_ascii=False))
raise SystemExit(0 if out['passed'] else 1)
