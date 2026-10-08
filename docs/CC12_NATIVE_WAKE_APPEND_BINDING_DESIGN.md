# Native wake append: concrete binding design

Read-only design for the known **00810190 body, 1,183 bytes / 303 instructions**. The selected plan uses the accepted raw 0042B2F0 and 00810160 children, adds a same-translation-unit raw tail facade for the existing 00414C60 kernel, and borrows the genuine `GameNativeGeometryGlobals` reset storage through its public accessor. It creates no owner or reset cells. Source implementation remains subject to Root's review of this plan; no Source, build, link, execution, fixture, live address query, shared-ledger or Ghidra change was made for this design.

The worker merged current committed main with the prescribed registry-union resolver before analysis, preserving pending raw-dependency work. Both earlier evidence ZIPs remain unchanged. Root subsequently reported independent raw-dependency acceptance and registration underway; this design makes no additional runtime admission.

## Selected public and private interfaces

Proposed ordinary C++ entry, **not** a drop-in Original binary ABI:

```cpp
void append_native_unit_wake_00810190(
    void* actual_wake, const float* actual_world_position,
    float heading, float yaw_rate,
    bsp::game::GameNativeGeometryGlobals& actual_geometry_globals);
```

The wrapper obtains `actual_geometry_globals.zero_vector_00f87574().data()` through the existing public accessor. That acquires the stable address; it must not read or snapshot any reset value. Heading/yaw argument representations should be copied with four-byte `std::memcpy` into `uint32_t` words and passed as words, avoiding extra x87 loads or NaN quieting in the wrapper. The future compiled wrapper must prove that those copies and pointer acquisition use integer or status-neutral bit moves, with no floating arithmetic, environment operation or unexpected helper call. This is a required future artifact check, not an assertion about uncompiled code.

The private naked kernel can have `__fastcall` parameters `(void* wake, const uint32_t* actual_reset_words, const float* position, uint32_t heading_bits, uint32_t yaw_bits)`. Thus ECX is wake, EDX is the genuine reset address, and the three stack words are position/heading/yaw in their original order. The original `RET 12`, local allocation and all original parameter offsets remain valid. Float interpretation happens only at the original assembly operations. Using raw integer cells also avoids inventing a C++ `float` overlay on the owner's bit-word array.

The caller must supply the actual process instance returned by `bsp::game::game_native_geometry_globals()`, real live wake storage through at least `+0x3DB`, a valid ring head at every native head observation, and three readable position cells. It owns synchronization and lifetimes. Wake, position and reset regions may alias one another when all accessed ranges remain valid, but they must not overlap the active public/private adapter/kernel frames or formal argument slots. Copies of whole wake records, synthetic `ShipAiWakeTrail` values and new reset arrays are excluded. Scalar by-value argument copies are an explicit interface adaptation; arbitrary aliasing into a machine call's own stack frame is outside this Source contract.

## Genuine reset owner and the smallest selected binding change

The current compiled geometry getter is exactly six bytes: `MOV EAX,<process>; RET`. Its DIR32 relocation reaches one **writable 12-byte `.bss` object**, zero-filled initially, with COFF storage class 3 (local). The class has one mutable three-word array, a private default constructor, deleted copying and an escaping mutable-array accessor. This is real process-lifetime storage, not a missing provider. The native F87574/F87578/F8757C cells are likewise writable loader-zero storage, but their Original addresses are not the Source object's addresses.

The new parent TU cannot bind that current local object with a guessed `extern`, nor substitute Original virtual addresses or literal zeroes. The selected pointer interface leaves the canonical object, getter and every existing consumer unchanged. Its only raw-body context operations are:

| Place | Added operation and proof |
| --- | --- |
| Immediately after original `SUB ESP,0x3C` | `MOV [ESP+0x28],EDX`, saving the reset address at raw-kernel entry ESP−20. |
| Before original 00810235 | `MOV EDX,[ESP+0x34]`, after the distance gate and before the first reset comparison. |
| Before original 008102A3 | The same pointer reload, after the math calls and decay branch, before copying reset values. |

