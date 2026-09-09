# Font glyph selection and special records

Read-only audit on 2026-09-09, with `bsp` / `/battlestationspacific.exe`
verified before analysis batches. No C++, shared metadata, Ghidra annotations
or runtime state changed. This follows FONT_GLYPH_SOURCE and FONT_DATA_IMPLEMENTATION.

## Selection00ad4480

ABI: ECX font, one stack argument whose low16 bits are the key, EAX payload
pointer, RET4. The key routes before any ordinary tree search:

| Key | Returned payload |
| --- | --- |
| 0020 space or000A LF | Embedded font+4Ch |
| 000D CR | Embedded font+8Ch |
| Exact key in unsigned16 tree | Payload pointer at found node+10h |
| Any other missing key | Embedded font+6Ch |

Even a tree entry for space/LF/CR does not override the special routing. Ordinary
search is00ad4370; iterator dereference00ad3e70 checks owner nonnull and node
different from owner sentinel, returns node+Ch, and selector loads its+4 pointer.
No uppercase transform occurs in this selector. This lookup differs from input
acceptance00ad4500: a missing input byte can be rejected even though rendering
would have a fallback payload.

## Special record initialization

The final initialization in00ad4c30 runs after reading all DAT records and
releasing the data stream. Each payload is32 bytes:

* font+4Ch is first cleared across all32 bytes. Payload word+12h is then set to
  signed(font.height+14h)/4, truncating toward zero. Assembly sign-extends height,
  adds3 for negative values, then SAR2. Payload resource fields+18h/+1Ch receive
  the two resource values used for ordinary glyph payloads. Other metrics and
  float fields stay zero, including padding+16h/+17h.
* font+6Ch is a raw eight-DWORD copy of the payload at tree key0091h. The loader
  checks the iterator and does not supply a replacement if0091h is absent. This
  copy includes native padding and resource pointers; no refcount operation is
  visible around the copy.
* font+8Ch clears its first24 bytes (including all metrics and padding), then
  stores the same two resource values at payload+18h/+1Ch. It has zero advance.

For positive height26, the space/LF metric is6, not6.5. A projected signed division
must preserve truncation toward zero for negative heights too. Current FontData
contains scalar fields and excludes resources/native padding, so selecting its
0091 map entry can provide equivalent known fields but cannot claim a raw32-byte
fallback copy. Special records should explicitly separate known scalar fields
from recovered resource bindings.

## Where uppercase_only is applied

Font byte+48h is checked in text consumer00ab8f00 before its width/truncation
pass. When nonzero, it calls00a9ec30 with manager singleton00f8bc4c in ECX and
the UTF16-string object on the stack. The helper modifies each16-bit string
element in place, iterating the stored unsigned length rather than stopping
at the first NUL; RET4.

Each element goes through00a9eba0: ECX manager, stack16-bit value, resultAX,
RET4. It scans an override array between manager+4024h and+4028h for the first
matching value. A match selects the corresponding entry in the array at+4034h
through+4038h, with iterator/range checks. If no match, it invokes CRT towupper.
The override-table population and locale setup are not recovered in this audit.
Do not replace this route with ASCII-only uppercasing or assume FontData keys
themselves should be transformed. The byte-input acceptance chain remains
unchanged; rendering/measurement strings use a distinct16-bit path.

## Immediate consumers and next drawing boundary

00ab8f00 accumulates payload unsigned word+12h times context+1D8h while measuring
text and trimming for an ellipsis. The larger layout builder00aba270 also uses
+12h for horizontal advances and wrapping. This supports an advance-width role
for that metric; exact rounding/layout behavior is a separate port.

00aba270 selects a payload for each character and passes its+18h and+1Ch values
to00b189f0 for material slots0 and1 on its guarded initial binding path. Resource
ownership and rebinding across varying glyph resources remain unresolved. It
then passes the payload to quad writer00ab98f0, whose assembly establishes:

| Payload field | Consumer evidence |
| --- | --- |
| +10h | MOVSX signed16, added to horizontal origin (bearing/offset role) |
| +14h | MOVZX unsigned16, scaled by context+1D8h to form right edge |
| Floats+0,+4,+8,+Ch | Four UV corners use(+4,+0),(+C,+0),(+C,+8),(+4,+8) |
| +12h | Used by outer layout advance, independently of quad width |

The quad's vertical extent comes from a separate supplied height argument; it
is not inferred from glyph+14h.00ab98f0 additionally addresses interleaved vertex
streams, writes color/indices and uses x87 arithmetic. Its complete ABI/layout
and all branches are not recovered here, so the UV/metric table is a dependency
map rather than a complete quad port.

The next bounded implementation is scalar special-record construction plus
00ad4480 selection using actual decoded FontData. A subsequent text draw must
recover the full quad writer and material-resource binding, then use installed
atlas textures. Existing descriptor/DAT probes alone do not establish native
text placement or rendered font parity.

## Byte evidence

Inclusive ranges below match installed disk and saved Ghidra bytes. The postload
range is a fragment; other entries cover complete small functions.

| Range | SHA256 |
| --- | --- |
| 00ad4480..00ad44fd | fe786a41f924b7a9ef37af1db008c658c9d494a19f10a90d2adf400579efd1ce |
| 00ad3e70..00ad3e93 | d69b7ad3915dfd8af546e06b38b633bd5d84eb7b68f0d1125797e8838c73491f |
| 00ad508f..00ad514d | 5c9c1d581bb0e8ea33ef4125dcb3812433750cf705c2ba02be77ad34dc7b62c8 |
| 00a9ec30..00a9ec62 | b44208a831618093f557f5eb7e6c213eacc124999bfe7f1e27d887e9ecde5e8c |
| 00a9eba0..00a9ec2b | 3c27403db935d93d5ec17d19c555eb9a5ca01cf5a730ad738c3605be94342894 |

No build or runtime test was required for this audit. Required mapping tables,
renderer resources and text-owner lifecycle remain explicit dependencies.
