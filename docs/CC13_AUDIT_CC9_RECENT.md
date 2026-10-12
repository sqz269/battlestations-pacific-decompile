# Audit of cc9's recent behavioural landings (cc13-audit6)

Addresses: 0081DE10 00864680 00848410 00BF0DF0 00B94CA0 00B95750 00B96B90 00B96C10 00B96C50 00B97100
00B952D0 00B95AA0 00B95D30 0078C9B0 00B9A450 00B9A4C0 00BA0090 0078CFF0 0068A0D0 0078CF20 0078C890
00B960C0 00B9CF50 00BA0C00 00BA1370 008830AA 00795C10 00797ED0 0078E630 0078E6F0 007A0650 00791C90

Packet `cc13-audit6`, branch `agent/cc13-audit6`, 2026-10-12. Requested by the cc13 lead for cc9. **Read only:**
no Ghidra writes, no ledger records, no edits to cc9's files. Findings that need a change are given as unified
diffs for cc9's lanes to apply. Base: `origin/main` at `5a4509b2c`.

**Selection.** `git log --since="36 hours ago" origin/main` over `src/game_hosts_*.cpp`, `src/units.cpp`,
`src/ship_*.cpp`, `src/dyn_*.cpp`, `src/hull_*.cpp` and the ocean files they bind. The 11 items already in
`docs/CC13_AUDIT_CC9_SWITCHES.md` are excluded: the tail-gunner, line-of-fire, bomb-drift, kill-queue,
retire-queue, land-command, lift-height, capture-party, torpedo-group and engine-movie switches, and the GUNNERY
165/167 hull-contact fix. Also skipped as non-behavioural: `f9fde7416` (harness helm-order syntax), `45f751b46`
and `c2a886a8a` (harness and trace plumbing).

**Method.** For each item I read the diff, then compared the C++ with the disk listing (`bsp.py disasm-raw`, bytes
from the installed executable). Every constant was read from the PE at its load width (`tools/pe_const_read.py`).
For each x87 sequence I tracked the register stack by hand through every FXCH, FSUBR/FDIVR and pop. Stack
offsets were recomputed after every PUSH and POP, because several of these routines reuse argument slots as
temporaries.

**Checklist.**
- Rule 2 (boundary before attribution): `ghidra proto --brief` places `0078D184` in `0078CFF0`
  (`0078CFF0..0078D1A8`), `0081DE10` in its own body (`0081DE10..0081E34E`) and `00BA0EB0` in `00BA0C00`
  (`00BA0C00..00BA136A`).
- Rule 7 (argument counts):
  - `00B96B90`: `RET 1Ch`, seven arguments;
  - `00BF0DF0`: `RET 8` (mean, sigma);
  - `00B94CA0`: `RET 8` (kx, ky);
  - `00795C10`: `RET 8`;
  - `0078E630`: `RET 4`;
  - goaround slot `+14h` (`0078E6F0`): `RET 4`.
- Rule 8 (register provenance): the `EBP` in `00BA0C00` is the frame pointer (`00BA0C01 mov ebp,esp`), so
  `[ebp+8]` is argument 1.
- `tools/verify_report_calls.py` takes a `reports/*.json`. This audit has no host table, so there is nothing for it
  to check.

**Status vocabulary.** Every item is reconstructed and build-tested on main. "FAITHFUL" means the audited path
matches the listing in:
- control flow, comparison sense and NaN behaviour;
- constants, operand order, call order and argument order;
- RNG draw count, order and stream.

It does not mean game-validated. Precision follows `docs/CC13_FPU_PRECISION.md`: the host evaluates in double
and rounds at every image `FSTP`. That matches the image at 53 or 64 bits. On the main thread's 24-bit control
word, products of floats may round differently in the last place. This affects every x87 item here equally and
is not repeated below.

## Verdicts

