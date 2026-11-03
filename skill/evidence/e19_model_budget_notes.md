# Vision workbench

Maintained production and current PL status: [PROJECT_RESUME](../../PROJECT_RESUME.md),
[live_overlay](live_overlay/README.md) and [two-phase DMA](live_overlay/dma_two_phase/README.md).
The earlier model/NN experiments below retain their dated historical conclusions;
they do not describe the deployed live detector.

Current offline CDC evidence and maintained runners: [CDC_REVIEW.md](CDC_REVIEW.md).
Full isolated synthesis `20261007_082538_camera_nn_parent` completed successfully;
VTC audit `20261008_134812` and provenance/fanout review `20261008_135311` passed.
These do not establish routed camera timing or physical HDMI acceptance.
CSI recovery `20261008_140539` passed 438 scenarios/22381 atomicity checks;
real CSI AXI audit `20261008_142432` passed 375 reads/183 writes/11963 pair checks
and OOC (2 Info, zero Warning/Critical). Full isolated CSI/Gamma integration
attempts `20261008_143247`, `143526` and `152044` failed before synthesis on Tcl
file-read errors. Bounded probes `155250`/`155711` failed 4/4 imports despite
readable files; simple source probe `155457` passed. Root cause is unresolved.
Independent output-SOF acknowledgement work: [overlay_ack/README.md](overlay_ack/README.md).
Physical timing remains pending.

2026-10-05 rotation candidate: `blocks_train_20261005_03`, 180-degree training
augmentation and checkpoint selection using spatial instance matching on four
rotations of the 2 validation images. Original 13 training images are unchanged;
today's six-object photograph was not used in training or checkpoint selection.
Model3 matches the original 20 count/category checks and current raw-photo
assistant review. The new six colors still await user confirmation. Two out of
four pixel rotations of the new frame still confuse an orange pentagonal prism,
so this is not a completed arbitrary-orientation recognizer. Evidence/configs
are under `toolchain/cache/vision_benchmark_20261005`; model2 evidence is preserved.

Current candidate: four-shape YOLOv8n instance segmentation plus independent
per-color saturation voting. Runtime ROI is measured from the actual black
border. MobileSAM proposes annotation masks; it is not the final detector.
The old rules remain regression evidence rather than the implementation plan.

## Evidence and limits

`recognition_status.json` indexes 20 reviewed scenes: 13 training, 2 validation,
5 isolated test images from two placements. All match object count and the
color/shape category multiset. Selection used the validation split, never test
images. Masks are assistant-reviewed proposals, not pixel-accurate human truth.
No true contour IoU, arbitrary-view 3D accuracy, all 20 combinations, or board
FPS has been established. Two user-excluded ROI failures and quality-rejected
banded images retain their original files and records.

`blocks_train_20261004_02/status.json` identifies the checkpoint selected at
epoch 50 during 120 CPU training epochs. `configs/recognition_candidate.json`
is the maintained analyzer configuration. The latest held-out rotated four
object scene is `20261004_220536_capture_79f603c1`; it was not used for training.

Software export simulation passed 18 images with exact PyTorch/TorchScript raw
tensor agreement. Export-mask IoU is backend agreement, not physical accuracy.
The original export report underestimated total BRAM: xc7z020 has 140 BRAM36,
equivalently 280 BRAM18 or 645,120 bytes. INT8 weight storage (2,937,564 bytes)
exceeds that total. The 409,600-byte largest observed feature map alone fits the
total, but other buffers and camera logic also need memory. DDR tiling or a
smaller model is required. This correction does not invalidate export equality.

## Maintained scripts

