# Raw Movie default storage review

Independent qualified pass of `54fb69ac5e00304636dcb6609034b8026f50802d`,
based on `71dbafc86780bd90afbb69873b7f5d2badc5a3f7`. No blocking discrepancy
was found in the normal raw constructor composition. This is static Source
evidence with explicit valid-storage inputs; caller/retirement admission,
final link, original ABI and runtime behavior remain separate.

## Original body and actual composition

The existing configured `C:/Users/sqz269/bsp.gpr` and
`/battlestationspacific.exe` were checked through the read-only BSP wrappers
before native analysis batches. Saved pseudocode, complete assembly and live
142-byte output were inspected. The actual original PE remains 12223752 bytes,
SHA-256 `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Independent PE/Capstone decoding agrees with the live bytes: `AAFE20..AAFEAD`
inclusive is 142 bytes/32 instruction starts, ending in plain `RET`. Its code
SHA-256 is `d3b5e4fd890aa8e80177acad4cff7cea6b74d66e674b8645eedd1880a4f45269`.

Native ECX is the incoming raw destination, captured in ESI at `AAFE39` and
returned in EAX at `AAFEA0`. `AAFE37` pushes type 10; the sole native call at
`AAFE3B` targets the actual `AA9390`. No original stack argument is inferred
from the new C++ interface. The original frame/handler `CB7834` is unclaimed.

The new function requires fresh DWORD-aligned storage covering `00..12F` and
the genuine initialized, address-stable, disjoint D7A24C word retained through
the call. It directly composes unchanged
`construct_native_gui_widget_base_00aa9390(raw,10,actual_one)` and returns the
same raw storage. It adds no callback, default constant, cached word, heap,
whole-value initialization or semantic widget cast.

The base header/source blobs are exactly
`61c72c99fab59f5c86da392495d7bae5381607cf` and
`997a33c5ac7bbf7b57e133feb53b91163e7308b5`. The existing eight-byte
`NativeGuiTextIdentityPrefix` is the genuine generic initial prefix used by
this base, not a complete Text or `GuiWidgetOwner` overlay. Fresh base object
inspection confirms separate actual-cell reads at +5B and +158 around the
real sentinel allocation call at +153. The wrapper passes the reference's
address, without reading or substituting its value. No actual-one producer
or lifetime owner is added; future callers must supply that genuine cell.
This checks reuse and input epochs, not a fresh complete native base-body audit.

## Exact writes and preserved storage

Independent native and fresh COFF extraction produce the same 15 ordered
`(offset,width,value)` stores, using hexadecimal offsets: profile `D5C270` as
a DWORD at 0; DWORD zeros at EC, F0, F8, FC and 100; separate byte zeros at
104/105; then DWORD zeros at 108, 110, 114, 118, 124, 128 and 12C. They cover
exactly 54 distinct bytes. Numeric
profile words remain evidence identities, with no native code/table invocation.

The stores preserve F4..F7, 106..107, 10C..10F, 11C..123 and every other byte
unwritten by the unchanged base and this derived body. E4..EB remains untouched.
The highest written byte is 12F. Existing `GuiMoviePoolTraits` has 134h slots,
130h payload and trailing index DWORD at 130h; no constructor store touches that
index. The function does not allocate/free a pool slot or transfer its custody.
The old semantic `GuiMovieFields{}` assignment and its callers remain unchanged
and are not promoted as substitutes for this raw contract.

Every derived store follows the actual base call. If that call throws, this
wrapper performs no derived write or extra catch/cleanup. Existing base C++
failure cleanup and the caller's separate raw allocation/operation custody
remain required. This does not establish native FH3/SEH, faults, private-stack
identity, arbitrary reentry or allocator ABI.

## Independent static validation and limits

Fresh MSVC Win32 constructor/base objects passed
`/c /std:c++17 /O2 /Ob2 /DNDEBUG /MD /EHsc /W4 /WX /fp:strict`.
The new entry is 164 bytes/25 instructions under its new cdecl two-argument API.
Its sole call is a real `IMAGE_REL_I386_REL32` relocation at +0D to the exact
AA9390 public symbol, defined in the separate strict base object. Both relevant
function emissions match the primary objects' code and relocation tuples.
This is definition/relocation evidence, with final link still pending.

The existing report-call checker independently passes 1/1. All 10 source pins,
2 native export pins and 10 artifact pins match. Candidate changes are exactly
the four new declared files; base/pool/semantic APIs, callers, guards and shared
metadata are unchanged. No CMake registration or whole-build result is claimed.

The rerunnable static verifier, two objects and narrow listings are retained in
`local/native-gui-movie-default-storage-review/`. `object_audit.json` SHA-256:
`e248d4a7f1a328287dcb691d3b3d32c77846f1a361b7e83ac02eaad2c58cd5bc`.
The report preserves original module/export/source/artifact records and exact
store arrays, local artifact paths and hashes.

AA1770/default-copy selection, AAFFF0 copy, AAFEF0 destruction, AAD560 properties,
AAE770 loaded hook, Page children, decoder/clock/callback/renderer/Lua/provider
retirement and actual caller bindings remain unclosed by this packet. No
fixture, new test, full build, native/object execution, game, probe, DLL load,
source/CMake/shared-metadata change, Ghidra mutation or guard flip occurred in
this review. Additional native/vendor body, original ABI and activation credit
are zero.
