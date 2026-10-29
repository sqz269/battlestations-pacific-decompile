# Qualified raw Model geometry association

Status: **Win32 build/object-checked; direct runtime unproved**. This dependency is based
on frozen `48195196e`; both are now covered by the integrator build below.
The worker checked that parent packet byte-for-byte before adding this entry.
Its source CPP/HPP remain unchanged; integration adds validation documentation. This packet owns
only three new files and B75170; it changes no Text/Model/Group implementation,
shared transport, CMake/ledger entry, activation or function-body credit.

## Entry and operand views

Original B75170 is123B, ECX actual Model, four stacked DWORDs and RET10h, with
no result contract. Entry ESP+4 is unused, ESP+8 is geometry, ESP+C is float178,
and ESP+10 is float17C. The Ghidra prototype remains incomplete; the instruction
listing, not its undefined C signature, supplies these operand facts.

The Source prototype is:

```cpp
void set_native_model_geometry_storage_00b75170(void* actual_model,
    NativeModelStorageGeometryFrame&,
    NativeModelStorageGeometryContext&,
    NativeModelStorageGeometryAcquired&);
```

Its noncopyable frame borrows four actual initialized volatile DWORD cells.
Construction takes their addresses without reading values. The unused operand
is never read. Geometry is captured once at entry to the admitted body; scalar
values are not passed by value or converted to C++ floats. Explicit operand
aliases are supported when all reached accesses remain valid, including a
second scalar cell that aliases the Model178 destination written earlier.
Frame/context/acquisition storage itself is disjoint from operands/native data.

## Native order retained

| Site | Required operation |
| --- | --- |
| B75170 | Capture incoming geometry from its actual argument cell |
| B75178 | Capture old Model180 AFTER that incoming capture |
| B75184 | On unequal pointers publish requested180 |
| B75190 | For nonnull requested, sample/call CURRENT CE221C on requested+4 |
| B7519E | For nonnull captured old, separately sample/call CURRENT CE2220 |
| B751AE | Only at returned/observed zero dispatch captured old's genuine current0 |
| B751B0 | Read CURRENT first scalar argument, after all preceding callbacks |
| B751B6 | Sample genuine CURRENT D7A260 once, after that first argument read |
| B751BE..C7 | Compare and conditionally store178 |
| B751CF | Read CURRENT second scalar argument AFTER the first possible store |
| B751D5..DE | Compare/store17C using the SAME captured sentinel |

Equality skips only reference operations and their import reads; it still
executes the scalar tail. A changed association publishes before retain/release.
An increment callback can change the current decrement target. The old geometry
identity remains captured even when callbacks replace Model180. There is no
final180 clear or reload; any callback-installed replacement survives.

The current import targets and completed operations are recorded separately.
The existing actual-owner contract supplies CURRENT0 at the captured old's SAME
actual+4 zero. Its physical count address and zero are checked before dispatch.
No model/node extent or scene-null assumption is applied to opaque geometry.
Missing owners/targets are errors, not successful defaults. This code does not
use the semantic atomic retain/release helpers.

## SSE behavior and qualification

The scalar helper executes the Original MOVSS, UCOMISS, LAHF, TEST AH,44h, JNP
and conditional MOVSS sequence twice. XMM0 holds the single sentinel sample.
There is no C++ floating conversion, comparison, NaN normalization, global
default, early scalar read or alternate arithmetic path. Equal scalar values
skip stores; ordered unequal and unordered comparisons store the original bits.

Diagnostic MOV/MOVSS stores use disjoint metadata and do not change MXCSR or the
flags consumed by the compare/branch pairs. Thus argument aliasing and callback
changes reach the same authored scalar instruction order. The helper neither
changes the FP control state nor clears exception status. Current MXCSR applies
to its UCOMISS instructions, including signaling-NaN/denormal behavior.

Integrator object inspection confirms the authored MSVC Win32/SSE scalar
sequence. Extra Source metadata/admission and C++ calls have their own ABI;
Original GPR/EFLAGS/XMM outputs, private stack identity, register transport,
RET10h compatibility and binary detours are not promised. No x87 operation is
introduced. Unmasked FP traps, SEH/nonlocal escape recovery and Original fault
or exception-object identity remain excluded. An interrupted frame stays
entered/failed and cannot authorize recovery or replay.

## Admission, ownership and failure

The context borrows the real481 `NativeModelStorageContext` and an actual
D7A260 cell. Checked host admission precedes the body: one registered live
`NativeModelStorageReference`, SAME actual+4 positive count, D62DE8 profile and
same allocated/not-returned188h Model pool slot. The481 private helper is not
reused or changed; the new file checks only its public real pool/owner views.
No logical SceneResource, NativeModelOwner, second count or association is made.

Caller obligations still matter: the Model was constructed with THIS storage
context, its481 lifetime acquisition has not started destruction, and a native
holder keeps the receiver accessible through all later stores. Public admission
does not inspect the reference's private phase or permit reentry during an
explicit positive-count scalar destruction. Imports, opaque member counts and
canonical companions must be genuine and remain valid at their reached calls.
The same node scene/tree owner registry and decrement cell are required.

The one-shot frame and acquisition enter before the geometry operand read.
Completed callbacks/stores, sampled imports, captured identities and scalar
bits/read/store progress remain in the caller-retained record. Nested calls
need distinct frames/acquisitions and live operand backing. A throwing callback
may have effects beyond completed flags; those flags do not invent credits.

B75170 has no native local rollback/cleanup. The Source catch only marks failure
and rethrows. After an entered failure, retain operands, record, Model and
affected graph; make no further graph call/replay and follow the existing host
retention/_Exit policy. Dropping an entered/failed frame terminates rather than
silently disposing its evidence or fabricating cleanup. No reset/retry API is
provided. Successful completion alone does not establish unrelated owner
retirement or GUI cleanup.

## Evidence and remaining work

The live read client verified project `bsp`, `/battlestationspacific.exe`,
language/image base against `config/target.json`; that configuration selects
`C:/Users/sqz269/bsp.gpr`. Read-only `bsp.py show 00b75170 --asm` and
`bsp.py ghidra proto 00b75170` supplied the123B/40-instruction body and RET10h.
No Ghidra mutation, export rewrite or Original-body execution occurred.

Existing `native_model_owner.cpp:277-293` documents the older typed boundary;
it is unchanged. This raw provider closes the actual association entry, not
its Text helpers' NativeModelOwner/NativeModelReference transport. Mandatory
internal Text188 construction, Model scene/tree routing, Group integration,
primary4C publication and nonempty content remain primary/GUI-worker decisions.
This does not make the child graph independence-ready or GUI-complete.

The worker checkpoint had only static checks during the A1 hold. Integrator
source `eb18518db` registers this TU, passes MSVC Win32 Release and both existing
CTests, and retains object disassembly in
`cc16-progress/local/codex_gui_dependencies_geometry_disasm.txt`. The emitted
scalar helper preserves MOVSS/UCOMISS/LAHF/TEST44/JNP and conditional stores,
with one sentinel sample and the late second operand. Inactive reference/MSD
pairs pass rawpair0/ordered0/teardown0 against accepted A1 ON799 using candidate
SHA `05F362BB61DC3618D6CB560A0F3F3AEB332E720282CA726F0D62B04A89D2A0D2`.
These runs never call this provider. Actual alias/callback/FP execution,
lifecycle and Original ABI parity remain unproved; no new whole-body credit.
