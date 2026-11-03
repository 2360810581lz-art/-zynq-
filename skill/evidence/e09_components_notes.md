# PL color-group CCL and moments (offline primitive)

Current accepted-ID path selects **membership-only** mode:
`run.py --members-only --cosim`, frozen/exported evidence
`20261017_202413_hls_269cf4fd`. Fourteen BFS host cases and four RTL cases
pass. HLS estimates: LUT1963, FF2240, BRAM18=7, DSP1,7.967ns; input loopII1.
Compile with both `LIVE_COMPONENT_MEMBERSHIP` and
`LIVE_COMPONENT_MEMBERS_ONLY`. Root/run ABI is unchanged; component output
contains only its zero terminal and count0. This deliberately removes duplicate
moments/features already computed by the accepted kernel. Status1/2/4 remains
valid; the original raw64 record limit/status8 does not apply in this mode.
Actual six-kernel host composition `20261017_204541_hls_e22a8270` preserves all
four histories' final production results. Hardware root raster replay and
parent DDR scheduling are separate; see [resolver](../root_replay_hls/README.md).
Earlier full-feature mode and its evidence below remain available.

`live_components_stream` uses a pipelined row-run extractor and a concurrent
union/feature backend. Input is 1..640 by1..360 AXIS8: background0, warm1,
yellow2, green3, blue4. Warm deliberately merges red/orange, matching the
current foreground semantics. The input must already include production
green-support recovery. No classifier is implemented here.

Output: at most64 AXIS256 component records with area, bbox, first and second
moments, and group. Bit layout is in `components.cpp`. Coordinates are half
resolution, inclusive; first moments permit a centroid, not a calibrated arm
grasp coordinate. status1 invalid size,2 provisional-label capacity,4 framing/
invalid group,8 output capacity. Valid-size faulty frames drain input and
publish no records. Invalid dimensions consume no input. count0 identifies
empty frames; downstream must not wait for a stream record in that case.

Capacity is1023 provisional labels; no silent label wrap. The input pixel
loop has achieved II1, but the union backend can apply backpressure depending
on run/overlap density. Do not claim a worst-case one-pixel-per-clock CCL.

Evidence:

- `20261017_183837_hls_e560a1dd`: independent host/C simulation BFS oracle,
  four historical production-supported masks and ten synthetic/fault cases.
  Historical raw components:6,6,6,5; these are not final merged object counts.
- `20261017_184221_hls_681de8cd`: cleanly separated DATAFLOW process region;
  complete640x360 history-frame RTL pass, latency231181cycles at target10ns.
  HLS: LUT3578, FF3428, DSP23, BRAM18=27, estimated8.571ns; input loop II1.
  A runner initially marked this failed because it incorrectly searched stdout
  for the cosim table; the actual report says Pass. The parser is corrected in
  maintenance source; frozen job inputs/receipt are not rewritten.
- `20261017_184415_hls_119f601f`: separate passing export receipt, ZIP CRC and
  all packaged Verilog hashes match the tested RTL. Neither job is board-tested.

## Membership extension (separate ABI, verified offline)

Build the same source with `--members` to select `live_components_members`.
Original `live_components_stream` ports and behavior remain unchanged.
The extension adds AXIS64 row runs and AXIS32 provisional-to-final root mapping;
component records also carry their final root. Exact layout is in `components.h`.
Every output has an all-zero last=1/valid=0 terminal, including empty/faulted
frames. All data records have last=0. Arm and drain all three sinks concurrently
before starting; never wait for final status before draining the run stream.
Runs may have escaped before a late error: accept none unless final status=0.
Root maps include components filtered out by min_area. Run-ledger storage belongs
in external DDR, bounded by width*height*8+8 bytes, not full-frame BRAM.

`20261017_190558_hls_9efd17ec`: all14 host cases pass an independent per-pixel
BFS oracle, including one-to-one root/component assignment, exact coverage,
terminal framing, moments and late-fault discard. Four RTL cases pass:640x360
history, empty, transitive union and late framing error. Full history latency
231428cycles; input pixel II1; HLS LUT3944, FF3827, DSP23, BRAM18=27,8.571ns.
Export ZIP CRC and exact RTL hashes pass in
`20261017_191124_pl_primitives_review/verification.json`.
`20261017_190105_hls_ea6762f8` independently preserves all14 original-ABI host
cases. No full-system timing/FPS or board claim follows from these tests.

This is not a drop-in replacement for `detector.c::components`. Production
also needs blue-core percentile trimming, yellow support bounds, accepted-part
pixel labels and fragment adjacency/merging. The membership extension provides
the required pixel identity, but those corrective rules are not yet migrated.
Parent integration must preserve raster-order tie decisions (record emission
order differs), then compare final objects against the existing detector before
switching software. ROI, green recovery and dependable shape classification
also remain to be migrated/validated. Do not silently remove existing rules.

19:45 update: isolated [yellow bounds](../yellow_bounds_hls/README.md) and
[fragment merge/top8](../parts_stream_hls/README.md) now pass their C/RTL suites
and local HLS budgets. Four histories pass host composition of those kernels
against final production boxes. They consume accepted raster-ordered IDs,
not this kernel's raw roots; no direct CCL connection is installed. Green
recovery, blue bounds, eligibility/core rejection and the accepted-ID adapter
must still preserve the above production semantics.

```powershell
.\toolchain\.venv\Scripts\python.exe -B toolchain/vision_workbench/live_overlay/components_stream_hls/run.py --prepare-only
.\toolchain\.venv\Scripts\python.exe -B toolchain/vision_workbench/live_overlay/components_stream_hls/run.py --cosim
.\toolchain\.venv\Scripts\python.exe -B toolchain/vision_workbench/live_overlay/components_stream_hls/run.py --members --cosim
```

The preparation step reuses four actual historical images and excludes the
two user-excluded ROI failures. It freezes masks, reference detector and hashes.
Native HLS/Vivado tasks must remain serial under the shared toolchain lock.
