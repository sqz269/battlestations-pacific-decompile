# cc13 leaf reconstructions against the original bytes, pass 1

Addresses: 00BB9F40 00BBA030 008EF6A0 008EF6D0 008EE5F0 0085D020 00809E10 0042B2F0 00481C90
00481D40 0046F440 0046FC80 00439BD0 0073E780 0073E7C0 00569E40 0059A840 0059A860 00709A60
00709BB0 0078E2B0 0078FEB0 00438B10 00BF7030

Packet `cc13-fixture1`, branch `agent/cc13-fixture1`, 2026-10-12. Project `C:/Users/sqz269/bsp.gpr`,
program `/battlestationspacific.exe` (checked by `Client.verify()` before the byte reads). No game
runs, no Ghidra writes. Descriptive names are hypotheses, not recovered symbols.

## The machinery

The project has no emulator and does not load the original executable. Its native fixtures copy
a routine's original bytes into a private executable page in the test process and call them
next to the reconstruction:

- `tools/ghidra_export.py verify-seeds` reads eight seed ranges, compares the disk bytes with the
  saved Ghidra program, and writes the ignored `local/seed_reference.hpp`. CTest runs
  `native_math_differential` (`tests/native_math_tests.cpp`) only when that header exists.
- `tools/verify_camera_reference.py`, `verify_gui_geometry_reference.py` and
  `verify_timestamp_reference.py` do the same for larger routines. They use capstone to find
  every absolute operand and relative CALL, and add a patch list so the probe can point those
  operands at its own copy of the constants.

This pass uses the second pattern. There is no new framework:

- `tools/verify_cc13_leaf_reference.py` packs 23 code ranges and 13 data ranges into one payload
  and writes the ignored `local/cc13_leaf_reference.hpp`. The data ranges are 9 constants, the
  two map offset globals, the CRC-32 table (BSS) and the scene-text buffer (BSS). It writes
  `reports/cc13_leaf_reference_audit.json` with a hash of each range.
  - Each range is read from the disk executable and must equal Ghidra's `read_memory` bytes.
    Bytes past a section's raw data are loader zero-fill.
  - Every absolute memory operand or immediate inside the image is relocated to the payload copy
    of a listed range.
  - Every relative CALL is relocated to a listed routine.
  - The tool refuses on anything else: an unlisted address, a call to an unlisted routine, a
    branch out of the routine, or disassembly that does not end at the RET.
  - It found 28 relocations.
- `tests/cc13_leaf_fixture_tests.cpp` is registered in `CMakeLists.txt` as CTest
  `cc13_leaf_differential` and is built only when the header exists. It links `bsp_core` and
  compiles the four cc13 band sources that only `bsp_game` builds: 0040, 0045, 0067 and 0090.
  - It runs one case per routine on a fixed xorshift input set and compares results bit for bit
    with `memcmp`. Output buffers start from the same sentinel on both sides.
  - It pins the x87 control word to 53-bit precision and round-to-nearest.
  - It calls the native code through `__fastcall` function pointers. A `__thiscall` routine is
    called as `__fastcall` with an unused EDX argument; the stack cleanup is the same.

Substitutions and limits:

- **sqrt.** `LIBCRT_sqrt` (00BF7030, `_CIsqrt`) dispatches into CRT code that is not copied.
  Calls to it go to the bytes `FSQRT; RET` instead. At 53-bit precision, with float-exact
  arguments, this gives the same value as the CRT's SSE2 path.
- **Precision.** After the device is created the game's main thread runs at 24-bit precision
  (docs/CC13_FPU_PRECISION.md). The SSE2 reconstructions cannot follow that mode, so these
  cases do not cover it.
- **Virtual call.** 00809E10 gets its launch speed through `[record+3Ch]->vt[38h]`. The fixture
  supplies that slot with a C++ `__fastcall` function that returns the chosen float.
- **Globals.** The map offsets 00E08890/00E08894 are written into the payload copy for each case.

Run it with `python tools/verify_cc13_leaf_reference.py` (it needs the Ghidra bridge), then
`./scripts/build.ps1`.

## Results

Status before: `reconstructed_build_tested`, or no reconstruction record (`-`). Status after:
`semantic_native_fixture_tested`, the status that 00401C20 and the other `native_math_tests`
routines already use. The pass/run column counts input sets.

