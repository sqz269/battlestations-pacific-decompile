# Actual Text glyph-quad storage and child schedule

Baseline is sealed `c8c38e7d768e9ad298161d70d7d25f028434f05b`. This packet adds only `native_gui_text_glyph_quad_storage.hpp/.cpp`, this document and [its report](../reports/cc16_native_gui_text_glyph_quad_storage.json). No old Source, CMake, ledger, Ghidra analysis, application, owner or global view is changed. The existing ordinary `oct18_tail_c::glyph_quad_00ab98f0` remains intact. This live-home storage interface supplies **zero additional Original, vendor or duplicate-body credit**. Genuine entered child/provider bindings and whole frontend activation remain gated.

Whole Native `00AB98F0..00AB9FC2` is 1747 physical bytes, 485 instructions and 37 blocks, with 19 direct ordinary calls and four captured-target virtual calls. Its bad `undefined(void)` prototype was not used as an ABI. Assembly, exact EH bytes and the configured `bsp.gpr`/`battlestationspacific.exe` program were inspected read-only with Ghidra autostart disabled. No Native code was executed.

## Stable shared call contract

`write_native_gui_text_glyph_quad_storage_00ab98f0` takes captured actual Text, `volatile uint32_t (&homes)[10]`, its own borrowed `NativeGuiTextGlyphQuadStorageContext&` and a fresh retained acquired record. This agrees with the single-line supplier's required `write_glyph_quad_00ab98f0(actualText, homes)` binding; Root supplies the concrete adaptation without a shared context/owner edit.

| Zero-based home | Native input | Native use/mutation |
| --- | --- | --- |
| 0 | captured glyph pointer | overwritten by signed bearing, unsigned width, raw-right FLOAT spill, then `firstVertex+1`; glyph remains separately captured |
| 1 | actual stable float3 pointer | read for initial Y/X; reread for X only in the late long-reference positioning arm |
| 2 | actual vertex-stream pointer | captured once, later overwritten by `firstVertex+2`; stream remains separately captured |
| 3 | actual current index-write pointer | captured after all four vertices/colors, before six low16 index stores |
| 4,5 | same float3 pointer | never read by the full callee; not normalized or rewritten |
| 6 | quad index DWORD | one late load, then two wrapping integer doublings |
| 7 | unsigned font-height DWORD | lowWORD zero-extended into the home, then bottom FLOAT spill, repeated individual `FF` byte stores, raw allocation, then child pointer |
| 8 | first-vertex DWORD | captured once before the first output-position store |
| 9 | unsigned UTF16-unit DWORD | lowWORD reread for membership comparisons; full current DWORD captured later before `AB81C0` |

The same input float3 is aliased at homes 1/4/5. The caller zeroes Y/Z once before its emission loop, writes X from its separate pen before each call, and rebuilds all ten homes for each normal invocation. This callee does not copy a position back to that pen or reset input Y/Z. It writes output vertex Z, which is a different backing. After return the caller advances from its separately captured glyph `+12` and CURRENT Text `+1D8`; it must not recover glyph/stream/height from mutated homes 0/2/7.

The caller may reuse the same homes only after whole normal return. A completed record's home pointer is historical identity, not permission to follow new values. Entered failure retains caller homes, float3, actual mapped streams and all provider/acquisition frames, with no retry, pen advance, inferred unlock or replay.

## Ordinary prefix, exact widths and x87 epochs

Native frame base `F = entryESP-58h`. Homes begin at `F5C`; the private scratch array models `F14..F48` as fourteen actual DWORD words, initialized only at their native writers. The same `F24/F28` pair becomes geometry scratch, shader header, wide header, pivot and offsets. No `NativeString`, `FontGlyphData`, `GuiTextLifetime` or `GuiWidgetOwner` is cast into this storage.

The prefix captures glyph in its Native EDX role, reads signed WORD bearing at glyph `+10` and unsigned WORD width at `+14`, and reads actual Text `+1D8`. The x87 `FST` of raw left keeps the unrounded stack value for right-edge arithmetic. Raw right is spilled into home 0 before division; unsigned low16 height and raw bottom are spilled through home 7. The explicit `+1` then `-1`, the 960/720 double loads and CURRENT process `E12FD4` vertical-scale load remain in the original arithmetic order. The first output-position Z is stored before bottom subtraction/spill. Left/top are later FLD/FSTP copied into the actual first output position.

