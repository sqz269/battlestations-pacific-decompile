# Raw Movie default construction

Frozen published base: `71dbafc86780bd90afbb69873b7f5d2badc5a3f7`.
This adds only `construct_native_gui_movie_default_00aafe20` over actual raw
storage. The original in-house body already had a semantic reconstruction;
this raw-storage repair receives zero additional native-body credit. Existing
semantic APIs, callers, pools and guards are unchanged. It does not admit a
decoder consumer, Page children or playback.

## Native evidence and explicit Source API

The target-verified complete body is `00AAFE20..00AAFEAD` inclusive, exclusive
end `00AAFEAE`: 142 bytes and 32 instruction starts. Native ECX is destination,
ESI preserves it, EAX returns the same destination, and AAFEAD is plain RET.
There are no original stack arguments. The sole direct call is
`00AAFE3B ->00AA9390`, with actual immediate type10 pushed at AAFE37.

The new two-argument C++ API requires actual fresh DWORD-aligned storage
covering `+00..+12F`, and a non-default `const volatile uint32_t&` to the genuine
D7A24C storage. The actual input address is fixed, disjoint and retained through
the call. It composes the real existing
`construct_native_gui_widget_base_00aa9390(raw,10,actual_one)` and returns the
same raw address. It does not dereference/cache/default the constant itself.
The unchanged base reads that cell separately before and after its actual
sentinel allocation and retains its existing C++ allocation-failure cleanup.

The base header/source and eight-byte identity-prefix schema are unchanged.
The latter's historical Text name does not turn this allocation into a Text or
a semantic `GuiWidgetOwner`: it is the same proven generic initial prefix used
by the complete raw AA9390 helper. No surrogate base, returning callback,
replacement heap, successful-null fallback or extra rollback is supplied.

## Exact derived writes

After AA9390 returns, native AAFE42 writes profile D5C270 at offset0. That is a
numeric identity, not a callable PE vtable. The following stores remain in
native order, with their native widths:

| Native instruction | Offset | Bytes | Value |
| --- | --- | --- | --- |
| AAFE42 | 0 | 4 | D5C270 |
| AAFE48 | EC | 4 | 0 |
| AAFE4E | F0 | 4 | 0 |
| AAFE54 | F8 | 4 | 0 |
| AAFE5A | FC | 4 | 0 |
| AAFE60 | 100 | 4 | 0 |
| AAFE66 | 104 | 1 | 0 |
| AAFE6C | 105 | 1 | 0 |
| AAFE72 | 108 | 4 | 0 |
| AAFE78 | 110 | 4 | 0 |
| AAFE7E | 114 | 4 | 0 |
| AAFE84 | 118 | 4 | 0 |
| AAFE8E | 124 | 4 | 0 |
| AAFE94 | 128 | 4 | 0 |
| AAFE9A | 12C | 4 | 0 |

These are 15 ordered stores covering54 bytes, including the profile rewrite.
In particular, F4/F5..F7,106..107,10C..10F,11C..11F and120..123 stay untouched.
So do every other byte AA9390 and this derived body do not write, including
E4..EB. There is no whole-object initialization or semantic string/container
overlay. `GuiMovieFields{}` and `MovieWidgetState` remain separate projections.

The existing actual Movie pool has 134h slots with 130h payload and a trailing
slab-index DWORD. This constructor does not allocate/free a slot, write the
slab index, register ownership or supply AA1770's default/copy selection and
cleanup. If the base throws, this function adds no derived effects or catch;
the existing base and the caller's separate allocation custody remain in force.
The original native FH3 frame/handler CB7834 is not reproduced or claimed.

## Strict static validation

Strict MSVC Win32 objects passed `/std:c++17 /O2 /Ob2 /DNDEBUG /MD /EHsc /W4
/WX /fp:strict` for the new constructor and the unchanged actual base source.
The emitted constructor is164 bytes/25 instructions under its new cdecl ABI.
It has one actual call relocation to the exact public AA9390 symbol, whose
definition is present in the separate strict base object. This is object
evidence, not a final link claim. The primary owns CMake/common build/link.

A static Capstone inspection of the hash-pinned original PE's complete142
bytes and the new COFF function checks all15 ordered destination stores,
widths and values, same-address EAX return, sole call, and exclusion of every
listed untouched range. The object preserves separate byte stores at104/105.
The original live bytes and disk bytes agree; original hash is
`d3b5e4fd890aa8e80177acad4cff7cea6b74d66e674b8645eedd1880a4f45269`.
The existing `verify_report_calls.py` receipt checks the single original direct
call row. Local compiler/object/audit receipts and source hashes are recorded
in the report. No native code, object, fixture, probe or game was executed.

Read-only wrappers verified the existing configured `bsp` project
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, x86/image400000 before
the analysis batches; live/snapshot count was77631. Full pseudocode was read
first, then complete assembly/live bytes and bounded disk disassembly. No
Ghidra edits, reimport, annotation, shared ledger or CMake changes were made.

## Limits and next admission

The API is normal Source construction with explicit raw-storage/constant
inputs, not the original register/stack ABI, FH3/SEH/fault transport, private
stack identity or allocator ABI. It supplies no copy AAFFF0, destructor
AAFEF0, properties AAD560, loaded hook AAE770, factory AA1770, Page children,
callback epoch, decoder/FP/renderer/Lua/provider or lifetime admission.

The published frozen base does not contain the separate private movie-root
packet; it was not imported here. That packet remains default OFF under primary
review. Existing subtitle and child-Page guards remain false and untouched.
No build beyond the two strict objects, no new tests, DLL/module loads or runs,
no vendor body, and no additional body/ABI/gameplay/activation credit. Common
link/build and independent review remain primary obligations before admission.