Complete CFG stack analysis reaches all 303 instructions with consistent joins. Direct stack accesses and the two indirect local vectors passed to length helpers leave words at entry ESP−20 and ESP−8 unused. The selected ESP−20 word is inside the original 0x3C local allocation; no original argument or local offset changes. Its active offset is +0x34 before EBP is pushed. Only the earlier region uses the context pointer. The first original EDX operation is a definition, `LEA EDX,[EAX+EAX*2]` at 00810351, so the added EDX values are dead before original EDX use. Calls may clobber EDX; that is why the reset-copy arm reloads it.

Replace only the six original absolute reset operands: FLD at 00810235/247/25A and MOVSS at 008102A3/2B3/2C0 become the same-width operations on `[EDX]`, `[EDX+4]`, `[EDX+8]`. All three added MOVs are flag- and FP-neutral. The saved item is an address, never a reset-value snapshot. Original compare short-circuiting and each later X/Y/Z re-read remain intact, including interleaved residual stores that may alias those cells. No reset value is read on the early distance-return path.

This deliberately changes body encoding: three added integer instructions, six changed addressing operands, normal constant/call relocations, and possibly changed branch encodings. A future build must map the whole Original body to the complete new kernel and account for all added instructions. It must not claim an unchanged 1,183-byte/303-instruction body or equate an ordinary public bridge with the Original entry.

An alternative could move the **sole same-class canonical object** from the getter-local static to an externally linked static data member, remove the old local definition, and make the existing getter return that one object. That can preserve the getter API, mutable-reference semantics, process lifetime and initial zeroing while permitting absolute relocations to the object's three cells. It must not add a second owner, a mirror array or separate mutable aliases. It would require coordinated header/owner edits and fresh proof of one 12-byte writable object, member offset/data address, initialization/guard behavior, all getter/consumer relocations and actual absolute parent operands. The old six-byte getter shape is a target to verify, not guaranteed before compilation; existing escaped references cannot be rebased in a live process. This broader storage-linkage change is **not selected** by the present borrowed-pointer design.

## Selected 2D facade and rejected shortcut

The public `length_2d_00414c60(const std::array<float,2>&)` bridge is physically **12 bytes / five instructions**: PUSH EBP; MOV EBP,ESP; MOV ECX,[EBP+8]; POP EBP; JMP. Its REL32 operand at +8 targets the actual private **66-byte/26-instruction** kernel. That target is COFF storage class 3, so declaring its anonymous-namespace name externally from a new TU would not create a valid link contract.

Root selected a new public naked `float __fastcall raw_length_2d_00414c60(const float*)` facade **inside `vector_helpers.cpp`**, with a declaration in its header. Its only operation should be JMP to that TU's existing `length_kernel`. The expected encoding is five bytes, `E9 <real REL32>`; no compiled facade exists yet. Future acceptance must prove the complete emitted facade and actual local-kernel relocation, while preserving the existing typed public bridge and all 66 kernel bytes/relocations.

A tail JMP adds no stack slot, changes no general register or EFLAGS, and touches no x87 value, CW, SW, TOP/tag state or MXCSR. The private kernel sees the original caller return address and ESP. This preserves its relative stack-scratch addresses and the caller's EBP previous-index value.

The existing cdecl bridge itself also has no FP/environment operation or EFLAGS-writing instruction. An adapter consisting of PUSH ECX; CALL that public bridge; **LEA** ESP,[ESP+4]; RET could forward the correct pointer and preserve the resulting flags. However, it makes the kernel stack eight bytes deeper than a direct call, changes its scratch-memory FDP provenance and exposes a different exception/call stack. ADD ESP,4 would additionally clobber EFLAGS. The cdecl adapter is therefore not the selected strict binding. Neither form proves Original-versus-current CRT policy or full FNSTENV byte equality; relocated instruction/data pointers already differ between binaries.

## Actual call-site contracts

Here K is the private raw kernel's entry ESP. All original surrounding instructions remain in their observed order.