| # | Item | Commit(s) | Verdict |
|---|---|---|---|
| 1 | `kShipyardHangarHiddenBound` (`0081DE10` in `00864680`), ON | `0949e06c1`, `ccd374cda` | FAITHFUL for the footprint arm; the two labelled gaps are real |
| 2 | Ocean spectrum build: polar normal draw, Phillips term, initial spectrum, setters, constructor | `7433d478a` | FAITHFUL; private Mersenne state seeded `1105h`, not stream `00BD2F10` |
| 3 | 64-point FFT `00B952D0` and time advance `00B95D30` (height vector) | `7433d478a` | FAITHFUL; choppy vectors correctly labelled partial |
| 4 | Live field wiring: `0078C9B0` scene keys, `00B9A450`/`00B9A4C0`, render step `00BA0090` from `0068A0D0` | `7433d478a`, `ce8d63ef1`, `e103dabd0` | FAITHFUL |
| 5 | Water-height readers the live field feeds: `0078CF20`, `0078C890`, `00B960C0` | `4ce2c2bd0` (exposure; code `ec611f564`) | FAITHFUL |
| 6 | Coverage mask `00B9CF50`, which `eb9fbc91b` makes reachable | `eb9fbc91b` (code `ec611f564`) | **DEFECT** (NaN only): the host takes NaN coordinates as inside a region |
| 7 | Wave-damp region box `00BA0C00` via `00BA1370` | `eb9fbc91b`, `e103dabd0` | min/max FAITHFUL; which meshes it covers and their translation are **UNCERTAIN** |
| 8 | `kMovieKeyframeModifierBound` (`00795C10` modifier call, `00797ED0`/`0078E630` events), ON | `1115e208c`, `aacd10eb7` | FAITHFUL |
| - | GUNNERY 174 (`b2a0d0a04`) | | logging only; no behaviour to audit |
| - | `kMissionCheckpointBound` (`01557b829`), OFF | | not audited in depth: OFF, and no installed script reaches it |

## 1. `kShipyardHangarHiddenBound` (GUNNERY 176, flipped ON in 176.1)

**`00864680` caller side.**
- `008646B5 comiss xmm0(0),[esi+10h]` / `JB 008647B9` refreshes only when the record's timer is `<= 0`.
- `008646C2` clears `+20h`. The next three steps fill it again:
  - `008646C8..008646CD` calls `vtable+5Ch(6)`, which is `IsKindOf(6)`;
  - `008646D5` calls `0081DE10`;
  - `008646DA` stores the result into `+20h`.
- `008647A4..008647B0` re-arms the timer with `0.5` (`00CE3800`).
- `008647B9 cmp byte [esi+20h],0` / `JNE 008646A1` returns `AL = 0` before the ray.

The host evaluates the test at every LOS call, not on the 0.5 s refresh. The comment labels this.

**`0081DE10`.**
- **The `+1130h` arm.** `0081DE31 cmp [ebp+1130h],0` / `JLE` returns 1 when the value is `> 0`. The host labels
  this arm as unbound.
- **Ship ends.** The vector `(0, 0, class+A0h * 0.5)` goes through the ship's `+CCh` matrix (`004142E0`): the bow
  at `0081DEB2..0081DEDD`, the stern with `FCHS` at `0081DEEE..0081DF37`. The constant `00D7A280` is the double
  `0.5`.
- **The yard walk.** For each node of `[[00E188A8]+19CCh]+364h`, `00848410` expands `+780h..+788h` and keeps
  objects with `+5Ch` set and `+5Dh`, `+60h` and `+5Eh` all clear (`00848458..00848471`). This is
  `shipyard_hangars_usable`'s filter.
- **The limits.**
  - `limX` is the object's `class+A0h * 0.6000000238418579` (`00CEFF98`, a double).
  - `limZ` is the object's `class+A4h * 0.75` (`00CEC9D8`).
- **The local frame.** Both ends go through the object's `+110h` inverse (built by `00B63D50` from `+CCh`): the
  bow lands at `[esp+8Ch..94h]` and the stern at `[esp+80h..88h]`.
- **The tests.**
  - The absolute value is `x > 0 ? x : -0.0 - x` (`00D7A208` is `-0.0f`).
  - Bow: `0081E22A fcomi` with `JBE` leaves unless `limX > |bx|`, then `0081E25D fcomi` with `JA` hides when
    `limZ > |bz|`.
  - Stern: `0081E290 fcompi` with `JBE`, then `0081E2BC fcompi` with `JA`.
  - A NaN coordinate fails every test, as `std::fabs(NaN) < lim` does.

**Verdict: FAITHFUL** for what the switch binds.
- The host's `lx` (dot with the right axis) and `lz` (dot with the forward axis) are the inverse of an orthonormal
  frame. Unit scale is labelled.
- Length and Width come from the class rows `length` and `width`, which are `class+A0h` and `class+A4h`.
- The `+1130h` arm is still the gap that matters for freshly launched boats; GUNNERY 176.1 already traces its
  writers.

