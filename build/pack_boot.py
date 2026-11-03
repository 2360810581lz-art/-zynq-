"""Pack matching explicit inputs; never writes SD, flash, JTAG or serial."""
import argparse,hashlib,json,shutil,zipfile
from pathlib import Path
from run_tool import run_command
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__)
    for n in ['bit','hdf','app','fsbl','bootgen','output']:p.add_argument('--'+n,type=Path,required=True)
    a=p.parse_args()
    for n in ['bit','hdf','app','fsbl','bootgen']:setattr(a,n,getattr(a,n).resolve(strict=True))
    with zipfile.ZipFile(a.hdf) as z:
        bits=[x for x in z.namelist() if x.lower().endswith('.bit')]
        if len(bits)!=1 or hashlib.sha256(z.read(bits[0])).hexdigest()!=sha(a.bit):raise ValueError('HDF/bit mismatch')
    a.output=a.output.resolve();a.output.mkdir(parents=True,exist_ok=False);inputs={}
    for n,dest in [('bit','design.bit'),('app','application.elf'),('fsbl','fsbl.elf'),('hdf','design.hdf')]:
        original=getattr(a,n);shutil.copy2(original,a.output/dest);inputs[dest]=sha(original)
        if sha(a.output/dest)!=inputs[dest]:raise ValueError('Snapshot mismatch')
    (a.output/'boot.bif').write_text('the_ROM_image:\n{\n [bootloader] fsbl.elf\n design.bit\n application.elf\n}\n',encoding='ascii')
    run_command(a.bootgen,['-arch','zynq','-image','boot.bif','-o','boot.bin','-w','on'],a.output)
    boot=a.output/'boot.bin'
    if not boot.is_file() or not boot.stat().st_size:raise ValueError('No boot image')
    (a.output/'manifest.json').write_text(json.dumps({'inputs_sha256':inputs,'boot_sha256':sha(boot),'hdf_bit_match':True,'hardware_verified':False,'timing_closed':False,'uart':{'app':'UART1','fsbl':'UART0'},'scope':'packaging only; verify BSP UART settings and timing reports separately'},indent=2)+'\n',encoding='utf-8')
    print('Packaged',boot)
