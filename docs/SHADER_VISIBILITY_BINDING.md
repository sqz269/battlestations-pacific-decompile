# Visibility constant binding

The scalar `cVisibility` is supplied by a draw entry, not by a Lua descriptor
default. Its registry semantic is43, with declared float shape1x1 and array
count1. In compiled metadata its register byte is at+33h (`8+43`), and its
reflected count byte is at+69h (`3Eh+43`). Nominal declaration position319
is outside the explicit77-register prefix and is not a usable binding slot.

This read-only analysis verified project `bsp` and program
`/battlestationspacific.exe` before every live batch. Fresh decompile,
disassembly and byte evidence is in ignored `exports/bsp/visibility_analysis/`.
No Ghidra annotations were changed by this investigation.

## Material packing

`00b42350` takes ECX=material pass and stack entry/override, returning RET8.
Assembly00b42a83 reloads the entry pointer into EBP from the original first
stack argument. The visibility fragment00b42e4a..00b42ea3 independently checks
the vertex metadata at pass+70h and pixel metadata at pass+74h:

1. Read the stage's metadata+33h register byte; FFh skips that stage.
2. Read the float bits at entry+18h using MOVSS.
3. Write that value into component X of the corresponding shared register
   buffer: VS base0108ebf4 or PS base0108dbec, plus registerIndex*16.
4. Write positive zero into Y, Z and W using the cleared XMM0 register.

The fragment does not consult the reflected count, clamp the input, multiply
another fade, or clear unrelated registers. MOVSS preserves the source scalar
bits. The nearby metadata+34h operation is a separate semantic, `cLODFade`,
computed through00b73770; it must not be folded into this visibility upload.
Caller00b43410 later uploads the positive constant tail starting at77, as
described in [SHADER_COMPILED_CONSTANT_BINDING.md](SHADER_COMPILED_CONSTANT_BINDING.md).

## Pixel use and normal-pass selection

The normal compile route supplies builder+A9h from base descriptor
`VisilityFade`, whose default is true. The effect descriptor's `OutputAlpha`
default is also true; the base debug shader's explicit `OutputAlpha=false`
does not override the dummy effect's own flag. See
[SHADER_DESCRIPTOR_FLAG_ROUTE.md](SHADER_DESCRIPTOR_FLAG_ROUTE.md).

When both flags are enabled,00b39880 emits
`Color0.a=SYS.DiffuseColor.a * saturate(cVisibility);`. If effect OutputAlpha
is enabled but visibility is disabled, it emits the direct DiffuseColor alpha
assignment. If effect OutputAlpha is disabled, neither assignment is emitted.
These branches exist in both fog and non-fog paths; premultiplication remains
separately controlled by the existing non-fog branch.

The upstream effect dispatcher00b45360 receives the same entry pointer.
For ordinary finite values, its COMISS/JBE branches00b45438..00b45468 and
00b45489..00b454ba select effect+C8h+4*mode when entry+18h is at least1,
or whenever mode is nonzero. In mode0, a value below1 selects effect+100h.
The compared literal00d7a24c has bits3f800000. The exact comparison also takes
the JBE branch for unordered input; this is not a general NaN validation rule.
Additional shadow, final-LOD and optional-object branches precede this selection.

Thus explicit visibility1 is a grounded full-visibility input for the host's
mode0 debug draw. It is not evidence that every native entry is initialized
to1. This bounded investigation did not establish the entry producer or its
default, nor any fade animation or scene ownership policy. The adapter must
keep the value explicit until that upstream route is recovered.

## Evidence and integration boundary

The following windows match the original executable and saved Ghidra bytes:

| Address | Bytes | SHA-256 |
|---|---:|---|
| 00b42e4a visibility packing fragment |90|56708bbe42d6220da6af4d8ffaa9ca56495b5fc5defba480b9403f58dfdcdf0c|
| 00b45360 complete dispatcher |359|f93649a9fd94f436d20e1abadf2c0f2198d78393242d68639c9c29b07624450c|
| 00b3a4af source-emission inspection window |134|819b6ab9af8e76abb580633fac14548debd6cb8740f8c41f293ed0ef9772b77b|

The smallest integration is to reflect the final compiled PS's cVisibility
register and upload `(entryVisibility,0,0,0)` there. A host value1 preserves
the debug fixture's alpha1 through the native generated expression. This is
an explicit draw adapter, not a reconstruction of native entry creation,
complete material-tail upload, binary ABI compatibility, or game validation.