## 2. Ocean spectrum build (`7433d478a`)

**RNG stream.** `00B95760 push 1105h` and `00B9579B call 00BF0CF0` seed a stack Mersenne state at
`[esp+24h]`, local to `00B95750`. The shared generator `00BD2F10` is never touched. The host's
`RandomState state{}` with `kOceanSpectrumSeed = 0x1105` matches. The scene's ocean therefore cannot shift
gameplay RNG pairs.

**`00BF0DF0`, the polar normal draw.**
- **The words.** The x87 stack is pre-loaded with `2^-32`, `2.0` (`00D7A308`), 1 and 1. Each word is tempered
  inline (`0xFF3A58AD << 7`, `0xFFFFDF8C << 15`, which are MT19937's masks pre-shifted), then:
  - `FILD`;
  - `+2^32` when negative (`00CE3978`);
  - `* 2^-32`, stored to float;
  - `*2 - 1`, stored to float.
- **Draw order.** `u1` (stored `[esp+8]`) is drawn first, then `u2` (`[esp+4]`).
- **The loop.** `s = u1^2 + u2^2` is stored to float. `00BF0F50 FCOMI` with `JAE` loops while `s >= 1`. There is
  no `s == 0` rejection, and the host has none either.
- **The result.**
  - `log(s) * -2.0` (`00D19660`) `/ s`;
  - `sqrt`;
  - `* u1` (after `00BF0F8C pop esi`, `[esp+4]` is `u1`, because `s` overwrote `u2`'s slot at `00BF0F48`);
  - `* arg2 + arg1`, that is `* sigma + mean`.

  The host matches each step and its rounding points.

**`00B94CA0`, the Phillips term** (args `kx`, `ky`; ECX = spectrum).
- `ky^2` is stored first, `kx^2` is kept extended, and their float sum is taken.
- `00B94CE9 FCOMPI` with `JBE` clamps `k` to `1e-8` (`00CF7FE8`) only when the minimum is greater. A NaN `k` is
  kept, as in the host.
- `00B94D05 FDIVRP` gives `1/k`.
- `00B94D31 FDIVR qword [00D7A250]` gives `-1.0 / kL^2`. That value is kept extended and `kl^2` is subtracted.
  The host's `neg_inv` is a double, not a float, which matches.
- Directional term:
  - `dot = (kx/k) * cos + (ky/k) * sin`;
  - `00B94DAB FUCOMPI`, `LAHF`, `TEST AH,44h`, `JP` gives 0 only when `dot == 0`;
  - a NaN takes the `pow` path, which is the host's `value == 0.0f` test.
- `|dot|^power` and `|k|^4.0` (`00CE3D34`) come from `FYL2X`/`F2XM1`/`FSCALE`.
- The result is `A * e * D / k4`, rounded once. The post-`POP ESI` offsets map to `e`, `D` and `k4` in that
  order.

**`00B95750`, the initial spectrum.**
- `L = speed^2 / 9.81` (`00CF9058`) is stored to `+78h`; `l = L * +8Ch` to `+7Ch`.
- `h0` is built over 65 x 65 cells (`n` runs to `JLE 20h`, the row test is `cmp edi,1040h` with `JLE`):
  - `g1` is drawn at `00B957D9`, then `g2` at `00B957F4`;
  - `P(kx, ky)` is evaluated, then `scale = float(1/sqrt2) * float(sqrt(P))`;
  - the stores go to `+0` (re) as `scale * g2` and to `+4` (im) as `scale * g1` (`00B95884`, `00B95894`).
- The `1/k` vector `+60h` uses the clamped `k`. The `omega` vector `+50h` is
  `sqrt(float(float(sqrt(sum)) * 9.81))`, with no clamp. The image runs two separate 64 x 64 loops; the host fuses
  them, which is equivalent because they are independent.

**Setters and constructor.**
- `00B96B90` maps `arg2..arg7` to `+6Ch/+70h` (FCOS, FSIN), `+74h`, `+80h`, `+84h`, `+88h`, `+8Ch`. `arg1` (the
  seed) is never stored.
- `00B96C10(dir, speed)` and `00B96C50(height, choppy)` match.
- `00B97295..00B972DA` pushes `00D638CC..00D638E0`: `1.570796`, `32`, `60`, `2.0`, `0.02`, `5e-5`.

**Verdict: FAITHFUL.**

## 3. FFT `00B952D0` and advance `00B95D30`

**The butterfly** (`00B953CC..00B9542F`).
- `t1 = x1*u1 - y1*u2` and `t2 = y1*u1 + x1*u2` are each stored to float.
- `x[i1] = x[i] - t1` and `y[i1] = y[i] - t2` are written before `x[i] += t1` and `y[i] += t2`.

**The twiddle recurrence** (`00B9543D..00B95474`).
- `z = u1*c1 - u2*c2`, then `u2 = u1*c2 + u2*c1`, then `u1 = z`.
- `c2 = float(float(sqrt(float(1 - c1))) * inv_sqrt2) * sign`, computed before
  `c1 = float(float(sqrt(float(c1 + 1.0))) * inv_sqrt2)`.

**Scaling.** For `dir == 1` the image multiplies by `0.015625` (`00D04FC8`), which equals the host's `/ 64.0`.

**The advance.**
- **The phase.** `phase = float(omega[w] * t)`, then FCOS and FSIN.
- **The indices.** `k` is `ecx + 820h` and `-k` is `840h - ecx - n`; that is `idx` and `4224 - idx`.
- **The register trace** (`00B95E65..00B95EA5`) gives:
  - `re = (a + b) c - (d + cm) s`;
  - `im = (d - cm) c + (b - a) s`.

  Here `a`/`cm` are `h0(-k)` and `b`/`d` are `h0(k)`, exactly as the host writes them.
- **The output index.** `(m & 3Fh) << 6 | (n & 3Fh)`.
- **The choppy vectors.** They exist (`+30h`/`+40h` via the `1/k` pointer `[esp+2Ch]`, second FFT at `00B960A7`).
  The host header labels them partial.

**Verdict: FAITHFUL** for the height path.

## 4. Live field wiring

**`0078C9B0`.**
- `00B96C50(+54h height, +58h choppy)` runs first.
- Then `00B9A450(+4Ch * pi` (`00CE3D28`), `+50h)`. That routine calls `00B96C10(dir, speed)` (`00B9A45E`/`00B9A46B`)
  and stores `speed` to `+59Ch`.
- Then `+5Ch` goes to `[ocean+B8h]` (`0078CA0E`), and later `00B9A4C0(+2Ch, +30h, +34h)`.

**`00B9A4C0`.**
- `1/arg2` goes to `+B4h`.
- `arg3` goes to `+20h`.
- `arg1` and `arg2` go to `+18h`, `+1Ch` and `+B0h`.

The host's `inv_tile_b4 = 1/LayerScale` and `time_scale_20 = TimeScale` match.

**`00BA0090`.**
- `[01090494] = t * 10.0` (`00CE3DC0`).
- `vtable+8(t * [+20h])` on `[+BCh]`.

**The render time.** `0068A0D0` calls `0078CFF0([[00E188A8]+19FCh], [00F876A4])`. `0078D180` reads argument 2 and
multiplies it by `0.10000000149011612` (`00D7A3A0`). `00F876A4` is the mission clock (226 references,
`docs/CC12_WORLD_MISSION_CLOCK_OWNER_READINESS.md`), and that is what the host passes.

**`008830AA`.** It passes `[[00E188A8]+19F0h]+A8h` and the terrain node `+3CCh` with no further test. The host's
gate is only the switch.

**Verdict: FAITHFUL.**

## 5. Water-height readers

**`0078CF20`.**
- Calls `0078C890(x, z)` and spills the result as a double (`0078CF49`).
- Calls `00B9CF50(x, z)`.
- Then `FMUL qword` and `FSTP`, which is the host's `float(mask * wave)`.

**`0078C890`.**
- `+F9h` set returns 0.
- `kx` and `kz` are `+B4h * x` and `+B4h * z`, stored to float.
- `floor(kz)` is taken first, then `floor(kx)`; the host's order does not matter here.
- `u` and `v` are the fractional parts, stored to float.
- The sample is `00B960C0(u, v) * +24h`.

**`00B960C0`.**
- `fu = float(u * 63.0)` (`00D099A0`), and the same for `fv`; then `floor` and `FISTP`. Each cell is clamped to
  `0..3Fh`.
- The next cells are `(c + 1)` taken as a signed `& 3Fh`, then clamped.
- The four taps are `-0.0 - re` at:
  - `cv*64+cu` (`00B961F0`);
  - `cv*64+nu` (`00B96228`);
  - `nv*64+cu` (`00B96258`);
  - `nv*64+nu` (`00B96299`, `FCHS`).
- `top = a + (b - a) tu` and `bottom = c + (d - c) tu` are each stored to float. The result is
  `top + (bottom - top) tv`.

**Verdict: FAITHFUL.** All three match.

## 6. Coverage mask `00B9CF50`: NaN coordinates (DEFECT, low severity)

**The image's in-box test** (`00B9CF6E..00B9CF90`):
- `x` against `x0`: FCOMI then `JB` to the next region.
- `z` against `z0`: FCOMI then `JB`.
- `x1` against `x`: FCOMPI then `JB`.
- `z1` against `z`: FCOMPI then `JAE` into the sample.

FCOMI sets `CF` on unordered, so a NaN `x` or `z` fails every region and the routine returns `1.0` (`00B9CFA0`).

**The host's test.** It skips a region on `x < x0 || z < z0 || x1 < x || z1 < z`. Each of these is false for NaN,
so a NaN coordinate is taken as **inside** the first region.

**What follows.** The host then truncates NaN through `truncate_unsigned`, which is undefined behaviour in C++ and
gives `0x80000000'00000000` on MSVC. It reads `bitmap[row*stride + col]` from that. The image reads no bitmap
for NaN.

**Why it matters now.** Before `eb9fbc91b` the host had no regions, so this was unreachable. With the live field ON,
a NaN ship position yields a masked height instead of `1.0 * wave`.

**The rest of the routine matches:**
- `row_f = (1 - (z - z0) * sz) * h` and `col_f = (x - x0) * sx * w`, each stored to float.
- The `FISTP qword` truncations of `w - 1` and `h - 1`, with unsigned `JA` against the truncated index
  (`00B9D03B`, `00B9D08D`).
- The byte read `[+4] + row * trunc(w) + col`, divided by `255.0` (`00CE4B48`).
- The tail: `0 > q` gives 0 (`00B9D0D9`); `q > 1.0` (`00D7A24C`) gives 1, and otherwise `q` (`00B9D100 JBE` goes
  to the store of `q`).

**The fix** (for cc9's lane; the file's owner is packet `cc9_ocean_waves`):

```diff
--- a/src/ocean_wave_field.cpp
+++ b/src/ocean_wave_field.cpp
@@ -79,4 +79,6 @@
                                    float x, float z) noexcept {
     for (const OceanCoverageRegion& r : regions) {
-        if (x < r.x0 || z < r.z0 || r.x1 < x || r.z1 < z) continue;
+        // 00B9CF73..00B9CF90: FCOMI/JB thrice and FCOMPI/JAE; an unordered
+        // compare (NaN) leaves the region, so a NaN point gets 1.0.
+        if (!(x >= r.x0) || !(z >= r.z0) || !(r.x1 >= x) || !(r.z1 >= z)) continue;
         const float row_f = static_cast<float>(
```

**Expected pair effect:** identical on every row unless a NaN position reaches the sampler.

## 7. Wave-damp region box `00BA0C00`

**The box loop matches** (`00BA0F2D..00BA0F94`):
- Vertices come from `004768D0(index)` into `[esp+64h..6Ch]`.
- `z0` is replaced on `FCOMI`/`JBE` (vertex below, equal or unordered).
- `x0` likewise.
- `z1` is replaced on `FCOMPI`/`JBE` against the vertex.
- `x1` likewise.
- The start values are `1e10` (`00CE4970`) and `-1e10`.

This is the host's `!(p > lo) ? p : lo` form. The scales `1/(x1 - x0)` and `1/(z1 - z0)` match as well.

**What is uncertain is the frame and the mesh set.**

**What the image does.**
- `00BA1370` passes **one** node for both arguments (`00BA1376 push edi`, `push edi`). That node is the terrain
  render node `[landscape+3CCh]`, a single `cMesh` object, built at `00883058` and registered at `0088308C`.
- `00BA0C00` collects that mesh object's own draw sections: `BSP_MeshObject_GetCount58` (`00B72B40`) and
  `BSP_MeshObject_GetElement` (`00B732C0`).
- It adds that node's `+120h`/`+128h` (`00BA0FD9..00BA1077`, `ESI = [ebp+8]`, argument 1) to every box.

**What the host does.**
- It walks **every** hierarchy item of the `.mmod`.
- It adds each item's composed world translation (`world[i][12]`, `[14]`, that is item Matrix x parents x
  landscape).

**When they agree.** Only when the sections the host collects all belong to the single mesh object
`BSP_TerrainHeightField_AttachModel` builds, and their items carry no local translation.

**What would settle it.**
- Read `00ADE820` to see which resource of the model becomes the `+3CCh` node, and whether item matrices are baked
  into its vertices.
- Log, for each of this installation's terrain models that has an `oceanwavedamp` subset, the owning item's index,
  its Matrix translation and its parent.
- If every damp item is the attached mesh's item and has a zero translation, the host is equivalent.

**The fix if not.** Take the translation from `landscape_world[12]`/`[14]` and restrict the sections to the
attached mesh:

```diff
--- a/src/ocean_coverage_regions.cpp
+++ b/src/ocean_coverage_regions.cpp
@@ -297,3 +297,5 @@
                 // 00BA0FD9..00BA1077: + [node+120h] / [node+128h].
-                const float node_x = world[i][12];
-                const float node_z = world[i][14];
+                // CONDITIONAL (CC13_AUDIT_CC9_RECENT item 7): ESI = [ebp+8] is the one
+                // terrain render node 00BA1370 passes, not the section's hierarchy item.
+                const float node_x = landscape_world[12];
+                const float node_z = landscape_world[14];
```

This is OFF behind `kWaveFieldLiveAmplitudeBound`, and the docs say each terrain model here has at most one damp
section, so the exposure is small. Settle it before the flip.

## 8. `kMovieKeyframeModifierBound` (SQUADRON_LAND_TASK 5fj, ON)

**`00795C10`** (`RET 8`; argument 1 the camera holder, argument 2 `dt`).
- `ts = [cam+3B0h] + dt` is stored to float (`00795C4F`). `00795C5D FCOMPI` with `JBE` returns 0 while
  `start > ts`.
- If `+F8h == 0`, `00791020` runs.
- If `+E4h != 0`, slot `+Ch` is called with three arguments: `elapsed = float(clock - start)` (`00795CA5`,
  `00795CAC`), `&pose` and `dt`. The `dt` is re-read at `00795C85`.
- Only then does `00795650` run.
- The image reuses argument 1's slot for `clock` and argument 2's for `elapsed`. Every later read of those slots
  (`00795CD4` onward) is preceded by a store, so nothing stale leaks.

The host passes `(clock - start, dt)`, and goaround's `t = max(elapsed + offset, 0)` (`00791C90`) reads the same
values. The goaround vtable is `00D04C8C` (`007A066D`); its slot `+Ch` is `00791C90`.

**`00797ED0`.**
- It walks the `std::list` at `+68h` and re-reads the end sentinel `[edi+4]` every iteration (`00797EE2`).
- It skips states 0 and 3 (`00797F0A..00797F17`).
- It calls `0078E630(dt)`.

The host's index loop re-reads the size, which agrees.

**`0078E630`.**
- It first calls the modifier's slot `+14h(dt)` (`0078E642..0078E64F`). The host does not make this call. For
  goaround the slot is `0078E6F0`, a bare `RET 4`, so the omission is exact. It would have to be bound if
  `gamecamera` or `fpscamera` were ever built; both stay unsupported.
- The event fires on `dt > 0.0` (`00D7A218`), `COMISS` then `JBE`.

The host's take-before-fire order is labelled.

**Verdict: FAITHFUL.**

## Summary for cc9

- **One code defect.** Item 6: the coverage mask's NaN gate. A one-line diff is given above. It is pair-neutral
  unless a NaN position occurs.
- **One open question to settle before `kWaveFieldLiveAmplitudeBound` is flipped.** Item 7: which meshes
  `00BA0C00` sees and which translation it adds. A conditional diff is given.
- **Everything else audited is faithful to the listing:**
  - the ON hangar-hiding arm, with its two labelled gaps confirmed;
  - the whole ocean height path: draws, Phillips term, initial spectrum, FFT, advance, setters, time scales and
    samplers;
  - the ON movie-keyframe modifier and event wiring.
- **No RNG findings.** The ocean uses its own `1105h`-seeded state and draws two normals per `h0` cell (65 x 65), `g1` then `g2`, each from
  rejection-sampled word pairs, in the image's order. No audited item draws from `00BD2F10`.
