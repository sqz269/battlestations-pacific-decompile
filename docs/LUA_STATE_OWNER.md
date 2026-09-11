# Lua state owner and cached fundamentals

Addresses: `00b6a020`, `00b65e80`, `00884770`, `00b68340`.
Supporting lifetime evidence: `00b67980`, `00b66de0`, `00b67700`,
`00441210`, `00441a20`, `007fefe0`.

`PcStorageLuaOwner` implements the existing `PcStorageLuaHost` boundary with
stock Lua 5.1.1. Its name describes an inferred role, not a recovered symbol.
The original owner layout and register ABI are not reproduced. Original Lua
library names are retained. Bootstrap and close have concrete implementations;
the game VFS, singleton lifetime manager, and `DoFile` script-with-overrides
operation remain required external contracts.

## Open and libraries

`00b6a020..00b6a356` is `__thiscall(owner, uint32 mask)`, `RET 4` at
`00b6a354`. It sets `owner+0` to one, creates the interpreter with
`luaL_newstate` (`00a6a260`), stores it at `owner+4`, and installs panic
callback `00b669c0`. Owner construction and its stack-object bookkeeping are
separate operations; the storage view embeds this owner at `storage+38h`.

The `{name, opener}` table at `00d62bb8` has eight entries. The loop at
`00b6a058..00b6a08f` opens base even when bit zero is clear, then opens each
remaining library only when the corresponding bit is set. Each call pushes
the opener, passes the library name, and executes `lua_call(L,1,0)`.

| Bit | Name | Native opener |
| --- | --- | --- |
| 01h | empty string / base | 00a67210 |
| 02h | package | 00c2ff90 |
| 04h | table | 00a66010 |
| 08h | io | 00a652b0 |
| 10h | os | 00a64190 |
| 20h | string | 00a63910 |
| 40h | math | 00a61c50 |
| 80h | debug | 00a61620 |

The initial stack top goes to `owner+8` at `00b6a0a3`, before bootstrap
scripts. `PC=true`, then `X360COMP=true/false` selected by byte `0108ff20`,
run through `luaL_loadstring` and, when compilation succeeds,
`lua_pcall(L,0,LUA_MULTRET,0)`; return statuses are ignored. Region string
`0108ff28` is compared case-insensitively with EU, USA, and JAP. A matching
canonical `REGION` assignment runs; null or an unknown string adds no global.

At `00b6a303..00b6a31e`, global `DoFile` receives native callback `00b69e00`.
That callback borrows the current state and calls the existing
`00b69d40` script-with-overrides operation. The reconstructed owner requires
its callback explicitly; supplying a bare `luaL_dofile` would omit game VFS
and override behavior. The C++ callback carries an extra host context via a
closure upvalue, an interface adaptation. It must not throw C++ exceptions.

Finally, `00b6a323..00b6a348` retrieves the fundamentals singleton twice,
takes size from `+8` and bytes from `+4`, calls `luaL_loadbuffer` with chunk
label `Scripts\\fundamentals.lua`, ignores load status, and calls
`lua_call(L,0,LUA_MULTRET)` unprotected. Assembly was necessary because the
decompiler loses this executable's register arguments.

## Fundamentals source

`00884770..0088482c` takes no arguments, returns the singleton in EAX, and
uses global `0108ff1c`. Its first-use path locks the singleton lifetime
manager, allocates 0Ch bytes, invokes constructor `00b68340`, and registers
the object for lifetime cleanup. This lifecycle is documented rather than
reimplemented by the storage owner.

`00b68340..00b68458` takes `this` in ECX, returns it in EAX, and ends in
`RET`. Its vtable is `00d62c18`; `+4` is a byte allocation and `+8` is its
size. At `00b683a0..00b683c4` it opens `Scripts\\fundamentals.lua` through
VFS singleton `0109ceec`, vtable `+4`, mode 2; `+18h` checks the stream.
On success, `+30h` gets the size, allocation is stored at `+4`, and `+24h`
reads the bytes. The stream reference is decremented and released at zero.
The read-result/actual-byte-count output is not checked here. The failure
branch does not establish valid byte/size members. The VFS cache projection rejects
missing or incomplete reads. A successfully opened zero-length file is valid:
there is no nonzero-size gate before allocation/read or `luaL_loadbuffer`.