| Script | Purpose |
| --- | --- |
| `benchmark.py` | Reviewed historical, temporal or explicit-manifest count/category regression |
| `prepare_dataset.py` | Immutable dataset version; hardlinks reviewed image/label files |
| `train_instances.py` | CPU training and validation-only checkpoint selection |
| `simulate_export.py` | Software export numerical comparison |
| `export_stem.py` | Model-specific first-layer INT8 weights and independent integer fixtures |
| `integer_stem.py` | Freeze bias/Q30/SiLU; exact one/two early block software comparisons and fixtures |
| `prepare_conv_tiles.py` | Immutable independent ordinary/depthwise/partial-sum integer convolution fixtures |
| `conv_tile_runtime.py` | Wide-channel packing, retained partial sums and NHWC tile assembly checks |
| `simulate_stem_quant.py` | Stem, all convolution weights, or calibrated convolution input/output fake quantization |
| `setup_mobile_sam.py` | Pinned annotation-helper setup on D: |

Run model scripts with existing `D:/Python38/python.exe`; extra thin dependencies
are in `toolchain/model_deps`, while existing Torch is reused. Task-runner/MCP
scripts use `toolchain/.venv/Scripts/python.exe`. New data, cache and receipts stay
under `toolchain`. Benchmark and dataset destinations refuse to overwrite saved
evidence; do not regenerate `nn_stem/testdata` without a new version.

```powershell
.\toolchain\fpga.ps1 analyze --input D:\312312\FPGA\toolchain\jobs\20261004_220536_capture_79f603c1\frame.png --config D:\312312\FPGA\toolchain\vision_workbench\configs\recognition_candidate.json
D:\Python38\python.exe toolchain/vision_workbench/benchmark.py --method trained_instances --group all --training-status toolchain/jobs/blocks_train_20261004_02/status.json --output toolchain/cache/vision_benchmark_new
```

Unused rule experiments were archived with hash and ZIP CRC checks at
`toolchain/archives/vision_rule_trials_20261004/`. Keep captures, labels and failed
receipts. Recognition validation precedes HDMI ROI/object overlay integration.
HLS work is documented in `camera_base_2019_1_fixed/hls/vision_nn_stem/README.md`.

## Quantization and latest live check

The current model3 integer prefix includes the first C2f after two early
convolution blocks. integer_c2f.py freezes its four convolutions and unsigned
Q24 residual/concat scaling from the existing 13 training-image calibration.
All 21 reviewed images retain counts/categories and matched floating instance
boxes have agreement IoU at least 0.9676. This is backend agreement; physical
contour quality and arbitrary-rotation recognition remain unvalidated.

prepare_tensor_program.py converts that prefix into instructions and immutable
readback oracles. Its cache stress changes RGB at the same dimensions, then
returns to the original real input; the mirrored numerical fixture is neither
a new physical scene nor a training example. References are under cache/nn_c2f.
Board orchestration and the PS interpreter are maintained in
camera_base_2019_1_fixed/hls/vision_nn_stem/board, described in its README.

integer_backbone.py extends the frozen first three modules with sequential
Conv/C2f operators. Current through5 parameters and program are under
cache/nn_backbone/model_03_through5_01 and program_through5_01. They cover output
channel tiles up to the current 64-channel modules and a 128-channel input tail,
with the original first-three-module instructions/parameters/tensors preserved.
All 21 reviewed images retain counts/categories and floating-instance box
agreement at least 0.9709 IoU. Actual five-module execution and timings are in
the matching physical-board program receipt, not in the PC comparison.

--compact uses tensor_storage.py to align maximum-size per-tensor reservations
across all fixture dimensions, with disjoint 64-byte guards. Numerical files
and parameters are preserved; only tensor pointer operands move. A round-trip
instruction comparison and range sweep are checked before new manifest writes.
The first seven-module references are in nn_backbone/model_03_through7_01 and
program_through7_compact_01. All 21 reviewed PC images passed with the rest of
the network still float. Global PS gather/compiler input now supports512;
model_03_through9_01/program_through9_compact_01 include the actual384-channel
tail and256-channel output. PL still executes32-channel tiles. Input slices
are packed once and reused across output slices; arithmetic/oracles are preserved.
PC21/21 and matching physical13/13 are separate receipts. SPPF maxpool, PAN
resize/skip paths, prototype and detector decoding remain incomplete.

