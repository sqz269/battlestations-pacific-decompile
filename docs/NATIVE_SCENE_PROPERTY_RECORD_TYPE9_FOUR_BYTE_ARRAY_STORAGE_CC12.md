# Qualified native type-9 four-byte-array storage

Worker implementation for
`cc12_scene_property_record_type9_four_byte_array_storage`. The whole
`[008EF360,008EF3D4)` constructor contains 116 bytes and 36 instructions,
including both branches. Only its two CALL operands bind current genuine
providers; all other 108 bytes match the installed Original.

Root Source credit is **1 for the qualified current-provider raw-storage
domain**, following independent fresh validation and the current Main Win32
build. The provisional storage name and evidence are saved in the configured
Ghidra project, and the affected export is refreshed.
This is a qualified successful raw-storage interface. Private Original CRT/EH,
class ownership, the full clone routine, startup and gameplay remain unproved.
The name describes observed storage behavior and is a hypothesis.

## Native body and physical interface

The installed PE SHA256 is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
The complete constructor SHA256 is
`dce8fb8621af17a5791a0e80cca91982def8914d2f3a9e516ee1a2344f05b5e3`.
Before and after the accepted process, the batch verified the configured
`C:/Users/sqz269/bsp.gpr`, actual `/battlestationspacific.exe`, complete
installed/live 116 bytes and all 36 live instruction starts. No Ghidra write
or listing repair occurred.

`construct_native_scene_property_record_type9_four_byte_array_storage_008ef360`
is an MSVC Win32 naked padded `void* __fastcall` entry. ECX contains actual
fresh/unowned writable 56-byte root storage; EDX is unused. Entry stack `T+4`,
`T+8`, `T+C` contains element count, actual data pointer and the full flag DWORD.
Both exits return root in EAX and execute `RET0C`. ESI/EDI are saved/restored.

The qualified count is `1..3FFFFFFF`, with an actual stable readable,
nonwrapping `4*count`-byte span. Root, input and active frame are disjoint.
DF must be clear for the current CRT. Elements are opaque four-byte groups;
there is no float, x87, numerical-value or NUL-terminated-string contract.

Two native `ADD EDI,EDI` instructions compute the byte count. The first
precedes the phase/tag stores; the second follows those stores. The subsequent
`CMP byte [T+C],0` chooses borrowing versus copying using only the low flag
byte. The implementation preserves the native DWORD additions and order,
without introducing a count guard or signed C++ multiplication.

| Root bytes | Effect |
| --- | --- |
| `00..07` | Phase token `00CE89D4`, tag 9 |
| `08..17` | Preserved |
| `18..1F` | Zero |
| `20..23` | Actual retained or newly allocated pointer |
| `24..27` | Computed byte count `4*element_count` |
| `28..2B` | Preserved |
| `2C` | Byte 1 in both branches |
| `2D..33` | Preserved, including DWORD `+30` |
| `34..37` | Zero |

There are 29 written and 27 preserved bytes. Byte `+2C` does not distinguish
ownership, and the phase token does not authorize virtual/destructor dispatch.

A zero low flag byte retains the exact input pointer without allocation or
dereferencing input bytes. Input remains caller-managed while used; it must
not be freed as a new child. ECX returns root, EDX returns the actual input,
and all six CMP arithmetic flags satisfy `EFLAGS & 8D5 = 44`, including AF zero.

A nonzero low byte pushes computed byte count to an owned, real private CDECL
size adapter. That adapter calls unchanged canonical allocation with
`{object, bytes, bytes}` and receives no native reconstruction credit. Source
reads data after allocation, stores the actual child at `+20` before memcpy,
and calls the genuine VCRUNTIME copying provider. Observe the complete child
while live, then free it once through canonical free before freeing root.
Final `ADD ESP,10h` defines all six arithmetic flags from actual entry T:
`(T-24)+16=T-8`. Provider ECX/EDX residuals are recorded without assertions.
EBX/EBP follow normal current-provider ABI; no blanket ES/FPU/MXCSR or provider
DF preservation, exception-safety or `noexcept` claim is made.

