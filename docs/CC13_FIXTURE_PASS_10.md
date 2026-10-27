# cc13 fixture pass 10

Packet cc13-fixture10 adds native fixtures for 16 record-copy leaf routines. The method is the
same as in passes 2 to 9:

- `tools/verify_cc13_leaf_reference.py` copies each routine's original bytes, relocated,
  into the test image.
- `tests/cc13_leaf_fixture_tests.cpp` (`run_fixture10`) runs the native bytes and the C++ on
  the same inputs: 3000 cases of 80h raw words, one word in four forced to a NaN or infinity
  pattern, half of those signalling. The destination is prefilled with CDCDCDCDh and the
  whole 80h-word buffer is compared, so a write the image does not make (padding, bytes next
  to a byte field) is caught as well as a wrong value.
- Each case runs under both 53-bit and 24-bit x87 precision control.

The deciding precision is that of the calling thread (docs/CC13_FPU_PRECISION.md). Every row
here is called from main-thread game code, so every row is judged at 24-bit. Copies do no
arithmetic, so the two precisions agree on every row. Rows that match bit for bit at 24-bit are
promoted to `semantic_native_fixture_tested`.

## Picks

The candidates came from `local/kept/fixture_candidates.tsv` in the cc13 tree. The filters
were:

- the row is still `reconstructed_build_tested` on origin/main;
- the routine is not already in the reference tool;
- neither its address nor its source file is leased by another lane.

From what was left, straight-line copy routines were taken (no calls out, no data
references).

## Results

The figures are matches out of runs.

| Address | C++ symbol | Thread (callers) | 24-bit (deciding) | 53-bit (informational) | Outcome |
|---|---|---|---|---|---|
| 0047D250 | `bsp::cc13_seg7::record54_copy_0047d250` | STL record range copy/fill helpers (main) | 3000/3000 | 3000/3000 | fixed, promoted |
| 0047F930 | `bsp::cc13_seg7::record54_copy_assign_0047f930` | as 0047D250 | 3000/3000 | 3000/3000 | fixed, promoted |
| 0047D2E0 | `bsp::cc13_seg7::record4c_copy_0047d2e0` | as 0047D250 | 3000/3000 | 3000/3000 | fixed, promoted |
| 0047F9C0 | `bsp::cc13_seg7::record4c_copy_assign_0047f9c0` | as 0047D250 | 3000/3000 | 3000/3000 | fixed, promoted |
| 0047F0C0 | `bsp::cc13_seg7::record40_copy_0047f0c0` | as 0047D250 | 3000/3000 | 3000/3000 | fixed, promoted |
| 00454970 | `bsp::cc13_seg5::shake_key_copy_00454970` | ShakeChannel_CopyConstruct (main) | 3000/3000 | 3000/3000 | fixed, promoted |
| 007BAA80 | `bsp::cc13_seg47::plane_state_record_copy_007baa80` | PLANE_SYNC construct and serialize (main) | 3000/3000 | 3000/3000 | fixed, promoted |
| 007BAAF0 | `bsp::cc13_seg47::plane_control_state_copy_007baaf0` | BSP_Plane_ApplyControlStateMessage (main) | 3000/3000 | 3000/3000 | fixed, promoted |
| 0080B890 | `bsp::cc13_seg49::turn_path_segment_copy_0080b890` | TurnPath array helpers and planner (main) | 3000/3000 | 3000/3000 | fixed, promoted |
| 00811D10 | `bsp::unit_ai_order_copy_00811d10` | UnitInstance_FlipPendingAiOrderSlot, UpdateShipMotion (main) | 3000/3000 (the image arm) | 3000/3000 | OFF switch; not promoted |
| 00BC7F00 | `bsp::water_weather_cc13::w2_record30_copy_00bc7f00` | FUN_00bc81d0 (main) | 3000/3000 | 3000/3000 | promoted |
| 00924280 | `bsp::entity_mission_seg60_argon::scene_property_copy_float16_00924280` | BSP_MissionEntity_ApplySpawnDescriptor (main) | 3000/3000 | 3000/3000 | fixed, promoted |
| 00B8E750 | `bsp::cc13_scene2::copy_track_key_block_00b8e750` | virtual calls from GUI screens (AnimationTrack_GetKeyBlock) (main) | 3000/3000 | 3000/3000 | fixed, promoted |
| 00952DF0 | `bsp::cc13_seg62::copy_44_00952df0` | STL_inst_00955030 (main) | 3000/3000 | 3000/3000 | fixed, promoted |
| 00952F30 | `bsp::cc13_seg62::copy_backward_44_00952f30` | STL_inst_009551f0 (main) | 3000/3000 | 3000/3000 | fixed, promoted |
| 00955090 | `bsp::cc13_seg62::fill_44_00955090` | FUN_0095b130, a catch funclet (main) | 3000/3000 | 3000/3000 | fixed, promoted |

