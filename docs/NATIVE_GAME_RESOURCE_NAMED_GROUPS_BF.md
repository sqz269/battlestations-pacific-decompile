# Native game-resource named point groups

Addresses: 00717F20, 00718000, 00718870, 004FBA10, 00484270, 004B3FC0, 0071ABB0, 0071B3E0

The class-model binders consume named `Points` items from the game resource's
classified pointer vector at +64h. These source interfaces recover the complete
lookup chain and its point accessors over the actual borrowed resource/item
storage. They acquire no references and require no copied resource model.

| Address | Bytes | Original ABI and behavior |
| --- | ---: | --- |
| 00717F20 | 221 | ECX resource, stacked name/index, RET8; scan all entries and return whether the wrapping match count is nonzero. |
| 00718000 | 214 | Same arguments; return the last exact match or null. |
| 00718870 | 47 | Same arguments; call contains, then perform a fresh find scan if its low-byte result is true. |
| 004FBA10 | 30 | ECX item, RET; null begin returns zero, otherwise signed wrapped byte-distance divided by 12, truncating toward zero. |
| 00484270 | 80 | ECX item, stacked destination/index, RET8; validate, reload current begin, then copy the point through three ordered FLD32/FSTP32 pairs. EAX returns destination. |

The five bodies total 592 bytes. The STL name at 00484270 is retained. The
unsigned comparison at 004B3FC0 remains a library contract: the source uses CRT
`memcmp` because these callers observe only zero/nonzero. Its original 142-byte
body supplies an independent comparator in the fixture, not a new library port.

## Storage and ordering evidence

The 95-byte item constructor 0071ABB0 stamps profile CFD8FC, initializes the
28-byte SBO name at +8, sets Identifier index +24h to FFFFFFFF, initializes the
category string at +28h, and clears point-vector begin/end/capacity at
+48h/+4Ch/+50h. The 339-byte reader 0071B3E0 passes item+8 to the character-string
reader at 71B451, item+24h to the U32 reader at 71B458, and item+44h to Vector12
push-back at 71B4F0 after reading three components. Both producer bodies are
audited references; this batch does not reconstruct or execute their parsers.

Lookup compares counted, case-sensitive bytes, including embedded NULs. The
key's capacity/length and each item's name length are captured before the
comparison. Equal lengths and current item+24h equality are also required.
Names use the existing `NativeLegacySboStringStorage` and its canonical counted
assignment in the fixture.

The initial iterator is captured before validation. Each iteration captures
current end before checking current begin, and tests that captured end after a
returning handler. Dereference/increment guards reload end. The native
`CMP owner,owner` branch never invokes validation. A handler can change the
container while the captured iterator remains live; the wrapper's second scan
then observes the changed container. No extra validation or fallback is added.

The point getter retains its captured index through a returning invalid-parameter
handler, then reloads begin. The x87 stores occur between loads, so a destination
overlapping the next input component propagates the earlier stored value.
Signaling NaN quieting, denormal status and signed-zero bits are preserved by
literal x87 instructions. The compiled object retains all six instructions.

## Verification and limits

MSVC Win32 `/O2 /fp:strict /W4 /WX /EHsc` compilation and both existing CTests
pass. One ignored fixture executes all five copied original bodies against their
source interfaces. Five group scenarios each exercise contains/find/guarded
lookup: inline embedded-NUL names, heap names, empty names, a missing counted
name, and returning validation that changes the second scan. Three point cases
cover special-value bits/status, overlapping output, and repair of a null begin.
Six count cases cover zero, partial strides, negative differences and address
wrap. All original/source results agree. Thirty-one direct/tail call rows cover
the five implemented bodies and the two audited producer references.

The fixture supplies borrowed raw resource/item records and a controlled
returning CRT handler. It does not execute the item parser, full resource
classification, real class-model binder or game executable. Tests use the masked
default x87 control word; unmasked hardware exceptions and full floating-point
environment equivalence are not claimed. These are new C++ interfaces, not
drop-in ABI replacements. The real vehicle binder, entry activation and gameplay
remain open. No workers were dispatched.

## Retained evidence

The immutable manifest `local/named_groups_bf/manifest.json` retains 303 exact inputs and 87 artifacts, including 3 linked production objects, original lookup/access/library/producer bytes, compared output images, compiled disassembly, compiler/library inputs and saved Ghidra comments/exports. Source commit `e05f4d9e47f1f90fc2d3efff524df46c09d340f1` contains the tested source. The build began at `18c7d4c9b5b138e6b76b0b7cdc520bd18e906a5c` with owned edits present; source hashes and compiler dependency records identify the actual inputs. No source changes followed verification.