The complete first10-module backbone includes integer SPPF and its512-input tail.
References are model_03_through10_01/program_through10_compact_01 under nn_backbone.
NumPy and Torch pool/conv tensor oracles agree; all21 reviewed PC images pass
with the rest of the network still float.1149 instructions exceed the older1024
ABI; the matching4096-capacity firmware and its separate board receipt are required.

prepare_overlay_history.py generates full1280×720 RGB streams from three frozen
historical placements plus a disabled-overlay frame. vision/pl/tests/
verify_overlay_history.py runs the actual existing AXI overlay RTL, compares
all pixels and sidebands under stalls/valid gaps/frame-latched updates, and
renders actual simulator output with render_overlay_history.py.
Job233715_overlay_history passes all3,686,400 pixels. This is overlay RTL proof,
not HDMI-screen or integrated integer-recognition/camera-frame alignment proof.

integer_neck.py models the complete PAN skip graph with explicit code domains,
Q24 branch rescaling and nearest2 duplication. prepare_neck_program.py preserves
actual skip DDR tensors and compiles the backbone/PAN into2118 instructions.
Whole-graph fixtures use stride32-aligned RGB sizes (32x32 minimum, a64x96
rectangle, white64x64 and real320x320/mirror); the earlier single-pixel backbone
fixture is not a valid fixed2x-resize/skip graph input. model03_pan_01 passes
all21 reviewed PC images with Segment heads/decoding still float.

prepare_segment_specs.py and integer_segment.py cover all81 learned Conv2d
plus prototype transpose with integer affine/Q30/LUT references. Wrapped Conv
blocks are replaced before their original SiLU, while raw output logits use an
identity LUT. Grouped depthwise input tiles and linear outputs have independent
synthetic oracles/compiler checks. Four1x1 phases reproduce the2x2/stride2
prototype transpose arithmetic; actual parity-scatter scheduling is pending.
model03_accuracy_01 passes21/21 reviewed images, but DFL, sigmoid and final
box/mask/NMS still float there; no full head hardware claim.

verify_integer_decode.py checks production C nn_decode_ops.h DFL/sigmoid versus
Python integers and Torchfloat64. Three branch LUTs, each tested on2100 anchors,
pass exact C outputs with maximum DFL error0.00012206. This primitive evidence
does not connect final decode to the classifier or replace pending box/NMS/mask
implementation and complete hardware acceptance.

prepare_head_program.py appends36 head branches and prototype four-phase scatter
to the checked PAN program. tensor_storage.py also packs readonly parameters
into the existing noncacheable13000000..14000000 section, with inverse-pointer
checks. All2649 instructions fit the4096 ABI;1661 retained tensors reserve
69,189,952 bytes and1259 parameter files reserve9,853,440 bytes.
Job20261006_004617_nn_program_parent passes all16 physical cases, including
all81 learned Conv2d and the prototype transpose. Final decode is separate.

integer_postprocess.py connects integerDFL, sigmoid lookup and Q12/Q13 box decode;
lookup tables freeze at setup. integer_mask_nms.py checks the production C
nn_postprocess_ops.h on the host: stable NMS Q12/Q8,32-channel mask dot product,
integer-ratio bilinear threshold/crop, bounded buffers and explicit overflow.
integer_frontend.py uses that C library during integer prediction, restores
the floating reference functions after each call, and asserts selected box/
coefficient/prototype provenance. model03_integer_frontend_01 passes all21
reviewed images with integer neural/postprocess decisions. Its ROI contract is
the project's square rectification and agnostic NMS0.5; PC geometry reporting/
color vote adaptation and full ARM/camera integration still require completion.

prepare_final_program.py adds the full ARM finalizer and its lookup tables,
144-word packet and8 native masks. finalizer_host_01 verifies production C
composition with real32-bit ABI pointers in an owned arena, exact packet/masks,
guards and invalid version handling. verify_final_masks.py independently
recomputes the mask oracle with NumPyINT64 dot and rational interpolation;
real/mirrored six-object masks each pass all3,687,936 pixels and moments.
This separate reference avoids using the C primitive as its only oracle.

