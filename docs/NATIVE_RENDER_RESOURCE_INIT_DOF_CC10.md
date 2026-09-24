# Render-resource initialization: B118AF DOF post effect

Addresses: `00B107F0`, fragment **[00B118AF,00B11D7E)** only.

This fragment constructs and publishes the actual service `+74` post effect,
`HDRFinalPass_DOF.mshd`, then configures two input textures, ten borrowed
parameters and its output color surface. It stops before `B11D7E PUSH 40h`,
which begins the next frame allocation. No complete initialization, teardown,
native FH3/SEH or gameplay behavior is claimed.

| Routine | Coverage | Interface |
|---|---|---|
| `00B107F0` | Partial: only `[B118AF,B11D7E)`, 1,231 bytes. Earlier `[B107F0,B118AF)` belongs to published packets; later `[B11D7E,B13029)` remains excluded. | Native ECX service, three stack DWORDs, eventual `B13026 RET 0C`. New C++ interface consumes a retained predecessor and the same original context. No native return or argument-cell read occurs here. |

The last included instruction is `B11D79 CALL B4CB70`, length five, inclusive
final byte `B11D7D`. The containing function remains `B107F0..B13028`; there
are no missing Ghidra function definitions or new native names. BSP wrappers
verified `C:/Users/sqz269/bsp.gpr` and `/battlestationspacific.exe`; Ghidra was
read only. All 1,231 live bytes match the installed PE, SHA-256
`2334b146cd6fd8034c0707adc288aa87446e430e41810727fa2ab80bcd5ef31c`.

## Construction and current loads

The new retained attempt contains one genuine `NativePostEffect20ConstructionBlock`
and eleven distinct native string headers. Its provider is prepared in the same
existing post-effect context before executing the fragment. Preparation failure
retains its partially prepared block and records the `B118AF` frontier.

At `B118B1`, shared CRT allocation requests `20h`; native caller cleanup is
`ADD ESP,4`. Only after the allocator returns do EDI/ESP14 diagnostics change
to this raw allocation. EH state becomes 39, including the null branch. A
nonnull allocation constructs the name at `B118DA`, sets mask bit `10h`, then
calls existing `B4E470` at `B118FA` with the actual name header, vertex count
three and null optional input. Its original ABI is ECX actual allocation,
three stack words, EAX actual result and `RET 0C`. State is 40 at that call.

The mask test at `B11903` is captured before publishing the returned pointer
into `+74` at `B11908`. The null allocation branch publishes null. No old `+74`
release or extra initialization is invented. EH state disarms before effect
name cleanup. Cleanup captures the current data pointer, clears mask `10h`,
then, for nonnull data, captures current length plus one before obtaining the
current `419CC0` string pool and calling `BD1510`. Headers remain stale.

Texture zero comes from current service `+64` through existing `B4D170` and
`B4CB10`. Current `+74` is read afterward; `B4CBA0` obtains its actual material
and `B189F0` binds slot zero at `B1195A`. After that call returns, current bloom
`+28` is read through `B54CD0/B4CB10`, current `+74` is reloaded, and slot one
is bound at `B1197B`. These existing providers preserve their actual count,
publication/retain/release and current-profile boundaries.

The three additional literal pointers are explicit input through
`NativeRenderResourceInitDofNames`: `D5E348`, `D5E338`, `D5E324`. The other
names come from the original retained continuation context. Live literal bytes
matched the PE. No global, alternate pool or new resource domain is introduced.

## Borrowed parameter registrations

| Literal | Actual source | Words | Registration / EH state |
|---|---|---:|---|
| `cMiddleGray` / `D5E3F0` | service `+8C` | 1 | `B119B5` / 42 |
| `cBloomScale` / `D5E3E4` | service `+94` | 1 | `B11A1B` / 43 |
| `cBloomSampleOffset` / `D5E3D0` | current bloom `+428` | 2 | `B11A83` / 44 |
| `cMinLuminance` / `D5E3C0` | service `+98` | 1 | `B11AE9` / 45 |
| `cMaxLuminance` / `D5E3B0` | service `+9C` | 1 | `B11B4F` / 46 |
| `cSceneColorSampleOffset` / `D5E430` | service `+04` | 2 | `B11BB2` / 47 |
| `cNewHDRParams` / `D5E374` | retained EBP, service `+A0` | 2 | `B11C12` / 48 |
| `cBlurredColorSampleOffset` / `D5E384` | service `+18C` | 2 | `B11C78` / 49 |
| `cFocalPlaneData` / `D5E338` | service `+26C` | 4 | `B11CDA` / 50 |
| `cFocalPlaneData2` / `D5E324` | service `+27C` | 4 | `B11D3C` / 51 |

