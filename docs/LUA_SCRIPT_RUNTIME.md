# Actual Lua script execution and DoFile

Addresses: 00B69D40 00B66CA0 00B69E00 00B66C00 00B679B0 00B677E0 00B67720
0041E870 00BDEF90.

`LuaScriptRuntime` supplies real stock Lua 5.1.1 calls behind the existing
`GuiLuaScriptHost` reconstruction. Its `run_file` and `run_chunk` delegate to
`run_gui_lua_file_00b69d40` and `run_gui_lua_chunk_00b66ca0`; no second script
ordering implementation was added. The existing GUI screen calls still use a
clear obfuscation flag. `PcStorageLuaOwner` accepts the static `do_file` callback
with the runtime as its context; the caller supplies the actual cached
fundamentals bytes and keeps the file host/runtime alive until the Lua owner
closes. The runtime accepts the state as an argument so it also works during
owner bootstrap and in nested calls.

## Recovered behavior

| Entry | Native ABI and evidence | Result |
| --- | --- | --- |
| 00B66CA0 | ECX owner, stack NativeString pointer and low-byte flag, RET8 at 00B66DD9 | Load and call one chunk |
| 00B69D40 | ECX owner, stack NativeString pointer and flag, RET8 at 00B69DE2 | Base first, then ordered content variants |
| 00B69E00 | Incoming Lua state in ECX, EAX=0, plain RET at 00B69F17 | Lua DoFile callback, first argument and no returned values |

The native loader uses singleton `0109CEEC` virtual `+4(path,2)`, checks stream
`+18` and 64-bit `+30` size, allocates the size's low word and calls stream `+24`
to read it. Null stream, not-open and zero size return silently before touching
Lua. The read's status is not tested. `LuaScriptFiles` exposes a complete-byte
read contract with mode **2**, not a disk fallback or generic search. Missing,
not-open and empty cases remain silent; a host that cannot provide the declared
size must fail explicitly instead of returning incomplete/uninitialized bytes.
Native huge-file truncation and its early-return stream-reference leaks are not
portable ownership requirements.

At `00B66D40..00B66D61`, the flagged transform starts in prefix mode: replace
each byte by space (`20h`), testing the original byte for `01h` before doing so.
After the first marker, rotate every remaining byte by four bits. The marker
itself becomes a space. No marker makes the entire file whitespace. Length is
unchanged. Assembly confirms the rotate instruction and the complete 64-bit
loop comparison that the pseudocode describes poorly.

`00B66DA2` passes the bytes, original size and path string to `luaL_loadbuffer`.
It ignores the return code, releases the successful stream reference, then
`00B66DC4` calls `lua_call(L,0,LUA_MULTRET)` and frees the buffer. Thus successful
return values remain on the stack. Compile failure leaves an error string which
is then called as a function; a runtime raise also escapes this native boundary.

`00B69D68` runs the base before `00B69D8B` calls `00BDEF90`, even when the base
is absent or empty. It executes every returned 8-byte string entry in order,
including duplicates, passing the same flag to each. It does not recursively
search variants of a returned variant. `00BDEF90` uses the VFS manager's
suffix vector at `+48/+4C` and virtual `+8` existence checks for
`stem + '_' + suffix + extension`; this is a separate file-host responsibility,
not directory enumeration or generic provider candidate resolution.

`00B69E20` constructs a non-owning wrapper through `00B66C00`. That constructor
also replaces the global `DoFile` callback (`00B66C2E..00B66C49`), including when
the callback was invoked through an alias after the global was overwritten.
`00B679B0` describes arguments as kind3/start1/count=`lua_gettop`;
`00B677E0 -> 00B67720` with index0 selects argument1. `lua_tolstring` at
`00B69E60` accepts string/number, then `0041E870` copies with an unchecked
`strlen`, so embedded NUL truncates and null conversion would fault. The
callback calls `00B69D40` with flag0 regardless of extra arguments. Its owner
does not close the borrowed state, and its final zero return discards all
included chunks' return values from the Lua call frame.

## Host boundaries and existing adapters

The concrete host uses `lua_pcall` for each call rather than terminating the
process. It still calls the error string after compile failure. An error stops
the current file operation before later overrides, throws to C++ callers, and
preserves prior globals and successful results; only the failed error value is
popped. The DoFile bridge catches C++ failures and raises with `lua_error` only
after C++ buffers and exception catches have unwound. The callback message is
capped to 1023 bytes. Invalid filename types raise an explicit Lua error instead
of reproducing the native null dereference. These are documented host
adaptations, not native recovery behavior.

The existing `shader_lua.cpp` adapter remains separate: it resolves explicitly
supplied shader names, fails missing files, suppresses successful returns, and
checks compile status. It is not the general native runtime, and this packet
does not silently change its established shader API. The earlier
`docs/GUI_LUA_READER.md` statement that obfuscation is not reconstructed is
superseded for `run_gui_lua_chunk_00b66ca0` by this packet.

## Evidence and validation

Every live query/export used the verified saved `bsp` project and
`/battlestationspacific.exe`. Pseudocode was checked against assembly for the
register ABI, transform, ignored compile status, argument indexing and borrowed
wrapper registration. Updated names/evidence are in the sharded ledger; locked
Ghidra name annotations remain pending the integrator's post-merge batch.
The integrator restored the five-byte internal free gap
`00B66DCF..00B66DD3` and saved/refreshed exports. `00B69D40`'s decoded, disk-matched
tail extends through `00B69DE4` inclusive (`00B69DE5` exclusive), but Ghidra's
stored function-body metadata still ends at `00B69DCC`. The tail repair report
explicitly records this remaining metadata limitation. `00B69E00` ends at
`00B69F17` inclusive; it needs no body repair.

`python tools/ghidra_export.py verify-seeds` passed. `scripts/build.ps1` passed
MSVC Win32 Release `/W4 /WX` and both existing tests. The ignored, focused
`local/lua_script_runtime_fixture.cpp` linked the actual Lua/core libraries and
passed installed `scripts/fundamentals.lua` bootstrap plus `DoFile` execution of
installed `fonts/fonts.lua`; ordered duplicate variants, missing/empty files,
mode2, multi-return stack preservation, nested/number/alias callback behavior,
invalid argument adaptation, flagged decoding/no marker, and compile/runtime/
nested failure propagation also passed. No permanent test suite was added.

This is reconstructed, build-tested and host-fixture-tested. It is not an ABI
replacement, native differential execution of these three routines, complete
VFS integration, or game validation. The installation was only read.
