# DamageableClass Damage/Sections entry and cleanup fragment

Addresses: 0087CD4D 0087CD59 0087CD6A 0087CD88 0087CD9C
0087D1F8 0087D207 0087D20C 0087D218. Enclosing original: 0087CA80.

The fragment has ordinary C++ Source with a fixed caller-retained owner over the
actual Lua storage. It covers the two nested table gates and their matching
cleanup tail. A successful gate leaves Damage and Sections live for the
excluded continuation; this is not a reconstruction of the Sections loop.
The worker performed a focused strict MSVC Win32 compile, inspected the entire
candidate COFF and actual selected provider definitions, and passed the existing
registered baseline build. Integrator registration and admission remain pending.
No whole original function or whole enclosing reader is added.

| Region | Exact bytes | Instructions | Excluded successor |
| --- | --- | --- | --- |
| Entry gates | 0087CD4D..0087CDA8, 92 bytes | 20 | 0087CDA9, first iterator construction |
| Matching cleanup | 0087D1F8..0087D21C, 37 bytes | 6 | 0087D21D, FakeExplosionEffects setup |

The native entry prefix SHA-256 is
`70f46b6a768440386cf895adb3aa0bff585904b509cb65f96c4b2b48581e1fde`;
the cleanup tail is
`f5c76492acc16faa3291124b19165e269f75e99f0ab2cb5480b471fbf212d223`.
The complete enclosing 3,238-byte body and all 858 instruction starts match
the installed PE, saved listing and fresh live listing. Its body hash remains
`00d679920bd1b8f8653e9a0ab1de627f954cfadaf48b8bf0642676fde5162808`.
The retained 37-state FH3 audit supplies the cleanup relationships; no new
Native child, key-data, handler or FH3-data opening was performed. Live queries
used the verified `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe` client.

The original enclosing ABI is ECX=actual descriptor, stack=actual Lua row,
RET4, with E4h locals and four saved registers. Neither selected region is an
independent entrypoint. With S denoting ESP after that allocation/save sequence,
the actual row is in EBP, descriptor in ESI, dead scalar scratch at S+44h,
fresh Sections storage at S+80h, and live Unique at S+94h. Each Lua object is
14h bytes. Storage and the scratch binding must remain stable, disjoint and
aligned at least four bytes through this owner's whole lifetime and callbacks.
The Unique owner encloses this owner and remains responsible for final cleanup.

| Native site | Actual operation and retained state |
| --- | --- |
| CD59 / CD62 / CD6A | Lookup Damage into S+44h; raise to decimal state10; call B661B0 |
| CD71 false | Jump directly to D20C; Sections has never been constructed |
| CD88 / CD94 / CD9C | Lookup Sections on the actual Damage object into S+80h; raise to decimal state11; call B661B0 |
| CDA3 false | Jump to D1F8; both fields are live |
| CDA9 true | Leave both fields alive for the excluded iterator/loop continuation |
| D1FF / D207 | Lower11->10, then destroy actual Sections with B67700 |
| D210 / D218 | Lower10->1, then destroy actual Damage with B67700 |

Retained decimal state10 has parent1 and action C9696B, cleaning S+44h;
state11 has parent10 and action C96976, cleaning S+80h. Both actions call
B67700. Unique remains the separate state1 responsibility throughout.

`NativeDamageableClassDamageEntryFragment` is noncopyable and nonmovable.
The caller creates it before calling `open(actual_row)` exactly once. This
ensures an owner exists before the first protected lookup. The owner stores
only a reference to the actual scratch binding and the cleanup state; it never
copies a Lua object, owner, index, tracked slot or interpreter.

False from `open` means the matching actual fields have already been closed
and control corresponds to D21D. True means state11 remains active and control
corresponds to CDA9. The caller must retain the owner across the excluded
continuation and all inner guards, then call `close()` only at the matching
normal tail. This packet supplies no callback that pretends to run that
continuation and no fallback iterator, vector, category or effect operation.
No descriptor bytes are written by this fragment.

The destructor is an ordinary noexcept propagation guard. If a normal explicit
Sections cleanup throws, state10 has already been stored; unwinding this owner
destroys Damage once without retrying Sections. If Damage cleanup throws,
state1 has already been stored; it is not retried. A failure in the second
lookup leaves only Damage owned. A first lookup failure leaves neither new
field owned. The owner must unwind before further work after an error, and
must never be replayed, reopened or reentered by callbacks. Secondary failure
in its noexcept destructor follows ordinary termination. Native FH3, SEH,
Lua nonlocal transfer and double-exception identity are not established.

