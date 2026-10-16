# CC13 segment 95: engine utilities (cc13-seg95)

Segment 95 is `00BD2F10`-`00BD9210`. This packet closed the 34 functions earlier cc13 waves left without a
reconstruction record. It also covered one vtable slot that Ghidra never made into a function (`00BD6D50`).
The code is in `src/cc13_seg95.cpp` and `include/bsp/cc13_seg95.hpp` (namespace `bsp::cc13_seg95`), registered in
`bsp_core`. None of it is wired into the game.

What "reconstructed" means here: the code is build-tested only. It is a new C++ interface, not ABI-compatible,
not fixture-tested and not game-validated. Names are hypotheses.

Every routine in this segment is a game helper built on lower layers. Lua/LuaPlus core, CRT and STL bodies are
out of scope. The packet calls them rather than reimplementing them:

- **LuaObject primitives** `00B65F50`-`00B68100`: the game's 14h stack-slot LuaObject wrapper.
- **Native-string helpers**: `0041DD20`, `0041DD40`, `0041E870`, `004260B0`.
- **Sized pool**: `00419CC0` + `00BD1510`.
- **Archive emit helpers**: `00BD6990`, `00BD6AC0`, `00BD6BD0`, `00BD6100`, `00BD7B30`, `00BD7F50`, `00BD49B0`.
- **CRT**: `_fopen`, `_fclose`, `_fwrite`, `operator new`/`delete`, `_invalid_parameter`.

They are reached through the named hooks of `cc13_seg95::Host`, at the same call sites as in the binary. Three
pure CRT calls are made directly: `_stricmp` (00BF7FBF), `atol` (00BF8417) and `memcpy` (00BF7680).

## Outcomes

| Outcome | Count | Addresses |
|---|---|---|
| Reconstructed | 31 (30 pool + 00BD6D50) | see the table below |
| Existing implementation recorded | 0 | none had a body in src/; only host hooks existed (cc13_seg30, frontend_menus, engine_services_cc13) |
| Unreferenced, recorded only | 4 | 00BD5880, 00BD5A10, 00BD6E60, 00BD7000 |
| Skipped | 0 | |

The unreferenced census was run with `local/seg95_dwordscan.py`. It looks for every rel32 E8/E9 to the target in
`.text` and every absolute dword equal to the target in the image. As a positive control, 00BD2F10 returned its
calls. Nine pool functions had no reference of either kind. The five under 80 instructions are reconstructed
(00BD5040, 00BD5140, 00BD51E0, 00BD5330, 00BD7AA0). The four larger ones are recorded only, with their reading
in the ledger evidence.

## Reconstructed functions