render_arm_result.py requires a successful physical report and renders actual
ARM packet/masks over the frozen historical RGB blob resized into target-mask
coordinates. It does not fabricate a fresh camera or HDMI screenshot.

`verify_color.py --output <fresh-folder>` builds the production C color binding,
checks all 16,777,216 RGB triples against OpenCV HSV, then votes on 98 objects
in 21 frozen history images. Raw masks preserve the prior color labels while
counts are independently checked. Prior actual ARM masks are also tested on
native historical ROI pixels and separately on enlarged NN-input pixels.
`color_host_02/verification.json` includes version2 color packet tests and
immutable board fixtures. IDs are unknown0/red1/orange2/yellow3/green4/blue5;
shape fields retain the frozen model's raw0..3 indices.

Board runner `hls/vision_nn_stem/board/verify_color_board.py` accepts
`build --parent <verified-PL-parent> --host <host-verification.json>
--overlay <overlay-host-verification.json>` followed
by `board --job <matching-SDK-job>`. Both use the shared native lock. Accepted
job `20261006_174812_nn_color_parent` passed14 color packet cases, including
empty/error/in-place/repeated frames, and restored the camera application.
It uploads historical native RGB and prior actual ARM masks; it measures color
execution only and does not claim new neural inference or safe camera DMA.

`verify_nn_overlay.py --color-job <passed-color-job> --output <fresh-folder>`
maps actual ARM version2 packets with production C into native display boxes,
centers and the real black-border quad. It checks exact Python arbitrary-precision
geometry, independent OpenCV coordinates, malformed inputs/empty frames, and
compiles the same source for Cortex-A9. Its four-frame fixture runs through
`vision/pl/tests/verify_overlay_history.py --fixtures <fixture-folder>`.
The accepted host/RTL stages use `overlay_history/nn_actual_01` and native job
`20261006_182239_overlay_history`. This does not establish camera ownership or
HDMI screen acceptance. New color-board builds include the checked geometry
source and also compare actual ARM overlay configurations, with invalid ROI/
matrix cleanup. The previous accepted color-only job's software is preserved.

`20261006_182656_nn_color_parent` passed17 physical color+coordinate cases;
its six-object measured stage is86.8..87.5ms including cache, excluding neural
execution/ROI/JTAG. `prepare_arm_overlay.py --job <passed-geometry-job>
--output <fresh-folder>` uses the actual ARM config bytes for four full-frame
overlay fixtures. It never substitutes host configurations for ARM readback.

`verify_preprocess.py --output <fresh-folder>` verifies production
`vision_preprocess.[ch]` against the maintained Python black-border/OpenCV
reference. Accepted `cache/preprocess/host_04` passes21 historical images with
exact native784x784 and NN320x320 RGB pixels, row padding and13 rejected inputs.
This earlier preprocessing-only stage supplies corners from Python. The later
runtime pipeline below detects the black border on ARM.
`verify_preprocess_board.py build --parent <verified-PL-parent>
--host <host-verification.json>` then `board --job <fresh-SDK-job>` uses the
shared native lock and preserves terminal receipts. Accepted
`20261006_235228_preprocess_parent` passes13/13 physical cases; six-object ARM
Build+Run+cache is about446.16ms, excluding border detection/neural/color/JTAG/
camera/display. Frozen camera reload succeeded; fresh capture/HDMI stay deferred.

`prepare_runtime_pipeline.py --output <fresh-folder>` checks the production C
runtime composition and freezes independent integer references for the C-detected
ROIs. Maintained `cache/runtime_pipeline/host_04/verification.json` passes all21
historical images and four failure/generation checks; host03 is earlier frozen
evidence whose source hashes no longer match the maintained optimization.
The host NN boundary uses
a stub for composition tests; network expectations come from the frozen Torch
integer model, rational NMS, NumPyINT64 masks, OpenCV color and arbitrary-precision
geometry. No host execution of the real ARM network is claimed.