| Original call site | Selected target and inputs | Stack/x87 result |
| --- | --- | --- |
| 00810273 | Actual raw 77-byte `native_unit_wake_length_0042b2f0`; ECX=live residual at wake+0x3D0. | Pre-call ESP K−72; zero x87 inputs, one ST0 result; ECX restored; no argument cleanup. |
| 00810280 | Genuine current intrinsic `_CIsqrt`; ST0=the float-spilled gate distance squared. | ESP K−72; one x87 input/result; no stack argument. Original result spill and double-quarter multiplication remain. |
| 00810416 | The same raw 3D entry; ECX=K−36, three actual local cells `(dx,+0,dz)`. | ESP K−76; zero inputs/one ST0 result; no Y displacement introduced. |
| 0081043F | Genuine current intrinsic `_CIatan2`; ST0=delta-X, ST1=delta-Z. | ESP K−76; two x87 inputs/one result. Keep float spills and pinned promoted-float pi constants. |
| 008104E2 | Selected same-TU 2D tail facade; ECX=K−60, two local planar-span cells. | ESP K−76; zero inputs/one ST0 result. Tail facade adds no stack/register/FP/flags effect. |
| 00810566 | Actual raw 43-byte `copy_native_unit_wake_sample_00810160`; ECX=current head record, already-pushed source=next-head record. | Pre-call ESP K−80; RET4 returns it to K−76; zero x87 net change; EAX=destination, ECX=source, EDX unchanged. Do **not** zero EDX to fill the unused C++ parameter spelling. |

The raw length and copy declarations/bodies are real compiled providers, not new approximations. The private 2D math body is already exact except its genuine cutoff/CRT relocations. The local raw bodies/facade preserve EBX/ESI/EDI/EBP; CRT preservation remains part of the external intrinsic ABI contract. The parent saves/restores those registers through its existing prologue/epilogue. Whole-parent local x87 depth remains at most four slots including the recovered leaves, excluding CRT interiors. The public adaptation does not promise Original incidental volatile-register or return-EFLAGS identity. Its ABI is the stated ordinary C++ interface.

The actual SDK x86 `ucrt.lib` from the sealed production link has unique physical `__CIsqrt` and `__CIatan2` import members. Both agree with the existing executable's `_CIsqrt`/`_CIatan2` IAT and FF25 thunks. Original CRT interior dispatch, mutable globals, diagnostics, exception handling and loaded-module selection remain external. The plan uses those genuine providers; it does not replace them with ordinary `sqrt`/`atan2`, a fabricated backend or a runtime-parity assertion.

## Observation order and remaining admission

Retain the original point/residual/head loads, float spills and planar squared-distance gate before external writes. Residual comparison must read the three actual reset cells conditionally in order; reset copying must re-read each one at its original site after math calls, with the original interleaved stores. Recompute position from the current pointer/residual after mutation. Cache only the reset **address**, not cells or position values across these boundaries. Preserve each later head re-read, heading/yaw MOVSS store, previous-heading write before length comparison, raw sample-copy overlap behavior, head update after copy, and the shared final tail. None of these are replaced with C++ whole-record assignments or helper-result summaries.

Parent constants remain the exact six original payloads: float 16; double 0.25; promoted-float half-pi/two-pi; double 55/50. The already implemented length providers retain their exact double 1e-10 cutoff. A future parent must pin actual read-only data and every relocation, not choose host-library pi values. The genuine reset words remain mutable and separate from these constants.

Ready now: the raw 3D and sample-copy providers, existing exact private 2D body/public bridge, genuine geometry owner/getter/accessor, native parent/preimage/order evidence and actual current CRT import providers. Missing Source work: the same-TU raw 2D facade, public bit-preserving geometry-borrowing adapter, complete raw parent with the three context MOVs/six operand changes, and their full compiled/artifact proof. Root must approve the Source scope before those edits. Existing typed APIs remain intact. No application caller, entity constructor graph, process-owner consumer, loaded backend, runtime ABI/FP behavior, startup or gameplay is admitted here.

The compact [report](../reports/cc12_native_wake_append_binding_design.json) references both immutable sealed archives and retains the selected physical objects, sources, import members, complete parent instruction/stack analysis and all eleven current source/object pin comparisons. Its own evidence family is `local/cc12_native_wake_append_binding_design_evidence`. No large application/constructor graph was expanded, and this design adds no Original-function count.
