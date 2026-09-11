# Gameplay force-request events

Packet `orch3_gamepad_force_events_g` reconstructs the three event producers
`00873450`, `00873560`, and `00873750`, their definition factories, common handle
operations, and scalar destruction wrappers. The new interface is in
`include/bsp/gamepad_force_events.hpp`; implementation is in
`src/gamepad_force_events.cpp`. Descriptive names are hypotheses. This is typed
Win32 C++, not native event layout, allocation, intrusive ownership, or SEH ABI.
Analysis used read-only, target-verified `bsp.py` Ghidra wrappers against
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, plus original PE decoding
for missing definitions and flow gaps.

## Request production and ownership

The native definition virtual creators receive `ECX=definition`, one stack
subject pointer, and return an allocated event or null in EAX with `RET 4`.
They allocate 20h bytes, then call their event constructor with `ECX=storage`,
stack `(definition, subject)`. Each constructor returns `this` with `RET 8`.
The apparent floating-point second parameter in the fading decompilation is
actually a pointer: `00873578` loads the subject argument and `008735DF` reads
its transform pointer at +110h.

All events initialize native references +4 to 1, active byte +Ch to 1, borrowed
subject +10h and definition +14h, and event type +18h to 6. Native primary
vtables are D0DCD0, D0DD08, and D0DD40; secondary vtables at +8 are D0DCCC,
D0DD04, and D0DD3C. The typed event records the fields, without manufacturing
either native vtable or the surrounding event-system reference manager.

| Event constructor | Actual request class / native allocation | Definition fields |
| --- | --- | --- |
| `00873450` | Constant / 14h, D0DB78, kind 0 | channel +20h, duration +24h, amplitude +2Ch |
| `00873560` | Fading / 18h, D0DB8C, kind 1 | channel +20h, duration +24h, radius +28h, amplitude +2Ch |
| `00873750` | Alternating / 28h, D0DBA0, kind 2 | channel +20h, duration +24h, selector byte +2Ch, values +30h/+34h, periods +38h/+3Ch |

Each creates the already reconstructed concrete `GamepadForceRequest` class,
then calls `00A95BF0` with **ECX=0, EDX=request**. Zero selects the first active
gamepad in the canonical current backend groups, independently of the subject.
The event stores the returned request ID at +1Ch only after submission. The
device registry owns a successfully inserted request; the event owns only its
numeric handle. No duplicate request state or force-output algorithm is added.

The common vtable +8 operation `00872180` passes the actual handle field to
`00A957D0` and returns whether it is now zero. The +34h operation `00872160`
calls `00A957F0` only when the handle is nonzero, then writes zero after the
call. A callback that changes the handle during removal is therefore followed
by this final zero store. The +2Ch helper `00872150` is a genuine `RET 8` no-op.
It does not age the request; the canonical device request pump does that.

The three scalar wrappers `00873530`, `00873720`, and `00873860` restore the
common event vtables, invoke the existing ref-counted base destructor
`00BD30F0`, and free storage only for flags bit 0. **They do not cancel the
request, release the subject, or release the definition.** `delete_force_event`
preserves this distinction. Its storage must come from the typed factories or
standard `new` when flags bit 0 is set. The surrounding event system must invoke
the explicit cancellation operation when cancellation is intended.

## Fading event geometry

The fading amplitude is calculated once during construction; the resulting
request then performs the existing time fade. Native behavior is:

1. Start with gain 0. Check the current game global E188A8, field +1ED4. If this
   target is absent, skip subject/pose reads but still allocate and submit a
   fading request whose amplitude is the actual multiplication `amplitude * 0`.
2. Read the subject's existing transform at +110h. If transform flags +5Ch lack
   bit 2, call canonical `refresh_camera_world_00b6db70`. Copy source position
   from transform +120h/+124h/+128h, i.e. world matrix +F0h entries 12..14.
3. Reload the current game global after the source refresh. Use its current
   +1ED4 target, not the earlier pointer. If target byte +C8h is false, call the
   required actual pose refresh `00414DB0`; continue using that captured target.
4. Subtract the copied source position from target +FCh/+100h/+104h, spilling
   each x87 result to float. Calculate length with `0042B2F0`. Compute
   `1 - distance / radius` with x87 and spill to float. Replace it with zero only
   when ordered `gain < 0`. There is no upper clamp or positive-radius guard.
5. Allocate the request, multiply definition amplitude +2Ch by gain, and submit
   to device index zero. No target-presence shortcut replaces this multiplication;
   notably, a NaN amplitude remains NaN when gain is zero.

