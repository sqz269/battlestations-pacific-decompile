# Native post-effect draw-record constructor (BN)

Address: `00B51BD0..00B51C10`, 65 bytes, 24 listed instructions, no listing gaps.

The complete wrapper is reconstructed in `native_post_effect_draw_record.cpp` through
the existing raw `initialize_native_render_entry_00b51a20`. It preserves the caller's
40-byte allocation and all original argument staging, including the x87 loads/spills.
It introduces no record layout, allocation, owner, reference count, or service callbacks.

## Original ABI and instruction order

ECX is the actual raw record. The six DWORD stack arguments are leading(float), section,
geometry, model, camera, visibility(float). EAX returns the captured original ECX and
`B51C0E RET18h` removes those arguments. ESI is saved/restored; EAX/ECX/EDX and x87
status are ordinary native working state. No native FH3/SEH frame or cleanup state exists.

| Native instructions | Effect |
|---|---|
| B51BD0 | FLDZ creates exact positive zero for the depth override |
| B51BD2 / B51BD6 | Capture current stacked camera and geometry pointer words |
| B51BDA..B51BE3 | Save ESI, push flags555h, reserve visibility/depth, spill zero depth |
| B51BE7 | Capture actual receiver in ESI |
| B51BE9 / B51BED / B51BF1 | x87-load visibility, read model word, x87-spill visibility |
| B51BF4..B51BFF | Push camera; read section; x87-load leading; push model/geometry/section |
| B51C00..B51C03 | Reserve leading word, restore ECX receiver, x87-spill leading |
| B51C06 | Call B51A20 with eight DWORD arguments; callee RET20h |
| B51C0B..B51C10 | Return the same allocation, restore ESI, RET18h |

The source adds the existing `NativeTracelineRenderAccess*` in EDX and saves it in one
extra stack word outside the original frame. Input stack argument offsets increase by4;
outgoing argument slots keep their offsets. Access is reloaded after all original stores, then discarded after the
callee returns. This is a new source interface, not a drop-in original binary ABI.

Using a normal C++ forwarding call would omit the native x87 float conversions and can
change exceptional float encodings/status. The wrapper retains their exact order and
does not substitute MOVSS, memcpy, zero initialization, or a typed record constructor.

## Existing initializer and ownership boundary

`B51A20..B51AAA` stores leading+00, visibility+18, section+04, geometry+08, model+0C,
camera+10, flags+1C in that native order. It compares depth with the current D7A218 cell.
The positive override branch writes+14 directly; otherwise it gets the current camera view,
dispatches the model's current table+48, transforms the returned point and writes x87
`section+34 + transformed Z` to+14. The wrapper supplies positive-zero depth and flags555h.
Both key DWORDs+20/+24 remain untouched. There is no hidden ownership write at+04:
that word is the borrowed section pointer, not a reference count.

The existing concrete `native_traceline_render` initializer and camera/model/math providers
are reused. `NativeTracelineRenderAccess` must bind the actual current cells and services;
its sphere dispatch directly supports B6E8C0 and retains the existing service boundary for
other current model profiles. This packet adds no fallback implementation for those profiles
and does not establish complete parent-constructor/service composition. Live section/model/
camera storage and the dependency's nonthrowing service domain remain prerequisites.

Both native callers allocate28h through BF681B and remove4 bytes themselves:

| Parent | Allocation/call/publication | Arguments |
|---|---|---|
| B4E470,20h D61EC0 | B4E7B9/B4E7BB; call B4E7EC; publish+18 B4E7F5 | leading0, EDI section, EBP geometry, current+10 model, current+0C camera, visibility1 |
| B4E840,24h D61EC8 | B4EB6F/B4EB71; call B4EBA2; publish+1C B4EBAB | same argument construction |

Both allocator-null branches publish zero. Successful wrappers return the same allocation;
the parents later free the raw record through CRT. The record only borrows its children;
their owner lifetimes remain with the parent/mesh graph. This function supplies no unwind
cleanup, automatic deletion, retain/release, late key initialization, or recovered parent EH.

## Verification

`scripts/build.ps1` passed for MSVC Win32 Release with the one configured existing CTest,
`reconstructed_math` (1/1). The first attempt compiled the wrapper/core but ended with
MSB8066 CMake regeneration failures for the game and ship-motion probe: CMake reported
that it could not restore `CMakeFiles/generate.stamp`'s timestamp, `Access is denied`.
Its log is retained; the unchanged-source rerun completed successfully. No source fix or
unrelated registry/linker change was made in response.

The generated wrapper is71 bytes with one relocation to the existing B51A20. It equals the
65 native bytes after only six input-displacement adjustments, access save/reload/discard,
and normalization of that call relocation. All native x87 bytes, outgoing argument stores,
return identity and RET18h are unchanged. The call audit checked7 direct edges with no
failures; the model+48 edge is explicitly indirect and not counted as verified by that tool.

The report pins the original PE/live span, source dependencies, exact caller/callee sites,
build artifacts and generated object evidence. Ghidra target was verified through the read-only BSP CLI;
no saved-analysis, name ledger or Ghidra mutation was made. No new native differential fixture, original binary ABI, hardware
fault unwind, GPU output, parent construction closure, or runnable-game validation is claimed.
