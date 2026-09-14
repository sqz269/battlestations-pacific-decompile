# Actual system-constant gathering

Addresses: `00B46A70`, `00A8FCF0`, `00A8FD10`, `00A8FD90`, `00A8FDB0`,
`00B3CE50`, `00B3CE60`, `00AF0460`, `00AD5700`, `00AD5740`, `00B7AAB0`, `00BEE070`.

The source executes the complete B46A70 instruction schedule against raw
camera, scene, renderer, service, clock, foliage, fog, light and shadow storage.
It uses the existing raw camera/matrix/fog/ambient/scene and renderer-upload
providers. No `CameraFrameState`, `D3D9StateCache`, `SystemShadowMapOwner` or
texture companion is substituted for a native owner.

| Routine | Coverage | Original ABI |
| --- | --- | --- |
| B46A70..B47638 | complete, 3017 bytes | ECX optional scene, EDX camera, RET |
| A8FCF0..A8FD0E / A8FD10..A8FD2E | complete, 31 bytes each | ECX shadow, EAX borrowed texture; RET or tail target getter |
| A8FD90..A8FD93 / A8FDB0..A8FDB3 | complete, 4 bytes each | ECX raw target, EAX `[+10]` / `[+18]`, RET |
| B3CE50..B3CE53 / B3CE60..B3CE63 | complete, 4 bytes each | ECX raw texture, EAX unsigned `[+28]` / `[+2C]`, RET |
| AF0460..AF0463 / AD5700..AD5703 | complete, 4 bytes each | ECX raw foliage owner, FLD `[+2C]` / `[+08]`, ST0, RET |
| AD5740..AD5743 | complete, 4 bytes | ECX raw foliage owner, AL `[+11]`, RET |
| B7AAB0..B7AAB6 | complete, 7 bytes | ECX raw light, EAX `[+174]`, RET |
| BEE070..BEE073 | complete, 4 bytes | ECX raw clock, EAX address `+40`, RET |

BEE070's four bytes were verified directly even though the saved project has
historically lacked a containing function at that leaf. Names describe observed
behavior and are hypotheses, not recovered symbols. Ghidra remained read-only.

The context borrows current actual global cells and numeric original profile
views. It does not cache their pointed-to owners. The raw sampler singleton
getter is the complete existing 4DE4B0 implementation, including its persistent
`NativeSamplerLoaderOperation` and actual AA0 manager behavior. Its +18 shader
time remains an allocation preimage until a real writer changes it. The source
does not initialize that word.

The caller supplies 1232 initialized bytes as the original prefix's stack
preimage. B46A70 never clears this prefix: matrix gaps, float3 padding and absent
fog, scene, shadow or context values stay untouched. The source copies these
bytes into its private local before executing the native writers. This is an
explicit source input; it is not a claim about unknown original stack contents.
Each call needs a fresh persistent frame, whose sampler operation survives any
failure and cannot be retried.

The register and stack trace establishes six transposed camera/service matrices,
camera translation, sixteen current parameter words, current projected axes,
clock and foliage scalars, optional fog fields, optional scene lighting and
shadow fields, the optional camera+43C float4, and finally two constant uploads.
The scene pointer is captured in EBP and saved at local+10 before fog uses EBP
as its loop counter. Lighting obtains scene+1C, then lighting+1C's first native
list identity and light+8; a returning `_invalid_parameter_noinfo` handler does
not replace the captured list identity. The host CRT is an explicit exception
and invalid-parameter-domain boundary.

All original x87 loads, spills, divide direction and SSE copies remain explicit.
Clock interval uses FILD64/FILD64/FDIVP/FSTP32. Unsigned shadow dimensions use
FILD32, conditional addition of current CE3978, then FSTP32; reciprocal results
use the original shared FLD1/FDIVRP/FXCH/FDIV schedule. Shadow matrix stores use
MOVSS and retain their exact transposed read order.

| Indirect site | Current accepted profile and substantive provider |
| --- | --- |
| B46D04 | timer D68D50/+1C=BEE070; raw interval address |
| B47541, B4756C | shadow D5B5D8/+08=A8FCF0; current F8BBF0 target flags or shadow+384 fallback |
| B4754A | texture D61948/+3C=B3CE50; reported width at +28 |
| B47575 | texture D61948/+40=B3CE60; reported height at +2C |

