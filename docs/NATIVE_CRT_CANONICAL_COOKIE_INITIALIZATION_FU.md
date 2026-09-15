# Canonical original-entry security cookie initialization

This packet supplies `bsp::initialize_native_crt_canonical_security_cookie_00c1815e()`
as a separate MSVC Win32 no-argument cdecl entry for the complete original
`___security_init_cookie`. The existing borrowed context API remains unchanged.
The new entry requires the actual writable cookie at `00E15590` and its distinct
complement at `00E15594`, in the admitted canonical `00E15000` page. It does not
create storage, bind the host CRT cookie, call an owner accessor, or add arguments.

| Original routine | Coverage | Original ABI | Source |
| --- | --- | --- | --- |
| `00C1815E..00C181F1` | complete, 148 bytes / 52 instructions | no arguments; cdecl; plain `RET` | `src/native_crt_canonical_cookie_initialization.cpp` |

The Ghidra library name `___security_init_cookie` is retained. The new C++ name
describes this source variant; it is not a recovered symbol. No Ghidra mutation
was performed. `local/canonical_cookie_initialization_fu/desired_annotation.json`
asks the primary to preserve the existing name and comments and append evidence.

## Body and original imports

The native EBP frame allocates 16 local bytes and reads the current cookie before
clearing the two FILETIME words. It preserves EBP, EBX, EDI and the conditionally
used ESI with the original push/pop schedule. A nondefault current cookie with a
nonzero high word takes the fast path and writes only its complement. The other
path performs the five real imports in exactly this order:

| Native site | Original IAT | Actual import | Argument bytes / cleanup |
| --- | --- | --- | --- |
| `00C18193` | `00CE2100` | `KERNEL32!GetSystemTimeAsFileTime` | 4, stdcall |
| `00C1819F` | `00CE21DC` | `KERNEL32!GetCurrentProcessId` | 0, stdcall |
| `00C181A7` | `00CE223C` | `KERNEL32!GetCurrentThreadId` | 0, stdcall |
| `00C181AF` | `00CE22AC` | `KERNEL32!GetTickCount` | 0, stdcall |
| `00C181BB` | `00CE2270` | `KERNEL32!QueryPerformanceCounter` | 4, stdcall |

The FILETIME words, PID, TID, tick count and both QPC words are XORed in native
order. The QPC output occupies the other eight local bytes, receives no prior
initialization stores and is read even if the ignored BOOL is false. An exact
`BB40E64E` becomes `BB40E64F`; otherwise a zero high word becomes `v | (v << 16)`.
Zero may remain zero. Publication writes cookie before complement. No repair,
extra entropy, branch, register load, flag operation or exception translation is
inserted. Four explicit opcode encodings retain the original absolute operands
without the extra DS prefixes that MSVC may add for numeric assembly operands.
Two explicit `TEST` encodings also retain the original ModRM bytes instead of
MSVC's equivalent swapped-register forms.

## Caller ordering and future owner integration

Current target-verified live callers and xrefs identify exactly one direct caller:
the complete ten-byte `entry` body at `00BFD2BD` calls `00C1815E` immediately and
then jumps at `00BFD2C2` to `00BFD0DD`. There are no argument pushes, stack cleanup
instructions or enclosing cookie-protected frame in that entry stub. This proves
original initialization precedes that startup continuation; it does not prove
the continuation's full service or SEH readiness.

The current canonical owner provides a concrete future integration point in
`GameNativeCanonicalDataOwner` construction, `src/game_native_mutable_crt_data.cpp`:
both disjoint owner subsets transfer before page commits; the read-only owner
initializes, the mutable owner initializes and verifies exact PE-backed pages,
then `verify_joint()` validates the joint domain. The existing lines 155-157
construct a two-word context and call the borrowed initializer. Replacing that
invocation with this no-argument entry after `verify_joint()` is the earliest
proved point in that established path. Keep the following complement/default
validation and page verification, successful parent readiness ACK, and only then
release publication of the permanent process owner. No owner source was changed
by this packet, and the new entry is not currently called by that owner.

`initialize()` uses the existing atomic process claim and consumes the inherited
reservation once. Constructor failure rolls back without publishing; a failed
claim is cleared by the existing owner policy. The initializer itself has no
once guard or synchronization and does not establish a seed policy. Invoke it
only while that actual domain is exclusively admitted and before native frames
capture the cookie. Do not insert a second initialization after publication or
while native cookie-protected frames exist. The current owner remains responsible
for lifetime through process teardown. These are source-order integration
constraints, not a new runtime validation of the owner or its consumers.

## Verification and evidence limits

The source pin is `c8010d8fa964667a58d9cec6144d3fc9b97e7a49`. Read-only Ghidra queries
use `tools/bsp.py` with autostart disabled; its client verifies the saved `bsp`
project, `/battlestationspacific.exe`, x86 language and image base before each
query. Original body, caller, cookie pair and all five import slots agree with
the supported installed PE. All eight existing seed spans also agree.

The prebuild guard records the absence of the owned object and retains exact
source/header/startup, actual owner/provider inputs and both x86 compiler
candidates. The first `scripts/build.ps1 -Diagnostic` strict Win32 build and both
existing math CTests passed, but its full object audit rejected two symmetric
`TEST` ModRM differences at offsets 36 and 119. That entire first source/object/
archive/build/guard attempt is preserved. After explicit opcode corrections, a
fresh guard and second full build passed both existing `reconstructed_math` and
`native_math_differential` CTests. No tests were added. The final postbuild guard
confirms unchanged inputs and
toolchains. Retained source-specific CL command/read/write records connect the
source and header to the actual compiler output. A separate SDK guard captured
the actual import declarations and x86 KERNEL32 import library before the owned
object existed, and confirmed those seven inputs unchanged after the build.

The complete COFF object has exactly one nonempty code section, containing all
148 native bytes. Every emitted byte is accounted for; the only five code
relocations are genuine `DIR32` Win32 import operands. Substitution of their
original IAT addresses gives exact equality for all 148 bytes, including all
four fixed global operands, local accesses, branches and epilog. No global
symbol relocation, helper body or private cookie object is introduced. All COFF
sections and relocation records are retained and inventoried, including compiler
metadata. The archive has one member with the expected name, byte-identical to
the actual object; its header and both linker tables independently resolve the
unique public symbol to that same member. The two full linker-table payloads are
retained without retaining the whole archive or build directory.

`reports/native_crt_canonical_cookie_initialization_fu.json` records the direct
caller and indirect import rows. The standard live call verifier checks the two
direct entry edges; import rows are explicitly skipped there and separately
verified by PE import decoding and complete instruction bytes. Appendix evidence
retains failed methods and commands rather than overwriting them. The top-level
local seal inventories every file by its full relative path, excluding only
itself, and records physical-file and committed Git-blob hashes separately.

No initializer, target body, forced DLL, ad hoc probe or game code was executed.
Existing math CTests do not exercise this initializer. The static original ABI
and instruction schedule are established; source code and IAT locations move,
so original instruction addresses, OS fault continuation identity, unwind/SEH
behavior, full startup and gameplay remain unvalidated. Other CRT owners and
services are separate prerequisites.
