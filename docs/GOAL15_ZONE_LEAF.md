# GOAL15 zone leaf: 00423640

Completed bounded reconstruction and max-effort arithmetic/ABI review, 2026-10-15.
Names below are hypotheses, not recovered symbols. This is a new Source API with
mandatory native dependency bindings; no production binding, game/runtime, or
drop-in ABI claim is made.

| Routine | Native body | Original ABI evidence | Coverage |
| --- | --- | --- | --- |
| `00423640` | `00423640-0042395F` | ECX avoid-zone manager; two stack arguments, requested owner and params; `RET 8` at `0042395D` | complete bounded orchestration through mandatory bindings; strict MSVC Win32 object checked |

The export was generated with `python tools/bsp.py ghidra export 00423640` after the bridge verified target project `bsp`, program `/battlestationspacific.exe`, language `x86:LE:32:default`, base `00400000` against `config/target.json`. That config names `C:/Users/sqz269/bsp.gpr`. No Ghidra mutation was made. The current live xrefs query returned no references. `docs/SHIP_AI_OPEN_ITEMS.md` section 222 also records no rel32 or absolute references, but that historical disk scan has not been rerun here; the offline generator interpretation remains provisional.

## Established orchestration

1. Convert input parameter float +00h to signed int with `CVTTSS2SI` (`0042364E`), lookup the incoming manager's group with `004120D0`, and if nonnull invoke `0041D150(group, requested_owner)` to remove zones for the owner (null owner clears the group). `00423656` saves incoming ECX without changing it; `0042365E` passes that register to `004120D0`, whose body reads manager +58h and +08h. This establishes the manager receiver instead of an invented contour-owner receiver.
2. Read the scene chain head from `[[[00E188A8]+19CCh]+34Ch]` unconditionally (`00423676-00423686`). If owner arg is null, consider node +08h only when nonnull and its vtable +5Ch predicate with argument `44h` returns AL nonzero (`004236B9/004236BB`). If owner arg is nonnull, use it directly without that predicate. Nodes advance at +04h; a null owner scan visits every node, including rejected/null node payloads.
3. Preserve original params +04h once at `004236CA-004236CF`, before the per-owner loop. Per owner call `00880250`, explicitly round ST0 to float through `FSTP`/`FLD`, and write the rounded result to params +14h and +04h (`004236F2-00423702`). Divide the saved original cell size by that rounded value in x87, add f64 0.5, consume ST0 through `00BF7420`, and clamp signed EAX to 1..16 into params +08h. This includes exceptional conversion behavior; it is not an ordinary C++ cast.
4. Read source `[owner+3D0h]` vtable +10h and +14h extents (`00423738`, `0042374B`). Both must exceed one; otherwise the parameter writes still occurred. Allocate x pointer slots with `00BF55BE`; x*4 unsigned overflow saturates requested size to FFFFFFFFh (`MUL`, `SETO`, `NEG`, `OR`). Allocate x separate y-byte columns, without zero initialization, and initialize through `00880420(owner, columns, 1, params)`.
5. Repeatedly call `00421CE0(owner, columns, 1)` until it returns null in EAX. The source decompile mis-types EAX as a float; assembly `MOV ESI,EAX; TEST ESI,ESI` at `004237C1-004237C3` establishes a pointer. A DWORD sequence counter begins at zero once for the whole invocation, wraps on increment, and formats `AvoidZoneG %s %d #%03d` with `all` (live bytes `61 6C 6C 00` at `00CE3954`), integer layer and signed counter. The formatter takes five pushed words and the caller cleans `14h` at `004237F8`.
6. Replace the path name in place. If the new length equals old +04h length, keep the old buffer. Otherwise obtain a sized-pool block of length+1 (flag 1), return any old +08h buffer with old length+1/flag 1, store +04h/+08h and write the terminator only in the allocate branch. The length-zero branch returns old storage and zeros both fields. If final data is nonnull, memcpy exactly the stored length (no copied terminator).
7. Call `0041EA30(path, params)`, then append the path pointer to the params list whose proxy/head/size lie at +18h/+1Ch/+20h. `0041C0B0(head, head->previous, &path)` returns a 0Ch node, `0041FDD0(&params+18h,1)` checks/increments size, then writes `head->previous=new_node; new_node->previous->next=new_node` (`004238C4-004238CA`). This is tail insertion; the new node's next/previous/value are initialized by its allocator.
8. After null trace result, free each allocated column and the column array through `00BF6989`. No local EH frame or rollback frees are present in this function. Do not add stronger exception cleanup silently.

## Existing schemas and dependencies