Original numeric profile addresses are identities. The source reads and checks
the corresponding target through the borrowed profile at the dispatch boundary;
it never dereferences a numeric game table as a host address or calls its numeric
entry. Other derived profiles fail at that reached boundary. A8FD10's color
sibling and both target leaves are also supplied for consumers, but B46A70 uses
only the depth getter. Both target flag reads use the captured global target;
false flags reload the receiver's fallback. No retain, allocation or COM size
query occurs. Reported +28/+2C differ from saved dimensions +34/+38.

The final VS and PS paths independently reload current E13078 and F8D394. The
same local prefix reaches both. A failed HRESULT does not suppress PS. Existing
raw B21820/B218C0 execute current real COM dispatch, original counter writes and
optional synchronization cleanup. There is no outer gather rollback or cleanup.

The dependency `34c5ce54` supplies the three raw B7AA20/B7AA30/B7AA40 light
environment getters unchanged; it was consumed as narrow cherry-pick `c03a4cb0`.
The generated-model dependency remains ancestry merge `ca2fdcdd` and is not
needed to execute this gatherer independently.

Validation uses full live-Ghidra/installed-PE span comparison, the strict Win32
build after all eight seed comparisons, both existing CTests, the mechanical
direct-call gate, and one real-D3D9 fixture with two focused scenarios. The
fixture retains real device vptrs, temporarily observes the original COM slots
94/109, executes the underlying real HAL methods, and restores/checks every
observed slot before releasing both devices. The first upload changes the
current renderer and count and returns an injected failure after the real call;
the second uses the new renderer and count. Scene-null and raw fog/lighting/
shadow paths execute; caller-provided raw fixture storage is not evidence of
the whole game's constructed owner graph. Copied original four-byte width and
height instructions are compared with their source counterparts.

Full original B46A70 execution was not established. The installed PE lacks a
relocation directory, and initial attempts to reserve its preferred low address
ranges found occupied process mappings. The final fixture keeps the PE as raw
input data and executes only the bounded original leaves. Full original callee
ABI, native private stack aliases, asynchronous faults/FH3 unwind, nonreturning
or throwing profile alternatives, active EndFrame and game/visual validation
remain unproved. The report carries exact logs, manifest hashes and the final
immutable compiler/link/tool/runtime artifact closure.


## Primary captured-profile correction

The original reads each owner profile once at B46CFF, B4753A, B47543, B47565 and B4756E. Integration forwards those captured values into the private dispatch bridges; it removes the extra owner[0] reads previously made inside the bridges. The current borrowed table slot is still read at dispatch. Each private fastcall bridge consumes its added stack word with RET4; the public gather interface and prefix layout are unchanged. The original80027137 worker archive remains frozen, and combined-library validation follows this correction.

## Primary integration at1bbc1662

The combined library includes the raw gather and three consumed light-environment address getters. Primary review corrected five private bridges to use the profile captured by the original instruction; public context/API is unchanged. The strict MSVC Win32 build, eight native seeds, both existing CTests and47 numeric call rows passed. Fresh Ghidra and installed-PE captures matched24 spans/5157 bytes including parent/profile context. Fifteen existing names/comments were preserved, augmented, saved and read back; affected exports were refreshed. BEE070 was already present as a saved function.

Two real Direct3D9 HAL source cases passed with the same308-word prefix at VS/PS upload, original preimage preservation, current renderer/count changes after ignored VS failure, and raw fog/lighting/shadow with unsigned dimension/x87 reciprocal behavior. Original/source execution covered only the four-byte width/height leaves. The full original gather, uncached axes, private ABI/FH3/SEH and game behavior remain unexecuted. Worker frozen evidence describes80027137; the primary checkpoint describes corrected1bbc1662.

Immutable checkpoint: `local/checkpoints/1bbc1662/native-system-constant-gather-wave/validation.json`; SHA256 `78ee9018977843340db954b871be19c91a27e56c175f83a216053571a02b2f66`; 3377 artifacts and58 actual mapped runtime modules. The checkpoint retains exact sources, build/probe inputs and outputs, compiler dependencies, searched libraries, tools, native bytes and annotation before/after state.