All sources remain actual addresses; this fragment neither loads their float
values nor initializes their fields. The table uses original shader names and
does not infer new service layouts. Receiver-before-source order is preserved
for rows two, four, five, seven and eight. Bloom is reloaded after its temporary
name construction, and row seven uses inherited EBP directly.

The one/two-word calls reuse `B18B20/B18B00` wrappers (`RET 8`). The last two
native sites use `B18AC0` with vector count one. That existing 32-byte wrapper
`[B18AC0,B18AE0)` computes wrapping DWORD `4 * vector_count`, forwards matrix
zero to `B17E10` at `B18AD8`, then executes the three-byte `RET 0C` at `B18ADD`.
The source reuses the established expansion from
`src/native_render_pass_initializers.cpp`: actual `B17E10` with words four and
matrix zero. This is caller composition, not a new standalone library port.
The wrapper's 32 live bytes also match the PE.

Each registration is followed by current name-data capture, EH disarm to `-1`,
current length-plus-one capture, current pool acquisition and `BD1510` return.
That return receives `(data,length+1,1)`, consumes `0Ch` and ignores the third
word. The raw headers and diagnostic pointers do not own returned storage.
Native private stack aliases and instruction-exact fault behavior are excluded.

Finally `B11D70` obtains current service `+4C`'s primary surface via `B4CB20`.
Current `+74` is reloaded before `B11D79 B4CB70`, which reloads its actual `+08`
frame and calls existing `B1FAB0` for color slot zero. Incoming publication and
retain precede captured outgoing release inside that existing provider.

## Retained state and lifetime boundary

The previous post header changes only by adding two phase values and a one-use
`dof_identity`. Entry checks require the complete predecessor chain, original
context identity, original argument cells, exact `B118AF` frontier, mask zero,
EH state `-1` and inherited EBP. The stage claims its predecessor before provider
preparation. All four state records advance together or become failed together.

`B4E470` already supplies its canonical `NativePostEffect20Reference`, borrowing
the actual `+04` count in the same actual-owner domain. No second pass companion
or resource credit is created for `+74`. Its constructor's nested ownership and
registration paths remain authoritative; no blanket holder/surface/texture
registration is added. The seven service-pass companion registrations remain
an external gap, and full-service release is still not admitted.

On failure the caller retains provider preparation/acquisition records, every
previous child block, name headers, actual publications and current diagnostics.
Existing child providers keep their own failure semantics. In particular,
`B4E470` may have completed native construction before host companion binding
fails; the caller does not free that raw object or replay its cleanup. No caller
rollback, retry, resource free, metadata unbind or automatic child reset occurs.
Caller storage, original context, service, argument cells and all predecessor
records must remain alive and immovable. Existing explicit child quiescence
requirements still apply before destruction/reset, including failed preparation.

At `B11D7E`, ESI is the same service, EBX is `FFFFFFFF`, EBP remains service
`+A0`, and EDI/ESP14 retain the raw `+74` allocation. ESP20 retains half aligned
width, ESP58 aligned width, ESP24 aligned height, ESP18/1C original dimensions
and ESP1D8 half aligned height. Mask is zero and EH state is `-1`. Original
argument cells remain borrowed without reads. These are source diagnostics,
not a native machine stack/register image.

## Verification

The report verifier passed all 66 rows: 65 caller-fragment calls and the
existing `B18AC0 -> B17E10` supporting row. Native seed verification matched all
eight existing seed ranges. The final `./scripts/build.ps1` passed MSVC Win32
Release `/MD` and all three existing CTests: `reconstructed_math`,
`native_math_differential`, and `tool_tests`. No new tests, native probe or app
wiring was added. Math tests and earlier leaf probes do not establish composed
DOF runtime execution, full initialization/teardown, native ABI/FH3 or gameplay.
