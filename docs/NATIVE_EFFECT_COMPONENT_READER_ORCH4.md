# Native effect-component base reader

The complete216-byte868BF0 body now has a source interface over actual14h
tracked Lua stack objects. It composes the existing genuine native Lua getters
and destructor. The earlier `GuiLuaHost` interface remains a separate host
projection; its registry references are not interchangeable with this storage.

Original ABI: ECX actual component, one stack Lua object pointer, RET4. The
reader looks up `Autostart`, stores Lua truthiness at byte10, then destroys that
temporary. It reuses the same20-byte temporary for `Delay`: exact NUMBER values
use B66380 and the actual late-read CRT conversion mode; all other types,
including numeric strings, yield zero. The result is stored at14 before cleanup.

`NoFilterDist` uses a second20-byte temporary. Field lookup occurs before the
CURRENT component+18 fallback is loaded through x87 FLD32/FSTP32. That order
matters if Lua's lookup metamethod changes the component. B66330 selects an
exact NUMBER or that captured float; the result is stored at18 before release.

HandlerC95188 selects FuncInfoDC7040 with three states and mapDC7028. State0
and state1 useC95170/C95178 to destroy the first temporary atEBP-34; state2
usesC95180 to destroy the second atEBP-20. Each action transitions directly
to-1. Source arms each state only after its successful lookup, and disarms it
before ordinary destruction. A second C++ exception during true unwind
terminates. There is no invented cleanup of a failed field lookup.

The same packet supplies the complete10-byteAF3E10 resource setter: ECX
resource, one stack byte argument, RET4. It stores that entire byte at+70.
It does not normalize the value to a boolean or touch any other byte.

## Evidence and validation

Both whole bodies, the three unwind thunks, FuncInfo and state map match live
Ghidra and installed-image bytes. Prior names/comments and exact call rows are
in `reports/native_effect_component_reader_orch4.json`. Names are descriptive
hypotheses. Correct existing library names are retained.

The strict MSVC Win32 build and all three existing CTests passed. One ignored
probe compares copied original code against the compiled source using real
Lua5.1.1 and the actual native object helpers through ABI bridges. Four reader
scenarios agree on the full component bytes, stack and tracking counts, lookup
mutation count and x87 exception flags. They cover ordinary numeric values,
truthiness, numeric-string rejection, mutation during the final lookup and a
signaling-NaN fallback. The resource setter matches for a non-boolean byteA7.

The copied original uses the same source Lua providers; this validates reader
composition and ordering, not a differential test of those providers. No native
exception path, original FH3/SEH, binary ABI compatibility or gameplay is
claimed. Full Particle reader871D00 and resource acquisition870DD0 remain
source-absent pending genuine cache/resource-loader closure. In particular the
AF4BA0 parser and its constructor/text-loading/destruction providers are still
needed; the base reader and setter do not substitute for them.
