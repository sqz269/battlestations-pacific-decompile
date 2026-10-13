# cc13-fe9c: remaining tactical library bodies

Continuation of cc13-fe9 (docs/CC13_FE_9.md) over the segment 12/17 list left open there. Code is in
`src/frontend_seg12_17_cc13.cpp` / `include/bsp/frontend_seg12_17_cc13.hpp` (namespace
`bsp::cc13_fe3`, section "packet cc13-fe9c"). Every routine is *reconstructed* and *build-tested*;
none is ABI-compatible, fixture-tested or game-validated. Evidence: disasm-raw listing (x87 checked
against the PE bytes where reg-reg forms occur), live decompile for structure, strings and constants
from the PE on disk. Call rows: `reports/cc13_fe9c_calls.json` (verified with
`tools/verify_report_calls.py`).

## Routines

- 005021E0 LoadDownloadableClasses: each class entry's +46h is "owned" when the bit for
  `atol(UnlockDLCMinimum without "DL_Content_") - 1` is set in the 64-bit mask `[00E188A8]+2060h`.
  `__allshl` gives 0 for counts of 64 and above (n = 0 included), which makes the entry owned.
  The function fetches the Lua global VehicleClass and only releases it.
- 0050A400 ReadVehicleClassRow: the ECX the callers set is never read and the fourth stack argument
  is unused (RET 10h). The flag pushed per row is `Race == 1 || Race == 4`.
- 0050AA10 LoadVehicleModel: pack lookup order `mpak/Class_own_<n>.mpak`, `mpak/Class_<n>.mpak`,
  then the placeholder `giachievement_model_load_temp`; the last queued job gets the callback
  00509FB0.
- 00504130 LoadClassTables: despite the ledger name, most of the body builds the preview scene
  lighting (sun direction, diffuse, six ambient cube faces, HDR parameters, a shadow owner) before
  running the two autoload class scripts. The cube-face scale is `s * 1.5` stored back as a float and
  multiplied into each channel in extended precision (D8 C9 `fmul st(0), st(1)` confirmed from the
  bytes); face 3 has scale 0.5.