`verify_runtime_pipeline_board.py build --parent <accepted-pipeline-parent>
--host <runtime-host-verification.json>` then `board --job <matching-SDK-job>`
executes original historical BGR through ARM border/warp, the real PS/PL network,
ARM masks/color and original-image overlay configuration. It uploads no corners,
native ROI, NN input or golden tensor. Job `20261007_022805_runtime_pipeline_parent`
is the matching SDK; consult its board_status.json for the terminal board result.
`prepare_runtime_overlay.py --job <passed-runtime-job> --output <fresh-folder>`
feeds actual ARM configuration bytes and uploaded BGR into the existing four-frame
overlay RTL simulation. This still does not verify a physical HDMI screen.

`D:/Python38/python.exe -X utf8 -B toolchain/vision_workbench/verify_runtime_pipeline_board.py
recheck --parent <accepted-runtime-job>` repeats board verification into a fresh
`runtime_pipeline_recheck` job with `board_status.json`, preserving the accepted
parent receipts and SDK. It also checks routed margins and bit/HDF/PS-init
identity. No new bitstream or SDK build is implied. JTAG target discovery recovered
on2026-10-08; recheck212726 at10MHz failed on DAP transport during case2 and restored
the camera. Read-only code comparisons at5MHz passed16 reads/859904 bytes;
recheck213517 is running. The whole transfer deadline is3600 seconds for slower
JTAG, with the per-case ARM deadline and acceptance checks unchanged.
The old runtime does not contain FPC1.

`verify_runtime_pipeline_board.py cache_scratch --parent <passed-runtime-job>`
builds a separate matching SDK from the accepted standalone runtime. Only the
eight CPU-owned scratch sections at0x11800000..0x12000000 become write-back cached;
mailbox/JTAG control sections keep their non-cacheable attributes. `board --job
<new-SDK-job>` repeats the12 historical cases, checks actual MMU descriptors,
numeric results and memory guards, and records border/total speedups against the
frozen baseline receipt. The original accepted source and PL artifacts are preserved.
Accepted `20261007_035243_runtime_pipeline_parent` passes12/12,508 field checks,
all tensor guards and49 MMU descriptors. Six-object border execution is
941.57..952.58ms and total execution4267.65..4281.14ms, compared with the frozen
uncached baseline's5910.71..5953.64ms and9237.73..9279.57ms. These timers exclude
JTAG, camera acquisition and display. The maintained runtime probe now retains
the same scratch mapping; new builds verify its descriptors and can compare
against a parent's accepted runtime receipt. Old accepted source snapshots stay frozen.

`build_camera_nn.py prepare --parent <accepted-three-IP-parent>` snapshots the
camera BD/RTL/XDC and accepted HLS IP repositories into an isolated job. `run --job
<prepared-job>` builds full camera+three NN IPs+overlay/control at the existing7ns
video/NN clock, with separate HP2 DDR access. Failed timing/DRC or missing internal
constraints preserve reports and block bit/HDF export. Original camera sources
and SD are unchanged. The camera input-delay model still needs hardware confirmation.
Vivado2019.1 LATE ordering leaves the old IP's -add_delay entries active. The
maintained build explicitly excludes the fixed eight-line camera example XDC
after checking its contents, verifies synthesis/implementation constraint order,
and checks the routed XDC for exactly one four-line camera model. It uses
the 2019.1 Performance_Explore strategy for placement/fanout optimization without
changing the accepted scheduling margins. route_metrics.json retains global and
NN-clock slacks even when the final timing gate fails.
`build_camera_nn.py constraints --job <fresh-prepared-job>` generates the full BD
and checks the active XDC order without synthesis, routing or deployment. Its
receipt proves exclusion only; it does not validate camera pin frequency or UI.
`prepare` optionally accepts `--control-candidate <accepted-control-job>`.
This checks the frozen functional/CDC evidence and all artifact hashes, verifies
the maintained receiver matches its baseline and the actual CSI registered-enable
contract, then copies four accepted VHDL files into the new repository snapshot.
Only that snapshot gains the default-false `kSharedPPIEnable` model/user parameter,
XGUI callback and revision increment. The fresh BD explicitly enables it after
checking both lane counts and the direct PPI connection. Both generated synthesis
and simulation wrappers must contain the true generic. Production IP/BD remain
unchanged. Seven host integrity/rejection checks are recorded in
`cache/camera_control_package_01/verification.json`.
Accepted candidate packaging/constraints job `20261007_082243_camera_nn_parent`
passed those native Vivado checks and both active XDC-order checks, with unchanged
original source hashes. This is packaging evidence, not full-design CDC signoff.

