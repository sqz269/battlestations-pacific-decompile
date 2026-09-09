# Gamma capability and cache initialization

This bounded audit closes the initialization questions in
[GAMMA_RAMP.md](GAMMA_RAMP.md). The target was verified as saved project `bsp`,
program `/battlestationspacific.exe` before each Ghidra batch. Executable
displacement searches were used only to locate candidate instructions; receiver
identity and subobject offsets were checked in the surrounding code. No code,
Ghidra mutation, build or display-gamma change was performed.

## Capability bytes come from D3DCAPS9.Caps2

Renderer constructor `00b32410` initially sets `+1B53h` and `+1B54h` to zero at
`00b326e3` and `00b326e9`. Later in that constructor, `00b3289b` calls
`00b2c8e0` with ECX renderer. This routine gathers many renderer capabilities;
only its immediate gamma-producing prefix is audited here.

At `00b2c91a`, the routine calls the IDirect3D9 pointer at renderer `+1990h`,
virtual `+38h`, with `(this, adapter=0, device_type=1, &caps)`. This is
`IDirect3D9::GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &D3DCAPS9)`.
The buffer starts at the pre-call stack baseline `ESP+1F4h`. The DWORD loaded
from `ESP+200h` after the call is therefore buffer offset 0Ch, **Caps2**, not
Caps or DevCaps2.

The local Windows SDK header
`C:/Program Files (x86)/Windows Kits/10/Include/10.0.26100.0/shared/d3d9caps.h`
defines the structure fields and the two exact bits:

| Renderer destination | Native operation | Source capability |
| --- | --- | --- |
| `+1B54h`, written at `00b2c936` | `(Caps2 >> 20) & 1` | `D3DCAPS2_CANCALIBRATEGAMMA = 00100000h` |
| `+1B53h`, written at `00b2c96a` | `(Caps2 >> 17) & 1` | `D3DCAPS2_FULLSCREENGAMMA = 00020000h` |

There is no HRESULT check after GetDeviceCaps and no zeroing of the output
buffer before that call in this prefix. A failed capability query must not be
treated as establishing false flags. The constructor's earlier zero values
would be overwritten from the unvalidated output. A typed failure result is
an explicit host boundary, not evidence of native fallback behavior.

The only direct call reference returned for `00b2c8e0` is the constructor call
above. The inspected ordinary device-reset paths do not invoke it again. This
does not prove the absence of every indirect call in the entire executable,
but it establishes the constructor source of these two fields and avoids
guessing them from current windowed/fullscreen presentation settings.

## Cached gamma is initialized through a subobject

Renderer construction passes **renderer+34h** as ECX to `00b29430` at
`00b3246c`. At `00b29473` that cache-subobject constructor XORPSes XMM0, then
stores positive float zero at subobject `+1938h` via MOVSS `00b29476`.

```text
renderer+34h + cache+1938h = renderer+196Ch
```

This is the cached float read by gamma setter `00b21960`. Searching only for
displacement `196Ch` misses its initialization because the instruction uses
the cache-subobject base. The ordinary constructed initial cache is exactly
positive zero, not a sentinel or an uninitialized field.

The runtime gamma setter writes the original requested float to renderer
`+196Ch` at `00b21a01`, but only after the capability gate and ordered-equality
comparison permit the update. It does not store the clamped normalized value.
Getter `00b1fee0` simply FLDs this same field into x87.

An executable displacement search also found a `+196Ch` write in unrelated
`004ddb90` and several `+1938h` candidates outside the renderer. Offsets alone
are not object identity; those hits are not treated as renderer gamma writes.
The scoped confirmed writes are the cache constructor and gamma setter.

## Binding-cache clearing preserves gamma

`00b241c0`, called with renderer+34h, releases logical binding references and
clears binding states. Its trailing sequence clears cache fields through
`+1934h` (DWORD index 64Dh) and stops; gamma at `+1938h` (index 64Eh) is not
included. The constructor `00b29430` explicitly initializes gamma separately
before invoking this clear routine. It is therefore incorrect to model binding
cache invalidation as zeroing the entire typed cache including gamma.

`00b26170` initializes render and sampler defaults through cached setters. Its
inspected body does not write the gamma cache or the two capability bytes.
The reset routines that subsequently call the gamma virtual pass the cached
value itself:

| Caller path | Cached float load | Virtual `+F0h` call |
| --- | --- | --- |
| `00b29670` | `00b2990f` | `00b29923` |
| `00b29e60` | `00b29fc8` | Following indirect `+F0h` invocation |
| `00b2abd0` | `00b2adae` | `00b2adc2` |

The inspected cache invalidation/default-state sequence preserves that cached
float; there is no established gamma-invalid bit or forced sentinel write in
these bodies. External callbacks, broader renderer re-creation and concurrent
changes remain outside this bounded negative claim. Destruction and construction
of an entirely new renderer naturally reinitializes both capability/cache state.

## Startup zero versus reset cached input

Startup's virtual `+F0h` call supplies positive zero. For an ordinarily
constructed renderer with no intervening gamma request, cache is already
positive zero. If FULLSCREENGAMMA is false the setter exits at its first gate;
if true the ordered-equal cache comparison skips generation. Thus startup zero
normally causes **no SetGammaRamp call**, not an unconditional identity-ramp
upload. A prior request or exceptional cached value can change this outcome.

Reset passes the cached float, so finite unchanged values similarly compare
ordered-equal and skip. NaN remains unordered and can run the exceptional power
pipeline on repeated requests, as detailed in GAMMA_RAMP.md. Calibration byte
`+1B54h` affects only the eventual SetGammaRamp flag, selecting 1 when the
CANCALIBRATEGAMMA bit is present and 0 otherwise. It does not bypass the
FULLSCREENGAMMA or cache gates.

A faithful reset dependency should preserve these virtual-call and cache
semantics. There is no evidence here supporting a forced gamma upload on every
reset, a cleared gamma cache during ordinary binding invalidation, or a forced
fullscreen capability from the presentation mode.

## Verified fragments and remaining boundaries

End addresses are exclusive. These are instruction fragments, not newly
reconstructed complete routines. All rows matched installed PE and saved-program
bytes; evidence JSON is ignored at
`exports/bsp/owner_textures/gamma/capabilities_evidence.json`.

| Start | End | Bytes | SHA-256 |
| --- | --- | ---: | --- |
| `00b2c8e0` | `00b2c970` | 144 | `fbbd5535a4a1a64318f44099cb91bf1455803d1ccef2514f3c79a9f250080386` |
| `00b29473` | `00b2947e` | 11 | `4f43f5b3d36c2f964adf73287b175f8a792227496c4f8bfa57e8595ad2563131` |
| `00b326e3` | `00b326ef` | 12 | `f6a9609bb8fe61bb9c8d69c34b1944fe74f780981360060f306c8424d5370e73` |
| `00b2990f` | `00b29925` | 22 | `e2a583c3e718cc52ef267c4d8cd17ef3ff42fbdb19ffff3219d75f032e15d88d` |

Remaining work includes the gamma power helper's exact numerical/exception
contract, full capabilities routine reconstruction, runtime query failure
behavior and full reset callback integration. This audit provides static field
provenance and call behavior; it does not report a successful display-gamma or
game-reset experiment.
