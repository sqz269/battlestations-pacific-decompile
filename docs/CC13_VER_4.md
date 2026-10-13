# CC13 VER 4: verification of ship damage, flooding and sinking

Packet cc13-ver4 (Claude, cc13 lineage). This is a read-only check of the host code that decides
ship damage, flooding, sinking and the sink kill, against the disk bytes. The sources were
`bsp.py disasm-raw`, `sc2_x87.py` (local/tools_scene2 in the cc13 tree) and a byte check of every
register-register x87 subtraction and division quoted here. Flags and line numbers are from
origin/main 2ae7640f0.

No host file was edited. `src/game_hosts_units.cpp` is leased by cc9-lua52 and
`src/game_hosts_gunnery.cpp` by cc9-gunnery36. The library files checked (`unit_forces.cpp`,
`unit_damage.cpp`, `unit_hit_path.cpp`, `ship_hydro_forces.cpp`) are unleased, but no fix was
needed in them. Nothing was found that the host substitutes without a label, so no new model
files were written. There were no game runs.

## 1. Result

**The flooding and sinking chain matches the image.** The only differences are the substitutions
the host already labels and last-bit rounding. There is no divergence that changes a mission
outcome. The chain, step by step:

| step | image | host | verdict |
|---|---|---|---|
| hull damage | `00470510` | `hull_damage_00470510` (unit_hit_path.cpp:17) | match |
| part damage falloff | `004705C0` (`1 - d / range`, 0047066F DE E1) | `part_damage_004705c0` | match |
| unit AddDamage | `0095DA00`: lockout, then the cheat table and `0087D730` | `difficulty_scaled_damage` (game_hosts_gunnery.cpp, used at :10119) | scalings match; lockout omitted (2.1) |
| apply damage | `00879070` | `apply_damage_00879070` (unit_damage.cpp:197) | match (NaN health aside) |
| leak manager init | `0074F490` | `build_leak_model_0074f490` (game_hosts_units.cpp:3172) | match; `+30h` not stored (2.3) |
| AddLeak (message 90h) | `0074F090` | `add_leak_0074f440` (game_hosts_units.cpp:35878) | match |
| leak tick | `0074F930` | `unit_leak_tick_0074f930` (unit_forces.cpp:24) | match; last-bit in the cap ratio |
| flooding weight | `00933A3A..00933A4F`: `-f(water * 10.0)` from the **pre-tick** water (`[ecx+10FCh]` read at 00932A39) | `ship_hydro_forces.cpp:391` with `in.leak_water_mass` read before the tick | match |
| wreck redistribution | `0074EC50` | `ship_wreck_sink_block_00824fe5` (game_hosts_units.cpp:3246) | match; the denominator source differs in the last bit (2.2) |
| KillDepth kill | `008263C1..00826628` | game_hosts_units.cpp:34182-34259 | match |

Details checked from the bytes:

- **`0074F930`.** The rate floor is `COMISS 0.001f` and is strict. The applied rate is
  `min(rate, cap)` (FCOMI / JBE; a NaN keeps the rate), and water grows by `f(applied * dt)`.
  `+2Ch` is the sum of the floored rates and `+28h` the sum of the water. With `+28h > 0` and
  `+5Dh` clear, the cap is `f(((1 - health) * [settings+404h] * capacity) / total)`. At 0074FA00
  the subtraction is `DE E1` FSUBRP, which gives `1 - health`. When that cap is below 1, every
  water is scaled and every rate zeroed. Finally `+24h = total > 0.01f`.
  - A **live hull at full health holds no water**, because the cap is 0. Its allowance grows to
    0.2 x capacity at health 0. Only a wreck (`+5Dh`) floods without limit. So flooding never
    sinks a live ship: it sinks after the kill, through the wreck path.
- **`0074F090`.**
  - The station is `trunc((fwd . d + L/2) / +04h)`, clamped to the range [0, n/2 - 1].
  - The side is `d.x f.z - f.x d.z > 0` (0074F19A `DE E9`).
  - The rate law, with k = `00E1AECC`: `u = |D/R|^(1/k)`, then `z = (f(T u) + amount) / T`
    (0074F249 `DE F1`), then `w = |z|^k` and `D' = f(w R)`. The rate grows by `f(D' - D)`.
  - k = `f(log f(sqrt(DologSzorzo)) / log 2)` = 0.5 at DologSzorzo 2, so `D'^2 = D^2 + R^2
    amount / T`.
- **`0074F490`.** Each side has three stations at `z = (L/(n-1)) i - L/2`, with
  `x = Width * side`. The rate cap is multiplied by 3 for IsKindOf(0Eh), or for IsKindOf(0Ch)
  with Mass below 500 (`CE3840`, FCOMPI / JA). Then `+34h = 2 * 0.2 * capacity / TimeToDeath`
  and `+30h = +34h / DamageToDeath`.
