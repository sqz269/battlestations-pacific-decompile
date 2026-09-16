# Raw common particle properties (ORCH4)

`load_native_particle_type_property_00b015c0` in
`native_particle_type_property_raw.cpp` composes the complete `00B015C0..00B01C11`
body (1,618 bytes, 556 instructions) with existing raw providers. The previous
interface in `native_particle_type_property.*` remains available. Native entry
is `ECX=actual definition`, one stacked pooled 4h suffix header, `RET 4`, and
`AL=recognized`. The new C++ overload is a source interface, not a binary ABI
replacement. Names remain descriptive hypotheses.

## Context and provider boundary

The context borrows the same actual raw string pool, an optional raw B01350
texture context, and views of five current native table cells. The texture
context is required only after the Texture arm is reached. It must share the
same pool and publications; its cache/owner pointers are required only when
B01350 reaches its initial atlas miss. There is no synthetic renderer, model,
shader, or allocation callback. B015C0 has no Model property.

The actual object word at `+0` selects a final numeric profile. The source reads
that profile's current `+10` cell before arming Shader state 4. It dispatches
only the three existing raw setters and two genuine `RET 4` no-op bodies:

| Definition | Final profile | Usual current `+10` target |
|---|---|---|
| Sprite | D5DD18 | B089E0 |
| Axial | D5DCC0 | B06210 |
| Floating | D5DCEC | B07C80 |
| Object | D5DB00 | AF80E0 |
| Tracer | D5E048 | B0A040 |

Factory B00CE0 explicitly calls B08830 at B00D45, then stores D5DD18 at
B00D4A; it calls B058E0 at B00D9A, then stores D5DCC0 at B00D9F. Its remaining
arms call the final Floating/Object/Tracer constructors B00770/AF89E0/B0A0B0.
Constructor base profiles D5DFF4/D5DF30/D5DFB0 are not admitted. An unknown
profile or current target is an unsupported source boundary; it does not
silently select a default shader. The tables remain borrowed current cells,
not invented function-pointer identities.

## Complete property behavior

Each attempted property obtains token zero anew through AEE3C0. The first five
comparisons use `_stricmp`; the last seven use AEDF80. Texture, Layer, and Shader
return the captured key bytes directly, preserving their nonzero local header;
the nine flag arms use AEE2A0 and clear the header. A match fetches token one.
AnimFade uses the other stack header, matching the original local placement.

| Property | Action / destination |
|---|---|
| Texture | Raw B01350; recognized even when its boolean result is false |
| Layer | AF5660 copy, 41E870 consumed 8h header, raw AF4360; store index at +74 |
| Shader | Capture current table `+10`; dispatch one of the five targets above |
| Stops | Full byte +28 = signed `atol(value) > 0` |
| RandomRotationDirection | Full byte +29 |
| EmitLight | Full byte +64 |
| Distort | Full byte +65 |
| AnimOnOff | Full byte +4C |
| AnimRndStartFrame | Full byte +60 |
| AnimRandomPlay | Full byte +61 |
| AnimLoop | Full byte +62 |
| AnimFade | Full byte +63 |

The flag writes replace the whole byte with zero or one, as SETG does. CRT
`atol` is signed 32-bit LONG on the required Win32 build. Unknown keys return
false after all twelve normal cleanup paths.

Layer keeps three distinct ownership facts. After AF5660 copies token one,
the original token bytes are returned under state 3. The source saves the copy's
data pointer (native ESI), constructs a separate consumed 8h header from that
pointer, and zeros the old argument header before 41E870. Only after that
construction does it read current `definition+14` for AF4360. AF4360 reads the
consumed header's current length/data and consumes its allocation on normal
return. B015C0 stores the result at `+74`, disarms, and returns the saved ESI
pointer by its current C-string length. The final return does not reload the
copy header, and the consumed argument is not an invented caller unwind slot.

## Failure state and retained child

Handler CBB4C0 references FH3 data DF3514, whose five-row unwind map starts at
DF3538. Only four states are actually entered by this body:

| Entered state | Arm / native state store | Unwind action |
|---|---|---|
| 0 | Texture, B01647 | CBB4A0: AEE2A0 on argument header |
| 1 | Layer before AF5660, B0170E | CBB4A8: AEE2A0 on argument header |
| 3 | Layer after copy, B01721 | CBB4B0: AEE2A0 on copy local |
| 4 | Shader after current target capture, B0182D | CBB4B8: AEE2A0 on argument header |

Map state 2 also names CBB4B0 and has predecessor 1, but no state-2 store is
executed by B015C0. Other predecessors are -1. Each action is an eight-byte
LEA/JMP tail action to AEE2A0. The source advances the state before invoking its
action and terminates if an unwind cleanup itself throws.

Normal disarm ordering is explicit: Texture captures the current token pointer,
then disarms before returning bytes; Layer stores +74, then disarms before the
saved-pointer return; Shader disarms before AEE2A0. The flag paths never arm a
value-token cleanup state. The source catch implements only this caller map;
it does not make allocation faults or Windows FH3 behavior ABI compatible.

The caller supplies `NativeParticleTypePropertyRawAcquired`. Its persistent
headers precede its embedded `NativeParticleTypeTextureRawAcquired`, which in
turn retains the cache child. A failed unresolved texture child must remain
alive with this parent and its borrowed contexts. It must not be retried,
reset, or replaced with a temporary frame. Inherited failed-child destruction
rules remain in force. Normal completed calls own no surviving token; particle
texture records retain their ordinary definition ownership.

## Evidence and validation

The complete native body and all five caller sites were checked. Callers
AF8BD0/B064A0/B07D60/B08AC0/B0AD50 pass the actual definition in ECX and a pooled
suffix header on the stack, and consume AL. The machine-readable report lists
every one of the body's 80 direct call sites, the five resolved possibilities
at the single indirect Shader call, the five caller sites, and four unwind
tail jumps with their containing functions.

An ignored Win32 probe copied the complete installed native body, matched its
bytes against live Ghidra, and executed 42 original/source comparisons. The
native body's direct children were relocated to genuine reconstructed raw
pool, layer, shader, and texture providers. Its indirect slot used explicit
native ABI bridges to those same shader bodies; the source used final numeric
profiles and borrowed current numeric cells. Comparisons covered all twelve
properties, each flag with negative/zero/positive values, all five final shader
profiles with two names each, one redirected current slot, Layer hit/miss,
unknown key, and a real atlas Texture path. Exact 98h owner bytes (relocated
pointers normalized), 1Ch records, and actual raw-pool bump/live/peak matched.
No permanent test was added. Strict MSVC Win32 `/W4 /WX` build and all three
CTests passed after native seed verification. The same 42 comparisons passed
again against the final built `bsp_core` library with its actual Lua/zlib
dependencies; artifact hashes and commands are recorded in the report.

This validates supported normal source composition. Original FH3 failure
execution, forced child allocation failures, renderer cache misses in this
property probe, malformed-input faults, drop-in ABI compatibility, and full
game behavior are not established.

Prior evidence remains in `docs/NATIVE_PARTICLE_TYPE_PROPERTY.md`,
`docs/NATIVE_PARTICLE_TYPE_RESOURCES.md`,
`docs/NATIVE_PARTICLE_TEXTURE_PROVIDER_ORCH4.md`, and their reports. This packet
adds the raw property overload and does not change those earlier interfaces.
