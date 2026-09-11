# Actual resource-registry lookup discovery

This read-only packet proposes **one complete qualified source entry, B19E90
(108 bytes)**. Its tree lookup and both concrete creators now have complete
actual-storage source. B1B730/B1B810 still require a registry lifetime binding;
they are not replaced with a prepopulated registry, a fast-path-only getter, or
a callback. The consumer B1A4F0 remains outside this packet.

The worktree is based on main `49b628a2`. Evidence is 36 fresh guarded spans,
1,353 bytes, plus 13 saved-entry/neighbor queries and current source pins.
Queries verify `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe` before
each live operation. Bytes match the installed PE; F8D41C is separately marked
as the PE loader's zero-initialized virtual tail, not a live-null observation.
No source, Ghidra, shared metadata, build, probe or game execution was performed.
The prior eight-leaf sealed bundle remains untouched.

Main's string source/header gained an adjacent actual BE0A30 copy adapter after
this requested base. That fragment is not reached by the proposed lookup;
41DD40, 443D00 and the selected leaf source are unchanged. Both string-source
versions and their finite diffs are pinned, without merging or editing them.

## Complete current lookup

B19E90 occupies `[00B19E90,00B19EFC)`. ECX is the actual registry, stack+4 is
the actual name header, EAX is the returned resource, RET4. It never reads the
registry vtable and does not copy, normalize or destroy the name.

1. Capture the name and compute tree = registry+4. Call the now-complete actual
   B19D60 with a local two-DWORD `{owner,node}` output.
2. Read output.owner, then capture **current tree+4 head before** invoking
   returning BF6713 for null/mismatched owner. Read output.node only after that
   possible handler. Compare the node against the captured head; equality
   returns zero.
3. On a hit, validate the captured output.owner again for nonnull, then compare
   the captured node with **current output.owner+4 head**. Each failing check
   invokes returning BF6713 and continues. Do not reuse the earlier head or
   invent a null/failure return after validation.
4. Read current node+14 factory, then its current vtable and vtable+4 function.
   Invoke that function with ECX=factory and no stack arguments. Forward EAX
   unchanged. No factory/resource retain, release, name cleanup or post-call
   read follows; the body has no EH registration.

Actual registry storage is 10h bytes: profile+0, preserved tree word+4,
head+8, count+C. The tree's head/root and 1Ch node/key/value layout are already
established by the prior discovery and completed raw B19B90/B19D60 source.
Their existing 443D00/CRT dependency is selected, not duplicated.

## Concrete factory profiles and lifetime limits

| Current factory identity | +0 deleting entry | +4 selected creator |
| --- | --- | --- |
| D64470 | BBC650 | BBC6F0 |
| D644AC | BBC7A0 | BBC810 |
| D644E8 | BBC890 | BBC6F0 |
| D644F0 | BBC8E0 | BBC810 |

The prior full BBC900/BBC5F0/BBC740 evidence establishes the actual allocations,
registration and final D644E8/D644F0 writes for CausticsTextureSource and three
ShoreWaveTextureSource keys. Fresh xrefs confirm those writers. Both initial
registration profiles and both final profiles select the two completed
creators. Their actual profile words reside in PE `.rdata` with characteristics
40000040 (readable initialized data, no write flag). This is original loaded
profile evidence, not an observed game-process mapping or permission test.

The full final factory deleting leaves BBC890/BBC8E0 are each 31 bytes: capture
flag bit0, retain the incoming address, write base identity D64468, optionally
free the captured allocation through BF65AC, return that address, RET4. They
do **not** unregister a registry entry. B19E90 consequently requires actual
existing factory objects to remain alive through the lookup; it does not own
or extend their lifetime. Base D64468 selects pure-virtual BF698E at +4 and is
outside the proposed live lookup domain. Base-profile deleting entries
BBC650/BBC7A0 are not invoked or newly reconstructed by this packet.

The two returned resources are allocated and fully constructed by existing
BBC6F0/BBC810 -> C30470 -> B19980 source, with count+4=1 and final
D64478/D644B4. Their exact allocation failure and constructor unwind boundaries
are in the pinned eight-leaf audit. B19E90 has no additional ownership action.
Later resource destruction (BBC6D0/BBC7F0 -> C304A0) remains a separate named
incomplete route; successful lookup does not establish it.

## Current getter and its remaining ownership closure