All 78 lexical x87 instructions match the whole Native mnemonic order: prefix 40, shadow 2, offset-mode position 13, `reference+EC == 1` position 9, `reference+EC != 1` position 14. The field's descriptive meaning remains provisional; the literal native test is preserved. The final pivot block uses only MOVSS. No C++ floating arithmetic, floating return helper, algebraic cancellation, FP reset, CW normalization or status save/restore is added. Thus each group uses the then-current ambient x87 CW/status, including any genuine provider effects between groups. Original status/fault-PC and binary ABI identity still require execution/compiler proof; mnemonic equality alone does not establish them.

The `reference+EC != 1` arm preserves the mandatory `F40` FLOAT spill/reload before subtracting `D7A358`, rather than keeping a single extended expression. The other two branches preserve their own `F24/F28` offset spills and `F30` target writes. Child position is passed to the whole required raw `AA8240` after current reference position, mode/reference-EC/offset reads, not to a typed owner projection.

TL/TR/BR/BL output uses CURRENT stream `+8` buffer, `+C` stride, `+10` position offset, `+28` UV offset and signed `+34` packed-color offset at their actual read phases. Negative packed offset uses the four CURRENT attribute offsets at `+38/+3C/+40/+44`; otherwise the packed DWORD comes from the live home 7. Float-color writes use the separately captured `D7A24C` bit pattern. Raw integer DWORD moves reproduce Native MOVSS bit transfers without additional FP arithmetic/status effects. Individual repeated home-7 `FF` byte stores retain their order around position/UV/color writes. No format, glyph-size, font, index-range, default mesh or overflow policy is invented.

Six WORD indices are written in Native order: `[1]=q4+1`, `[0]=q4`, `[3]=q4`, `[2]=q4+2`, `[4]=q4+2`, `[5]=q4+3`, with DWORD wrap and low16 stores. Membership gates read CURRENT raw Text `+1B0`, `+1AC`, DWORD `+DC`, DWORD `+1A4` and UTF16 pointer `+1A8`. Null pointer takes the original genuine `F8BE50` empty-cell arm. There is no replacement default string. Matching zeroes the four output UVs; second/third indices are reloaded from live homes 0/2 only after the corresponding stride read.

## Required child and provider schedule

The context borrows the one genuine Text pool and empty cell, actual raw string-pool context, authenticated RO constant/literal refs, and the SAME `GameNativeGuiTextState::vertical_scale_00e12fd4()` backing. That is distinct from widget `E12FC4`. It creates no process, Text/Model, renderer, font, material, global cell, semantic child owner or authority bank.

The child path requires genuine whole raw services, not a typed glyph-child continuation or a pending/domain status reported as completion. Required methods have no implementation/default in this packet. Their nonmoving nested acquisition backing and real Model/renderer/font/material/stream/child providers must already be prepared before entry and before reached callbacks. The methods reuse existing whole bodies or identify an actually unclosed helper such as raw `005DAD40`; they cannot substitute a no-op or fabricated model/font/glyph. These concrete bindings remain a Source/activation gate.

The full sequence is:

1. Allocate through real pool `AB78A0`, store allocation in live home 7 and arm state 0. Conditional whole raw `AB9650` returns captured child; leave state 0, write child into home 7 and pass **that home's address** to actual `005DAD40(parent+194)`. Later operations retain the captured child even if publication/home 7 changes during append.
2. Reread CURRENT parent `+1B0`, capture its actual node `+4C`, current profile and slot `+10` target, and invoke that captured real target with flags `26h`, parent 0. Set the returned node on captured child via `AA6720`, ensure whole draw sections `AB8530`, capture current child `+34` target and call value 1.
3. Reread current listener `+1AC`, call `AA6BC0`, store child byte `+78=1`, reread reference `+1B0` and pass its actual `+1C8` name header to whole raw `AB8C30`. Prepare shadow through exact FLD/FSTP/MOVSS epochs and call whole `AB6C30`.
4. Construct the actual shader temporary at `F24/F28` using existing genuine raw `41E870` and authenticated `CEFD78` literal. Arm state 1, call whole raw `AB8E70`, capture buffer, disarm, then capture length+1 before fresh actual `419CC0/BD1510` return. No typed `std::string` projection appears.
5. Capture CURRENT full home 9, call whole actual `AB81C0` over the SAME `F24/F28` header, arm state 2, call whole raw `ABA8D0`, then freshly capture/call child current `+50` with actual child `+50` color storage. Capture wide buffer, disarm, then capture wrapping `2*length+2` before fresh actual pool return.
6. Attach via whole `AAA5A0`, write CURRENT pivot scalar to the two reused words, freshly capture/call child current `+30`, reread current reference and call whole raw `AA6750` into `F18/F1C/F20`. Read current parent mode, reference `+EC`, offsets or late input X, perform the exact selected x87 position branch, then call whole raw `AA8240`.

