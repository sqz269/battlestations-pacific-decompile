# Whole raw avoid-manager group lookup

The complete 66-byte lookup is reconstructed as a separately callable MSVC
Win32 Source entry. It preserves every physical instruction and operand, with
zero relocations or masked bytes. The older typed index projection remains.

| Entry | Physical body | Coverage | Original interface |
| --- | --- | --- | --- |
| `004120D0` | `004120D0..00412111`, 66 bytes / 32 instructions | complete | ECX raw manager, one signed layer stack word, EAX borrowed group, RET 4 |

The Source API is `lookup_native_avoid_manager_group_004120d0`, declared in
`native_avoid_manager_group_lookup.hpp`. Its unused EDX parameter is a Source
fastcall slot; Original has no EDX input. Existing `Cc13AvoidZoneManagerNative`
(78h, group slots +08h and count +58h) and `Cc13AvoidZoneNativeGroup` (14h,
signed layer +10h) are reused and asserted. These schemas describe consumed
storage, rather than recovered class/factory/owner identity.

## Exact raw operation

After four callee-save pushes, capture the count DWORD once and interpret it
as signed. Initialize index and fallback index to zero. A captured count <= 0
returns the CURRENT slot0 word without reading the incoming layer word. It
can return null; it does not repair an empty/uninitialized manager table.

For a positive captured count, read the actual incoming layer cell once at
ESP+14h. Walk slot addresses in four-byte DWORD steps. Read each pointer and
its +10h key without a null check. First equality returns a fresh load of the
matching slot. Otherwise signed CMOVL records the current physical index when
key < layer. Advance index and slot and use signed JL against the captured
count. Exhaustion freshly loads the fallback slot. No qualifying key retains
fallback0. No count reload, capacity bound, sorted-order break, defensive
pointer validation, allocation, lock, global, callback or cleanup is added.

An unsorted table chooses the LAST PHYSICAL qualifying index, which need not
have the greatest qualifying key. The normal factory maintains signed sorted
order, but this query does not require or enforce it. Both chosen-slot return
loads and modulo-DWORD address formation are literal instructions. No private
C++ argument copies, host vector, lower_bound or cached returned slot is used.

## Callers, lifetime and integration

Fresh live references contain 22 sites in 13 functions. Physical PE bytes show
21 E8 CALLs and one E9 tail JMP. Ghidra currently labels all 22 references
UNCONDITIONAL_CALL, so the receipt keeps physical transfer kind separately.
The tail wrapper rewrites its actual incoming layer cell before transferring.
Representative verified transfers:

```text
0042365E CALL 004120D0
0082ADBA JMP  004120D0
```

The grid-path caller tests the returned pointer before owner removal at
`00423671`. That removal is the separately complete raw Source entry. A null
owner delegates to the complete group clear. Group add, factory and deleting
lifetime chains are separate actual Source implementations; none is called
by this leaf. All 22 caller-site arguments/receiver setup and containing-body
ranges were inspected and captured. Original consumed ECX/stack shape agrees,
including the tail wrapper's reused stack word.

The result is borrowed. The query changes neither manager nor groups and
establishes no ownership, lifetime retention, lock or concurrent-mutation
policy. Raw positive counts may walk beyond the schema's nominal 20 slots;
actual required cells must be readable. Invalid-cell memory faults are not
caught or repaired. There is no FH3, current C++ guard or unwind action here.

The older `avoid_zone_group_for_layer_004120d0` uses a host table and returns
an index, including -1 on empty. It is not this raw provider. Its historical
endpoint wording ends at119 rather than the observed111, and its never-null
wording is not an Original guarantee; those historical files remain intact.
The existing grid-path Source still requires a __thiscall binding. The new
fastcall function-pointer type is not an automatic C++ assignment to that
binding; actual caller composition is a separate integration step.

## Verification and remaining limits

Target verification used existing `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, x86 base00400000. Fresh live and installed PE
bytes agree over all66 bytes and32 instructions. Strict MSVC Win32
`/std:c++17 /EHsc /MD /W4 /WX /permissive- /fp:strict /O2` compilation passes.
The entire COFF function section equals Native, with no relocations, addends,
masked operands or unaccounted bytes: three conditional branches, two RET4
exits, no direct/indirect calls, no absolute data operands and no FP operation.

One ignored fixture executes a protected executable copy of the complete
installed66B alongside the new public Source entry. Seven finite input cases
cover zero/signed-negative count, null slot0, first duplicate equality,
unsorted last-physical fallback, no qualifying key and signed extreme keys.
Its final case points group+10h at the ACTUAL incoming layer stack cell. Other
group words overlap the leaf's saved-register/return-address storage; the
fixture only compares the borrowed integer result while the key is live and
never dereferences the returned temporary record. This is a raw readable-cell
alias witness, not a Native group factory or general C++ object lifetime claim.

The fixture checks Golden result agreement, actual argument-word contents,
unchanged manager cells and RET4 stack balance. Manager/ordinary group backing
uses the genuine current heap. It passes under strict /MD compilation and
`/link /MANIFEST:EMBED`. There is no handwritten lookup reference algorithm,
provider substitute, tracked test target or broader test suite.

Ignored reproduction helpers under `local/goal15l_avoid_manager_group_lookup/`:
`compile.cmd`, `verify_bytes.py`, `manager_group_lookup_probe.cpp`, and
`run_probe.cmd`. The byte verifier accepts a registered object path as argv1.
`run_probe.cmd archive` links the final primary archive instead of the overlay
object; an optional argv2 overrides that archive path. Primary registration,
full build and registered-archive checks remain pending at this worker commit.

The reviewed Ghidra hypothesis name is retained and old evidence is appended.
No worker Ghidra, CMake, registry or old projection changes were made. The
exact standalone body and bounded Golden execution do not establish Native
class ownership, whole caller adoption, fault-instruction-address identity,
private CRT/frame identity elsewhere, whole-game ABI or gameplay validation.