## Fresh differential evidence

The new `local/t9a` family built exactly three fresh translation units:
the new whole constructor including its own adapter, unchanged
`singleton_lifetime.cpp`, and a new probe. No BSP archive, old object, prior
recipe execution or accepted process replay was used. The new probe/helpers
were derived from prior text with explicit provenance. Compile flags included
MSVC x86 `/O2 /MD /W4 /WX /fp:strict /permissive- /EHsc /Gy /GL-`, plus
`/Oi-` for Source. The executable embeds an `asInvoker` manifest.

Only Original operand bytes `[51,55)` and `[66,70)` were rebound to the same
real adapter and memcpy thunk used by Source. The other 108 bytes stayed
literal in the whole Source object, linked Source and bound Original RX image.
No external Type-8 private symbol, fabricated NativeMpkg receiver or provider
mock was introduced.

| Call | Elements | Bytes | Full flag word | Storage |
| --- | ---: | ---: | --- | --- |
| Source | 2 | 8 | `A638F200` | Retained input |
| Bound Original | 3 | 12 | `B4926700` | Retained input |
| Source | 3 | 12 | `69C3F101` | Real copied child |
| Bound Original | 2 | 8 | `8725D480` | Real copied child |

The single accepted process passed all four calls with four actual 56-byte
roots and two actual copied children: six allocations and six frees, children
first. After each call it checked all 56 bytes of every root, all 96 guarded
input bytes, every live child's complete byte span, and every other capture.
Saved artifacts include four 140-byte captures, four roots, two guarded
inputs, both complete children and the bound Original image.

Raw capture distinguishes R before arguments (`0012905C`), Q before CALL
(`00129050`) and actual entry T (`0012904C`). It captures the three actual
dead argument slots before PUSHFD, then verifies ESP, EAX, saved registers,
guards, full argument DWORDs and all defined flags. Borrowed calls produced
mask `044`; copied calls produced `004` from `00129034+10h=00129044`. Offline
decoding independently checked all captures and artifact contents without
another target process.

All **33 complete code spans** passed COFF/map/relocation and runtime
bookends. These comprise every retained function emitted by the three fresh
TUs (2 Source, 9 canonical, 18 probe), plus the complete 14-byte cookie helper,
memcpy import thunk, cold throw thunk and map-owned chkstk extent. The cookie
gate includes its failure tail JMP; the external failure body is unexpanded.
Emitted cold exception-support bodies are byte-gated without EH execution or
admission. Mechanical checks cover complete bodies; manual assembly review
covers constructor, adapters, callers and critical main/verifier/flag paths.

Before any domain allocation/target entry and after all frees, the probe
verified all code spans and actual I386 UCRT/VCRUNTIME providers: IAT target,
export RVA/address, MEM_IMAGE allocation base, mapped NT path equal to opened
file NT path, file identity, full SHA256 and ASLR-correct 32-byte export prefix.
UCRT SHA256 is `60c5a497b52de80a3a0677564270dbea7e486086637debd567b4dffb28584c1b`;
VCRUNTIME SHA256 is `2fa6efc053203460a23d3a25158f227d895d2dadc63acc1a372da97c3a4281c3`.

## Preservation, failures and remaining integration

The exact immutable union contains 18,512 prior artifacts: the tag-9 audit
(252 files plus 5,765 prior pins), Root Type-8 `local/t8p1` (381 plus 17,427)
and Root reference-55 `local/ref55p3` (101 plus 18,016), deduplicated without
changing any historical association. Actual Source/tools/headers/libraries
remain strict pins. Ten unconsumed dispatch/CMake/context inputs were frozen
as copies. Consumed headers total 184 and libraries 7.

