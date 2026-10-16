# Whole raw traffic pointer-range erase

`erase_native_traffic_pointer_range_00488ff0` reconstructs the complete original
`00488FF0..00489049` (90 bytes, 37 instructions). It uses the actual receiver,
five actual caller argument cells, and current SDK services. The retained
Ghidra/library name is `STL_inst_00488ff0`; the Source name describes behavior,
not a recovered game symbol or class. This packet does not reconstruct resize,
insertion, traffic placement, or the world-layout parent.

## Physical contract and order

Original entry has ECX = raw receiver and five stacked DWORDs, in order:
output address, first owner word, first position, last owner word, last position.
It returns the actual output address in EAX and executes `RET 14h`. There is no
incoming EDX parameter. Source uses a fastcall entry with a Source-only unused
EDX slot to bind ECX and those five physical stack arguments. Owner words are
opaque 32-bit values; calling code supplies its actual header address. No
semantic class or layout beyond the reached raw DWORD cells is asserted.

The instruction schedule is retained:

1. Capture the first owner in EBP and receiver in EDI. Diagnose a zero first
   owner or unequal owner words. The real diagnostic may return. Read first
   and last from their actual stack cells only after that diagnostic.
2. Equal positions skip the receiver+8 read and store. Otherwise capture its
   current DWORD end, subtract last with DWORD wrap, and perform signed `SAR 2`.
   `TEST` flags survive both LEAs and the EBX save. The wrapped byte span is
   four times that signed count; the captured new end is first plus that span.
3. A positive signed count uses real `memmove_s(first, span, last, span)`.
   Ignore its result, pop the four physical arguments, and publish the captured
   new end to receiver+8. A zero or negative count still publishes that end.
   This includes distinct endpoints with zero count; no empty-range repair is
   inserted. Actual invalid memory continues to fault when reached.
4. Load the current output pointer from its actual argument cell only after
   copy and publication. Store output+4 = captured first before output+0 =
   captured owner. EAX remains that output pointer. All saves/restores and
   cleanup instructions are retained; no C++ pointer subtraction or signed
   overflow changes these machine operations.

There is no introduced snapshot, null repair, mutable global, provider callback,
exception translation, `noexcept`, or `noreturn` promise. Any reached argument,
header, output, or copy cells must have backing storage, including aliases.

## Real providers and callers

At `00489003` the original calls `00BF6713`. Source binds the already registered
`invoke_native_invalid_parameter_00bf6713`, which invokes the actual selected
SDK `_invalid_parameter_noinfo`. Its qualified current-CRT handler policy is
established separately in `CC12_NATIVE_INVALID_PARAMETER_NOINFO_CALL_SOURCE.md`; original
private CRT `00BF66EF`, errno, handler, Watson/debugger and exception identity
are not substituted or claimed here.

At `0048902F` the original calls library `_memmove_s` at `00BF67A7`. Source's
ordinary cdecl bridge calls actual `::memmove_s` from the installed SDK. The
selected `corecrt_memcpy_s.h` implements zero-count return, null-source or
destination EINVAL, insufficient-capacity ERANGE, and real `memmove` otherwise.
The emitted bridge calls real current `_errno`, `_invalid_parameter_noinfo`,
and `memmove`; there is no simulated copy. Original CRT internals and error
identity remain outside this service boundary. The owning body ignores the
service return and retains its exact subsequent publication schedule.

Both current live callers were inspected in full and captured against installed
PE bytes. `00489AF0` (57 bytes) invokes the helper at `00489B1D`, with two copies
of its ESI header, captured last/end, reread first/start, and local output cells.
`00495AE0` (179 bytes) invokes it at `00495B84`; its shrink branch supplies the
captured old end, actual header owner words, computed first, and local output.
Diagnostics and rereads in those callers remain their own work. In particular,
resize growth passes the address of its actual caller fill DWORD to `00493120`;
it is not a zero-fill API. Neither caller is claimed complete Source here.

## Verification and limits

