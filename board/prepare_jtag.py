"""Prepare a JTAG script from a verified bundle. Does not connect to hardware."""
import argparse,hashlib,zipfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--release',type=Path,default=ROOT/'board/release');p.add_argument('--output',type=Path,required=True);a=p.parse_args();r=a.release.resolve()
    names=['camera_claw_top.bit','camera_claw_top.hdf','fsbl_2019.elf','live_overlay.elf']
    for n in names:
        if not (r/n).is_file():raise FileNotFoundError(r/n)
    with zipfile.ZipFile(r/'camera_claw_top.hdf') as z:
        bits=[n for n in z.namelist() if n.endswith('.bit')];init=[n for n in z.namelist() if n.endswith('ps7_init.tcl')]
        if len(bits)!=1 or hashlib.sha256(z.read(bits[0])).digest()!=hashlib.sha256((r/names[0]).read_bytes()).digest():raise ValueError('HDF bit mismatch')
        if len(init)!=1:raise ValueError('No unique ps7_init.tcl')
        a.output=a.output.resolve();a.output.mkdir(parents=True,exist_ok=False);(a.output/'ps7_init.tcl').write_bytes(z.read(init[0]))
    for path in [r,a.output]:
        if any(c in path.as_posix() for c in '{}\n\r'):raise ValueError('Unsupported Tcl path character')
    script='''# Running this script resets and programs the selected Zynq board.
# Check target identity in XSCT first. No serial or motion packets are sent.
connect -url tcp:127.0.0.1:3121
targets -set -filter {name =~ "APU*"}
rst -system
after 1000
fpga -file {%s/camera_claw_top.bit}
source {%s/ps7_init.tcl}
targets -set -filter {name =~ "ARM*#0"}
ps7_init
ps7_post_config
dow {%s/live_overlay.elf}
con
puts "TOOLCHAIN_STATUS=SUCCESS"
exit
'''%(r.as_posix(),a.output.as_posix(),r.as_posix())
    (a.output/'run_jtag.tcl').write_text(script,encoding='utf-8');print('Prepared only:',a.output/'run_jtag.tcl')
