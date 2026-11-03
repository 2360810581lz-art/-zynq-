# Exact-membership object color (offline candidate)

Selected: `20261018_021715_hls_2cefcb0f` under toolchain/jobs. Sixteen C
cases and six RTL transactions pass; exported for Vivado/SDK/HLS2019.1,
xc7z020clg400-2. No board access, SD writes or production application edits.

Input BGR comes from the immutable accepted-label image, not a bbox crop.
The final parts merge/top8 membership maps original accepted IDs to object
slots. Only those pixels vote; discarded parts and background never vote.
Membership packets precede the raster, and each output is speculative until
the final status and parent's same-frame ownership checks pass.

Pixel HSV exactly follows OpenCV uint8 reciprocal-table rounding; byte order
is explicitly converted from low-B BGR to the legacy helper's low-R RGB.
Fixed provisional ordered rules: red centers7/179,tolerance8,S>=80;
orange13/7/80; yellow28/12/50; green60/25/20; blue110/20/20, all V>=20.
Strict distance improvement gives rule-order priority on pixel ties. Only
pixels satisfying a rule are eligible. A unique winning color needs at least
half of eligible votes. Confidence is the winning fraction of eligible votes,
NOT calibrated probability; sampled and eligible counts are both retained.
In particular high confidence with low eligible coverage is not arm validity.
Shape is always0/UNK. It is never inferred from color.

`color.h` documents packet fields and renderer-compatible semantic IDs.
Shape0/UNK makes the existing renderer display e.g. `GREEN UNK`; legacy shape
IDs cannot be copied directly. Parent must associate final geometry and color
by output SLOT and immutable frame token, not by nearest bbox or array reuse.
Parent faults/ROI loss invalidate both HDMI metadata and grasp results.
Image coordinates remain pixels, not calibrated robotic-arm millimeters.

Independent OpenCV/NumPy oracles cover four historical images, random RGB
with acceptedID64 and multi-part objects, five colors, tie/dark unknown,
empty scenes, overlap/out-of-range membership, late pixel/framing faults,
missing objects and invalid dimensions. Historical labels are algorithm
predictions; equivalence is not ground-truth classification accuracy.

Selected 20261018_021715_hls_2cefcb0f: six parallel distance lanes and a
three-level comparator tree, host16/RTL6/export passed. HLS estimates:
2678 LUT,1906 FF,2 BRAM18,2 DSP,8.321ns, raster II2 at a
10ns target clock. Initial2877 LUT/2335 FF dropped about12.8%/21.8% using
bounded arithmetic and one shared dual-port histogram. Five colors are
mutually exclusive, so five replicated counter banks add no pixel throughput.
The hue comparisons remain parallel. This is a resource optimization, not a
claim of greater measured FPS. Initial and intermediate frozen jobs remain.

The OLD mixed eleven-IP resource sum in `20261017_221942_pl_resource_budget`
is54201/53200 LUT and273/280 BRAM18, BEFORE parent adapters/classifier.
That historical estimate **does not fit additively**; current actual bank plus
camera planning is 43792 LUT/258 BRAM18 (022517), still not integrated fit. Do not install every standalone core as-is or
claim whole-system timing closure. Next integration must reuse existing logic
and memory/transport resources and measure actual integrated utilization.

Reproduce isolated verification from workspace:

```powershell
.\toolchain\.venv\Scripts\python.exe toolchain/vision_workbench/live_overlay/object_color_hls/run.py --cosim
```

`blue_branch_hls/run_pipeline.py --color-job <selected-job>` adds the actual
color kernel to the existing real CCL/root-replay/accepted/refinement/parts
composition; it consumes actual membership outputs, then compares color,
counts and confidence to independent fixture oracles. Host stream scheduling
still does not establish parent RTL, DMA or HDMI behavior on hardware.

Selected full C composition: `20261018_022110_hls_37ed8a83`, all four histories
pass with actual upstream green output and actual final parts membership.
