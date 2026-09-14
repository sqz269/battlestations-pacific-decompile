# Renderer Lua cache owner

`00B32410` allocates `0x4CC` bytes at `00B327FB`, calls `00B1BB90`, and discards
the returned pointer. The object is independently registered with the native
singleton lifetime manager, so its lifetime is held through the publication
cell at `00F8D434`. The new C++ interface accepts a caller-owned raw allocation
and explicit references to that cell and the actual manager publication at
`01090AA0`. It does not own or replace either global.

The live `C:/Users/sqz269/bsp.gpr` program `/battlestationspacific.exe` and
saved binary listing show the following native sequence:

| Entry and exact span | Original ABI | Established behavior |
| --- | --- | --- |
| `00B1BA30..00B1BAC0` (145 bytes) | ECX raw owner; EAX same; RET | Set base vtable `00D5E5A4`. First `00415350` returns the actual manager; capture its `+10h` section, enter it and increment its `+18h` depth. Publish `this` at `00F8D434`. Call `00415350` a second time, reload the current `00F8D434`, and call `00BD0C30`. Decrement and leave the captured first section. |
| `00B1BB90..00B1BBEB` (92 bytes) | ECX raw owner; EAX same; RET | Call base. Set derived vtable `00D5E5A8`, pass `ECX=this+4` to `00B66BD0`, then `ECX=this+4` and stack mask `1` to `00B6A020`. The embedded Lua owner is the full `0x4C8` bytes; no other member fits in the `0x4CC` allocation. |
| `00B1BAD0..00B1BB68` | ECX owner; RET | Restore base vtable, capture first manager section as above, call `00415350` again, reload `00F8D434`, call `00BCFCA0`, clear `00F8D434`, release captured section, and set final base vtable `00CE3818`. It unregisters the *current publication*, even if that differs from `this`. |
| `00B1BBF0..00B1BC41` | ECX owner; RET | Set derived vtable, close the embedded Lua owner at `+4` through `00B669A0`, then call base destructor. |
| `00B1BB70..00B1BB8B`, `00B1BC50..00B1BC6B` | ECX owner, stack low-byte flags, EAX original pointer, RET 4 | Call respective destructor; if flag bit 0 is set, free original allocation through CRT free `00BF65AC`. |

The native constructors/destructors use FH3/SEH frames. Their handler stubs
load these `FuncInfo` records; each map entry is `(toState, action)`. The first
three records have `maxState=2` and map transitions `0 -> -1`, `1 -> 0`;
`00B1BBF0` has `maxState=1` and transition `0 -> -1`:

| Entry | Handler / FuncInfo / unwind map | State 0 action | State 1 action |
| --- | --- | --- | --- |
| `00B1BA30` | `00CBC850 / 00DF4C18 / 00DF4C08` | `00CBC840` loads the saved owner and jumps to `00412430` (write `00CE3818`) | `00CBC848` loads the captured guard and jumps to `00411EE0` (depth decrement and leave) |
| `00B1BAD0` | `00CBC870 / 00DF4C4C / 00DF4C3C` | `00CBC860` -> `00412430` | `00CBC868` -> `00411EE0` |
| `00B1BB90` | `00CBC893 / 00DF4C80 / 00DF4C70` | `00CBC880` -> `00B1BAD0` | `00CBC888` adjusts the saved owner by `+4` -> `00B669A0` |
| `00B1BBF0` | `00CBC8A8 / 00DF4CAC / 00DF4CA4` | `00CBC8A0` -> `00B1BAD0` | none |

For the base constructor, state 0 is armed at `00B1BA50` **before** the first
`00415350` call; state 1 is armed at `00B1BA81` after section entry. The base
destructor similarly arms state 0 before its first getter and state 1 after
entry. Source cleanup guards call the established raw `00411EE0` and
`00412430` providers in reverse state order when a C++ exception escapes.
The derived constructor closes Lua and then destructs the base if open throws;
the derived destructor destructs the base if Lua close throws. A second C++
exception escaping a cleanup terminates rather than replacing the in-flight
exception. Native FH3 stack maps, SEH hardware-fault behavior, register-spill
aliases, and original callable ABIs are still outside this source interface.

`NativeRendererLuaOwnerStorage` stores only the vtable DWORD and the native
`NativeLuaStateStorage` at `+4`, with static size/offset checks. Existing raw
providers implement `00415350`, `00BD0C30`, `00BCFCA0`, `00B66BD0`,
`00B6A020`, and `00B669A0`. The vtable DWORDs are identities; the source code
does not create callable C++ vtables. Other Lua owner bytes, including native
slot pointer preimages, are left to their existing constructors and retain the
original preservation behavior. The parent renderer allocation remains in
`00B32410` and is integrated separately.

The source passed strict Win32 `scripts/build.ps1`, eight native seed matches,
and the existing `reconstructed_math` and `native_math_differential` CTests.
The follow-up owner fixture covers the additional source exception path; see
the report for its scope. Its Win32 probe uses the actual raw manager and real
Lua5.1.1 state with explicit fixture-only fundamentals/DoFile callbacks. It
checks normal lifetime, current-publication unregister, and an injected string
allocation failure during Lua open. This does not execute the original game
body or inject a manager-getter, registration, or cleanup-throw failure. Those
exception edges are established from the native map and source guard order.
The new context interface is
not a binary replacement, and no game startup, singleton drain, or renderer
script behavior has been validated here. Ghidra functions remain unrenamed in
this read-only worker session; the address/name ledger gives proposed names
for later locked annotation.
