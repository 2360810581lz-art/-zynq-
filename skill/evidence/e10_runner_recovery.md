# Vivado 2019.1 Tcl file-open failure: standard-channel lifetime

This directory holds bounded, process-local diagnostics. No vendor installation
files or API results were modified. API hooks are observation only and are not
required by the repair.

## Evidence

* `jobs/20261019_094453_file_api_channels`: Windows successfully opens
  `ipintegrator.tcl`, but the same handle is already present in Tcl FileInfo with
  writable mask 4. Tcl reports ENOENT without attempting ReadFile.
* `jobs/20261019_094743_file_api_stdio_baseline`: initial standard output handle
  `0x2ac` and standard error `0x2ec` both refer to task.log and have Tcl writable
  channels. At failure GetStdHandle has different output/error handles, while
  Tcl still retains the old records. Windows has reused `0x2ac` for
  `replace_bd_cell.tcl`. The Tcl record is writable, whereas source requests
  readable mask 2. The file exists and CreateFileW succeeds.
* The installed Tcl 8.5.14 implementation matches the upstream
  [TclWinOpenFileChannel](https://github.com/tcltk/tcl/blob/core-8-5-14/win/tclWinChan.c#L1302)
  loop: a handle match with different permissions returns NULL without setting
  errno. This explains the misleading ENOENT/No error. A writable open can also
  accidentally alias a stale writable channel. The exact native component that
  closes/replaces the OS standard handles has not been attributed.
* `jobs/20261019_094702_file_api_pipes`: same project/BD load with all three
  inherited standard channels backed by pipes succeeds, 9936 observed events,
  no source failure, no stale file channels. Vendor init and Tcl DLL unchanged.

## Repair

`toolchain/windows_pipe_runner.py` supplies pipe-backed stdin/stdout/stderr,
keeps input open while the child runs, and continuously drains merged output to
the job log in the parent. `pipeline.run_command` uses this launcher on Windows.
Exit codes, per-invocation success marker checks and bounded process-tree timeout
cleanup are retained. There is no source wrapper, source retry, vendor patch,
API hook, missing-file suppression, or change to FPGA inputs/constraints.

The original pipeline.py is saved in `before/`. Frozen RTL/helpers and old jobs
are never edited. New fpga.ps1 and MCP fpga_start_job tasks start fresh pipeline worker processes
and use the corrected launcher. Other Python processes that directly imported
the old in-process run_command need a reload. Generic upstream MCP sessions
use their own launcher and are outside this repair.

## Verification

* `20261019_095001_pipe_runner_checks`: through production pipeline entry,
  >384 KiB stdout/stderr drained; nonzero exit propagated; an earlier success
  marker cannot satisfy a later invocation; timeout stops the owned process.
* `20261019_094845_camera_checkpoint_pipe_recovery`: actual core synthesis and
  hierarchical camera diagnostic completed with unchanged qualified RTL.
* `20261019_095228_startup_isolation_camera_synth`: normal complete camera BD,
  all IP synthesis runs and top synthesis completed without the previous file
  failure, ERROR or CRITICAL WARNING. No diagnostic hooks were loaded.
* `recovery_review.json`: independently rechecked both successful jobs, protected
  files, input/source ancestry, report hashes, vendor DLL/init and known BOOT.
  Both builds give video 7 ns setup -1.755 ns, hold +0.037 ns, TNS -698.917 ns
  with 1169 failing endpoints. Internal register clock coverage passes; timing
  is not closed. The normal build retains 39 baseline CDC warnings and 188 DRC
  warnings (zero DRC errors), without CDC/DRC signoff. Worst paths run from
  owner3 coverage fault through cross-lane control to owner1 observer reset.
* `run_trace_*.py` are one-use instrumented probes, not build entry points.
  Their selection files prevent accidental repetition.

This repair does not certify board performance, MIPI external timing guarantees,
full-system fit, routed timing, or a release BOOT image. Board/SDK/SD unchanged.
