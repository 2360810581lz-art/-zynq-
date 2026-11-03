"""Compile frozen HDMI boxes against a fresh combined-design HDF; never deploy."""
import argparse
from datetime import datetime
import hashlib
import json
from pathlib import Path
import re
import shutil
import sys
import zipfile

HERE = Path(__file__).resolve().parent
PROJECT = HERE.parent
from run_tool import run_command
SDK_ROOT = Path(__import__('os').environ.get('XILINX_SDK_ROOT', 'D:/XILINX/SDK/2019.1'))

ARM = SDK_ROOT / 'gnu/aarch32/nt/gcc-arm-none-eabi/bin'


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def save(path, data):
    path.write_text(json.dumps(data, indent=2) + '\n', encoding='utf-8')


def build(hdf, out, bit=None):
    source_root = PROJECT / 'src/ps'
    manifest = json.loads((PROJECT / 'build/ps_sources.json').read_text())
    current = {p.relative_to(source_root).as_posix(): sha(p)
               for p in source_root.rglob('*') if p.is_file()}
    if current != manifest['source_sha256']:
        raise ValueError('Frozen visual sources changed')
    if bit is not None:
        with zipfile.ZipFile(hdf) as archive:
            bits = [n for n in archive.namelist() if n.endswith('.bit')]
            if len(bits) != 1 or hashlib.sha256(archive.read(bits[0])).hexdigest() != sha(bit):
                raise ValueError('HDF does not contain the supplied bitstream')
    out.mkdir(parents=True, exist_ok=False)
    receipt = dict(state='running', started=datetime.now().isoformat(),
                   hdf_source=str(hdf), hdf_sha256=sha(hdf),
                   bit_sha256=sha(bit) if bit else None,
                   bit_in_hdf_verified=bit is not None,
                   hardware_verified=False, SD_updated=False,
                   scope='Fresh combined-design BSP/FSBL and unchanged HDMI box sources; no deployment')
    save(out / 'build_status.json', receipt)
    try:
        shutil.copy2(hdf, out / 'camera_claw_top.hdf')
        shutil.copytree(source_root, out / 'inputs')
        if current != {p.relative_to(out / 'inputs').as_posix(): sha(p)
                       for p in (out / 'inputs').rglob('*') if p.is_file()}:
            raise ValueError('Visual inputs changed while freezing')
        cfg = {'xsct': SDK_ROOT / 'bin/xsct.bat'}
        print('Generating fresh SDK hardware, BSPs and FSBL', flush=True)
        run_command(cfg['xsct'], [str(HERE / 'build_sdk.tcl'), str(out / 'sdk'),
                    str(out / 'camera_claw_top.hdf'), str(PROJECT / 'src/ip')],
                    out, timeout=1200, require_marker=True, log_name='sdk.log')
        log = (out / 'sdk.log').read_text(errors='replace')
        if re.search(r'(?:^|\s)(?:fatal )?error:|Build Failed', log, re.M):
            raise RuntimeError('SDK reported compiler errors')
        for bsp_name, uart in [('mipi_hdmi_bsp_2019', 'ps7_uart_1'), ('fsbl_2019_bsp', 'ps7_uart_0')]:
            mss = (out / 'sdk' / bsp_name / 'system.mss').read_text()
            for direction in ['stdin', 'stdout']:
                if not re.search(r'PARAMETER\s+' + direction + r'\s*=\s*' + uart + r'\b', mss):
                    raise ValueError('BSP UART configuration mismatch')
        src = out / 'inputs'
        bsp = out / 'sdk/mipi_hdmi_bsp_2019/ps7_cortexa9_0'
        flags = ['-std=c99', '-O2', '-Wall', '-Wextra', '-Werror', '-mcpu=cortex-a9',
                 '-mfpu=vfpv3', '-mfloat-abi=hard']
        for inc in [src / 'src', bsp / 'include', src / 'live', src / 'ps']:
            flags.extend(['-I', str(inc)])
        sources = sorted((src / 'src').rglob('*.c')) + [src / 'live/main.c', src / 'live/detector.c', src / 'ps/vision_black_border.c']
        objects = []
        for index, source in enumerate(sources):
            obj = out / (str(index) + '_' + source.stem + '.o')
            objects.append(str(obj))
            print('Compiling ' + source.name, flush=True)
            run_command(ARM / 'arm-none-eabi-gcc.exe', flags + ['-c', str(source), '-o', str(obj)],
                        out, timeout=120, log_name='compile_' + str(index) + '.log')
        elf = out / 'live_overlay.elf'
        run_command(ARM / 'arm-none-eabi-gcc.exe', ['-mcpu=cortex-a9', '-mfpu=vfpv3', '-mfloat-abi=hard',
                    '-Wl,-build-id=none', '-specs=' + str(src / 'src/xilinx.spec'),
                    '-Wl,-T', '-Wl,' + str(src / 'src/lscript.ld'), '-L' + str(bsp / 'lib'),
                    '-Wl,-Map,' + str(out / 'live_overlay.map'), '-o', str(elf)] + objects +
                    ['-Wl,--start-group,-lxil,-lgcc,-lc,-lm,--end-group'], out, timeout=120, log_name='link.log')
        run_command(ARM / 'arm-none-eabi-nm.exe', ['-S', '-n', str(elf)], out, timeout=30, log_name='symbols.log')
        expected_sizes = dict(liveCameraBuffer=8294400, frameBuf=18662400,
                              live_result=268, live_roi_before=72, live_boxes=280, live_shown=268,
                              live_command=64, live_boxes_before=280, live_box_trace=20480,
                              live_status=144, live_roi=72)
        symbols = {}
        for line in (out / 'symbols.log').read_text().splitlines():
            words = line.split()
            if len(words) == 4 and words[3] in expected_sizes:
                symbols[words[3]] = dict(address=int(words[0], 16), bytes=int(words[1], 16))
        if {n: v['bytes'] for n, v in symbols.items()} != expected_sizes:
            raise ValueError('Unexpected visual buffer ABI')
        ranges = sorted((v['address'], v['address'] + v['bytes']) for v in symbols.values())
        if any(a < 0x100000 or b >= 0x40000000 for a, b in ranges) or any(a[1] > b[0] for a, b in zip(ranges, ranges[1:])):
            raise ValueError('Invalid DDR layout')
        if symbols['live_command']['address'] % 64:
            raise ValueError('JTAG command is not cache-line aligned')
        fsbl = out / 'sdk/fsbl_2019/Debug/fsbl_2019.elf'
        if not fsbl.is_file() or not fsbl.stat().st_size:
            raise ValueError('Missing FSBL')
        shutil.copy2(fsbl, out / 'fsbl_2019.elf')
        if sha(hdf) != receipt['hdf_sha256'] or sha(out / 'camera_claw_top.hdf') != receipt['hdf_sha256']:
            raise ValueError('HDF changed during SDK build')
        save(out / 'input_sha256.json', current)
        save(out / 'bsp_sha256.json', {p.relative_to(out / 'sdk').as_posix(): sha(p)
             for bsp_name in ['mipi_hdmi_bsp_2019', 'fsbl_2019_bsp']
             for p in (out / 'sdk' / bsp_name).rglob('*') if p.is_file()})
        receipt.update(state='succeeded', elf_sha256=sha(elf), fsbl_sha256=sha(fsbl), symbols=symbols,
                       uart=dict(app='UART1', fsbl='UART0'), source_build=manifest['vision_source_build'],
                       control_protocol=2, status_words=36, detector_profile=False, edge_capture=False,
                       release_ready=False)
    except Exception as exc:
        receipt.update(state='failed', error=str(exc))
        raise
    finally:
        receipt['finished'] = datetime.now().isoformat()
        save(out / 'build_status.json', receipt)
    print(json.dumps(receipt, indent=2), flush=True)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--hdf', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--bit', type=Path, help='Optional matching bit included in HDF; never programs hardware')
    args = parser.parse_args()
    build(args.hdf.resolve(), args.output.resolve(), args.bit.resolve() if args.bit else None)
