# Raw node detach frontier BI

BE9C40 and BF03E0 are ready for a bounded source packet within the established
D642C0 memory and D691B0 physical profiles. Both concrete seek providers already
have actual raw source. This discovery changes no C++, Ghidra state or runtime.
The baseline is e37775a4; the completed scalar worktrees remain unchanged.

| Routine | Coverage | Native ABI and result |
| --- | --- | --- |
| BE9C40..BE9C85 | complete | ECX actual wrapper; no stack arguments; RET; ESI saved. No Boolean result contract; final EAX is reader pointer. |
| BF03E0..BF03F4 | complete | ECX actual reader; stack low distance and ignored pointer; RET8. Forwards provider EAX without checking it. |
| BEF540..BEF57C | complete | ECX raw memory stream; stack low/high/origin; RET0C. High ignored; EAX old selected base, EDX new cursor. No Boolean result. |
| BF4F20..BF4F3F | complete | ECX raw physical stream; stack low/high/origin; RET0C. EAX full SetFilePointerEx BOOL. |
| BE9DF0..BE9E75 | partial semantic comparison: BE9E13..BE9E3A attachment branch only; whole body bytes/listing retained | ECX actual node, RET. Tail calls at BE9E4B/52/61 are recorded by address with unread contracts; this is not a destructor reconstruction. |

BE9C40 loads node=[wrapper], then parent=[node+0C]. If parent is nonnull it
subtracts the child's entire declared length at node+1C from parent+20, modulo
DWORD. This debit happens before examining remaining length or reader state.
It is not the child's remaining length, not the actual seek distance completed,
and not a debit guarded by node+8 being attached.

If node+20 is nonzero, it captures reader=node+8 and calls BF03E0 with
(low=remaining, ignored=&local_zero). The local is zeroed before the call but
BF03E0 never reads or writes that pointer: it reloads stream=[reader], reads the
current vtable slot1C, and invokes it with (low, high=0, origin=1). Its RET8
consumes both caller stack arguments. The provider RET0C consumes its three
seek arguments. BF03E0 checks no return register or OS error and does not debit
any budget itself. BE9C40 ignores its result and writes node+20=0 after normal
return, including a physical FALSE return.

For both initially-zero and sought remaining lengths, BE9C40 next reloads
reader=node+8, decrements reader+60 by one with wrapping DWORD arithmetic, then
clears node+8. It does not release the wrapper, decrement node references, free
the node, clear its parent, change declared length, or erase path-array cells.
No null-wrapper/node, attachment, extent, underflow or duplicate-detach guard is
present. A second call can debit the parent again before faulting through the
already-cleared reader. A fault before normal seek return leaves earlier parent
debit in place and does not reach remaining/path/attachment writes.

The destructor's attached branch is different: BE9DF0 first tests node+8. Only
when attached does it debit the parent by declared length, decrement reader+60,
and clear node+8. It never calls BF03E0 and does not clear node+20. Thus explicit
detach advances the stream over remaining payload, whereas this destructor
branch merely updates accounting/attachment. Calling explicit detach from the
destructor would add native-absent seek behavior. Its later 419CC0/BD1510/BD30F0
calls are outside this comparison and have not been assigned host-method names.

Actual profile D642C0 slot1C at D642DC contains BEF540. Existing
`src/native_filestore_open.cpp:native_memory_stream_seek_00bef540` is the full
naked machine body, declared void but leaving native EAX/EDX. Origin0 selects
backing+8 data+8; origin1 selects stream cursor+10; every other origin selects
end+0C. It adds the low DWORD unchecked and writes cursor+10; high is ignored.
`dispatch_native_memory_stream_seek` already qualifies the current memory
profile/slot through borrowed `NativeRetainedMemoryOwnerContext` words. Typed
`MemoryStream::seek_00bef540` instead checks backing and bounds and returns a
Boolean; it cannot replace this raw path. The internal `memory_seek_result`
helper in `src/native_vfs_runtime_bindings.cpp` demonstrates capturing EAX from
the existing void-declared raw leaf when a source interface needs that result;
it is not an exported additional provider.

Actual profile D691B0 slot1C at D691CC contains BF4F20. Reuse
`src/native_physical_stream_conversion.cpp:seek_native_physical_stream_00bf4f20`.
It loads handle+8, passes low/high distance unchanged and the address of the
actual cached64 position at +10 directly to SetFilePointerEx. There is no handle
check, error translation, manual cached-position update, or size update. The OS
owns output writes, including failure behavior. The PE import directory
independently identifies IAT CE22D8 as KERNEL32.dll!SetFilePointerEx. No extra
physical service context is required by this existing seek leaf.

The smallest source packet owns only BE9C40 and BF03E0 plus new
`include/bsp/native_raw_node_detach.hpp`, `src/native_raw_node_detach.cpp`, and
its evidence files. It can borrow existing memory-profile context and qualify
the current physical profile words at their actual address, then call the
existing concrete seek source. Do not inject a generic seek stub or call numeric
original words as host pointers. Keep context-bearing interfaces distinct from
the original ABIs. If BF03E0 exposes its forwarded result, preserve memory EAX
as old base and physical EAX as full BOOL; never normalize both to success.
The sole observed BF03E0 caller ignores it. No new seek-provider reconstruction
is required. Name reading, child construction and intrusive destruction are
separate packets; this does not make the full hierarchy producer ready.

All19 direct BE9C40 call sites and the sole direct BF03E0 site have live body
attribution and bounded argument/return excerpts in the report. None tests a
detach return value. The two provider virtual rows are separately supported by
complete profile-byte parity; the call checker cannot resolve them. Destructor
tail rows and the symbolic OS-import row are included without claiming their
callee bodies were reconstructed here. Complete live/PE parity covers all five
listed function spans, both4Ch profile prefixes and the IAT word. Current source
hashes, scripts, call verifier output and all local evidence are retained. The
report inventories the whole local tree twice with sizes/SHA256/SHA512 outside
that tree. No build, probe, Ghidra mutation or gameplay validation was performed.
