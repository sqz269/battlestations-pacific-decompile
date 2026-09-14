# Native Lua field setters

Addresses: `00B67530`, `00B67580`. Source: `include/bsp/native_lua_field_setters.hpp`,
`src/native_lua_field_setters.cpp`. Machine-readable evidence:
`reports/native_lua_field_setters.json`.

The two bodies write directly into the actual Lua 5.1.1 table represented by
`NativeLuaObjectStorage`. They neither create another object wrapper nor alter
the owner's tracked stack-slot records. Their descriptive Ghidra names are
hypotheses, not recovered symbols.

| Native body | Coverage | Original ABI and inclusive body | Effect |
| --- | --- | --- | --- |
| `00B67530` | complete | ECX actual14h object; stack actual8h `NativeString*`, pointer; `RET 8`; `00B67530..00B6757F` | `lua_checkstack(L,2)`; push a length-delimited key; push the pointer as lightuserdata, including null; `lua_settable(L, current index08)` |
| `00B67580` | complete | ECX actual14h object; stack actual8h `NativeString*`; `RET 4`; `00B67580..00B675CF` | Same key preparation; `lua_createtable(L,0,0)`; `lua_settable(L, current index08)` |

The original listing reads `object.owner_00->state_04` before each Lua call.
It reads key length, then key data *after* `lua_checkstack`; a null data pointer
selects the empty byte at `0108FF2C`. For the final `lua_settable`, it reloads
owner, then index08, then the owner's state04. The source makes those loads in
that order, using volatile lvalue reads for the native object fields. It
ignores the `lua_checkstack` return just as the original does. The primitive
pushes are left on the same Lua stack until `lua_settable` consumes them. No
kind gate, table copy, private registry reference or duplicate stack slot is
introduced. The layout comes from the existing actual14h object and actual8h
NativeString declarations; callers pass both globals wrappers (`00B67980`)
and tracked table wrappers (`00927B40`/`00B67800`).

All **36 direct original calls** were checked in their containing function
listings. The report has an `address`/`native`/`function` row for each, and
`local/lua_setter_call_contexts.txt` retains the nine preceding instructions
at every site, including the key producer and ECX object setup. The 22 indexed
callers of `00B67580` contain 32 calls. The live `xrefs` display stopped at 25;
the remaining sites were recovered by walking those complete caller listings.

| Setter | Containing function start and direct call sites |
| --- | --- |
| lightuserdata | `006AE280: 006AE37D`; `008A9730: 008A9C15`; `00928A00: 00928C2F`; `00928C80: 00928D04` |
| new table | `00436730: 004367AE`; `00436C50: 00436CF2`; `004374F0: 0043757B`; `00438020: 0043822D`; `004DFB70: 004E029B`; `005E2F00: 005E304B` |
| new table | `00742C10: 00742CCE, 00742EA2`; `0077FAD0: 0077FDF1`; `00780DB0: 00780EB1, 00780FAB`; `007F2FD0: 007F306A`; `0081BA80: 0081BC35` |
| new table | `008B7D30: 008B7ECB, 008B7F64`; `008BF6A0: 008BF7FE, 008BF8F9`; `0090EA00: 0090EAE2`; `00928A00: 00928B53`; `00928C80: 00928D49` |
| new table | `00A13960: 00A139E5`; `00A2EEE0: 00A2EFC4, 00A2F2AD, 00A2F3EE, 00A2F5AC`; `00B1B890: 00B1B8C8`; `00B68AD0: 00B68B24`; `00BD6300: 00BD633D`; `00BD7180: 00BD7563, 00BD7686, 00BD776A, 00BD7895` |

The immediately visible producers include `0041E870` NativeString construction,
the object's `00B67980` globals construction and `00927B40` mission-self
lookup. Some callers build a key with `00BF7680` before passing its existing
header. The complete call contexts, rather than an assumed single key or
table kind, govern the contract: callers pass null and nonnull lightuserdata
and several table ownership paths. The setters themselves read only the
current object fields and key header.

Neither original body has an EH/SEH setup or checks the Lua stack-growth
result. A Lua error or longjmp after a push has only the primitive's native
stack effects; these wrappers do no rollback. The source has the same direct
call order and no C++ cleanup guard. Ill-formed key headers with a null data
pointer and nonzero length may overread the original global empty byte and
are outside the supported source comparison. The source signatures are C++
interfaces over the native layout; they are not drop-in `__thiscall` exports.
`00B67400` number-field conversion belongs to a separate integrator packet.

Live Ghidra bytes equal the installed PE at both complete 80-byte spans.
`verify_report_calls.py` accepted all 36 direct call rows. After
`ghidra_export.py verify-seeds` matched all eight seed spans, the MSVC Win32
Release `scripts/build.ps1` and both existing CTests passed. One ignored
native-byte replay relocates the two installed bodies and redirects their
eight Lua fastcall sites to Lua 5.1.1 ABI bridges. Its five original/source
cases match: nonnull and null lightuserdata, a new globals table, an embedded
NUL key, and an `__newindex` callback that rebinds a tracked object's current
index before the next setter. The runner, original bytes, source, executable,
toolchain, linked libraries, included SDK headers, call contexts and logs are
frozen and hashed under `local/lua_setter_evidence_manifest.json`. The probe
relocates the two `0108FF2C` empty-byte immediate loads to its own empty byte.
This proves the two wrapper bodies against the same stock Lua 5.1.1 runtime;
it does not execute the game's private Lua internals or validate gameplay.
