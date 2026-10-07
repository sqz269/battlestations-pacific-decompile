# Pilot-bot parked predicate and planeID methods (CC11)

Native evidence only; no source task or lifetime binding is supplied. Four previously
undefined bodies were verified against the installed image, defined under the Ghidra
write lock and saved in `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`.
Names below are descriptive hypotheses, not recovered symbols.

| Entry; end exclusive | Original ABI | Observed role |
| --- | --- | --- |
| 00999A60;00999A71 | ECX=predicate; RET; boolean in AL | Load predicate+4, return whether byte at referenced plane+184h is zero |
| 00999A80;00999A9F | ECX=predicate; stack flags; RET4; EAX=original pointer | Stamp base profile00D05840; free through00BF65AC when flags bit0 is set |
| 0099A940;0099A97C | ECX=predicate; stack input object; RET4 | Call input virtual+10h for planeID, then publish returned word at predicate+4 |
| 0099A980;0099A9B0 | ECX=predicate; stack output object; RET4 | Call output virtual+0Ch with the cached plane word |

The native profile00D1F31C has **four**, not three, entries:
`00999A60,00999A80,0099A940,0099A980`. Its final cell is00D1F328;
the distinct profile00D1F32C begins afterward. The input/output methods do not
establish an observer invalidation callback merely by occupying virtual slots.

## Producer and predicate

In the reviewed eligible tick branch,0099ADD0 first tests plane+184h nonzero,
0099ADD9 calls007B8AD0 for the formation-index-zero leader predicate, and
0099ADEC asks the current active head's virtual+38h. The branch allocates an
eight-byte predicate at0099ADF4 through00BF681B. For a nonnull allocation,
0099AE03 writes profile00D1F31C and0099AE09 stores the plane from bot+50h at
predicate+4. It passes that object (or null on allocation failure) in EDX to
009BBFC0, then passes the returned task to00999F50 at0099AE1C. The new current
head receives virtual+54h at0099AE2B and bot+74h receives the float constant
00D7A260. The containing tick and task-construction/prepend contracts are separate.

00999A60 unconditionally dereferences predicate+4. A valid nonnull plane reference
is required; it supplies no null guard, ownership, entity lookup, observer registration,
refcount or invalidation policy. Only AL is a boolean; the upper EAX bits retain
the loaded pointer bits. The meaning of plane+184h is left as the observed byte
gate rather than inferred from an unrelated object's same offset.

## planeID transfer operands

The input method builds a stack record and invokes the incoming object's
virtual+10h. Flattened stack operands are `(0,"planeID",4,&output_word)`.
After that virtual call,0099A96F reloads the output slot and0099A974 stores it
at predicate+4. The stack setup and later saved-register restoration imply a
callee-pop of16 argument bytes. Its output slot initially contains a stack
record address; a missing-field-to-null fallback is not present in this body.

The output method invokes incoming object virtual+0Ch with flattened operands
`(0,"planeID",4,cached_plane_word)`. The final datum is the cached word **by
value**, not its address. Both the literal4's type/size meaning and the actual
input/output implementations remain unresolved. A byte-count, raw pointer
wire format, persistent ID lookup or accepted missing-field policy must not be
invented from this call shape. No ABI fixture or native stream execution is claimed.

## Decompiler and validation limits

Assembly governs these small bodies. The current input pseudocode misidentifies
the post-call stack output as `local_14`, and the output pseudocode omits ECX's
predicate provenance and the flattened operands. The scalar-delete pseudocode
uses `extraout_EAX` on the free arm despite the native MOV EAX,ESI at00999A99.
There is no missing instruction gap in that scalar body. A read-only exact-flow
query was rejected by the bridge because script execution is disabled; no flow
flag or global CRT annotation was changed in response.

`tools/ghidra_define_function.py` recorded exact live/disk byte matches for all
four ranges. Existing call verification checks the one direct free call; the
two stream calls remain explicitly indirect. Definitions, reviewed naming and
comments were saved and exports refreshed. Raw pseudocode stays in ignored
exports; no C++ change or build is part of this native audit. Stable task identity,
predicate ownership/free ordering,009BBFC0 construction,00999F50 active prepend,
actual serializer providers and plane lifetime after death remain open.
