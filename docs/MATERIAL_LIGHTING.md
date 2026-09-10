# Material lighting values

Addresses: 00b179d0, 00b179f0, 00b17840, 00b18900.

The material owns one 68-byte lighting record at +38h. Its setter ignores the
serialized slot, copies all 17 DWORDs, and sets byte +10Ch to 1. Its diffuse
getter also ignores the slot and returns a live pointer to the record's first
four floats. The constructor initializes the record and clears that flag.

`reports/material_lighting_audit.json` contains complete PE/live-Ghidra matches
for all four bodies (495 bytes total), two initializer constants, original
ABIs, return instructions, old names/prototypes, plate annotations, and
exports. Each query verified project `bsp` and `/battlestationspacific.exe`
through `tools/bsp.py`. No Ghidra annotations or shared ledgers were changed.

## Layout and defaults

`00b17840` writes every component in the following order. The semantic name of
the first quartet comes from the already established getter and `cMatDiffColor`
consumer in `FONT_CONTEXT_MATERIAL_BINDINGS.md`. The other components remain
indexed values; resemblance to a standard graphics material layout is not
evidence for additional names.

| Component indices | Record offsets | Material offsets | Meaning established here | Default |
| --- | --- | --- | --- | --- |
| 0, 1, 2, 3 | 00h, 04h, 08h, 0Ch | 38h, 3Ch, 40h, 44h | Diffuse RGBA | 1, 1, 1, 1 |
| 4, 5, 6, 7 | 10h, 14h, 18h, 1Ch | 48h, 4Ch, 50h, 54h | Unclassified quartet | 0, 0, 0, 0 |
| 8, 9, 10, 11 | 20h, 24h, 28h, 2Ch | 58h, 5Ch, 60h, 64h | Unclassified quartet | 0, 0, 0, 0 |
| 12, 13, 14, 15 | 30h, 34h, 38h, 3Ch | 68h, 6Ch, 70h, 74h | Unclassified quartet | 0, 0, 0, 0 |
| 16 | 40h | 78h | Unclassified final component | 10 |

The four ones load from `00d7a24c` (`0000803f` bytes); the final ten loads from
`00ce38b8` (`00002041` bytes). Intermediate zeros come from `XORPS XMM0,XMM0`,
so their sign bits are positive. Both literal values were matched against the
original PE and current Ghidra memory. The initializer returns its input
pointer in EAX despite the old decompiler's void return type.

## Setter and getter

`00b179d0` sets byte material+10Ch at `00b179d6`, loads destination material+38h,
then uses `REP MOVSD` with count 11h at `00b179e5`. It consumes two stack
arguments but only loads the second, the source pointer. There is no indexing,
clamping, color conversion, or floating-point arithmetic. Every record bit,
including signed zero and NaN payloads, is copied. Repeated lighting entries
replace the same record, so the last entry wins regardless of slot value.

`00b179f0` is six bytes: `LEA EAX,[ECX+38h]; RET4`. It consumes but never reads
the index argument. The returned quartet aliases the stored record; writing
through that pointer changes the material color without passing through the
setter or changing byte +10Ch. Existing shader constant binding copies this
quartet to `cMatDiffColor` semantic 47. Component 3, material+44h, is the diffuse
alpha supplied to the separately reconstructed building instance writer.

## Constructor fragment

The complete `00b18900` body calls `00b17840` on material+38h at `00b18962` and
sets byte +10Ch to zero at `00b18983`. `MaterialLighting` reconstructs those
lighting-field operations only.

Other observed constructor stores remain documented without assigning new
semantics: +8/+Ch are zero; nine texture pointers +10h..+30h and their 16-bit
count +34h are zero; +100h is zero; +104h/+108h are FFFFFFFFh; and byte +10Dh
is zero. The effect pointer +7Ch starts zero and is then assigned with native
reference management. Vtable installation, reference count, textures, effect
lifetime, and these other fields belong to their own material implementation.
There is no evidence here that gaps in the object are zero-initialized.

## Original calling contracts

| Address | Inputs / output | Return |
| --- | --- | --- |
| `00b17840` | ECX 68-byte output record; EAX same record | RET |
| `00b179d0` | ECX material; stack ignored slot, source record | RET8 |
| `00b179f0` | ECX material; stack ignored slot; EAX material+38h | RET4 |
| `00b18900` | ECX material; stack effect pointer; EAX material; x86 SEH | RET4 |

## Host implementation and integration

`include/bsp/material_lighting.hpp` and `src/material_lighting.cpp` provide
`MaterialLightingValues` (17 floats), the initializer, and `MaterialLighting`.
Construction initializes the record and clears `flag_10c()`. The setter copies
all bytes and sets that flag. Mutable and const getter overloads return an
alias to the first quartet; `values()` exposes the complete stored record.
The class does not reproduce the original material's memory layout or ABI.

The existing `MeshLightingRecord` wire event already owns an ignored slot and
all 17 decoded values. Material construction can replay those events in order
through `set_lighting_record_00b179d0(record.ignored_slot, record.values)`, then
read diffuse alpha from `diffuse_color_00b179f0(0)[3]`. The same pointer can
supply the proved diffuse system-constant binding while its owner remains
alive and unmoved. No additional parser or synthetic material data is needed.

Whitespace and report-integrity checks passed. CMake, build, and the existing
installed-model probe are coordinated by the primary agent. No new tests,
native differential test, or game/render validation is claimed by this packet.