Captured profile/target words are DATA identity selectors. Required target resolution is pure/stable metadata; it cannot reenter, select a newer target or cast those words to code. All ordinary helper entry/return/failure boundaries are recorded before callbacks. Existing raw `41E870`, `419CC0`, `BD1510`, `41DD20` and `436430` are composed directly over the SAME genuine raw pool/manager cells. The shared whole `ABAED0` scope and wrapped/layout builders are untouched.

## Cleanup and proof limits

Native handler `CB80C8` names FuncInfo `DEEEFC`, map `DEEEE4`. All three states have parent -1: state 0 invokes `AB76F0` on CURRENT live home 7 (`[entryESP+20h]`), state 1 destroys shader header at `F24`, state 2 destroys wide header at that SAME address. Source uses current home 7 for state-0 cleanup, not an allocation snapshot, and calls the existing genuine raw header destructors for states 1/2. Resize/code-unit construction failure and normal-return getter failure gain no new local cleanup. Child publication and earlier vertex/index effects are not rolled back.

The acquired record is diagnostic/custody metadata, not another owner. It performs no destructor-driven native cleanup, no retry and no payload rediscovery. A secondary cleanup failure remains qualified; no fabricated recovery traversal follows. Genuine child lifetime, same-domain helper bindings, actual raw caller ownership and all frontend/Frame/Page/renderer gates remain external requirements.

Validation is static only: installed PE/body/data hashes, 485-instruction bounded listing, 19 direct/four virtual call receipts, three-state cleanup map, 78-instruction x87 mnemonic sequence, native home/operand read-write phases and unchanged dependency pins. No compiler, build, tests, probe, Native/game process, process control, Ghidra mutation, ABI/runtime comparison or activation was performed. Root owns shared integration and compilation. Original private-frame aliases, physical register/RET28 ABI, hardware faults, FH3/SEH/secondary transport, concrete provider implementation and gameplay remain unproved.

## Private backing qualification correction

The original recovery was sealed at `6d0dc92b8c5cac92fa658d4b96edf00cd5c58239`. Its reported pins and independently computed immutable Git pins remain preserved in the report. This follow-up corrects the private scratch representation only; it adds no body or activation credit.

The original `locals[14]` declaration made every backing object volatile, then stripped that qualifier before genuine raw string/wide helpers whose implementations use ordinary byte/memcpy access. That qualification change had no established language or MSVC guarantee. The backing now consists of ordinary DWORD objects. Reached native reads and writes retain explicit volatile views: all four existing asm pointer views remain unchanged, both normal-return data/length reads add an explicit volatile view, and the three color/pivot/output pointer arguments add explicit volatile views. The eight private-header `const_cast` expressions are removed. Genuine header construction, content, normal return and cleanup still operate on the SAME actual F24/F28 objects, without copied headers.

Mechanical comparison against the sealed Source accepts only this declaration/comment change and these pointer/view substitutions. All 22 asm block bodies are exactly unchanged (joined text SHA-256 `fc5975b3c80f3db93f7feb2b04e44d10272d738c6848c82e0499e7b6307c45ab`); the 78 x87 forms retain groups 40/2/13/9/14. The 19 required call-method names, 22 literal boundary markers, helper calls, external ten volatile homes, mutable home 7, local offsets, branch expressions, spill/reload instructions, EH states and call ordering are unchanged. The array still contains fourteen DWORDs at the same member position; adding or removing cv-qualification does not introduce a new backing object or change element size/alignment.

The corrected receipt uses exact immutable Git LF content hashes for the two Source files and this document. This static representation check does not establish compiled instructions, ABI, hardware fault or FP-status equality. No build, tests, probe, native/game process, process control, Ghidra write, application binding or activation was performed; the existing genuine provider and lifetime gates remain open.