There are no pins. Every deciding pass matches outright, with no NaN-only differences.

## Fixture-found defects

**Signalling NaN on copies.** Before the fixes only 00BC7F00 matched; the others matched in
0 to 1764 cases of 3000. Every one of these routines moves its float fields through FLD DWORD /
FSTP DWORD, which quiets a signalling NaN (sets bit 22). The C++ copied them as plain floats or
dwords and kept the signalling pattern. Each float field now goes through
`cc13x87::x87_fld_fstp_bits`; fields the image moves by MOV stay raw. The recipes, read from
the listings (f = FLD/FSTP, d = dword MOV, b = byte MOV):

- 0047D250, 0047F930: d+00, d+04, f+08..+48, b+4C, d+50. 0047D2E0, 0047F9C0: d+00, d+04,
  f+08..+44, b+48. 0047F0C0: d+00, d+04, f+08, f+0C, d+10, f+14..+28, d+2C, f+30..+3C. These
  are the `kRecord54/4C/40` step tables in `classes_seg7_cc13.cpp`; only the 'f' step changed.
- 00454970: f+00..+0C.
- 007BAA80, 007BAAF0: f+00..+20, b+24, b+25, b+26, f+28..+38.
- 0080B890: d+00, d+04, f+08..+34, b+38, f+3C, f+40. The C++ had copied +00..+34 as dwords.
- 00924280: sixteen floats from `*(record+20h)`. 00B8E750: six floats from track+18h..+2Ch.
- 00952DF0, 00952F30, 00955090: per 44h record, d+00 by MOV, then f+04..+40.

**Byte fields in 00811D10.** The image copies the flags at +14h and +30h as single bytes. The
member-wise C++ copy (`dst.sub_a = src.sub_a`) let the compiler copy the whole dword, padding
included.

**00811D10 is live.** `game_hosts_ship_ai.cpp` reaches it through promote_order and
`motion_step_00825f20`.

- The image arm is `unit_ai_order_copy_00811d10(dst, src, true)`, a step table of the recipe
  f+00..+10, b+14, f+18, f+1C, d+20, f+24..+2C, b+30, f+34, f+38, d+3C. The fixture tests it.
- The live two-argument function keeps the member-wise copy until the OFF switch
  `kUnitAiOrderCopyPc24Cc13Bound` (include/bsp/ship_ai_navigation.hpp) is flipped.
- With the switch ON, a signalling NaN in any of the twelve float fields comes out quieted,
  and the three bytes after each flag are left as they were in the destination. Ordinary
  float values copy the same either way.
- `tools/off_switch_on_build.py --names kUnitAiOrderCopyPc24Cc13Bound` builds the ON state
  cleanly.
- The ledger row stays `reconstructed_build_tested` until the flip.

The other fixed bodies have no live caller. Their callers in `src/` are unbound cc13 host
interfaces or cc13 functions that nothing outside cc13 calls.

## Build change

`src/entity_mission_seg60_argon.cpp` moved from `bsp_game` to `bsp_core` in
`cmake/startup.cmake`, so the fixture test can link 00924280. Its only include is its own
header.
