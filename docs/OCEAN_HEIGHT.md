# The water height sampler

Addresses: 0078CF20, 0078C890, 00B9CF50; read as contracts 00B960C0, 00BF85B0, 00932D6B,
004462D0, 00825F20.

Packet `cc_ship_inputs`, 2026-09-11. Reconstructed in `include/bsp/ocean_height.hpp` and
`src/ocean_height.cpp`; semantic C++ interfaces for MSVC Win32, not binary replacements.
Names are hypotheses. Project `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. No Ghidra mutation was performed by this worker.

`0078CF20` is the last unreconstructed input to the ship motion tick's throttle gate
(`docs/SHIP_MOTION.md` follow-up `ocean_height`). It has 31 callers across the image, all of
which subtract its result from a point's world `y`.

## `0078CF20`, complete

`float __thiscall(world, float x, float z)`, `RET 8` at `0078CF74`, body
`0078CF20..0078CF76`. `ECX` is the world object the ship motion path reaches through
`[[00E188A8]+19F0h]`; the two sub-calls receive `[world+A8h]` as their own `ECX`.

```
0078CF3E: wave = 0078C890([world+A8h], x, z)     ; x87, spilled as a double at 0078CF49
0078CF5F: mask = 00B9CF50([world+A8h], x, z)
0078CF64: result = float(mask * wave)            ; FMUL qword, then FSTP m32 at 0078CF69
```

Both callees get the same pair in the same order; the arguments are re-loaded from the
caller's own slots for the second call rather than kept in registers, which is why the
listing pushes them twice. The product is formed at x87 register precision and rounded to
float exactly once.

## What it returns for a flat sea: `0.0f`

`0078C890`, `float __thiscall(field, float x, float z)`, `RET 8`, body
`0078C890..0078C93E`, is the wave field:

```
0078C896: if (byte [field+F9h] != 0) return 0.0f;       ; FLDZ at 0078C89F
0078C8A8: k  = float [field+B4h]
0078C8BB: u  = float(k * x);  v = float(k * z)
0078C8DE: u -= 00BF85B0(u);   ; the fractional part, through the CRT x87 helper
0078C903: v -= 00BF85B0(v)
0078C928: h  = 00B960C0([field+BCh], u, v)
0078C92D: return float(h * [field+24h])
```

`00B960C0`'s body was not read, so everything below that call is `contract: unread`. The
shape above is enough for the flat-sea question: the field returns exactly `0.0f` when the
byte at `field+F9h` is set, and the whole result scales linearly with the amplitude at
`field+24h`, so a zero amplitude gives zero as well.

`00B9CF50`, `float __thiscall(field, float x, float z)`, `RET 8`, body
`00B9CF50..00B9D112`, is a coverage mask, not a second height. It walks the `30h`-byte
region array at `field+620h` (count at `field+624h`) and takes the first region whose
`(+10h,+14h)`-`(+18h,+1Ch)` rectangle contains `(x, z)`. Inside one, it forms two integer
bitmap indices from the region's `+20h`/`+24h` scales and `+28h`/`+2Ch` dimensions, each
clamped to `dimension - 1`, reads one byte from the bitmap at `region+4h`, divides it by the
float at `00CE4B48` and clamps the quotient to `[0, 1]`. Note the second index is formed from
`1 - scale*(z - z0)`, so that axis is flipped relative to the first.

**When no region contains the point it returns exactly `1.0f`.** That is the open-sea case,
and it is why the product form is a height at all: the mask is a per-region attenuation of
the wave field (a harbour or a sheltered bay reading `0`), not a term of its own.

So a flat sea is either the wave field disabled or its amplitude zero, with no region over
the point, and `0078CF20` then returns exactly `0.0f`. The stand-in the ship motion probe
carried was therefore numerically right for a flat sea; what it lacked was the evidence that
`0.0f` is a reachable state of the real sampler rather than an invented default.

## How the throttle gate uses it

`00825F20` samples the keel point and compares (`docs/SHIP_MOTION.md`, the wave gate). The
buoyancy model uses it the same way per hull element (`docs/UNIT_CONTROLLER_UPDATE.md`):

```
00932D6B: above = world.y - 0078CF20(ocean, world.x, world.z)
          span  = e[+08h] - e[+0Ch]
          above = clamp(above, 0, span)
          submerged = span - above
```

and `004462D0` does the same for a debris body. Every consumer treats the result as the
water surface `y`, and a positive wave height therefore lifts the surface and reduces the
computed submersion. Nothing scales it or offsets it on the way in.

## Routines and coverage

| routine | state | coverage |
| --- | --- | --- |
| `0078CF20` | reconstructed, build-tested, probe-exercised | complete |
| `0078C890` | reconstructed down to its two helper calls | partial: `00B960C0` (`00B960C0..?`), the field sample itself, is not read |
| `00B9CF50` | read in full from the decompiler; not reconstructed as code | complete as a contract; the host method carries it |
| `00B960C0`, `00BF85B0` | not read | none; `00BF85B0` is a CRT x87 helper used as a floor |

## Corrections

**To `docs/SHIP_MOTION.md` and `docs/UNIT_FORCE_COMMANDS.md`, `0078CF20` as a host method.**
Both list it as an unread external contract. It is a two-call wrapper, not a leaf: the
product of a wave field and a coverage mask on `[world+A8h]`. A host that models it as one
opaque method is still correct, but the flat-sea value `0.0f` is now evidenced rather than
assumed, and a host that wants waves needs two methods, not one.

**To the packet brief's description of the ocean object.** The brief calls `[game+19F0h]`
"the ocean object". The sampler's receiver is that object, but the wave field and the region
array both live on the sub-object at `+A8h`; `+19F0h` itself only holds the pointer. A lease
on the ocean object's methods would have to name `0078C890` and `00B9CF50`, which are in
segments 44 and 91, not next to it.

## Follow-up packets

| packet | addresses | what it answers |
| --- | --- | --- |
| `ocean_wave_field_sample` | `00B960C0`, `0078C890`'s `field+B4h`/`+BCh`/`+24h`, the producer of `field+F9h` | what the wave field actually samples (a tiled height texture is the obvious shape, given the two fractional coordinates) and who turns it on |
| `ocean_coverage_regions` | `00B9CF50`, `field+620h`/`+624h`, the region record `30h` | who builds the region array and what the bitmaps come from |

## no_ghidra_function

none. `0078CF20`, `0078C890` and `00B9CF50` each lie inside an existing Ghidra function body,
checked with `python tools/bsp.py ghidra proto <addr> --brief`.
