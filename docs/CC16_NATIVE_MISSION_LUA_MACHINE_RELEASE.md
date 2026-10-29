# Raw mission Lua machine release: 008844F0

This packet supplies the complete normal raw parent at
`008844F0..0088450A`: **27 bytes / 12 instructions**, one indirect current-slot
call and no direct calls. It is based on published
`5ec9904f55e48f5d3cd871444b510b5cb5e1532b`; later origin commits are excluded.
The existing semantic `MissionLuaHostState` API and its callers are unchanged.
No native/vendor/activation body credit is added.

## Boundary, ABI and null contract

Bounded target-verified queries used `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, x86 LE32, image base400000. Fresh live bytes agree
with the installed PE and all 12 exported instruction addresses. The body ends
at RET `0088450A`; `0088450B..0088450F` are five INT3 bytes. The older pseudocode
plate's `..0088450E` description is not the actual instruction boundary.

Native ECX is the host, captured in ESI. It takes no stack arguments, preserves
ESI and returns with plain RET. Native code unconditionally reads DWORD host+4:
there is **no host-null guard**. The actual OnDestroy caller at 004DC617 is
likewise unconditional. A null host is not an admitted normal input and is not
converted into a successful empty operation.

Only the captured machine pointer is tested. If it is null, the function returns
without a call or a write. Otherwise it reads that machine's current primary
table and byte slot0, pushes flag1, calls it at 00884500, then writes zero to
the same retained host+4 at 00884502. A callback replacement of host+4 is also
discarded by the post-return clear. The child is not read after it may have
deleted itself. A throwing/nonlocal callback does not reach this clear; no
rollback or recovery is fabricated.

The native function proves access to DWORD+4, not a complete C++ host class.
The caller must supply genuine valid raw storage through the callback and clear.
This implementation invents no 14h host/container type or construction policy.

## Genuine provider composition

The new C++ interface is:

```cpp
void release_native_mission_lua_machine_008844f0(
    void* actual_host, NativeGameCurrentVirtualCalls& actual_dispatch);
```

`NativeGameCurrentVirtualCalls` is an existing concrete final source component.
The new helper explicitly qualifies its real `virtual_scalar(captured,0,1)`;
there is no new abstract callback or successful empty default. That method
directly composes existing
`NativeGamePhysicsLifetimeCalls::physics_virtual_scalar`, which reloads the
captured receiver's actual current table and method, sets ECX to that receiver
and invokes the method with the complete flag word. No method is synthesized or
selected by casting an original PE address into a host code pointer.

The known original LuaMachine profile at CF8394 has one entry, 006B87C0; its next
DWORD is the `nSlu` text, not another method. That scalar closes its state when
its ownership byte is set and conditionally frees through BF65AC for flag1.
These are native identity/effect facts, not a callable-table binding installed
by this packet. An actual child constructor, callable table/code lifetime and
matching deletion heap must already be established. The source LuaMachine
class's existence alone does not admit an arbitrary raw pointer as that class.

The helper reads and clears a volatile DWORD using Win32 wrapping address
arithmetic. It does not cast to `MissionLuaHostState`, `LuaMachine`, a guessed
native class or a container. It supplies no allocator or table, touches no other
host field, and does not free the host. The mandatory dispatcher reference must
be a real retained component, and the child table must be genuinely callable.

## Strict object and call/stack evidence

The new source and both actual provider `.cpp` files passed MSVC Win32
`/c /std:c++17 /O2 /Ob2 /DNDEBUG /MD /EHsc /W4 /WX /fp:strict` compilation.
Independent COFF inspection records the 38-byte public helper, 8-byte concrete
dispatcher and 22-byte physical current-table dispatcher.

The helper's emitted load is unconditional `[host+4]`. Its machine-null branch
skips the only call and clear. The nonnull path pushes flag1, slot0 and the
captured machine, then directly references the real concrete dispatcher symbol;
the clear follows that call. There is no inline FP arithmetic or control change
in any of the three inspected functions.

MSVC correctly adjusts the source-service `this` pointer by +4 at this call,
and the existing delegate adjusts it by +18h before its tail jump to the physics
base method. These are compiler-owned service offsets, not native host fields.
The final dispatcher replaces ECX with the captured machine, loads current vptr
and method at slot0, pushes the flags word, and makes the actual indirect call.
It returns with RET0Ch for its three source arguments; the child method's
ordinary thiscall contract pops its single flag word with RET4. The public
helper retains/restores its host register across this chain before clearing+4.

The original has no direct native callee row to validate as a direct CALL.
The CALL EDX bytes/site and the real source-provider relocations and instruction
operands were inspected explicitly. The report does not falsely present a
known native profile value as a forced direct target.

This is a new typed C++ interface, not an ECX-entry/binary ABI replacement.
Incidental native EAX/flags, physical frames, invalid-storage fault identity,
FH/SEH, foreign exceptions/Lua nonlocal errors and concurrent/reentrant teardown
are not established. No fixture, executable, test, game, probe, DLL or original
game state was run or changed by this packet; only compile/static evidence is
claimed.

## Remaining adoption work

Root must register/build/link the new source and bind its real component into
the whole OnDestroy `call_008844f0` site using that site's actual raw host.
That binding must retain genuine child constructor/table/heap custody and the
host through the post-call clear. It cannot hide an unconstructed/null host
behind an added guard or substitute a semantic C++ container view.

Raw host construction and its later host-scalar deletion at 004DC62C remain
separate responsibilities. This body closes one normal source operation; it
does not complete the other OnDestroy providers, create a world/game/host,
activate a shutdown path or promote any frozen runtime receipt. Shared CMake,
ledger/Ghidra annotations and source caller adoption are intentionally untouched.
