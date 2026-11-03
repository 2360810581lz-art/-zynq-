"""Replay the three frozen motion variants with Vivado 2019.1 XSim. No board access."""
from pathlib import Path
import argparse,json,shutil,sys

HERE=Path(__file__).resolve().parent
ROOT=HERE.parents[1]
sys.path.insert(0,str(ROOT/'build'))
from run_tool import run_command
from check_traces import inspect,VARIANTS

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--bin',type=Path,required=True,help='Vivado 2019.1 bin directory')
    p.add_argument('--output',type=Path,required=True,help='New directory outside the package')
    a=p.parse_args();out=a.output.resolve()
    if out.is_relative_to(ROOT.resolve()):p.error('output must be outside package')
    programs={name:a.bin/(name+'.bat' if sys.platform=='win32' else name) for name in ('xvlog','xelab','xsim')}
    for program in programs.values():
        if not program.is_file():p.error('tool not found: '+str(program))
    out.mkdir(parents=True,exist_ok=False)
    for name in VARIANTS:
        dest=out/name;dest.mkdir()
        shutil.copytree(HERE/'comparison'/name/'inputs',dest/'inputs')
        (dest/'run.tcl').write_text('run all\nquit\n',encoding='ascii')
        sources=[p.as_posix() for p in sorted((dest/'inputs').glob('*.v'))]
        run_command(programs['xvlog'],['-i',(dest/'inputs').as_posix(),*sources],dest,timeout=90)
        run_command(programs['xelab'],['work.tb_automatic','-s','trace_sim','-debug','typical'],dest,timeout=90)
        run_command(programs['xsim'],['trace_sim','-tclbatch',(dest/'run.tcl').as_posix(),'-log','trace.log','-onerror','quit'],dest,timeout=180)
        events=[s.strip() for s in (dest/'trace.log').read_text(errors='replace').splitlines() if s.startswith('TRACE ')]
        (dest/'motion_trace.txt').write_text('\n'.join(events)+'\n',encoding='utf-8')
    result=inspect(out,check_sources=False)
    (out/'comparison.json').write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8')
    print(json.dumps(result,indent=2))
    return 0 if result['passed'] else 1

if __name__=='__main__':raise SystemExit(main())
