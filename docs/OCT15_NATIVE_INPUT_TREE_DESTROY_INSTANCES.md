# Actual input-tree destroy instances

This packet reconstructs five complete ordinary STL_Tree_Destroy bodies.
Descriptive C++ names remain hypotheses. Established library identities and
earlier typed and AV interfaces are preserved.

| Entry | Consumed range | Inclusive body end | Bytes / current starts |
| --- | --- | --- | --- |
| 006A0D60 | scalar | 006A0D93 | 52 / 23 |
| 006A14E0 | scalar | 006A1513 | 52 / 23 |
| 006A21C0 | int/string | 006A21F3 | 52 / 23 |
| 006A24F0 | int/string | 006A2523 | 52 / 23 |
| 006A2B70 | int/string | 006A2BA3 | 52 / 23 |

Each entry has one contiguous body, with no gap or local EH. All 52 bytes
equal the published 006A1DF0 reference after only the two verified E8
operands are normalized; the remaining 44 bytes match.

## Interfaces and ownership

Native ECX receives the actual 12-byte tree. Incoming EDX is unconsumed.
There are no stacked inputs; plain RET returns DWORD zero in EAX.
The scalar Source functions retain that input and return shape and directly
use published whole 0069FE70, without adding a pool or context argument.

The three int/string functions add a borrowed NativeStringRawPoolContext
and therefore use Source RET4. Their range provider is published whole 006A1B70, with its
existing real, trivial eight-byte NativeInputIntStringTreeIterator carrier.
Child unused EDX is canonically zero. Original register residue and private
frames are unmodelled.

Each body captures tree.head at +4, then head.left at +0, unconditionally.
It passes the same tree as both owners, the captured left and head as
endpoints, and a private ignored eight-byte output to the full-range
provider. No count check, empty or null guard, or subtree shortcut replaces
that call.

After child success, reload CURRENT tree.head and capture it for the
accepted current CRT free service. Then clear tree+4 BEFORE tree+8 and
return zero. Child failure propagates after its own progress and before
this entry's free or stores. No local EH, retry, rollback, guard or extra
payload cleanup is added. There is no direct opaque tree+0 access;
arbitrary admitted raw aliases can still overlap that word.

## Actual transfers

These are ten owned rows and two separately scoped read-only reference rows.

| Function | Native transfer | Scope |
| --- | --- | --- |
| 006A0D60 | 006A0D76 CALL 0069FE70 | owned |
| 006A0D60 | 006A0D7F CALL 00BF65AC | owned |
| 006A14E0 | 006A14F6 CALL 0069FE70 | owned |
| 006A14E0 | 006A14FF CALL 00BF65AC | owned |
| 006A21C0 | 006A21D6 CALL 006A1B70 | owned |
| 006A21C0 | 006A21DF CALL 00BF65AC | owned |
| 006A24F0 | 006A2506 CALL 006A1B70 | owned |
| 006A24F0 | 006A250F CALL 00BF65AC | owned |
| 006A2B70 | 006A2B86 CALL 006A1B70 | owned |
| 006A2B70 | 006A2B8F CALL 00BF65AC | owned |
| 006A1DF0 | 006A1E06 CALL 0069FE70 | read_only_scalar_reference |
| 006A1DF0 | 006A1E0F CALL 00BF65AC | read_only_scalar_reference |

## Verification

Baseline is published Main 44b08ee2f426ca45ec791e4c4d0804d9f809082d.
Fresh PE/live equality covers 260 owned bytes / 115 exact current starts,
plus a separate 52-byte / 23-start scalar reference. All five complete
native schedules and physical five-word range setups were reviewed.

Default ./scripts/build.ps1 exited zero for MSVC Win32. The existing three
suites passed in 8.98 seconds. No new tests, standalone probes or Native
execution were added. These suites do not execute these new bodies.

The entire actual CMake I386 object was read: 375 code bytes / 159
instructions, comprising five public bodies (325 / 133) and four helpers
(50 / 26). All ten code REL32s identify the intended range and free
providers. There are no noncode relocations, local CxxEH handler, alignment
NOPs or INT3 padding. Both scalar bodies and all four helpers match their
actual published scalar-reference object code. Relocations compare offset,
type and resolved symbol; object-local symbol indices differ.

## Limits and evidence

Public accesses use volatile raw DWORDs and unsigned32 address wrapping.
Source private output and argument frames, pushes and register schedules
differ. Native sets EAX zero BEFORE the public stores; actual Source code
sets it AFTER them. Semantic zero return is retained without claiming
original register or flag timing or private-frame aliases.

The current free service is noexcept and delegates current std::free.
No noexcept adapter surrounds child exceptions. Valid reached storage,
borrowed pool ownership and matching current CRT are required. Original
CRT, heap, FH3, RTTI, SEH, fault, cleanup-failure, binary ABI and game
equivalence remain unproved.

There is no generic Kind, model, provider, callback, public 006A1DF0
delegate, new context for scalar payload, or containing owner or unwind
promotion. Existing Source and fixtures remain unchanged.

Tracked report: reports/oct15_native_input_tree_destroy_instances.json.
Ignored Native evidence: local/oct15_native_input_tree_destroy_instances_evidence.json.
Full object evidence: local/oct15_native_input_tree_destroy_instances_coff.json.
Build log: local/oct15_native_input_tree_destroy_instances_build.log.
