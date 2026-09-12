# Native particle definition loading

This packet reconstructs nine complete functions in the existing `bsp` project,
`/battlestationspacific.exe`. Names are descriptive hypotheses. The installed
image and live Ghidra matched across all 9,026 owned bytes. The report records
each span, hash, original ABI, final instruction, and all 355 direct call sites.
Raw pseudocode had false early returns after BF6989; the primary integrator
archived documentation, repaired the three function bodies under the write
lock, and refreshed exports. No installed bytes were changed.

| Native entry | End exclusive | Native contract |
| --- | --- | --- |
| B03EC0 | B04824 | Cone; ECX definition, stack TextBuffer, RET4/AL bool |
| B02FD0 | B03895 | Sphere; same original ABI |
| B02210 | B02B25 | SmartArea; same original ABI |
| AF9D00 | AF9E8F | ECX definition; stack pooled name, builder, float; RET0C/AL bool |
| AF9F00 | AF9F1F | ECX definition; stack emitter child; RET4; no defined result |
| AF9F20 | AF9F42 | ECX definition; stack particle child; RET4/EAX reloaded row |
| AFA650 | AFA9AA | ECX definition; stack pooled suffix; RET4/AL handled |
| AFA370 | AFA4DD | ECX definition; stack C string; RET4; part emission type |
| AFA4E0 | AFA64D | ECX definition; stack C string; RET4; emitter emission type |

## Actual parser and storage contracts

The incoming TextBuffer is the canonical application's existing object. Line
and token owners have only a `char*` at +0. They are not the separate eight-byte
NativeString headers used for child kind/name arguments. All string allocation
and release use the same `NativeStringStorage`; parameter production uses the
same actual F8D344 physical pool borrowed by `NativeParticleDefinitionBindings`.
`loading.owners` and `loading.parameters.owners` must refer to that same service
instance. F8C2C8 remains the caller's shared scratch buffer, with the native
capacity and reentrancy obligations.

Each parser scans normalized lines until `{` or EOF, then performs another
read even if the scan failed. It consumes lines until `}` or EOF and returns
AL=1 on normal completion; upper EAX is unspecified. It neither requires
balanced braces nor invents an error return for EOF. Empty lines are skipped.

For a `Param` line, the parser first passes suffix1 to AFA650. Unhandled flags
continue through token1 name, suffix2/token0 scalar, builder construction,
zero-valued endpoint insertion, and suffix2/suffix1 curve parsing. The curve
parser's AL result is deliberately ignored; its partial mutations remain.
AF9D00 checks common names before the exact shape-specific names below.

| Common name | Definition destination |
| --- | --- |
| BornRatio | Float +18 from builder's first value; scalar token ignored |
| Lifetime | Parameter pointer +20 |
| ParticleEmission | Parameter pointer +24 |
| EmitterEmission | Parameter pointer +28 |
| Speed | Parameter pointer +2C |
| VerticalSpeed | Parameter pointer +30 |
| InheritedSpeed | Parameter pointer +34 |
| WindSensitivity | Parameter pointer +38 |

| Shape | Ordered native properties and destinations |
| --- | --- |
| Cone | InnerEmitSpeed +80, OuterEmitSpeed +84, MaxAngle +88, InnerDistance +8C, OuterDistance +90 |
| Sphere | EmittedSpeed +80, InnerRadius +84, OuterRadius +88 |
| SmartArea | EmittedSpeed +80, Radius +84, RadiusSpeed +88, RadiusAngle +8C |

Every parameter pointer is produced by the concrete AFBF60 conversion, then
its +0 multiplier is overwritten before publication. The scalar order is
`atof` to float stack spill, conversion call, `FLD float`, `FMUL double D7A358`,
`FSTP float` to parameter +0. D7A358 has bits `3f847ae140000000`; it is borrowed
and read after conversion, not substituted with a rounded literal or float.
The implementation uses x87 instructions for this sequence. Repeated
properties overwrite the old pointer without releasing it, as native does.

