# Material shader constant generation

`00b42350` contains useful bounded constant-generation slices, but is not ready
to replace as a whole. This read-only pass verified `bsp` and
`/battlestationspacific.exe` and compared its entire 4,288-byte body against the
original executable. Four directly relevant helpers and three float constants
also match byte-for-byte. Raw exports are in ignored `exports/material_constants/`;
hashes and contracts are in `reports/material_constants_evidence.json`.

## Inputs and side effects

The original ABI is ECX = selected pass, first stack argument = entry, RET8.
The caller passes an override as the second stack argument, but no direct read
of that argument was identified in this body's assembly. At `00b423b0`,
`[ESP+C8h]` is the **first** argument after four temporary pushes, not a third
or uninitialized argument as pseudocode may suggest.

The routine writes shared vertex float registers at `0108ebf4` and pixel float
registers at `0108dbec`, using 16 bytes per register. Metadata pointers are
pass+70h and pass+74h respectively. This is a selective update, not a clear of
the entire register arrays; skipped fields retain prior values. The caller
`00b43410` subsequently uploads positive tails beginning at global `00e13078`.

Before those updates, pass+5Ch onward holds pointers to dynamic texture records,
with unsigned count+6Ch. Record+4 supplies an object: virtual+2Ch obtains its
texture, which is bound to the slot at record+0. Virtual+30h then receives
`entry, VS_metadata, PS_metadata, 0108ebf0, 0108dbe8`. Those final two addresses
are four bytes before the actual float buffers; do not treat them as already
typed `float4*` arguments. Their object/header contract is unresolved.

## Smallest arithmetic slice ready for a semantic port

`00b73770` is a complete 139-byte thiscall helper with two float stack arguments,
RET8, and ST0 result. At `00b42ea4..00b42ef8`, metadata byte
`[pass+74h]+34h` selects the destination pixel register; FFh skips all work.
The call passes ECX = entry+8h pointer, input = float at entry+0, and fraction
`00d7a238` = float32 0.01. It writes `(result, 0, 0, 0)` to that register.

The helper reads an index at object+50h. If zero, its threshold is
`00ce77dc` = float32 0.995; otherwise it reads the float at
`object + 4 + 16*index`. Its mathematical operation is:

```
width = float32(threshold * fraction)
value = float32(1 - (input - (threshold - width)) * (1 / width))
if value < 0: value = 0
if value > 1: value = 1
```

Thus, for ordinary positive threshold/fraction, it fades from one to zero over
the interval `threshold-width .. threshold`. Calling it a distance fade is an
unverified semantic hypothesis: the meaning of entry+0 has not been recovered.
A descriptive port name should remain provisional.

Assembly explicitly stores width and final value as float32, while the interior
expression uses x87. Preserve evaluation order when seeking numerical agreement;
`(threshold-input)/width` is not automatically bit-identical. The comparisons
leave NaN as NaN, and there is no zero-width guard or index validation. A typed
port may require valid inputs but must not silently invent native clamping for
these cases. This helper is the smallest complete arithmetic dependency found
in this pass, and can be implemented before reconstructing the surrounding
material, animation, and callback objects.

## Bounded parameter-to-register packing slice

The earlier block `00b423c5..00b42693` is independently suitable for a typed
packing helper. Entry+4h leads to section+20h material. `00b17390` simply returns
material+80h. That is an inline pointer table, with count at table+80h
(material+100h), not a vector whose first word is a heap pointer.
The per-entry selector is `[entry+10h]+198h`.

Each parameter record has source pointer+8h, source word count+Ch, matrix flag
byte+10h, signed VS register indices beginning+14h, and signed PS indices
beginning+4Ch. Negative register indices skip that stage. For a nonmatrix
parameter, the block copies `word_count*4` bytes to the selected register;
the word count is not a float4 count, so a short final register is possible.

For matrix parameters, the source is a 16-float matrix and the packing rule is
`dst[4*r+c] = src[4*c+r]`. On the VS path, pass+70h metadata contains records
at+78h, count+7Ch, stride20h. Helpers `00b5b870` and `00b5b880` return record+0
(register) and record+4 (row count). The scan does not break on a match, so the
last matching record wins. Only row counts 2, 3, or 4 produce a write; absence
or other values leaves the VS buffer untouched. The PS matrix path always
writes all four rows and does not perform the metadata lookup.

This packing block is not a standalone original function; a new helper must
cite the address range and expose an explicitly new interface. Native x87
loads/stores in several matrix branches also mean raw integer-bit copying is
not a complete statement of special-NaN behavior.

## Remaining boundary

Other branches refresh object transforms, gather matrix palettes, compose
matrices, extract stream-derived vectors, write entry+18h as a scalar float4,
fetch owner color, bind further textures, and pack up to four object records.
For example the PS object-transform path uses its own destination register but
reads the row count from VS metadata+3Eh, a stage asymmetry that a full port
must preserve rather than normalize away. These branches were not recursively
reconstructed here. Shader metadata field names and object semantics remain
provisional.

No Ghidra functions, signatures, names, comments, or project state were changed.
No C++ or tests were added, and no ABI compatibility or game validation is claimed.