Thus the bytes are cached VFS content, not an embedded executable resource.
Owner open uses the cache, while the cache's first creation does read VFS.
`LuaStateOwnerEnvironment` copies supplied bytes for reliable host lifetime;
it does not simulate the global singleton or extract an archive by itself.

## Close and reader lifetime

`00b65e80..00b65e9c` takes `this` in ECX, `RET`. It calls
`lua_close` (`00a68a90`) immediately when state `+4` is nonzero and owns byte
`+0` is set, then clears state `+4`. It does not walk or invalidate every
LuaObject. The concrete class always owns its newly created state; borrowed
native wrappers are outside this class's contract.

Profile completion `007fefe0` closes the storage owner before destroying
reader `00441a20`. The reason the balanced global-root reader can survive is
specific: `00b67980` creates a 14h-byte object with owner at `+0`, kind one at
`+4`, `LUA_GLOBALSINDEX` (-10002) at `+8`, and tracked byte `+10h` **zero**.
The reader destructor tidies its object vector (`00441210`) through
`00b67700`. That destructor calls release helper `00b66de0`; assembly
`00b66de6..00b66dec` tests object `+10h` and jumps directly to the return at
`00b66f2b` when zero. No Lua API or closed interpreter is touched for this
root. Tracked child handles must already have been released by balanced
traversal. This does not prove arbitrary live child references survive close.

The optional `before_close` observer supports a host that uses stock Lua
registry references. It runs while the interpreter is valid and may retire
those references. This is an explicit host adaptation, not evidence of native
blanket reference invalidation. Its context must remain alive and it must not
throw, close the owner recursively, or perform another state close.

## Host safety boundary and validation

Native bootstrap script failure reaches an unprotected call/panic. The host
wraps bootstrap in a protected call, closes on failure, and throws the Lua
error to its caller. Native constants still use their original protected
calls; fundamentals and library opening retain their native call shape
inside the outer host boundary. Reopening a live owner is rejected instead
of reproducing the native pointer overwrite/leak. These are explicit host
differences, not game compatibility claims.

Validation records are in `reports/lua_state_owner.json`. A single ignored
fixture in `local/lua_state_owner_fixture.cpp` reads the installed 657-byte
fundamentals file without modifying or copying it into tracked source. It
checks actual platform functions, selected/absent libraries, case-insensitive
region normalization, registry retirement before close, repeat close/reopen,
and protected bootstrap failure cleanup. `DoFile` execution, real VFS cache
creation, native differential ABI, and game execution remain unvalidated.


## Correction from docs/NATIVE_SHADER_DESCRIPTOR_DEPENDENCIES.md

The actual 4C8h owner bootstrap is now in `native_lua_bootstrap.cpp`. It retains tracking fields and overwrites an earlier state without closing it; fundamentals loads through two required cache reads and an unprotected call. The prior projected owner remains a different interface. B669C0 discards the returned string pointer, not the stack value: numeric top values become strings and remain on the stack. Cache ownership and VFS routing remain external. See that document and `reports/native_shader_descriptor_dependencies.json` for evidence and limits.


## Correction from docs/NATIVE_LUA_FILE_LOADING.md

B66CA0/B69D40/B69E00 now have complete actual Lua/native-string consumers, including native vector and argument helpers. Stream length/release ordering, early-return retention, ignored statuses and fatal Lua behavior match original instructions. The actual BDEF90 producer and callable VFS/stream ownership remain explicit external contracts. B69D40 and4D0FA0 complete disk tails are fixture-executed but still absent from their stored Ghidra bodies. See that document and `reports/native_lua_file_loading.json`.
