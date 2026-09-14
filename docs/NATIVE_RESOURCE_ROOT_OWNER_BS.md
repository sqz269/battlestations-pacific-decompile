# Native structured-resource root and node lifetime (BS)

Addresses: `00BEA380`, `00BEA700`, `00BE9DF0`, `00BE9FC0`.

Four complete ordinary bodies (578 bytes) are reconstructed over actual 24h
node storage in `src/native_resource_root_owner.cpp`. Layout is profile0,
reference count4, borrowed reader8/parentC, owning tag10/14, depth18,
declared payload1C and remaining20. There is no native reader/parent retain.
These are new C++ interfaces; descriptive names are hypotheses.

BEA380 (295 bytes, ECX node, stacked reader, EAX node, RET4) writes base profile
CEB130/ref1, then D68BB4/reader/null parent/empty tag/depth0. It reads a tag and
DWORD payload using local budget1000. Tag copy and final return preserve the
native current/captured field distinctions. The payload is stored in declared
and remaining fields. It then reloads node+8, selects that reader's current
path slot, increments index+60 **before** copying into the slot. No magic,
payload-fit, depth, capacity, truncation or attachment check is added.

BEA700 (119 bytes, ECX reader, stacked output handle, EAX output, RET4) allocates
24h, constructs if nonnull and publishes the result; null allocation publishes
null. The allocation unwind frees only the captured raw allocation. Output is
not pre-cleared. Root-constructor failure does not roll back a path-index write.

BE9DF0 (134 bytes, ECX node, RET) stamps D68BB4 and, only when attached,
subtracts declared payload from a nonnull parent's **current remaining**, reloads
the current reader and decrements its path index, then clears node+8. It returns
captured tag data with current length+1 and stamps base CEB130. **It never seeks
unread payload.** The name header, remaining count, parent and reference count
are not cleared. Detached repeated destruction skips parent/path debit; callers
still require valid name lifetime. BE9FC0 (30 bytes, ECX node, stacked flags,
EAX captured node, RET4) destroys first and frees only flags bit0 after success.

`NativeResourceRootDispatch` borrows the existing concrete stream dispatch and
raw pool. For captured D68BB4/BD30E0 it reads the current deleting slot4 and
binds BE9FC0 with flag1; other stream calls forward unchanged. It composes the
existing BE9ED0 actual handle release, without another reference-count domain.
Alternate reached deleting targets throw; original numeric code is never called.

The three full EH handlers/maps are recorded in the report:

* BEA380: CC719B -> E01BD0/map E01BB8. State2 ->1 destroys current temporary
  through CC7193 (EBP-14); state1 ->0 destroys node tag through CC7188;
  state0 ->-1 stamps base through CC7180. Saved node is at synthetic EBP-18.
* BEA700: CC721B -> E01C90/map E01C88. State0 ->-1 invokes CC7210 to free the
  captured allocation at EBP-10. Its POP ECX/RET tail was outside the old
  function membership and is now repaired and verified.
* BE9DF0: CC70A8 -> E01A90/map E01A88. State0 ->-1 invokes CC70A0 to stamp base
  using saved node EBP-10. Normal base destruction disarms this action first.

Source C++ exception ordering follows these actions, with termination on a
second unwind exception. Original FH3/SEH/CRT identity, private stack/EH aliases
and hardware faults remain unproved. No failure-mode fixture is claimed here.

One fixture composes actual pool, memory stream, structured reader, root
creation, path tag, BE9ED0 decrement/current virtual deletion, and reader cleanup.
It verifies stream counters return to zero and releasing a root with unread
payload leaves the stream cursor unchanged. A supplied child-storage case
verifies declared-parent wrap-debit and detached skip; it does not claim child
construction. The fixture was corrected to use memory-stream cursor/end
**addresses**, not offsets, and to bind mutable data separately from the rdata
mapping service. The production load/root-dispatch path is not wired by this
packet; original binary ABI and gameplay remain unverified.
