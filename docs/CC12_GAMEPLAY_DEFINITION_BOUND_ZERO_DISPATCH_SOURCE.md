# Existing-bound gameplay lookup and raw zero dispatch

The existing `GameplayDefinitionReferences` domain now exposes a pure
`find_bound` lookup and an explicit `dispatch_bound_definition_zero` entry.
The latter uses the genuine BD30E0 Source provider and a private concrete
current-slot adapter, then shares successful metadata retirement with the
existing typed/raw host release path. Only the companion header and implementation
change. These are ordinary Source APIs with no new Native address or ABI claim.

`find_bound` compares the supplied raw address with existing canonical companion
addresses. Null and unknown addresses return null. The lookup reads no raw owner
memory or count and performs no allocation, binding, retain, decrement, publication
or retirement. Its result is borrowed and expires when that companion is retired.

The explicit zero entry requires a raw-context domain, an existing canonical
companion, a live actual count of zero, and exactly one genuine preceding 1-to-0
release. It checks current D0DA58 and the borrowed current slot0 value BD30E0,
then calls `invoke_native_ref_counted_delete_00bd30e0`. That provider reloads the
actual profile. Its private final calls object admits only the same owner,
captured D0DA58 and flags1, checks the fresh current profile and slot4 value
871440, and calls the genuine raw scalar overload with flags1. The compiler's
private calls object lives on the stack; it does not replace the actual owner's
Native vptr or install a Native table.

Only successful scalar return reaches the shared `retire` helper. The existing
typed and raw host paths preserve their selected scalar and flags1, then invoke
the same helper. Retirement finds and removes exactly the existing companion;
it neither accesses the freed raw owner nor changes its count. Unknown binding,
typed-domain use of the new entry, nonzero count, unsupported profile/slot or
throwing cleanup cannot become success. The new entry is `noexcept`; these errors
terminate, and failed scalar cleanup cannot retire metadata. Providers must remain
live and nonthrowing. Concurrent or reentrant terminal calls for the same owner,
retry, first bind at zero and invalidated storage are outside the contract.

The host domain remains 24 bytes: selected context0, borrowed table4, metadata
vector8 and immutable discriminator20. The actual Native definition remains
24h bytes with its existing atomic count at4. Existing binding, identity projection,
constructors and point-construction behavior are unchanged. Removing only the
declared additions and the retirement extraction reproduces both complete
preimage Source files exactly.

Root accepted the raw-context and producer/terminal prerequisites on main
`f2cb33e93`. A separate accepted baseline retains all 1,500 Root Source1496 pins,
195 selected Core objects, 11 App objects and 313 previously unique positive
definitions. The earlier preparation remains unchanged historical evidence.
The 1,672 selected current project inputs plus 55 existing quoted-header inputs
form a 1,727-file project closure; three Lua headers are separate external inputs.
All 3,835 quoted include edges resolve. The five Root-versus-worker CRLF differences
are recorded explicitly; the earlier readiness Source differs only in the two
already accepted renderer/string-process headers. Added evidence inputs are
existing files, not new implementation modules.

The normal worker Win32 build passes its two available checks in 7.39 seconds.
The native seed is absent in this worktree; Root's third check is not claimed.
No new tests or probe executables were added. Two isolated compiles preserve the
exact ordered Root arguments apart from path localization and include/assembly/
layout diagnostics. Their code, EH, non-debug data and indexed targets match the
actual frozen Root preimage and fresh worker candidate; debug/checksum differences
remain fully retained compiler metadata.

The whole audit retains 163 COFF objects and 83 full comparisons. Across the
80 provider objects, 79 are byte-identical to the previous worker artifacts.
There are 4,853 unchanged prior code sections totaling 250,134 bytes. Only the
two old host zero bodies and their two cookie handlers change; 15 changed/added
sections contain 1,015 bytes and 381 fully inspected instructions. Lookup is
49 bytes / 24 instructions with no calls or relocations. All seven old EH infos
preserve their fields and unwind actions. The new dispatcher has maxState0,
flags5 and the sole new SafeSEH handler. The compiler's abstract-base RTTI/vtable
metadata is retained; the concrete dispatcher targets the private override.

The actual archive graph closes 17 roots, 914 sections and 2,291 indexed edges,
with no missing or ambiguous project provider and no newly selected provider
object. It resolves the real BD30E0 and typed/raw scalar bodies. CRT, Lua, Win32
and existing dynamic provider obligations remain explicit. The 195 current worker
Core objects and 11 App objects are separately retained; none is claimed wholly
identical to Root across worktrees. The 80-object code/EH audit has its own scope.

No producer, raw section/vector caller, category path, application owner or
shutdown path is activated. Fresh-ID initialization, duplicate-loser disposition,
callback replacement prebinding, all publication sites, generic/throwing terminals
and actual Native callable-table binding remain separate gates. There is no
startup, gameplay or Native exception-dispatch compatibility credit.

Run the immutable artifact replay from the worker worktree with
`python local/gameplay_definition_bound_zero_dispatch_verify.py`.
It verifies Git/preimage/current snapshots, both complete archives, actual CL
records, every saved physical COFF inventory, all comparisons and the provider
graph without invoking a compiler, test, Native analyzer or Ghidra operation.
Historical Root live-object metadata resolves only to its exact already-frozen
size/hash counterpart; current Root build paths are not historical replay inputs.
The [report](../reports/cc12_gameplay_definition_bound_zero_dispatch_source.json)
gives the complete artifact map, exact receipt paths and Source/Git schema.
