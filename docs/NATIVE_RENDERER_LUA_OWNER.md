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

The native constructors/destructors use FH3/SEH frames. `00B1BB90` arms
cleanup state 0 after the base returns and state 1 after `00B66BD0`; the
corresponding source `try` scopes close Lua and then unregister the base if
opening throws. `00B1BA30` arms its captured guard state after section entry;
`00B1BAD0` similarly holds that guard through unregister and publication
clearing. `00B1BBF0` arms base cleanup during Lua close. C++ catch and RAII
preserve this normal exception order but do not reproduce native FH3 stack maps,
SEH hardware-fault behavior, register-spill aliases, or original callable ABIs.

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
These tests do not execute the new owner. The new context interface is
not a binary replacement, and no game startup, singleton drain, or renderer
script behavior has been validated here. Ghidra functions remain unrenamed in
this read-only worker session; the address/name ledger gives proposed names
for later locked annotation.