B1B730 is exactly 200 bytes `[B1B730,B1B7F8)`, no native inputs, EAX=current
publication, RET. Its first F8D41C read is a captured fast-path return. On a
miss it calls 415350, captures manager+10's native critical-section pointer,
forms a raw CE37FC guard, enters/increments physical section+18 if nonnull,
then arms state0. An entry failure before that point has no armed guard cleanup.

Under the captured section it rereads F8D41C. If still zero, allocate10h, capture
the allocation, arm state1, call B1AA70 when nonnull, and write D5E59C. Disarm
allocation cleanup back to state0 **before publication**. Publish the captured
result (including zero), call 415350 again, and pass the **current F8D41C** to
BD0C30 registration. A registration failure leaves the published owner and
sentinel intact and runs only guard cleanup. No rollback unpublish/free is
present. Normal exit decrements/leaves the captured section while state0 remains
armed, then rereads F8D41C for the return value.

Its CBC7C3 handler references FuncInfo DF4B38/map DF4B28. State1 action CBC7B8
frees `[EBP-18h]`, then state0 action CBC7B0 destroys the raw guard `[EBP-14h]`
through already-complete 411EE0. That helper uses the actual physical section,
not the SystemSingletonCriticalSection projection pointer. The existing
canonical lifetime/diagnostic source demonstrates the required bridge; no
second lifetime manager is needed or proposed.

B1AA70 is 104 bytes and B19C30 is 55. The former installs D5E594, allocates a
1Ch sentinel, stores current head, sets nil+19=1, rewires current head parent,
left and right to itself, and zeros count. It preserves registry+4. B19C30
performs its separate pointer/wrapped-field guards then color/nil stores; it
does not become a safer nullable allocator. Constructor handler CBC6B8,
FuncInfo DF4994/map DF498C, invokes CBC6B0 -> B19760 on failure. B19760 is a
full 17-byte leaf: **unconditionally clear F8D41C, then write this+0=CE3818**.
That occurs before the outer getter frees its captured allocation and unlocks.

Actual D5E59C slot0 is B1B710, whose complete 30-byte scalar deletion calls
B1B5F0 then optionally frees this and returns the captured address. B1B5F0 is
**111 bytes**, including a currently hidden returning-free tail. It passes
captured begin/end iterators to B1A2F0, frees the current sentinel, zeros current
head/count, clears F8D41C, then resets this profile to CE3818. It does not reset
the profile before tree cleanup and does not call BCFCA0 unregister. Its EH
action CBC770 also invokes B19760. A canonical lifetime binding must dispatch
the full actual current owner deletion; that body is not yet implemented.

B1A2F0 (201 bytes) has both complete-range and partial-range paths. The former
uses B1A260; the latter reaches B19890/B19F90 and its rotation/validation
children. Do not discard the latter because the destructor normally passes
begin/end. Full B1A260 (82 bytes) recursively destroys right subtrees, captures
left before the current key return, frees the node, then follows left after
returning free. It never reads or deletes factory value+14. Tree population is
still separate; this packet does not investigate it further.

B1B810 is a missing saved function but a complete 20-byte body
`[B1B810,B1B824)`: call B1B730, then pass its returned registry and the current
stack name to B19E90; ECX/options are unused, EAX forwarded, RET8. It cannot be
implemented honestly until the full getter/lifetime route is available.

## Exact next source proposal

Four new files: `src/native_resource_registry_lookup.cpp`,
`include/bsp/native_resource_registry_lookup.hpp`,
`docs/NATIVE_RESOURCE_REGISTRY_LOOKUP.md`,
`reports/native_resource_registry_lookup_audit.json`. Only **B19E90 / 108 bytes**
is proposed as a new full entry.

Its API accepts the actual existing registry and name, plus a context borrowing
the actual four immutable profile storages and the established returning CRT
boundary. After the original validation sequence, resolve the factory's current
native profile identity to that actual storage, read only its current +4 word,
and dispatch BBC6F0/BBC810 directly to their existing concrete source. Do not
read the registry vtable or add factory/refcount/profile-header validation.
Other profile identities or slot words are explicit **source-domain limits**,
not native exception behavior. Retain the original profile identities; create
no substitute tables or provider callbacks. B1B730, B1B810, registry/factory
deletion, population and B1A4F0 remain outside that source packet.

Saved-analysis repairs are reported for future primary action only: define
B1B810 through B1B823, preserve separate B1B800's five-byte jump and intervening
padding; extend B1B5F0 through B1B65E after repairing only its B1B633 free flow;
repair B1A29C and B1B720 returning-free flow without changing their already-full
body ranges. Preserve correct `_free` naming, old comments and neighboring
functions. No worker changes were applied.