```powershell
& D:/312312/FPGA/toolchain/.venv/Scripts/python.exe D:/312312/FPGA/toolchain/vision_workbench/build_camera_nn.py prepare --parent D:/312312/FPGA/toolchain/jobs/20261005_201051_nn_chain_parent --control-candidate D:/312312/FPGA/toolchain/jobs/20261007_065904_camera_control_candidate
```

Use `synth --job <new-prepared-job>` for full camera/NN/overlay synthesis without
routing or deployment. It preserves the synthesized DCP, detailed CDC/clock/
timing/utilization reports, rejects remaining black boxes, missing internal clocks
or unconstrained internal endpoints, and requires a complete CDC report with no
Critical checks. A passed synthesis inspection still leaves routed timing,
CDC warnings/protocols, physical input timing and hardware verification open.
Every native run holds the shared lock and rejects existing native sessions.
Accepted constraints-only job `20261007_053923_camera_nn_parent` generated the
full BD and verified both active constraint orders under Vivado2019.1, without
synthesis, route, bit/HDF or deployment. Register-slice parameters are configured
together and read back. The earlier052604 XGUI file-open failure remains archived;
independent053844 repeated file/IP tests passed but did not establish its cause.

`jobs/20261007_054224_sensor_register_readback` passed30 registers read twice on
the baseline hardware: OV5640 ID5640,302a=b0; actual3034/35/36/37=1a/21/46/05,
1280x720 output,HTS1896,VTS984. No sensor configuration writes or new images.
Frozen camera reload succeeded and original sources stayed unchanged. This is
register evidence only; physical pin frequency and external timing remain unverified.
`diagnose_camera_timing.py --parent <routed-camera-parent>` defaults to inspection
under the shared native lock. It requires one routed.dcp or target_routed.dcp and
its sibling routed_timing.xdc, verifies the DCP clock before reset_timing, preserves
all frozen constraints/explicit DDR bounds, then checks model and source hashes.
It never rebuilds the model from the working system.xdc. --check-only validates
and hashes without Vivado or a job; only explicit --sweep requests160 tap points.
Neither mode creates deployment artifacts or modifies hardware. Latest inspect
`20261015_181030_335346_camera_timing_diagnosis` reproduces the current280MHz
candidate's camera−0.247/−1.111ns, video+0.013/+0.056ns, CDC Critical0 and15
passing bus-skew constraints. Six focused regression tests pass; the older140MHz
−0.843ns result is a separate build. Full timing/physical release remains blocked.
Historical inspect job
`20261007_045856_camera_timing_diagnosis` retains camera setup-0.247/hold-1.111ns,
internal NN+0.171/+0.051ns and7 Critical CDC checks. Its10 bus-skew constraints
have positive slack (minimum5.953ns); this does not establish CDC signoff.
The full032949 camera parent failed and has no bit/HDF; do not rebase from it.
`cache/camera_sampling_budget_01/analyze.py` checks frozen diagnostic hashes and
derives necessary external-envelope bounds without Vivado or hardware access.
The latest default comes from032949/045856; the older160-point sweep comes from
030129/033013, a different routed checkpoint. Default taps0/12 have no permitted
input-envelope intersection; the widest sampled bound is0.120ns against the
modeled1.250ns width. This does not exhaust other clock taps/topologies or justify
replacing real input timing. Upstream Linux source review in
`cache/camera_clock_analysis_01/linux_review.md` leaves register3037=05 and the
physical forwarded-clock frequency unresolved.
After success, `verify_runtime_pipeline_board.py rebase --parent <passed-runtime-job>
--pl <passed-camera-PL-job>` extracts fresh HDF control addresses and builds a
matching SDK, then uses the same historical board verification. SDK aliases named
stem_system_wrapper.bit/hdf retain exact hashes of the full-camera artifacts.
The camera rebase also adds a diagnostic hook around the unchanged accepted
runtime body. It calls the production `VisionOverlay_Publish`, reads only mapped
registers, and checks submitted sequence, all56 coordinates, failure clearing,
invalid parameters and wrong-ID rejection on every historical board case.
This verifies AXI MMIO submission/shadow state; SOF acceptance and physical HDMI
display still require separate evidence. The hook passed Cortex-A9 compilation
in `cache/overlay_mmio_compile_01`; real MMIO verification remains pending.