| Address | Reconstruction | Source | Fixture result | Status change | Bug fixed |
| --- | --- | --- | --- | --- | --- |
| 00BB9F40 | `build_crc32_table` | src/band_0090_cc13.cpp | 1/1 match (whole 1 KB table) | none recorded, see below | - |
| 00BBA030 | `crc32_update` | src/band_0090_cc13.cpp | 14/14 match (0..299 bytes over the native table) | none recorded, see below | - |
| 008EF6A0 | `hex_digit_value_008ef6a0` | src/band_0080_cc13.cpp | 256/256 match (every byte) | - -> fixture | - |
| 008EF6D0 | `decode_hex_byte_token_008ef6d0` | src/band_0080_cc13.cpp | 170/170 match (null, 1..3-character tokens) | - -> fixture | - |
| 008EE5F0 | `escape_scene_text_008ee5f0` | src/band_0080_cc13.cpp | 201/201 match (null, random `\n " \` text, limits 0..44) | - -> fixture | - |
| 0085D020 | `segment_intersect_triangle_0085d020` | src/band_0080_cc13.cpp | 1953/2000 before the fix, 2000/2000 after | - -> fixture | **yes** |
| 00809E10 | `bsp::rocket_flight_time_00809e10` | src/rocket_task_cc13.cpp | 2000/2000 match (both branches; 0042B2F0 copied, not reached) | build -> fixture | - |
| 00481C90 | `component_min_00481c90` | src/band_0045_cc13.cpp | 500/500 match (one in five with equal y) | build -> fixture | - |
| 00481D40 | `component_max_00481d40` | src/band_0045_cc13.cpp | 500/500 match | build -> fixture | - |
| 0046F440 | `point_line_distance_0046f440` | src/band_0045_cc13.cpp | 2000/2000 match (unit dir; one in fifty on the line) | build -> fixture | - |
| 0046FC80 | `ray_plane_intersect_0046fc80` | src/band_0045_cc13.cpp | 828/2000 before the fix, 2000/2000 after (one in fifty parallel) | build -> fixture | **yes** |
| 00439BD0 | `solve_quadratic_00439bd0` | src/band_0040_cc13.cpp | 2000/2000 match (both disc signs) | build -> fixture | - |
| 0073E780 | `plane_from_normal_and_point_0073e780` | src/band_0067_cc13.cpp | 2000/2000 match | - -> fixture | - |
| 0073E7C0 | `plane_signed_distance_0073e7c0` | src/band_0067_cc13.cpp | 2000/2000 match | - -> fixture | - |
| 00709A60 | `vec3_add_00709a60` | src/band_0067_cc13.cpp | 2000/2000 match | - -> fixture | - |
| 00709BB0 | `vec3_add_scaled_00709bb0` | src/band_0067_cc13.cpp | 2000/2000 match | - -> fixture | - |
| 00569E40 | `remap_linear_00569e40` | src/band_0050_cc13.cpp | 2000/2000 match | build -> fixture | - |
| 0059A840 | `world_to_map_x_0059a840` | src/band_0050_cc13.cpp | 2000/2000 match | build -> fixture | - |
| 0059A860 | `world_to_map_y_0059a860` | src/band_0050_cc13.cpp | 2000/2000 match | build -> fixture | - |
| 0078E2B0 | `movcam_angle_windows_contain_0078e2b0` | src/band_0076_cc13.cpp | 2000/2000 match (wrapping windows included) | - -> fixture | - |
| 0078FEB0 | `movcam_angle_windows_clamp_0078feb0` | src/band_0076_cc13.cpp | 2000/2000 match (00438B10 copied and called) | - -> fixture | - |

Totals: 21 routines, 21 pass. Two failed on the first run and pass after the fixes below.
Ledger: 19 reconstruction records were added or updated with
`semantic_native_fixture_tested`: 9 replaced and 10 new.

The two CRC-32 routines pass, but their addresses are leased to `cc13-mopup1`, so their ledger
records are left for that lane or the lead. They should get
`semantic_native_fixture_tested`, src/band_0090_cc13.cpp, ABI `__fastcall(ECX = table)` /
`__thiscall(ECX = crc*, bytes, count) RET 8`.

00BF7030 is not under test; it is the substituted CRT call. 0042B2F0 and 00438B10 are copied
only as callees. 0042B2F0 is not reached because the fixture uses the virtual-call branch, and
00438B10 has no reconstruction of its own under test here.

## Bugs found

1. **0046FC80 `ray_plane_intersect_0046fc80`: the product was not rounded to float.** The model
   computed `out = float(origin + dir * t)` in double. At 0046FD62..0046FD9A the native stores
   each `dir.? * t` to a float slot (`FSTP [ESP+14h/18h/1Ch]`) and then adds the origin
   component that is still on the x87 stack. The fix is
   `out.? = float(origin.? + float(dir.? * t))`. On the first run 1172 of 2000 cases differed in
   the last place.
2. **0085D020 `segment_intersect_triangle_0085d020`: summation order, and a missing float store
   before each multiply by 1/det.** The trace of the listing gives:
   - det = `(hx*e1x + hy*e1y) + hz*e1z` (0085D10B..0085D129). The model added `hz*e1z` first.
   - The u, v and t dot products are each stored as a float (`FSTP [ESP+40h]` / `[ESP+48h]`)
     before the multiply by the float `inv`, and the product is stored again. Their orders are
     `(hy*sy + hx*sx) + hz*sz`, `(qy*dy + qx*dx) + qz*dz` and `(qy*e2y + qx*e2x) + qz*e2z`. The
     model multiplied the unrounded double sum, with `hz`/`qz` first.

   The fix changes only the order and the rounding; the reject tests were already right. On the
   first run 47 of 2000 cases differed in the output point, in the last place.

Both fixes are in this branch. `./scripts/build.ps1` passes: `reconstructed_math`,
`cc13_leaf_differential` and `tool_tests`. `native_math_differential` is not registered in
this worktree because `local/seed_reference.hpp` is absent here.

## Not covered (machinery limits)

| Candidate | Why skipped |
| --- | --- |
| 009FC260 leg-altitude law | It makes 8 calls, 3 of them indirect through the pointers at 00CE2210/00CE2218, plus 0042E740 and CRT helpers. It is not a leaf at the byte level. |
| 0080A000..0080A25C rocket homing steps | These are mid-function fragments of one routine with a shared stack frame. They have no callable entry. |
| 00722210 radix sort, 00724CE0 radix key | They call `__alloca_probe` and `_memset`, CRT code that is not copied. |
| 00412010 grid step | It uses a jump table at 00412050 whose absolute targets would need table relocation. Not attempted. |
| Any routine at 24-bit precision | The reconstructions are SSE2 C++. Main-thread gameplay math runs at 24 bits after device creation, which these cases do not cover. |

## Evidence classes

- Exported: the Ghidra listings for every row.
- Reconstructed: all 21. Build-tested: all 21.
- Fixture-tested: all 21, in the sense above. The cases use a fixed input set at 53-bit precision.
  Native code is called through its original register ABI, but the reconstructions are new C++
  interfaces, not ABI-compatible replacements.
- Game-validated: none.
