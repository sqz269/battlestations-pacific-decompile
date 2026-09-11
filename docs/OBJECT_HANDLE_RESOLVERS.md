# Installed object Handle callbacks

The startup scaffold previously stored three null pointers in a local resolver
structure and logged installation as implemented. Startup now installs the
three reconstructed callable C++ functions in a registry retained by
`GameStartupHost`. Live readers can bind that registry to actual object-table
aliases through `ObjectHandleReaderResolver`. Table allocation and population
remain dependencies of the world/entity owner; this work does not invent a
replacement registry or claim a running world.

## Native evidence

All observations use `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. Descriptive names are hypotheses. The two callback
targets that existed only as code labels were defined with verified disk bytes,
under the Ghidra write lock; prior state is recorded in
`reports/object_handle_resolver_definitions.json`.

| Address | Complete body / native ABI | Recovered behavior |
| --- | --- | --- |
| 006AD080 | 17 instructions, final006AD0B9 RET; ECX=input, EAX=pointer | Full32-bit zero gate, low16-bit handle, signed split, stride10h entry+C |
| 006AD0C0 | 6 instructions, final006AD0CE RET; ECX=object, EAX=zero-extended handle | Null gives0; otherwise word at object+174h |
| 00888AA0 | 24 instructions, final00888AFB RET; ECX=LuaObject, EAX=pointer | Get `Ptr`, convert to userdata, destroy temporary, return saved EAX |
| 00B662D0 | 5 instructions, final00B662DA JMP00A67910; ECX=LuaObject | Load underlying Lua state at wrapper-state+4 and index+8; tail-call `lua_touserdata` |
| 00BD4FC0 | 5 instructions, final00BD4FD5 RET4; ECX/EDX/stack callbacks | Store0109CED4/CED8/CEDC in order |
| 006AD0D0 | 5 instructions, final006AD0E4 RET; cdecl | Install006AD080,00888AA0,006AD0C0 through00BD4FC0 |

006AD080 tests the complete ECX word before truncation. Thus input00010000 is
not the null case: it selects handle0. Its comparison against00F89A10 is signed.
The selected index subtracts either00F89A0C or00F89A60 with32-bit wrap, then
shifts left4 and reads the pointer at+C from the current table pointer at
00F89A54 or00F89AA8. There is no inferred generation check, upper-bound check,
or null-on-invalid policy. The selected native entry address must be valid.
All table inputs are aliases, so subsequent calls see a replaced table pointer.

The constant at00CFAD08 is exactly `Ptr\0`. 00888AA0 receives the original
LuaObject in ECX, passes it to00B67800, then passes the returned temporary to
00B662D0. ESI captures the result across00B67700 destruction. Conversion uses
the existing Lua5.1.1 library's `lua_touserdata`: light userdata returns its
pointer, full userdata returns its storage address, and other types return null.
It does not dereference a pointer stored inside full userdata or convert a
numeric `Ptr` to an address.

## C++ integration

`ObjectHandleResolverSlots` now has typed callable fields. The reconstruction
adds explicit table/host arguments; these signatures are not original x86 ABI
entry points. `set_object_handle_resolvers_00bd4fc0` retains the unconditional
store sequence, and006AD0D0 installs real functions. The registry is retained
for the startup host's lifetime and exposed to later reader consumers.

The live adapter reuses the callback slots at each invocation and preserves
32-bit pointer result bits in the reader's signed destination word. Numeric
handles use the actual two table views. The table arm uses the real looked-up
token introduced by the preceding reader correction. `GuiLua51Host` adds a
required userdata conversion using its existing registry references and balanced
stack operations. No userdata copying or substitute object identity occurs.

The evaluated `GuiTable` projection has no userdata or metatable representation.
This adapter explicitly rejects that entry; native object Handle deserialization
uses the live Lua path. The callback body requires a valid live table, as the
native reader's table gate establishes. Lua longjmp/native SEH equivalence,
corrupt handles, native layout of the Lua wrapper, and game behavior are not
claimed by these C++ interfaces.

## Validation

MSVC Win32 Release and both existing CTests passed. One ignored fixture uses
actual native-layout stride10h entries and real Lua5.1.1. It checks the full-word
zero gate before truncation, both table branches, replaced table alias, object
word174h, actual installed callbacks, light/full userdata distinction, numeric
`Ptr` rejection, reader depth and interpreter stack restoration. The fixture
does not validate table-owner construction or world/serialized-game loading.

Run `./local/run_object_handle_resolver_fixture.ps1` after the standard build.
No permanent tests were added. Final integration and Ghidra readback records
are in `reports/object_handle_resolvers.json` and the panel/observer integration
report for this wave.