`ForceEventSpatialHost` is an explicit lifetime binding to the actual current
target, subject transform, and target pose refresh. `ForceEventTargetPose` binds
the existing valid byte and matrix by reference. No renderer/transform fields
or vectors are copied into a new production owner. `00414DB0` is a distinct
pose representation: +74h local, +CCh world, +3Ch parent, +C8h valid and +10Ch
secondary invalidation. Existing unit-instance/motion hosts already require
this operation. It remains a required implementation boundary in this packet;
it is not replaced by a no-op. No semantic claim that +1ED4 is a particular
camera or player class is needed.

`force_event_vector_length_0042b2f0` is also recovered here. Native x87 computes
`(y*y + x*x) + z*z`, spills the sum to float, compares it strictly above the
exact double `1e-10` at CE3820, then invokes the genuine CRT sqrt entry BF7030
and spills its result to float. Otherwise it returns positive zero, including
an unordered squared sum. The CRT body contains FSQRT at BF706C. The typed code
uses the installed C++ CRT `sqrt`; it does not reconstruct library internals.
Consequently a tiny nonzero separation can produce zero distance, a negative
radius can produce gain above one, and a NaN radius survives the ordered clamp.

## Address and analysis boundaries

| Address | Native ABI | Final instruction / inclusive end |
| --- | --- | --- |
| `00869010` | thiscall definition, stack subject, EAX event/null | `00869073 RET 4`, length 3 / `00869075` |
| `008690F0` | same, fading factory | `00869153 RET 4`, length 3 / `00869155` |
| `00869290` | same, alternating factory | `008692F3 RET 4`, length 3 / `008692F5` |
| `00873450` | thiscall event, stack definition/subject, EAX this | `00873507 RET 8`, length 3 / `00873509` |
| `00873560` | same, actual subject pointer despite decompiler type | `008736FB RET 8`, length 3 / `008736FD` |
| `00873750` | same | `00873833 RET 8`, length 3 / `00873835` |
| `00872150` | ECX event, two ignored DWORD stack slots | `00872150 RET 8`, length 3 / `00872152` |
| `00872160` | ECX event, no stack arguments, no consumed return | `00872177 RET`, length 1 / `00872177` |
| `00872180` | ECX event, no stack arguments, AL Boolean | `00872193 RET`, length 1 / `00872193` |
| `00873530` | scalar wrapper ECX event, stack flags, EAX old this | `00873558 RET 4`, length 3 / `0087355A` |
| `00873720` | same | `00873748 RET 4`, length 3 / `0087374A` |
| `00873860` | same | `00873888 RET 4`, length 3 / `0087388A` |
| `0042B2F0` | ECX float[3], no stack arguments, ST0 float | `0042B33C RET`, length 1 / `0042B33C` |

At this packet's inspection, 872150 and 872180 had no Ghidra function definitions.
Their complete ranges are 872150..872152 and 872180..872193. The three scalar
wrappers have complete outer stored bounds but omit three-byte `ADD ESP,4`
fallthroughs after incorrectly non-returning `_free`: 873552..873554,
873742..873744, and 873882..873884. Original PE decoding confirms those bytes.
No analysis mutation was made. Correct `CG_scalar_deleting_dtor_*` labels are
retained; the new descriptive game names remain ledger hypotheses.

## Validation and limits

`python tools/ghidra_export.py verify-seeds` matched all eight seeds.
`./scripts/build.ps1` passed the strict MSVC Win32 build and both existing CTests.
The single ignored local fixture `local/gamepad_force_events_fixture.cpp`, run
by `local/run_gamepad_force_events_fixture.cmd`, passed against `bsp_core.lib`
with explicit recording force/spatial hosts. It checks actual request classes,
all three parameter mappings, gamepad index zero, borrowed event fields,
destruction without cancellation, cancellation after callback mutation, stale
handle validation, canonical expiration, source transform refresh, target reload,
3-4-5 distance attenuation, missing-target behavior, lower-only clamping, and
selected NaN/threshold cases. It performs no hardware force, focus, or window call.

Native allocation failure is unchecked after an inner request allocation returns
null; the existing typed submit routine reports an invalid null request when a
device exists. The outer factory preserves allocation-null return. Standard
allocation and C++ exception cleanup do not claim the native allocator or SEH
contract. Typed event storage is initialized to handle zero before submission,
where native leaves +1Ch unwritten until the call returns. Null subject with a
present target and a target disappearing after refresh are explicit typed
errors, rather than unchecked native dereferences. No exhausted-ID collision
handling is added beyond the canonical request registry's documented behavior.
No exhaustive CRT/FPU-environment equivalence, native event-dispatch ABI,
hardware haptics, or gameplay validation is claimed.