- **`0074EC50`.** One stream-1 `U(0, 1)` is drawn per leak (lobby mode not 2). Each rate becomes
  `draw * 2cap / (+28h + sum of draws) + rate`, and the new rates go into the 92h message
  (route 7). The host applies them at once, which is labelled.
- **`00879070`.** It refuses damage when lobby mode is 2 or `+5Eh` is set, and refuses a hit on
  a unit whose health is already 0 or less. The invincibility floor is
  `(1 - f(1 - inv)) * max` (008790EB `DE E2`), and the amount is clamped to `health - floor`
  (00879111 `DE E9`).
- **KillDepth.** `sinkTime` (unit+828h) grows by dt. After 60 s, or 20 s for IsKindOf(0Eh), the
  hull drops the terrain collision mask. The two ends are `f(y +/- f(fwd.y * f(L * 0.5)))`, and
  the ship is killed (`00926D90(1)`) when both ends lie strictly below `[settings+3F4h]`.

The settings substitutions match this installation's `scripts/datatables/shipglobals.lua`
(2024-07-13): MaxLeakPercent 0.02, EnnyiVizEsKeszPercent 0.2, DologSzorzo 2 and KillDepth
-200. I did not re-read which settings offset each key loads into (+400h, +404h, +40Ch, +3F4h);
that mapping is cc9's citation.

## 2. Differences (none outcome-changing)

1. **`0095DA00` lockout.**
   - **Image:** a unit with `+71Ch` set (a parent link) and `+728h > 0` takes no damage at all
     (0095DA03..0095DA1B).
   - **Host:** `ShipHit::add_damage` (game_hosts_gunnery.cpp:10100) applies the difficulty
     scalings but not this lockout. Its own gate there is the command building's `006F1F20`.
   - `subobject_blocks_damage_0095da00` (unit_damage.cpp:240) models it but is not called.
   - **Effect:** none for ships. Ships have no `+71Ch` parent; the parent-linked units are land
     forts under a CommandBuilding (docs/CC13_GPR_41.md `die_00747400`). If `+728h` is ever
     positive for such a fort, it takes hits in the host that the image refuses. The writer of
     `+728h` was not traced.
2. **`0074EC50` denominator.** The image divides by the stored `+28h`, the last tick's total.
   The host re-sums the waters, which differs only in the last bit after a cap scaling.
3. **`+30h`** (`+34h / DamageToDeath`) is not stored by the host. No reader was found in the
   leak manager's own functions (0074E830..0074FA78). A reader elsewhere was not searched for.
4. **Last-bit rounding.** `unit_leak_tick_0074f930` computes the cap ratio in double. The image
   rounds each x87 step at 24-bit precision (the unit_forces.cpp header comment assumes wider
   intermediates). Same for the power law in AddLeak, where the host uses double `exp2` / `log2`
   and the image `FYL2X` / `F2XM1` / `FSCALE`.
5. **NaN edge cases.** `00879070` continues on a NaN health (COMISS / JAE); the C++ refuses.

## 3. Resolved for docs/CC13_GPR_41.md open question 5

`sumForces` (`+28h`, unit+10FCh) is the **total leak water**. `0074F930` writes it every tick, and
the hydrodynamics read it as the flooding mass (00932A39) and the total mass (00932B72).
`sumLeaks` (`+2Ch`) is the **total raw leak rate**: `0074F930` writes it each tick and `0074F090`
replaces it with `D'`. Two Lua natives also read the manager: `GetWaterLoad` (00891680) and
`GetLeaks` (008918D0). No script under this installation's `scripts/` calls either. Mission
scripts outside that folder were not searched.

## 4. Not covered (open)

- The kill path itself, `GameGunneryHost::Impl::kill_unit` (game_hosts_gunnery.cpp:11619), and
  the image's health-0 chain, `00877B90` -> vtable `1B0h` / `1A8h` -> `00824B60` (OnWrecked),
  which sets `+5Dh`. The host's kill_unit is a large cc9 body with its own labelled bound
  switches; I did not check it against the bytes here.
- The rest of the buoyancy integral in `009329C0` (the element loop and the planing torque).
  Only the flooding-weight block and the water-mass reads were checked.
- `008263C1` (KillDepth) is leased by cc13-ver3, so it has no record from this pass. It was
  checked (section 1).
- The hull-pass gate `hit+34h != -1` (008777DD) is deliberately not reproduced by the host
  (labelled at game_hosts_gunnery.cpp:10138). Whether hits ever carry a non-negative `+34h` is
  docs/EXPLOSION_RADIAL_DAMAGE.md's open producer question.

## 5. Records

Seven existing records were re-added with `--replace`, keeping their names and sources, with the
byte check appended to the evidence: 0074F930, 00879070, 00470510, 0095DA00, 0074F090, 0074EC50
and 0074F490. The previous versions stay in git history.
