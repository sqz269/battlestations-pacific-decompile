# Source of the text glyph tree

Read-only audit on 2026-09-09. Live analysis batches verified `bsp` and
`/battlestationspacific.exe`. No code, metadata or Ghidra annotations changed.
This follows TEXT_GLYPH_ACCEPTANCE and identifies a native source-loading route.

## Text context resolves a registered font

00ab8c30 takes GUI text context in ECX and a name-string reference on the stack,
RET4. If the supplied name differs from stored context+1C8h, it copies the name,
calls singleton getter007371d0, calls registry lookup00ac3570, then stores the
returned pointer at context+108h. This is the pointer later used by00ab6d00.
It takes font+1Ch into context+1D4h when the lookup succeeds, otherwise a default
float, and releases a distinct reference-counted object at context+1ECh.
There is no observed retain of the font pointer at+108h in this setter.

00ac3570 (ECX manager, stack name reference, RET4/EAX result) traverses the list
whose sentinel is manager+8. Nodes contain the font pointer at+8. It requires
equal string lengths, then compares requested data against font-name data at
font+10h using stricmp; font-name length is+Ch. It returns the first matching
font or null. This is registry lookup, not a font-file load on demand.

007371d0 creates a10h-byte manager through00ac3690 when global00f8bf44 is null,
using the singleton manager's optional lock and lifetime registration. Constructor
00ac3690 initializes the list sentinel at+8 and count+Ch. The text context
constructor00ab9650 initializes+108h to null. Loading the registry must precede
ordinary character lookup; only the space bypass avoids dereferencing it.

## Native font registry loading

Startup helper0073bae0 constructs `Fonts/Fonts.lua` and prefix `Fonts\\`, obtains
the registry through007371d0 and calls00ac3910. The third string argument comes
from008d4890 and remains an unresolved root/path component in this audit.
00ac3910 has manager in ECX and three stack arguments, RET0Ch; its decompiler
prototype loses an argument and stack variables are unreliable in places.

The loader constructs the existing Lua wrapper, initializes base facilities with
mask1, executes the supplied script, and iterates global `Fonts`. Per entry it
reads `scale_ratio` default1, `alphatexturescale` default1 and `uppercase_only`
defaultfalse, allocates an ACh-byte object through00ad55c0, appends that pointer
to its registry list, obtains `datafiles[1]`, reads Data/GFX and AlphaTexture
(default `white.tga`), then calls00ad4c30. The script route reuses the native Lua
and file-manager infrastructure, not a Windows font API.

Installed `Fonts/Fonts.lua` is1115bytes, SHA256
`26ebc6c0905f3aff36e95f40195c0fe8693d9f17474d2fcd3e51bcc7091fdd93`.
It defines Arial20/18/16/15 against `arial18.dat`/`arial18.tga`, and Viper19/
ViperTitle against `viper19.dat`/`viper19.tga`, with distinct scale expressions.
It is an original read-only asset and has not been copied into the repository.

00ad55c0 (ECX font, stack name/scale/alpha-scale/uppercase byte, RET10h) creates
the tree sentinel, sets nil byte+15h, self-links all sentinel tree pointers,
zeros count+8, copies name to+Ch/+10h, stores scale+18h and alpha scale+1Ch,
initializes path pairs and stores uppercase flag+48h. This establishes the actual
tree owner consumed by membership lookup; no host glyph table is substituted.

## Glyph DAT population and next source dependency

00ad4c30 takes font in ECX and five stack arguments, RET14h. It combines paths,
loads texture resources through renderer virtual+64h and opens the data source
through file-manager singleton0109ceec virtual+4 with flags2. Full path argument
mapping, texture ownership and renderer loading remain outside this bounded audit.
The data reads use stream virtual slots and literal argument0:

1. +38h supplies record count; +40h supplies a16-bit height.
2. For each record, +40h supplies the16-bit glyph key; four calls to+44h supply
   floats; two calls to+3Ch and one call to+40h supply16-bit metrics.
3. Allocate a20h-byte glyph payload, fill it, check tree membership through
   exact lookup00ad4540, and insert new keys through00ad4aa0.

The recorded key is copied unchanged as low16 bits into the insertion pair.
00ad4540 duplicates the unsigned16 lower-bound/equality lookup described in
TEXT_GLYPH_ACCEPTANCE; it does not uppercase keys. For duplicate keys the newly
allocated payload is freed and the parser **continues** to the next record.
Raw continuation00ad506b cleans the free argument;00ad506e decrements the loop
counter and00ad5073 branches back. The truncated pseudocode's final return after
free is incorrect. Existing glyphs therefore win over duplicates.

Height uses signed16->float32, MULSS scale, CVTTSS2SI, low16 store at font+14h.
Payload+12h and+14h metrics instead zero-extend their16-bit input before the
same scale/truncate sequence. The first metric at payload+10h is copied. Four
float fields occupy payload+0..+F; resource values occupy+18h/+1Ch. Payload
padding+16h..+17h is not initialized by these stores. Do not assign semantic
names or zero it in a native-byte-parity claim without evidence.

After records, the stream is reference-released. The loader builds special
records at font+4Ch/+6Ch/+8Ch; the+6Ch record is copied from required key0091h.
Missing0091h takes an invalid-iterator path rather than a supplied default.
Full payload destruction and tree/texture teardown remain untraced. The registry
holds font pointers and contexts appear to borrow them; complete lifetime/order
must be established before making a native ownership claim.

The installed `arial18.dat` independently supports a little-endian candidate
layout:6-byte header `{uint32 count,uint16 height}` and24 bytes per record
`{uint16 key,float[4],uint16[3]}`. Its4974bytes equal6+207*24 exactly; header is
count207,height26; all207keys are unique, range33..8482,0091h present,FF91h absent.
SHA256 is `1824a05fd233f289dd810b67116b42b418dd725f5e7d1acf3f657398b60c1787`.
This file inspection supports the stream-call sequence but does not independently
prove scalar reader signedness, endianness flags or truncated-input behavior.

The smallest useful next source port is the DAT decoder plus actual key-set
population, grounded in those scalar stream readers. It can then feed the existing
native membership algorithm from installed data. Keep it separate from claiming
the entire00ad4c30 texture/path/ownership routine is reconstructed. Lua registry
evaluation and name selection should follow using the installed script.

## Exact native byte evidence

The inclusive complete-function ranges below match installed disk and live saved
program bytes; hashes cover exactly those ranges.

| Range | SHA256 |
| --- | --- |
| 00ab8c30..00ab8cd4 | d434b39aa4b1049e73f05217bd1fec64ed4e1e017db853441ea7b8e006f93e06 |
| 00ac3570..00ac3602 | 2865369398027fba470de9644dbba149cf2d88bc54bbafebe2de659783cce7b2 |
| 00ad55c0..00ad5690 | 0efe2fd5d21694683d3f1535634ed7640d681cd8ac3a366dc1b574e6a4e50173 |
| 00ad4c30..00ad5180 | 100ec30100b3ceb0ee99bee69e467cd86b55d282e7d62862bb1d84d0b99d363e |
| 00ac3910..00ac3ede | 2b34b4fc491572ece3d0773cc36a80007ce656a4092c061b5d71783c6647935b |

No build or runtime test was performed. Source-format and pointer-routing
evidence does not establish text rendering or complete font ownership parity.
