# FPGA local toolchain

This installation combines vivado-mcp 0.3.26 with project-specific SDK 2019.1,
BOOT.bin, HLS and camera-reference task tools. It controls the local Xilinx
installation. No cloud build service or API key is used.

## Installation and sources

- Service, virtual environment, source checkout, pip cache, temporary files,
  logs and job artifacts: `D:/312312/FPGA/toolchain/`.
- Upstream: https://github.com/mapleleavessssssss-wq/vivado-mcp
- Audited revision: `41745b29af31a34d9b6756a9105313215d9f568e`.
- License: Apache-2.0. The upstream README identifies 2019.1 as its main baseline.
- Existing Vivado, SDK and HLS: `D:/XILINX/`.
- The D-drive virtual environment uses the already-present Python 3.12 runtime
  bundled with Codex as its base. No new Python installation was made on C:.
- Codex's existing `C:/Users/LZFY/.codex/config.toml` contains only the service
  registration. A pre-change backup is in `backups/`; it contains private local
  settings and should not be shared or committed.
- Vivado's D-drive `scripts/Vivado_init.tcl` loads the loopback-only Tcl listener
  for GUI/attach sessions. That file did not exist before this installation.

Only this service's paths are redirected. Existing vendor programs may still
use their normal user preferences under the Windows profile.

## Natural language use in Codex

Restart Codex or reload MCP servers to load the new `vivado` server in the app.
The server offers 32 upstream tools and 4 local tools:

- `fpga_environment`: tool paths, project path and artifact hashes.
- `fpga_start_job`: launch a task and return immediately with a job ID.
- `fpga_job_status`: completion, bounded log tail and artifact paths.
- `fpga_list_jobs`: recent task receipts.

Example requests:

```text
Check the current camera project and list outstanding timing/IP issues.
Run synthesis, then implementation; show timing and utilization reports.
Rebuild the SDK application with app UART1 and FSBL UART0, then package BOOT.bin.
Read a camera frame through JTAG and run the vision reference on the resulting PNG.
Run the HLS smoke test and inspect its estimated pipeline interval and resources.
```

The generic upstream `run_tcl`/`safe_tcl` tools support Block Design, simulation,
project operations, reports and hardware management. Long upstream synth/impl
tasks support `wait=False` and progress polling. Never operate on the same
project simultaneously through a native Vivado session and a local build job.

The upstream `vivado-mcp doctor` currently recognizes only its direct launch
arguments and flags this custom `server.py` registration as unverified. Do not
use its auto-fix to replace the custom bridge. `fpga_environment` and
`verify_mcp.py` verify the actual configured service instead. A closed GUI
naturally leaves port 9999 inactive; Tcl-mode MCP sessions work without that port.

## Local tasks

Run in PowerShell from the workspace:

```powershell
.\toolchain\fpga.ps1 doctor
.\toolchain\fpga.ps1 check
.\toolchain\fpga.ps1 synth
.\toolchain\fpga.ps1 impl
.\toolchain\fpga.ps1 bitstream
.\toolchain\fpga.ps1 sdk_build
.\toolchain\fpga.ps1 boot
.\toolchain\fpga.ps1 capture
.\toolchain\fpga.ps1 analyze --input D:\path\frame.png --config D:\path\vision_config.json
.\toolchain\fpga.ps1 hls --input D:\312312\FPGA\toolchain\examples\hls_smoke\run.tcl
```

Each task creates a new `jobs/<timestamp_action_id>/` with `status.json`,
`task.log` and relevant reports/artifacts. Failures propagate to a nonzero CLI
exit code. Local build/hardware jobs have a cross-process lock. Job logs can
also be read from the CLI with `status --job-id <id>`.

## Hardware changes and release sequence

For a PL change, use the following ordered steps and pass the resulting files
explicitly. Do not accidentally package the old project `.bit`.

1. Modify/register RTL or HLS IP and regenerate/validate the Block Design using
   Vivado MCP. Save the project.
2. Run `bitstream`; inspect `timing.rpt`, `check_timing.rpt`, utilization and DRC.
   Missing register clocks, unconstrained internal endpoints, negative setup/hold
   slack and DRC errors block this task. A successful command
   alone does not certify that every clock/path is properly constrained.
3. Run `export_hw --input <bitstream-job>/system_wrapper.bit`; this exports a
   new `.hdf`/`.hwdef` for the completed implementation into its own job folder.
4. Run `sdk_build --input <export-job>/system_wrapper.hdf`. This updates SDK
   `hw_2019` then regenerates BSPs and builds app/FSBL with the validated UART split.
5. Run `boot --input <bitstream-job>/system_wrapper.bit`. Bootgen snapshots the
   current app/FSBL and supplied bitstream with hashes and writes a new BOOT.bin.
6. Hardware deployment and cold-start testing are separate explicit steps.

For C-only changes, `sdk_build` then `boot` is sufficient. Default `boot` uses
the project's existing `.bit`; it packages existing products and does not
silently run Vivado or rebuild SDK. Manifest `hardware_verified` remains false
until the newly built image has been tested on the board.

