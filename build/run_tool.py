"""Pipe-backed native tool launcher; no deployment or motion commands."""
import argparse,os,re,subprocess,sys,sysconfig,threading
from pathlib import Path

def run_command(executable,args,directory,timeout=3600,require_marker=False,env_overrides=None,log_name='task.log'):
    directory=Path(directory).resolve();directory.mkdir(parents=True,exist_ok=True)
    env=os.environ.copy()
    if os.name=='nt' and not env.get('PROCESSOR_ARCHITECTURE') and sysconfig.get_platform()=='win-amd64':env['PROCESSOR_ARCHITECTURE']='AMD64'
    scratch=directory/'tmp';scratch.mkdir(exist_ok=True)
    env.update(TEMP=str(scratch),TMP=str(scratch),PYTHONDONTWRITEBYTECODE='1',PYTHONUTF8='1')
    if env_overrides:env.update(env_overrides)
    words=[str(executable),*map(str,args)]
    if os.name=='nt' and str(executable).lower().endswith(('.bat','.cmd')):
        if any(any(c in word for c in '\r\n"%&|<>^!') for word in words):raise ValueError('Unsupported CMD character')
        command='"'+env.get('COMSPEC','C:/Windows/System32/cmd.exe')+'" /d /s /c "'+' '.join('"'+w+'"' for w in words)+'"'
    else:command=words
    log_path=directory/log_name;errors=[]
    with log_path.open('ab',buffering=0) as log:
        offset=log.tell();log.write(('\nPIPE_COMMAND: '+repr(words)+'\n').encode('utf-8'))
        proc=subprocess.Popen(command,cwd=directory,env=env,stdin=subprocess.PIPE,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,creationflags=subprocess.CREATE_NO_WINDOW if os.name=='nt' else 0)
        def drain():
            try:
                while True:
                    data=proc.stdout.read1(65536)
                    if not data:break
                    log.write(data)
            except Exception as e:errors.append(str(e))
        reader=threading.Thread(target=drain,daemon=True);reader.start()
        try:code=proc.wait(timeout=timeout)
        except subprocess.TimeoutExpired:
            if os.name=='nt':subprocess.run(['taskkill','/PID',str(proc.pid),'/T','/F'],capture_output=True,timeout=30)
            else:proc.kill()
            proc.wait(timeout=30);raise RuntimeError('Tool timed out; owned process stopped')
        finally:proc.stdin.close();reader.join(timeout=15)
        if reader.is_alive() or errors:raise RuntimeError('Output collection failed')
        proc.stdout.close()
    with log_path.open('rb') as log:log.seek(offset);content=log.read().decode('utf-8',errors='replace')
    if code or require_marker and not re.search(r'^TOOLCHAIN_STATUS=SUCCESS\r?$',content,re.M):raise RuntimeError('Tool failed; inspect '+str(log_path))
    return content

if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--exe',required=True);p.add_argument('--output',type=Path,required=True);p.add_argument('--timeout',type=int,default=3600);p.add_argument('--require-marker',action='store_true');p.add_argument('args',nargs=argparse.REMAINDER);a=p.parse_args()
    run_command(a.exe,a.args[1:] if a.args[:1]==['--'] else a.args,a.output,a.timeout,a.require_marker)
    print('Tool finished; log:',a.output/'task.log')