Unknown names release builder/name state. Only cone then repeats the Emitter
and Particle token checks; sphere and SmartArea advance directly. This small
but observable allocation/consumption distinction is explicit in the common
loop. No default curves, missing-token text, parameter values, or child owners
are synthesized. Fresh construction alone still does not initialize omitted
curve fields, and malformed input retains the native invalid-memory risks.

## Flags and child publication

AFA650 recognizes PartEmissionType, EmitEmissionType, Looping,
FollowDirection, and Stops in that order. Emission values create a separate
pooled C-string copy before their original token is released. AFA370/AFA4E0
construct and compare actual NativeString temporaries with `PerSec` then
`PerMeter`, setting +74/+78 to 0 or 1; unknown values leave the field untouched.
Temporary allocation/release order, the first literal's captured pointer, and
the reloaded release size remain represented.

The three boolean flags use signed Windows `atol(value)>0`. Looping stores
byte +15 and, when true, stores 1 at current pointer +10's byte +66. False does
not clear that external byte. FollowDirection stores +1D; Stops stores +1C.

`Emitter name kind` interleaves token and NativeString construction for name
and kind, calls the concrete existing AF9FB0 factory with current +10, parent,
and the same TextBuffer, then destroys both temporary pairs before AF9F00.
The factory's captured current parser14 must route to these actual methods or
another real application implementation. Recursive emitters are not replaced
with a second factory or a successful placeholder.

AF9F00 ignores a null child; otherwise it x87-adds child +18 to parent +50,
stores the child at +3C+count4C*4, and increments the current count. AF9F20 has
no null check, adds child +24 to parent +6C, publishes at +54+count68*4,
increments the current count, then reloads +50+newCount*4 as its EAX result.
There is no added reference increment or capacity check. Reads after stores
preserve native alias effects.

## Required application composition

AF44C0 is an explicitly required suffix service: ECX four-byte source, stack
output/index, RET8/EAX output. Its complete body and AF4450 producer were
inspected. It finds the requested space-separated suffix and constructs a
pooled copy; AF4450 allows spaces/tabs within it. AEE3C0 single-token extraction
cannot replace this operation. The scratch fixture executes the original
installed-byte AF44C0/AF4450 bodies through real allocation/copy services.

B00CE0 remains an explicitly required particle child factory: ECX actual8h
kind, EDX actual8h name, stack parent/TextBuffer, RET8/EAX child, including the
child's captured virtual08 parse. Its full body through B00ED8 was inspected.
It selects five particle classes, reuses the parent on unknown kind, and still
dispatches after allocation failure. Those outcomes are not made successful.
The definition parser preserves its real arguments and publication ordering;
this packet does not reconstruct the five particle constructors/parsers.

These are new C++ entry points through required real services, not original
thiscall/FH3 binary replacements. The current foreign parser/member dispatcher,
native exceptional unwind, and in-game application loading remain separate
integration obligations. No Ghidra mutation was performed by this worker.

## Validation

`scripts/build.ps1` passed the existing Release Win32 build and 2/2 CTests,
including the seed-enabled native math differential check.
The worker adds no CMake registration or permanent test. The new source also
compiled with MSVC Win32 `/W4 /WX /fp:strict /MD /O2 /std:c++17` against the
concrete sibling headers/sources. All eight existing native seed spans matched.

The focused scratch fixture compared copied original bodies for all nine
owned routines against the reconstructed methods. It used actual AF44C0 and
AF4450 slices and shared concrete parameter/pooled-text dependencies. Three
shape inputs covered all common/derived names, all flag names, a nested sphere
emitter, unknown properties, repeated Lifetime, and trailing text; a fourth
covered EOF without an opening brace. Field/value bits, cursor, nested order,
and all 3,879 string-pool events matched. A direct member observation also
matched AF9F20 sum/count/slot/EAX and AF9F00's null no-op.

The fixture used finite Const curves and normal execution. It does not prove
independent parameter/pooled-text correctness, original EH, B00CE0 child
construction, malformed/null behavior, or gameplay. Probe executables include
`/MANIFEST:EMBED`. Commands, native byte captures, generators, sources, logs,
and hashes remain under `C:/Users/sqz269/bsp-ao-definition-loading`; the JSON
report records the exact artifacts and verification results.