SDK 2019.1 can return an XSCT success marker after an application compiler error.
The local runner now rejects compiler errors in the build log and an application
ELF older than its C/header/linker sources. A success marker alone is insufficient.
HDMI startup diagnosis and the current frozen C-only image are documented in
`camera_base_2019_1_fixed/tools/programming/HDMI_STARTUP.md`.
Each `run_command` checks only its own newly appended output for the success
marker; earlier successful stages cannot mask a later failing invocation.
Standalone NN stem validation has its own fresh bit/HDF/SDK parent under
`camera_base_2019_1_fixed/hls/vision_nn_stem/board/`, with explicit camera restore.

## Vivado 2019.1 Windows standard-channel repair

Local `run_command` starts native tools with pipe-backed standard channels on
Windows and drains output to the job log in the parent. Direct disk redirection
can leave stale Tcl file-channel records after Vivado replaces stdout/stderr;
a reused handle then makes a readable existing script fail with `No error` or
ENOENT. No vendor script or API return is patched, and failures still propagate.
Evidence and bounded verification are in
`diagnostics/runtime_file_api_20261019/README.md`. The generic upstream MCP
launcher is separate; use local job tasks or fpga.ps1 for the repaired path.

## Camera acquisition

`capture` requires the powered board, a running camera and JTAG/hw_server on
localhost:3121. It reads the current parked VDMA1 buffer address, geometry and
stride from registers, rather than assuming the linker's old buffer address.
It writes raw BGR, geometry metadata and PNG. No PS reset or programming occurs.

Live DMA capture is not atomic. Use stationary objects for calibration and
inspect images for tearing. Production vision needs explicit frame ownership.
This task rejects circular DMA mode instead of claiming a safe snapshot.

The ROI and shape rules are still uncalibrated. Actual object photographs and
ground-truth labels are necessary; heuristic predictions cannot automatically
serve as reliable training labels. Six-box category mapping remains unspecified.

## Verification and maintenance

`verify_mcp.py --session-only` exercises stdio initialization, lists the tools, starts
Vivado 2019.1, opens the real project, checks its device and run state, closes
the owned session, and saves evidence in `logs/session_repair.json`. This is
the preferred check during PL changes, when a previous bitstream may no longer
exist. Omit the option only when intentionally testing packaging of unchanged
baseline inputs; that mode compares BOOT.bin with the verified original image.

```powershell
.\toolchain\.venv\Scripts\python.exe .\toolchain\verify_mcp.py --session-only
```

The session-only check does not package or flash anything.
SDK rebuild and HLS C simulation/synthesis have been exercised locally.
Implementation-report generation and HDF export have been exercised on the
existing routed design. The vision-integrated RTL now passes synthesis
(`20261003_185823_impl_ab2dc7af`, including fresh synthesis). SDK hardware updates from its changed HDF
and hardware validation remain pending.

Local upstream patches in `session.py`/`gui_session.py` are required to launch
Windows batch files through cmd with redirected pipes. Vendor and installed
D-drive copies must stay synchronized; a running MCP worker retains imported
code until reloaded. A fresh stdio test passed, while the current desktop
connection still needs reloading. No global Windows registry policy was changed.

The build script waits for IP OOC runs before top-level synthesis. It rejects
incomplete/failed runs instead of accepting `Scripts Generated` as success.
Locked IPs block the build before target generation. The CSI source repository's
2017.4 subcores were migrated to 2019.1 and its revision increased to 11; the
exact prior source is preserved in `jobs/vision_repair_20261003/csi_subcore_backup/`.
Unused CSI/D-PHY ILA constraints are disabled when their optional debug modules
are disabled. The fresh stdio acceptance checks zero locked IPs after reopening.
Vision integration uses one check/apply Tcl entry point documented in
`camera_base_2019_1_fixed/vision/ps/README.md`. Diagnostics are archived under
`jobs/vision_repair_20261003/diagnostics/`.

The first independent-reader implementation completed routing but failed timing
(`20261003_181651_impl_f3879323`, WNS -5.244 ns, WHS -1.111 ns). Its clockless
register and unconstrained internal endpoint counts are both zero. Revised HLS
and removal of optional CSI debug probes passed internal routed timing
(`20261003_184615_impl_15af3bd9`, FCLK1 WNS +0.399 ns, WHS +0.059 ns), with
two camera input endpoints still failing. Subcore migration verification
`20261003_185823_impl_ab2dc7af` completed with the same margins, no DRC Error,
no locked IP and no unused-ILA critical warnings. Its failed receipt correctly
reflects camera setup/hold violations; full reports were saved before exit.
The camera's
12 MHz input oscillator is confirmed by the supplied AN5641 schematic; MIPI
pin frequency and input timing assumptions still need independent confirmation.
The build keeps reports for a completed route with failed timing and rejects
bitstream generation. Reset operations archive prior run logs under the new
job's `previous_runs/`, so stale warnings are not mixed into fresh logs.

To remove the registration, use `codex mcp remove vivado`. To remove only the
Vivado startup listener, use the D-drive environment's
`python -m vivado_mcp uninstall D:/XILINX/Vivado/2019.1/bin/vivado.bat`.
Keep the job artifacts and configuration backups needed for recovery.