`reports/goal15p_traffic_pointer_erase.json` records the complete 37-row Native /
emitted operand map. Fresh live bytes match the installed original executable
for all 90 owned bytes and both complete callers (326 bytes total). The strict
and CMake-registered erase entries are each 90 bytes. All 82 fixed bytes match;
only the two genuine provider CALL rel32 operands (eight bytes) differ. All
branches, memory operands, arithmetic, register saves, and `RET 14h` match.

The entire executable COFF contribution is accounted for, including SDK code:
strict object 274 bytes = 90 + 92-byte bridge + 92-byte SDK COMDAT; registered
object 278 bytes = 90 + 94 + 94. The registered bridge retains a frame pointer;
the strict bridge omits it. Each bridge matches its own emitted SDK COMDAT.
Every one of their five relocations is mapped to the actual current CRT API.
These extra bytes are not presented as original `00BF67A7` identity.

MSVC 19.51.36244 x86 /MD /O2 /Gy /W4 /WX /fp:strict /EHsc compilation passed.
The actual `scripts/build.ps1` registered Win32 build passed its two available
CTest checks (`reconstructed_math`, `tool_tests`); this worktree has no enabled
native seed-reference test. An ignored focused executable links only the real
registered `bsp_core.lib`. A copy into the actual outgoing output-argument cell
changes the eventual output destination, leaving the original destination
untouched; it verifies late reload, EAX, stack cleanup and end publication.
The same fixture also checks distinct positions with negative SAR count and
unchanged copy storage while publishing first-4. Both cases pass for public
Source and for all 90 captured Native bytes with only their two true CALL
bindings normalized to genuine current providers. No handler mutation is
simulated; diagnostic-return alias behavior is established by full instruction
correspondence, not a runtime handler experiment.

This is whole Source reconstruction with static/build/focused-fixture evidence.
It does not establish a drop-in original binary replacement, the complete game
ABI, original CRT/EH/errno/FP-environment identity, or reached game behavior.
The owned body has no floating-point instruction. No game execution is claimed.

## Remaining closure

Fresh lookup of `00493120` (436 bytes) shows no Source function record or Source
address mention. Its seven direct children are:

| Entry | Whole Native size | Current readiness |
| --- | ---: | --- |
| `0047E970` allocator | 85 | Tagged, no exact Source; real current allocation service exists, legacy exception/throw path requires explicit policy or closure. |
| `00486800` fill | 30 | Tagged, no Source; leaf candidate. |
| `00486820` copy backward | 47 | Tagged, no Source; current memmove_s boundary available. |
| `0048D660` copy | 48 | Tagged, no Source; current memmove_s boundary available. |
| `0048E4A0` fill N | 46 | Tagged, no Source; leaf candidate. |
| `00490240` length failure | 105 | Tagged, no exact Source; legacy string/logic-error/throw path remains explicit despite existing logic-error Source. |
| `00BF65AC` free | thunk | Genuine `singleton_lifetime_free` current-CRT service exists; original private CRT identity separate. |

The four copy/fill entries (171 whole bytes total) are the smallest useful next
candidate packet: their direct services are available, but their exact physical
ABI, aliases and caller cells still require full review before Source. No new
claim is made for them here. `00495AE0` needs whole insertion `00493120` and its
own caller-cell schedule in addition to this erase. Whole `0049E560`, layout
`004C71C0`, parent `004D5BD0`, and authored mutable-global producers remain gates.

## Reproduction and annotation handoff

Ignored self-contained artifacts are in `local/goal15p_traffic_pointer_erase/`:
`build_probe.ps1`, `audit.py`, `readiness.py`, full live/caller/provider captures,
`byte_audit.json`, the archive-only fixture, linker map, hashes and receipt.
Run `./scripts/build.ps1`, then `build_probe.ps1 -CompileOnly`, `python audit.py`,
then `build_probe.ps1` using those paths from the repository root. The audit
verifies configured `C:/Users/sqz269/bsp.gpr`, active program
`/battlestationspacific.exe`, installed PE, complete COFF extents and relocations.

Original prototype/name/comments are preserved in the ignored captures. This
worker makes no Ghidra mutation. Primary will annotate under its write lock,
retain correct library names, save and refresh affected exports after release.
