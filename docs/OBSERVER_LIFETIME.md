# Shared observer and callback-owner lifetime

The packet reconstructs the shared pair-unregister and callback-owner destruction
sequence used by `005BC640` attached-voice cleanup. The current caller index lists
283 callers of `006952A0`. Names beginning `BSP_` below are descriptive hypotheses,
not recovered symbols. The existing compiler identity
`CG_scalar_deleting_dtor_00694ea0` is preserved. `00694D30` is treated as the standard
remove algorithm specialization and bound to host `std::remove`.

The verified target is `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, through the configured loopback8089 bridge. All live
reads used `tools/bsp.py`. The only Ghidra writes were explicitly authorized,
locked CALL-return flow repairs, recorded with prior states and saved by
`tools/ghidra_flow_repair.py`. Each affected export was refreshed.

## Actual storage and integration

`include/bsp/observer_lifetime.hpp` defines actual Win32 storage with size/offset
assertions. Both endpoint bases have a native vtable at `+0` and an edge-pointer
array at `+4`, DWORD count at `+8`, DWORD capacity at `+C`. An edge has its current
vtable at `+0`, first endpoint at `+4`, callback owner at `+8`, and reference count
at `+C`. These arrays hold the actual shared edge addresses. No second graph,
copied reference count, or lifetime map is used.

At `005BC87C`, native forms callback-owner address `entry+18`; `005BC88B` writes
that base's vtable, `005BC891` loads its entity field from `base+14` (`entry+2C`),
and `005BC89D..89F` passes ECX=that entity address, EDX=that exact base address.
`005BC8A4..8AB` then passes the same base in ECX to `00695870`. Thus the entity
adapter must return the first endpoint at the address already represented by
the saved entity pointer; the callback owner must alias the actual embedded
16-byte base. The old semantic `VoiceAttachedEntry::lifetime_vtable_18` DWORD
alone does not provide the required storage. The parent owns that integration.

`NativeObserverLifetime` borrows the existing `SingletonLifetimeDomain`, actual
published lock-owner pointer `00E198E0`, actual dispatch-vector owner pointer
`00E198E4`, and `ObserverLifetimeServices`. The dispatch vector uses canonical
`SingletonPointerSlots` for its actual `+4/+8/+C` begin/end/capacity fields;
its untouched `+0` word is left uninterpreted. It is distinct from endpoint
count/capacity arrays.

The stable caller API is:

```cpp
lifetime.unregister_pair_006952a0(first_owner, callback_owner);
lifetime.destroy_callback_owner_00695870(callback_owner);
```

The only required engine dispatches are
`ObserverLifetimeServices::delete_edge_virtual_00(edge, current_vtable, flags)`
and `invalid_parameter_00bf6713()`. The edge service must call slot zero of the
freshly supplied native table on the same edge, with flag1; it owns deleting
that edge. The invalid-parameter service can return, as native does. A returned
validation call does not restart iteration or substitute new storage.

The real singleton registry's `destroy_registered` callback must bind table
`00CF7E70` to `NativeObserverLifetime::delete_lock_owner_00694ea0`. Registration
and shutdown are the existing `SingletonLifetimeDomain` implementation.
`TrackedCriticalSection`, `critical_section_create_00bd1860`, and
`critical_section_destroy_owned_0041cc80` are reused directly. Each native lock
capture adjusts the actual `+18` depth before/after the same Win32 OS calls.

## Recovered operations and ABI

| Address | Native input/result | Recovered operation |
| --- | --- | --- |
| `00693C50` | ECX=lock owner; RET | Clear published lock owner, write base table `00CE3818`. |
| `00694200` | ECX=raw8-byte storage; EAX=this; RET | Write `00CF7E70`, create tracked critical section, store at+4; constructor unwind calls `00693C50`. |
| `00694280` | No inputs; EAX=published lock owner; RET | Double-checked getter under captured lifetime-manager section, allocate8, construct, publish, re-fetch manager/register, unlock, final global reload. |
| `006944C0` | ECX=left array, EDX=right array; RET | Exchange contents through a copy of the original left array. Keep separate allocations and capacities. |
| `006949D0` | ECX=first owner, EDX=callback owner; EAX=edge or0; RET | Capture lock; choose the smaller unsigned count, equal chooses second; search opposite endpoint identity; release lock. |
| `00694D30` | ECX=first pointer, EDX=end pointer, stack=&value; EAX=new end; RET4 | Standard remove specialization. Host library binding, no library reimplementation. |
| `00694EA0` | ECX=lock owner, flags stack; EAX=original pointer; RET4 | Existing compiler scalar-deleting wrapper: destroy section, clear global, base table, free iff flags&1. |
| `00694F60` | ECX=three-word array, EDX=edge; RET | Remove every matching pointer; native SUB/SAR2 count and resize sequence. |
| `006952A0` | ECX=first owner, EDX=callback owner; RET | Nested pair lookup, invalidate dispatch slots, decrement edge count, remove from both endpoints and delete only at zero. |
| `00695530` | ECX=callback owner; RET | Invalidate its dispatch slots, exchange its array contents into a local empty array, remove/destroy captured edges irrespective of count. |
| `00695870` | ECX=callback owner; RET | Base table `00CE3CD4`, outer lock, separately nested locked count query, detach if captured count nonzero, unlock, reload/free array. |

`006952A0` invalidates matching pending entries even when reference count remains
nonzero. The decrement uses native modulo32 arithmetic. At zero, it captures
both edge endpoint pointers before the first array erase, erases the edge from
both arrays, then reads the current edge vtable before virtual deletion.

The two pending-slot scans capture the original owner, cursor, and address of
its end field. Each loop reloads the published owner and captures its end before
the first validation call. It validates that owner's begin/end and identity,
compares the saved cursor with that saved end, and separately reloads the
original end field before dereference, write, and increment. Returning validation
callbacks retain these exact captured values; the fixture exercises this.

`006944C0` is important to owner destruction: it clears the original owner's
count before the first edge callback while retaining the owner's allocation.
Its reserve sequences store capacity before allocation, reload current data/count
after the CRT new handler can run, and free the old allocation before publishing
its replacement. `00694F60` retains the inline grow/zero-fill branch even though
valid `std::remove` output normally shrinks the array. The native null destination
write guards and DWORD allocation products are preserved.

`00695530` captures the detached cursor/end and walks that captured span across
edge callbacks. Its local array is freed before unlocking. `00695870` frees the
owner's reloaded array after unlocking; native does not clear the resulting
dangling data pointer or capacity. The observed unwind targets `00C7EA30/38`
and `00C7EA90/9B` establish copied-array cleanup and lock-before-member-array
cleanup on exceptions. C++ cleanup mirrors those operations, without claiming
native SEH or drop-in calling-convention compatibility.

The library thunks were checked: `00BF55BE` jumps to allocator `00BF681B`, and
`00BF6989` jumps to `_free` at `00BF65AC`. They use the existing
`singleton_lifetime_allocate/free` boundary. No CRT or standard algorithm is
ported by this packet.

## Analysis state and validation

Seven missing `ADD ESP,4` instructions were recovered at `0069591B`, `0069566F`,
`00694586`, `00694612`, `0069464F`, `00694FD2`, and `00694ECE`. Each is three bytes
`83 C4 04` following a falsely terminating free call. The five functions now
have no remaining CALL gaps. Skipped alignment bytes after unconditional jumps
were left alone. Exact inclusive ranges and final-instruction lengths are in
`reports/observer_lifetime.json`.

No missing game-function start was found. Ghidra prototypes remain inferred
`undefined(void)` at several routines, notably `006944C0`; consequently its
caller `00695530` pseudocode still incorrectly removes the detached-edge loop.
The full assembly includes that loop at `00695630..0069565B`. Parent follow-up
may apply the ABI table above under the normal write lock; this packet did not
change names or prototypes in Ghidra. Correct existing compiler/library tags
were preserved and proposed names are recorded in the ledger.

`./scripts/build.ps1` passed for Release MSVC Win32. Existing CTest
`reconstructed_math` passed 1/1; it does not exercise this observer closure.
One ignored fixture, `local/observer_lifetime_fixture.cpp`, passed using:

```text
cmd /c local\run_observer_lifetime_fixture.cmd
```

The script runs installed `vcvars32.bat`, compiles with `/std:c++17 /EHsc /MD
/W4 /WX`, and links `build/win32/Release/bsp_core.lib`. It uses real Win32 locks,
the real singleton registry and actual pointer arrays; its edge virtual is a
controlled fixture service. It covers repeated unregister/refcount retention,
duplicate pointer removal, pending-slot holes, reentrant cleanup, owner count
detachment, ignored retained references on owner destruction, returning
validation with captured cursor/end, nonempty content exchange, exception lock
cleanup, and concrete singleton shutdown. No permanent tests were added.

The parent still needs to bind actual game edge virtuals, alias the actual entity
and voice callback-owner bases, and provide the live dispatch-vector publication.
Other edge registration/invocation and dispatch-vector construction are outside
this bounded closure. Allocator-failure/new-handler mutation, malformed spans,
concurrent shutdown, native differential behavior, and game runtime/visual
behavior were not fixture-tested. Existing critical-section construction and
singleton allocation policy are inherited from the cited canonical sources.
