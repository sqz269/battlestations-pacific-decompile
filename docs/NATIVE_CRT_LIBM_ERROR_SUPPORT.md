# Native CRT libm error support

`native_crt_libm_error_support_00c0f0e4` reconstructs complete
`___libm_error_support C0F0E4..C0F35E` (634 bytes, 176 native instructions).
`native_crt_default_matherr_00c28545` reconstructs complete default
`_matherr C28545..C28548` (three bytes, two instructions). The source uses the
full existing pointer decoder and actual owning CRT errno service. It does not
establish a currently registered callback, CRT startup, or game validation.

The [header](../include/bsp/native_crt_libm_error_support.hpp) specifies a new
qualified Win32 source interface; the [audit](../reports/native_crt_libm_error_support_audit.json)
records complete native spans, source inputs and immutable verification artifacts.
The correct existing library name `___libm_error_support` is retained. The
default target's `_matherr` interpretation follows its concrete call role and
three-byte body; the primary owns any saved-analysis annotation.

## Native evidence and dependencies

Fresh guarded requests use existing `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. The installed PE SHA256 is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Twenty-two spans total 934 bytes: the two owned bodies, complete registration
writer, actual decoder, errno accessor, thirteen-entry native dispatch table,
fourteen complete name strings and two callback-state words. Disk-backed spans
match the installed PE. The two state words are saved virtual zero-fill,
separately recorded from disk data; no live process values are inferred.

`C0F0BB[41]` publishes encoded `109ED80` before flag `109E1B8=1`; null registration
only clears the flag by DWORD AND. The separately reconstructed registration
and encoder establish the real storage contract. No currently registered code,
owner, or lifetime is fabricated. A captured callback remains callable after
a later flag clear. The actual full decoder `C04FDE[110]` retains its current
TLS/IAT/PTD/DecodePointer and module-gate dependencies. Its source and the
complete `C04EFB/BFBB61/C05920` support are linked as existing providers.

Native `BFFB8B[19]` depends on still-incomplete PTD getter `C051B7`. The existing
`LegacyCrtMathRuntime::errno_location_00bffb8b` remains the explicit owning-CRT
boundary, at verified member offset four. It is not a second errno variable.
The decoder's startup/locale/locks/invalid-parameter and TLS allocation
dependencies retain their existing qualified boundaries.

## ABI, state and identity

The naked cdecl entry retains original first, second, output and selector words
at entry ESP+4/+8/+C/+10. The fifth argument, ESP+14, is the added stable context.
It borrows current flag, current encoded word, decoder context, owning runtime
and canonical names at offsets 0/4/8/C/10. All five reference-member offsets
are verified through independently compiled C++ member accessors. The caller
cleans all five words. A general EAX or floating-point result is not promised.

The original flag comparison occurs before second/output/first pointer captures.
MOVs preserve its flags through eight separate local positive-zero byte writes
and the conditional branch. Nonzero flag reads the current encoded word and
calls the full decoder with actual outgoing argument storage; default selection
uses the full three-byte source. The selector is read from its current original
stack slot only after decoding. Unknown selectors and selector 26 still perform
this selection schedule. The decoded pointer is not validated or replaced.

Every admitted callback has actual `int __cdecl(CameraAxesCrtException*)` ABI,
balanced caller-cleaned stack, preserved nonvolatile registers, required net
x87 depth, and valid code/data lifetime. It receives the exact mutable 32-byte
record: type 0, name 4, first 8, second 10h, result 18h. All record offsets and
size are statically checked. Both arguments are populated on unary routes too.
Callback edits to every record field remain possible; the route's errno policy
is not reselected from a modified type. Result is reloaded after callback and
any owning errno accessor/store, including their possible record effects.

Canonical names preserve observable pointer identity. The source borrows the
actual original pointers or one explicitly mapped immutable rebuilt identity
domain shared by every admitted callback/consumer. The function reads a stable
mapping field where the native instruction had an immediate address. It does
not fetch names from mutable `__umatherr` storage or allocate private strings.

| Name | Original identity | Name | Original identity |
|---|---|---|---|
| tan | D571A0 | sin | D571B4 |
| pow | D571C4 | modf | D571C8 |
| log | D571D0 | log10 | D571D4 |
| floor | D571F4 | exp | D571FC |
| cos | D57204 | ceil | D57210 |
| atan | D57218 | asin | D57228 |
| acos | D57230 | exp10 | D6A968 |

The mapping, context and their binding identities remain readable and stable,
separate from mutable native state. Native first/second/output may alias each
other and valid mutable native words. Private source-frame, added mapping and
context aliases, or observation of caller instruction addresses are outside the
qualified domain. No FP-control/status restoration, EH guard, rollback, or
native fault-site/SEH continuation equivalence is supplied.

## Complete selector and x87 schedule

| Selector | Name/type | Initial result | Errno on callback zero |
|---|---|---|---|
| 2/3 | log, 2/1 | current output | 34/33 |
| 8/9 | log10, 2/1 | current output | 34/33 |
| 14/15 | exp, 3/4 | current output | 34/none |
| 24/25 | pow, 3/4 | current output | 34/none |
| 26 | no callback | FLD1 to output | none |
| 27/28 | pow, 2/1 | current output | 34/33 |
| 29 | pow, 1 | first to output before argument reloads | 33 |
| 58/61 | acos/asin, 1 | current output | 33 |
| 166 | exp10, 3 | current output | 34 |
| 1000..1005 | log/log10/exp/atan/ceil/floor, 1 | first to output before reloads | 33 |
| 1006 | pow, 1 | current output | 33 |
| 1007 | modf, 1 | first to output before reloads | 33 |
| 1008/1009 | acos/asin, 1 | current output | 33 |
| 1010..1012 | sin/cos/tan, 1 | first times local positive zero | 33 |

Unknown selectors perform no pointee access, callback, or errno write after
selection. The signed 166 boundary and unsigned thirteen-entry bounds are
preserved, followed by the actual indexed DWORD jump. The readonly rebuilt
table contains thirteen static function-symbol plus observed-label-offset
relocations. No dynamic initializer or runtime table write is introduced.
The offsets are derived from emitted MSVC Win32 code and rechecked against
every final object/link target. Toolchain/layout changes require revalidation.

All native x87 instructions retain their order. Early-copy routes execute
`FLD first; FSTP output` before rereading arguments, preserving output aliases.
Selectors 1010..1012 execute `FLD first; FMUL local+0; FST output`, keep the
extended product on the x87 stack while reloading both arguments, then FSTP
that retained product into record.result. Final `FLD record.result; FSTP output`
follows any errno path. Rounding, NaN quieting and hardware exceptions remain
the actual instruction effects. No host arithmetic or ordinary C++ double
copies replace this sequence.

## Verification and limits

The primary reviewed the complete source/header before the strict build.
The compile-only proof maps every one of 176 native service instructions to
255 emitted instructions, 784 bytes. It checks unchanged native instructions,
every mapped branch, all canonical-name/context expansions, decoder/default
and table relocations, all thirteen table destinations, and exact default
`33 C0 C3`. Independent Release and Debug object layouts agree. The compile-only
static-initializer experiment confirms readonly DIR32 relocations and no `.CRT`
initializer. The reference-member accessors prove the five context offsets.

The repository build uses an ignored local CMake hook to include this source
and the already existing decoder source missing from the worker base registry.
No shared CMake source, ledger, Ghidra analysis, original installation or test
source is edited by this packet. Strict build, existing CTests, eight seed
comparisons and the final archive/link proof are recorded in the audit.
The address-only link artifact is not executed. Existing tests do not exercise
this service; compilation and static instruction evidence are not a callback
fixture, hardware-exception experiment, binary replacement or game validation.


Primary integration passed against the frozen main library. Complete 634-byte/176-instruction service maps to 784 bytes/255 instructions; exact three-byte default callback. All 13 selector targets, five compiled reference offsets, full decoder/default/strcmp/OS-major/module-gate linked bodies and canonical identity/errno scheduling checked. The strict Win32 build passed both existing CTests and eight reference seeds with 56 unchanged source/header/build inputs. Fourteen actual archive members, 14 compiler commands and 266 read dependencies are frozen under `local/pow_chain_build_frozen/`; reviewed proof replays are under `local/pow_chain_primary/`. Ghidra names/comments are saved, prior values retained, reconstruction records registered and all affected exports refreshed. These static checks do not establish runtime, original caller ABI, native SEH or game behavior.
