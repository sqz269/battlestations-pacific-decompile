# Damageable-class Mesh field fragment

Addresses: `0087CA80`, `0087CB44..0087CBCE`. The fragment is inside the
damageable-class Lua reader; `0087CB44` is a call instruction, not a function
entry. Descriptive names remain hypotheses.

The new header/source provide an ordinary MSVC Win32 C++ interface for this
one field. They borrow the actual descriptor, Lua row, still-live `Unique`,
parent scratch, Lua owner and raw string-pool publications. They do not provide
the enclosing `0087CA80` reader, a class factory, a registry, model loading, a
Native ABI entrypoint or an application receiver. Worker compilation is separate
from integrator registration/admission.

## Retained parent gate and selected flow

The retained whole-parent audit is
`reports/native_damageable_class_lua_orch4.json`, SHA-256
`ff0e9d14717ddfd33456b3dced444d221a6e4164f79618a23fce07865383e31e`.
Its complete 37-state FH3 analysis is reused without reopening child Native
bodies, handlers or unrelated fields. Fresh read-only queries used `bsp.py
ghidra proto` and `ghidra disasm`; the client verified project `bsp` at
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, x86 and image
base `00400000` against current configuration. No Ghidra writes occurred.

Fresh installed-PE decoding reconfirmed the complete body
`0087CA80..0087D725`: 3,238 bytes, 858 instructions, SHA-256
`00d679920bd1b8f8653e9a0ab1de627f954cfadaf48b8bf0642676fde5162808`.
Every PE instruction start matches both the entire saved listing and the entire
fresh live listing; this gate was evaluated over full files, not their printed
heads. The selected fragment is 139 bytes and 42 instructions. Its seven
direct calls and every instruction are included in the accompanying report.

The parent takes actual descriptor in ECX, one live `LuaObject*` row on the
stack, and ends with RET 4. Let S be ESP after its E4h local allocation and four
saved registers. At the selected entry EBX is the descriptor, EBP the live row,
and the call's stack arguments are already prepared for the field object at
S+44h and the immutable `Mesh` key. `Unique` at S+94h is already live under
state 1; its normalized byte was stored at descriptor+34h at `0087CB41`.
The new interface starts after that store and does not repeat it.

| Original site | Fragment behavior |
| --- | --- |
| CB44 | B67800 lookup into the actual S+44h scratch object |
| CB55 | enter state 2 only after lookup returns |
| CB5D | B685C0 exact STRING or empty default into fresh S+10h string |
| CB69 | enter state 3 only after string construction returns |
| CB67/CB71 | retain destination/source identity check |
| CB7A | resize actual descriptor+38h header with source length and preserve=1 |
| CB7F..CB8F | reread source length; if nonzero, copy current destination length from current source data to current destination data |
| CB9D | lower to state 2 before pool getter/temporary return |
| CBB2/CBB9 | resolve current raw pool and return nonnull temporary data with length+1, DWORD wrap |
| CBC2/CBCA | lower to state 1, then destroy the actual tracked field object |

The normal successor is `0087CBCF`, which prepares Comment. The parent still
owns `Unique` and ultimately destroys it at `0087D706`. It must also own its
cleanup after a propagated fragment failure. Neither the descriptor nor any
tracked Lua object is copied. Descriptor+38h/+3Ch is accessed as its real
8-byte NativeString header, without a semantic class cast.

## Providers and borrowed storage

`native_lua_get_by_name_protected` wraps the real B67800 source in its existing
same-Lua-frame protected operation. The actual table owner/index is used; the
result registers its actual scratch address. Lua errors restore the entry
stack and throw `NativeLuaOperationError` without publishing the output.
The inherited error-handler position and caller/owner stability requirements
from `native_lua_objects.hpp` remain required. This is the existing ordinary
C++ error transport, not original Lua nonlocal-transfer identity.