Runtime rebuilds may inherit an accepted runtime job rather than the original
pipeline job. The build resolves that accepted ancestry, verifies the color-rule
manifest hash, and pins its path/hash in the new receipt. The board verifier
checks that manifest before preparing uploads. Cycles, failed parents and altered
rules are rejected; regression evidence is in `cache/runtime_color_provenance_01`.

Maintained exact sliding-covariance border optimization is board-accepted in
`jobs/20261007_051327_runtime_pipeline_parent/runtime_verification.json`:
12/12 historical cases,508 field checks,1670 tensor guards per case and49 MMU
descriptors passed, followed by successful frozen-camera reload. Six-object
border execution is872.857..875.800ms and total4206.220..4217.278ms against the
cached baseline's941.57..952.58ms and4267.65..4281.14ms. Neural execution remains
about2500..2511ms. Timers exclude JTAG/acquisition/display; this is not real-time
camera/HDMI verification. The production SDK main and SD image are not updated.

`simulate_stem_quant.py --mode all_conv_weights` checks all 82 Conv2d modules
with per-output-channel symmetric INT8 weights; `--mode all_conv_w8a8` also
freezes per-tensor input/output ranges calibrated exclusively on the 13 training
images. Both modes preserved reviewed counts/categories in all 20 images.
Biases, SiLU, tensor reductions and decoding remain floating point; these are
fake-quantization checks, not a full integer network or RTL. Mask-union backend
agreement was 0.99250..0.99742 for weights, 0.98541..0.99443 for convolution W8A8.
It measures perturbation of the reference, not true contour IoU. Reports are
`vision_benchmark_20261004/all_conv_weights_quant_01/report.json` and
`vision_benchmark_20261004/all_conv_w8a8_quant_01/report.json` under toolchain/cache.

The powered-board temporal capture `20261004_232418_capture_d85865ce` was checked
through the standard analyzer `20261004_232621_analyze_686a5642`: valid black ROI,
four expected categories, no clipped object. It was not added to training or to
the original 20-image summary. It is the same placement at another time, and
live DDR capture remains non-atomic. Further physical placements and credible
pixel annotations are needed before claiming boundary quality is resolved.

Display identity audits and their limits are maintained in
[`display_identity/README.md`](display_identity/README.md). Accepted actual
VDMA/OVL9 jobs `20261008_170101` and `20261008_170609` cover eight cases: 18
complete short frames and two first-line prefixes at 1280x720 (57856 pixels).
Park-then-commit can apply new config to an old image; polling current store
before committing can leave old config on a new image during a software delay.
These are verified counterexamples, not an accepted synchronization solution.
The next candidate must bind buffer/generation, detection and stream config,
including transition clearing and failures. No production or hardware change
is made; the timed-out full-resolution complete-frame attempt remains failed.