The genuine existing providers are the protected B67800 adapter,
`native_lua_is_table_00b661b0`, and `destroy_native_lua_object_00b67700`, all
from `src/native_lua_objects.cpp`. The table predicate returns false for kind0,
true for nonzero kinds other than2, and for kind2 compares actual `lua_type`
with LUA_TTABLE. It is not replaced with a truthiness test. Both lookups
construct and register their actual output addresses; the second lookup uses
Damage, not the original row. Destruction can move Lua stack slots and update
all tracked indices, so caching headers or indices would violate the contract.

Protected lookup uses the existing Lua5.1.1 same-frame operation and inherited
error handler. On failure it restores entry stack height and publishes no
new output before throwing the existing Source error. The borrowed owner,
objects and interpreter must remain stable; existing unchecked slot/reference
capacity and inherited error-handler-position constraints still apply.
This is an explicit ordinary C++ boundary, not a native Lua callback adapter.
The linked Lua and MSVC libraries remain real dependencies.

The focused compile used `/O2 /Ob2 /Oy- /MD /W4 /WX /fp:strict /EHsc /std:c++17`
with Win32/Windows/Release defines, and freshly compiled the actual Lua
provider translation unit. Indexed COFF inspection covered all physical
sections, symbols and auxiliaries, exact relocation targets, all code bytes,
the destructor EH record, SafeSEH index, literal strings and compiler metadata.

| Candidate physical section | Body | Bytes |
| --- | --- | --- |
| 4 | Owner constructor; state1 initialized before open | 21 |
| 5 | noexcept destructor with inlined ordered cleanup | 110 |
| 6 | Destructor EH handler | 29 |
| 7 | Explicit normal close | 57 |
| 9 | Open, nested gates, false cleanup and live success return | 151 |

There are 14 physical sections and 368 total code bytes. Open calls the first
lookup at +14h, stores state10 at +1Bh, and calls the first table predicate at
+23h. It passes that actual output to the second lookup at +3Ah, stores
state11 at +40h, and calls the second predicate at +47h. The successful branch
at +51h reaches +8Fh and returns true without cleanup. The false path stores
state10 at +5Bh before the Sections destructor +65h, and state1 at +75h before
the Damage destructor +7Fh. Explicit close stores10 at +0Bh before +15h and
stores1 at +25h before +2Fh. The destructor preserves the same ordering.
Open relies on the caller's already-constructed owner for C++ unwinding; it
does not reproduce the parent's native frame or supply a local substitute.

The destructor's .xdata physical section11 contains maxState0 and EH flags5;
.sxdata section10 names symbol index30, the physical section6 EH handler.
The ordinary constructor/open/close functions have no private cleanup object
or claim of native FH3 identity. .rdata sections12/13 contain the exact
`Damage` and `Sections` strings. The compiler's weak AVX2 bss symbol is a
header artifact, not an invented game global.

All three candidate project externals match actual definitions in the fresh
Lua object: physical49/125 bytes (protected lookup), 38/39 bytes (destructor),
and59/47 bytes (table predicate). Their whole bodies and selected transitive
callback42/29 bytes, raw lookup48/169 bytes, and tracked-release74/314 bytes
were inspected. All 66 Lua provider code sections, 4,240 bytes, and indexed
relocations match the retained ExplosionType provider receipt. This verifies
the actual Source provider chain, not new Native child byte equivalence.

`./scripts/build.ps1` completed successfully with the currently registered
baseline, including ExplosionType and excluding this candidate. Its two
existing checks passed: reconstructed_math (0.22s), tool_tests (8.05s), total
8.31s. No new test, candidate fixture, differential execution, receiver or
startup/gameplay run was added. The prior Root Source599 review reported
599 selected project inputs, 73 Core and3 App whole objects, 139 unique
positive Core definitions and3 checks; those are prior context, not admission
of this unregistered fragment.

The JSON report pins all owned source/doc bytes, actual provider inputs,
retained native audit, the prior primary review, baseline inputs and local
COFF/live-listing receipts. Shared CMake, ledgers and Ghidra remain untouched
by the worker. Full parent composition, Native ABI/FH3/exception behavior,
actual application receiver binding and runtime/gameplay remain held.