B685C0 is composed from its existing exact-string predicate and getter plus
`construct_native_string_header_0041e870` with the raw pool context. The old
B685C0 `NativeStringStorage` overload has a noexcept release boundary; using the
raw constructor avoids introducing that boundary into this fragment. Kind 2
and exact Lua STRING are required for the getter. Number and numeric-looking
non-string values take the empty fallback, so there is no number-to-string
conversion here. The original constants were checked directly in the PE:
`00CE5FD0` is `4d65736800`, and `00CE3A0C` begins with `00`.

The real raw constructor zeros the actual temporary header, scans its selected
C string, resizes and copies length+1 bytes. An embedded NUL therefore ends the
mesh name even if the Lua string is longer. The raw resize/destructor resolve
the current pool publication on every relevant allocation/return, including
large blocks and disabled small returns. They use the actual publication cells
01090AA8, 01090AA4 and 01090AA0; no pool is cached by this fragment. Header fields
are reread after provider calls. The copy uses the existing host `memmove`
boundary for BF7680 overlap behavior and omits a zero-byte call, consistently
with the reused raw-string providers.

All borrowed storage must remain valid and at stable addresses. The row,
Unique, descriptor, 14h-byte field scratch, 8h-byte string scratch and publication
cells must not overlap. Both scratch regions have four-byte alignment and no
live prior object on entry. Unique belongs to the same actual tracking owner
used by this parent invocation and remains registered. No replay, copied row,
cached tracked index, default provider or fabricated mesh name is accepted.

## Exceptions and lifetime boundary

The retained FH3 edges are state 3 -> 2 via C9691E/41DD20, state 2 -> 1 via
C96913/B67700, and parent state 1 -> -1 via C9690B/B67700. The new guard owns
only states 2 and 3. It lowers its state before calling cleanup; it never retries
a cleanup that throws and never destroys Unique. After normal completion only
parent state 1 remains live. On a C++ failure, completed descriptor mutations
remain visible and any active fragment temporaries are cleaned in that order.

Lookup failure has not published a field object. String-construction failure
cleans the field but does not invent cleanup for an incompletely constructed
string. Resize/copy failure cleans the completed string, then the field.
Failure during normal string cleanup still cleans the field. Failure during
field cleanup does not retry it. A second failure during guard unwinding meets
the ordinary C++ noexcept-destructor termination boundary. Native FH3 frame
identity, faults/SEH, Lua longjmp and double exceptions are held; the source's
state stores do not reproduce fault-sensitive native instruction timing.

## Validation and limits

Focused candidate and real provider objects were compiled using MSVC x86
Release `/O2 /Ob2 /MD /W4 /WX /fp:strict /EHsc /std:c++17`, with the project's
Win32/Windows/NDEBUG definitions. The candidate's full indexed COFF inventory
is retained in the report: 23 sections, all 513 code bytes across nine sections,
all helper COMDATs, both EH handlers, unwind funclet, cleanup methods and EH
metadata. The seven project-provider references resolve by exact decorated
symbol to actual compiled Lua/string definitions. The selected definitions and
their indexed relocations are retained, including raw pool getter/allocation/
return edges. No callback stub or unresolved project declaration was introduced.

The normal Win32 Release worker build passed, as did both enabled existing
checks (`reconstructed_math` and `tool_tests`, 7.20 seconds total). They are
recorded separately in the report. This fragment was intentionally absent from shared CMake during worker
review; a passing baseline build does not admit it. No new tests, original/source
differential execution, application receiver binding, startup or gameplay proof
is claimed. Root must review and register the fragment before integrated build
credit. Whole `0087CA80` Source remains absent.

The supplied Source 534 context and primary review receipt
`9fc3a11764a53ed6c100471213d97509b47f510617d6bea9c32f39c5b4c7f218`
are historical context. The current primary-review file hash was separately
verified, and current provider source/header hashes are pinned in this report.
This packet does not advance a whole-function Source count or borrow the older
startup poll observation as fragment validation.