- `cc13_left55::ShoreMaskParams` in `include/bsp/cc13_left55.hpp` (+00h depth, +04h cell_size, +08h signed block, +0Ch falloff, +10h ring_width) is embedded as the prefix consumed by `00880420`. This routine produces +04h/+08h/+14h. The new `NativeZoneLeafParams` defines only the touched 24h-byte prefix; its complete enclosing producer remains unknown because there are no callers.
- `Cc13X87cContourPathNative` in `include/bsp/cc13_x87c.hpp` is the complete 1Ch path producer schema: owner +00h, name length/data +04h/+08h, vector proxy +0Ch, first/last/end +10h/+14h/+18h. `00421CE0` allocates/constructs it. `Cc13AvoidZonePathNative` in `cc13_avoid1.hpp` is an existing partial view of the same point vector; that header explicitly identifies `00423640` as its caller.
- `Cc13X87cContourOwnerNative` supplies source +3D0h and transform data.
- The manager receiver reuses `Cc13AvoidZoneManagerNative`; group callbacks use `Cc13AvoidZoneNativeGroup`. The output path list is a separate 0Ch-byte list at params +18h, with head/size +1Ch/+20h, not the manager hull list or TerrainGridLayer registry list. `0041C0B0` produces its next/previous/path node words. The scene-chain overlay explicitly leaves +00h unread; only next +04h and owner +08h are established.
- Existing reconstructed dependency implementations remain available in `cc13_left55_seg55.cpp`, `cc13_x87c.cpp`, `cc13_avoid1.cpp`, `avoid_zone_geometry.cpp` and the native string-pool modules. `NativeZoneLeafBindings` requires every direct dependency and the actual global addresses. No adapter is installed by this packet; binding those existing reconstructions or native-compatible entries is the consumer's obligation. No dependency has a default stub.
- Callee-before-contract review: complete bodies of `004120D0`, `0041D150`, `00880250`, `00412F80`, `0041C0B0`, `0041FDD0`, `00419CC0`, `00BD1120`, `00BD1510`, `00BF7420` and `00BF7456` were inspected across this pass and its preserved handoff. Allocation/free thunks were checked live: `00BF55BE` jumps to `00BF681B`, `00BF6989` to `00BF65AC`. The `00BF7680` entry/copy setup and overlap branch were inspected; its established CRT identity is retained. Contour/simplify entry listings were checked for ECX provenance; their remaining bodies rely on the existing reconstructions. `00880420` uses the existing reconstructed body and schema.

## Arithmetic and ABI review

The exact ratio sequence is `004236F7 FSTP f32; 004236FB FLD f32; 004236FF FST [params+14h]; 00423702 FST [params+4h]; 00423705 FDIVR f32[saved original params+4h]; 00423709 FADD f64[00D7A280]; 0042370F CALL 00BF7420; CMP EAX,1 / CMP EAX,10h; MOV [params+8h],EAX`. No ambiguous `FMUL STi` occurs in this block. Live constant bytes `00 00 00 00 00 00 E0 3F` establish exactly 0.5.

The implementation calls the mandatory unit-length entry directly from inline x86 assembly and retains the spill/reload, two float stores, x87 division/addition and hardware converter. It reuses `native_crt_truncate_st0_00bf7420` from `native_render_batch_keys.cpp`: actual mutable `0109EEA4` nonzero selects `FSTP f64; CVTTSD2SI EAX`; zero selects the preserved `00BF7456` conversion and its low EAX result. Thus x87 control/exception behavior is retained for this local sequence instead of projected through MSVC `long double` (which would only be double). Initial layer conversion is an actual `CVTTSS2SI` memory instruction, including native indefinite/exception behavior for NaN and out-of-range input. Other dependency precision and exceptional behavior remain obligations of the required bindings.

Incoming ECX is the manager because the first `004120D0` lookup consumes it. Later saved ECX forwarding at `004237B8` into `00421CE0` and `00423897` into `0041EA30` is dead: the former overwrites ECX at `00421D04` before its first read; the latter at `0041EA4B`. Their stack-only dependency signatures retain their existing stdcall-compatible contracts. The new orchestration function has an explicit manager reference and bindings argument and is not the original `__thiscall` ABI.

## Verification and limits

- `cl /std:c++17 /EHsc /W4 /WX /fp:strict /O2 /c`: pass for the owned translation unit under MSVC Win32. Reproducible local script: `local/goal15_zone_leaf_compile.cmd`; object/listing: `local/goal15_zone_leaf.obj` and `.asm` (ignored artifacts).
- The emitted listing copies original params +04h once with `MOVSS` before the loop, emits `F3 0F 2C 00` for layer `CVTTSS2SI`, calls the required unit-length pointer then emits `FSTP f32; FLD f32; FST [params+14h]; FST [params+04h]; FDIVR f32; FADD f64`, and calls the shared hardware converter before the signed clamp. No intermediate C++ float/double arithmetic was inserted in that sequence.
- `python tools/verify_report_calls.py reports/goal15_zone_leaf.json`: 21 direct call rows, 0 failures. Four indirect rows identify both owner predicates and the two source extents separately. Report rows all belong to body `00423640-0042395F`.
- Full registered `scripts/build.ps1`/CTest is owned by the primary integrator and pending this worker's commit. No new tests were added. No native differential test, production dependency binding, startup/gameplay path, or drop-in binary ABI equivalence is claimed.
- The source deliberately retains uninitialized column allocation, same-length string-buffer reuse, terminator only on the resize branch, and node allocation/count/link order. There is no invented exception rollback. Borrowed native records, globals, callback storage and output list must be valid when reached; native/control storage must not overlap the new Source frame/bindings.
- Only the `00423640` name/function ledger entries are added. Ghidra annotations, CMake registration and packet-registry integration remain primary-owned; the lease stays held through integration.

Primary integration update: the registered MSVC Win32 build and all three
existing CTest suites passed. Ghidra annotation/save and standard export
refresh are complete. See [the batch receipt](../reports/goal15_reconstruction_batch.json).
These checks do not execute or establish original ABI/gameplay equivalence for this entry.