The new family sealed **420 artifacts plus its manifest**, with an exact
recursive inventory including failures and logs, no pycache files, and only
root `seal.json` excluded from its own hash list. Manifest SHA256:
`7d60a4b59f36e2c32f92acb106f5da0b771f6c592b2932dff4c96bbaa06b2880`.
Every prepare/build/static/launch/post stage ran once and passed.

Retained pre-target failures are limited to a rejected metadata CLI spelling,
two guessed directory reads, and three offline review/generation assertions:
an overbroad obsolete-name check, a negative substring check matching its
intended expression, and a manually entered retained-function count of 30
instead of 29. New helper files corrected each assertion; failed helpers and
receipts remain sealed. No compiler, static gate or target process failed.

The private baseline Win32 Release repository build and all three existing
CTest checks passed: `reconstructed_math`, `native_math_differential` and
`tool_tests`. Seed verification used a private output directory. Receipts are
separate in `local/t9_build`. The new Source TU is compiled by the fresh
three-TU fixture; Main CMake registration and the combined Main build remain
Root-owned.

A later public-report draft failed syntax review because its proposed key
contained invisible characters. The unexecuted draft and a separate receipt
were retained outside the sealed Source family; a fresh report helper removes
those characters. It did not change or rerun any sealed fixture stage.
The successful full-build receipt also initially counted repeated
multi-configuration CTest clauses instead of three unique test names; a
report attempt consequently found that receipt missing. Fresh receipt/report
helpers verified the existing successful log and three distinct registered
tests. Both failures were retained without rerunning compilation or tests.

The earlier audit supplies the selected clone caller. Helper `008EF7F0`,
private native allocation/copy routines, clone EH/null/default handling,
phase/class/native ownership and World/game behavior remain unexpanded and
unadmitted. No native Source credit is claimed for the new adapter or fixture.
No zero-size, overflowing, aliased, failed-allocation, reentrant or unwind
execution was attempted. Root must independently validate a new family and
complete shared registration/publication before admitting Source credit 1.

## Root complete-helper qualification

Root independently generated `local/t9p1`: three fresh TUs, 184 consumed
headers and seven actual libraries. Its exact 481-file seal is
`1086ac44ab534b26118b1bc83a577be8a1a1109a11e1db295f574a9e68d855db`.
The sealed summary records Source 0 at the point before this external decision;
its immutable historical status is preserved. All 23,517 prior artifacts and
all consumed inputs remained unchanged. No old objects, stages or accepted
processes were replayed.

Independent raw COFF, map, PE and packed-byte readers proved 31 logical symbols,
29 physical TU bodies, 286 symbol relocation checks, 280 physical operands and
six alias rechecks. All 47 complete packed spans include every retained and
mapped TU definition, all actual normal and std import thunks, cookie14,
stack43, and cold sized-delete16 to unsized-delete5 to free-IAT6. Cookie's named
GS failure destination remains unexpanded and unadmitted; cold code coverage
does not admit cold execution. Root reviewed all 6,195 bytes and 1,570
instructions of the linked Main, including target selection, comparison and
failure paths, code/provider gates and cleanup.

The sole process made exactly Source-retained, Original-retained, Source-copy
and Original-copy calls. Independent decoding checked all four Capture140
records and complete root56 arrays, both complete guarded input48 arrays, and
the live copied children16 and4. Six real allocations/frees completed with both
children freed before all four roots. COUNT/DATA/FLAG slots, captured R/Q/T,
RET12, full EAX, nonvolatiles, DF0, canaries and defined flags passed. ES is
recorded only, and copied ECX/EDX residuals remain unasserted. Four recorded CRT
bindings were decoded against frozen physical DLLs; this is recorded binding
evidence, not a new live-provider attestation.

The current Main build at `882064808e466755abd99639a3c1d0fe3577a10f` passed
Win32 compilation and all three existing checks with its 4,036 actual Source,
header and CMake inputs unchanged during the build. That context is a historical
build snapshot. Startup, gameplay and binary drop-in ABI remain unvalidated.