| Address | C++ | ABI | Behaviour (binary order) |
|---|---|---|---|
| 00BD2F90 | `random_normal_sample_00bd2f90` | ECX stream; (mean, sigma); RET 8; ST0 | `00BD2ED0` picks the thread's state for the stream, then `00BF0DF0` (ECX = state) runs on the same two floats. It calls the existing `RandomThreads::state_00bd2ed0` and `ocean_normal_draw_00bf0df0`. |
| 00BD3810 | `singleton_client_construct_00bd3810` | fastcall ECX this | Sets the vtable to 00D682D0. Locks the lifetime manager, sets 0109CECC = this, calls `00BD0C30(0109CECC)` and unlocks (`cc13_seg41::singleton_register`). |
| 00BD38B0 | `singleton_client_destruct_00bd38b0` | fastcall ECX this | Sets the vtable to 00D682D0. Locks, calls `00BCFCA0(0109CECC)`, sets 0109CECC = 0, unlocks and sets the vtable to 00CE3818 (`singleton_unregister`). |
| 00BD47B0 | `state_owner_construct_base_00bd47b0` | fastcall | Calls `00BD3810` and sets the vtable to 00D682F0. Zeroes +4..+30 (+2Ch excepted), runs `00B66BD0` on +38h, then zeroes +500..+52C. |
| 00BD4270 | `state_owner_destruct_00bd4270` | fastcall | Sets the vtable to 00D682F0. Releases the string at +528. Deletes the +514 vector if non-null and zeroes it. Releases +500, closes the Lua state at +38h with `00B669A0` and releases +24 and +14. Last, 00BD4270 calls `00BD38B0`. |
| 00BD4BF0 | `matrix4x4_equals_00bd4bf0` | thiscall(a; b) RET 4 | Compares sixteen floats with FUCOMIP; NaN counts as unequal. |
| 00BD5680 | `tagged_value_equals_00bd5680` | thiscall(a; b) RET 4 → AL | Switches on a.tag: 0 is stricmp; 1 and 4 compare dwords; 2 and 10 compare floats; 3 compares bytes; 5/6/7/8 compare vec3/vec2/matrix/vec4; 9 is true; anything else is false. |
| 00BD4EB0 / 00BD4F30 | `archive_{reader,writer}_ref_construct_*` | thiscall(ref; archive, key) RET 0Ch | Sets the vtable to 00CE3768 or 00CE3794. Stores a refcount block from `operator new(4)` set to 1 (null-checked), then the archive. Calls archive vtable+4(key). |
| 00BD4FE0 / 00BD5010 | `archive_{reader,writer}_ref_open_*` | thiscall(**ECX archive**; out, key) RET 0Ch | Constructs `out`, then returns `out`. |
| 00BD5040 | `lua_read_vector3_xyz_00bd5040` | ECX out; LuaObject by value; RET 14h | GetByName z, y, x; GetNumber x, y, z; destructs x, y, z, then the argument. |
| 00BD5140 | `lua_write_matrix4x4_00bd5140` | ECX matrix; LuaObject; RET 14h | For rows 0..3: SetNewTableAtIndex, GetByIndex, then SetNumericElement for cols 0..3 (zero-based keys). |
| 00BD51E0 | `lua_read_vector2_xy_00bd51e0` | ECX out; LuaObject; RET 14h | GetByName y, x; GetNumber x, y. |
| 00BD5330 | `lua_read_integer_string_00bd5330` | ECX key NativeString*, EDX int* out; LuaObject; RET 14h | Runs `00B68100`, `00B662B0` and `atol`, destructs the field, then stores `*out`. |
| 00BD7AA0 | `lua_set_integer_as_string_field_00bd7aa0` | ECX key; LuaObject + int; RET 18h | Runs `004260B0` then `00B674C0(key, text)` and releases the text. |
| 00BD5860 | `stdio_stream_destroy_00bd5860` | thiscall | Sets the vtable to 00D68340, calls `fclose(+0Ch)` with no null test, then sets the vtable to 00CE3784. |
| 00BD5CE0 | `stdio_stream_construct_00bd5ce0` | thiscall(NativeString* path) RET 4 | Sets +4 = 0, the vtable to 00D68340 and +8 = 0. Stores `fopen(path or "", "wb")` at +0Ch. |
| 00BD5D60 | `stdio_stream_construct_compact_00bd5d60` | same | Same body and the same `"wb"` mode, then sets the vtable to 00D68350. |
| 00BD8D10 | `stdio_stream_open_block_00bd8d10` | slot 1 of 00D68340; RET 8 | Sets FILE = +0Ch, emits the indent and the key, emits `"{\n"`, then ++depth and FILE = 0. |
| 00BD6D00 | `stdio_stream_close_block_00bd6d00` | slot 2 | Sets FILE, --depth, emits the indent, `"}"` and the terminator, then FILE = 0. |
| 00BD6D50 | `stdio_stream_close_block_compact_00bd6d50` | slot 2 of 00D68350 | Sets compact = 1, runs the same body, then compact = 0. Ghidra has no function here. |
| 00BD8D70 | `stdio_stream_write_key_value_00bd8d70` | slot 3; RET 10h | Emits the indent, the key, the value and the terminator. |
| 00BD8DD0 / 00BD8DF0 | `*_compact_*` | slots 1 and 3 of 00D68350 | Sets 0109CEE4 = 1 around 00BD8D10 or 00BD8D70. |
| 00BD6DB0 | `stdio_stream_write_string_00bd6db0` | slot 4 of 00D68350; RET 4 | When compact, passes a copy to `00BD6100`. Otherwise calls `fwrite(data, len, 1, FILE)`, or `00BD49B0` when there is no FILE. |
| 00BD6300 | `lua_set_new_table_named_00bd6300` | thiscall(LuaObject; key) RET 8 | Tag 0: name string, `00B67580`, release. Tag 1: `00B66670`. Tag 2: truncate the key, then `00B66670`. |
| 00BD7180 | `lua_set_typed_value_00bd7180` | thiscall(LuaObject; key, value) RET 10h | Switches on the eleven value types (see the ledger evidence). Type 4 calls `[0109CEDC]` (ECX = payload) before the key dispatch. Types 5-8 build a sub-table with 1-based element keys. |
| 00BD8EC0 | `lua_writer_enter_new_table_00bd8ec0` | thiscall(writer; key) RET 8 | Takes a checked back() of the LuaObject vector at writer+4 (three `_invalid_parameter` tests kept). Then runs `00BD6300`, `00BD5790` and `00442220` push_back, and destructs the child. |
| 00BD9130 | `vfs_flagged_exists_mount_callback_00bd9130` | thiscall(visitor; mount, name) RET 8 | When the byte at mount+0Ch is set, stores visitor+4 = provider vtable+10h(name). |
| 00BD9160 | `vfs_flagged_exists_read_result_00bd9160` | thiscall | Returns AL = visitor+4. |

## Corrections to earlier ledger readings

- **00BD47B0 / 00BD4270** carry the cc13-mopup1 name "LuaStateOwner". The object is actually the 0109CECC
  singleton:
  - `BSP_SaveStorage_Initialize` (00BEB2C0) builds it.
  - `BSP_PcStorage_Destroy` destroys it.
  - `BSP_SaveStorage_AppendArchiveBytes` (00BD49B0) is called on it.

  The Lua state owner is only its member at +38h. The names are left unchanged; the C++ uses `state_owner_*`.
- **00BD5D60** does not use a second mode string. It opens with the same `"wb"` (00D1D430) and only swaps the
  vtable to 00D68350.
- **00BD4FE0 / 00BD5010** take the archive in ECX and the out-reference as the first stack argument.
- **00BD5880** ("IterateTableRecords") is a nested-table matrix reader:
  `out[outer_key * 4 + inner_key] = value`. Nothing references it.

## Draws: 00BD2F90

The routine itself makes no draw. `00BF0DF0` makes the draws:

- It takes two 32-bit words from the stream's MT state per rejection round, so 2k draws.
- A round passes when s = u1² + u2² < 1, with probability π/4. So k ≥ 1, about 1.27 rounds and 2.55 words per
  sample on average.
- There is no zero test on s.

The only caller is `BSP_GunBot_LeadAimTick` (00902920).

## Uncertainty and simplifications

- **Key truncation.** CVTTSS2SI of an out-of-range float key gives 80000000h in hardware. The C++ cast is
  undefined for those inputs.
- **Reused argument slots.** The binary builds the temporary native string in place over the key's (or value's)
  stack argument slots. The C++ uses a local; there is no observable difference.
- **Hook signature.** The `[0109CEDC]` hook assumes the installed function takes only ECX, because no stack
  argument is pushed.
- **Object layouts.** The 0109CECC object's layout is known only from these two bodies. +2Ch, +508h-+510h and
  +520h are never touched here.
