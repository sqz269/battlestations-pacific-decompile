# The game's random streams, and a measurement option for gunnery pairs

Addresses: `00BD2F10` (BSP_Random_UniformFloatRange), `00BD2ED0` (BSP_RandomThreads_GetState),
`00BD2E60` (BSP_RandomState_UniformFloatRange), `00BA2C20` (BSP_RandomState_NextU32), `00BF0D20`
(BSP_RandomState_Refill), `00BF0CF0` (BSP_RandomState_Seed), `00BD2FD0` (BSP_RandomThreads_Seed),
`00BD2F40`, `00BD2FC0`, `0079D020` (BSP_HudMovieCamera_Construct), `0079A260`
(BSP_HudMovieCamera_Destruct), `00798C80` (BSP_HudMovieCamera_FirstStepReseed), `005CD1A0`,
`0084BAD0` (BSP_Explosion_ApplyRadialDamage), `0077CE60`.

Packet `cc9_rng_streams`, 2026-09-23. The host's gunnery drew every random number from one
substitute generator, so a behaviour pair that changed one aircraft moved unrelated gunnery
outcomes. That is `docs/AA_TARGETING.md` section 3. This packet reads the image's generator, adds
a measurement option that gives each gunnery consumer its own stream, and uses it to judge the
AA fixes one at a time.

## 1. The image's generator

**Algorithm: MT19937 with the original 1998 seeding.** The state is `9C8h` bytes:
- `+0h` is the word index.
- `+4h..+9C3h` are the 624 state words.
- `+9C4h` and `+9C5h` are two guard bytes. With `+9C5h` set and `+9C4h` clear, the routines write
  through a null pointer, which is a deliberate trap.

| routine | what it does |
| --- | --- |
| `00BF0CF0` Seed(seed) | `mt[0] = seed \| 1`, then `mt[i] = 69069 * mt[i-1]` (`IMUL 10DCDh` at `00BF0D07`) for i = 1..623, and index = 624. This is the original `sgenrand`, not `init_genrand`'s `1812433253`, which occurs nowhere in `.text` (the scan finds the twist constant `9908B0DFh` at `00BF0D30`, so a miss is meaningful). |
| `00BF0D20` Refill | the standard twist with `9908B0DFh`, `7FFFFFFFh` and the `E3h` (227) split |
| `00BA2C20` NextU32 | refill when the index reaches `270h`, then the standard tempering: `y ^= y>>11`; `y ^= (y & FF3A58ADh) << 7`, which equals `(y<<7) & 9D2C5680h`; `y ^= (y & FFFFDF8Ch) << 15`, which equals `(y<<15) & EFC60000h`; `y ^= y>>18` |
| `00BD2E60` UniformFloatRange(lo, hi) | u = NextU32 read as unsigned (`+4294967296.0f` if negative) times `2^-32` (`00D63B80`), then `lo + (hi - lo) * u` |
| `00BD2F10` (lo, hi) | `00BD2ED0` then `00BD2E60`. **The caller's ECX is the stream index.** |
| `00BD2ED0` GetState(ECX) | looks the calling thread up (`[00CE223C]`) in ten slots at `01090AC4` (count `01090AEC`). It returns `01090AF0 + (stream + 2*slot) * 9C8h`, or the fallback `00E14748` for an unregistered thread. |

The existing reconstruction `src/random.cpp` and `include/bsp/random_threads.hpp` already carries
all of this and is fixture-tested; this packet confirms it against the listing.

**Which stream the gameplay uses.** I censused ECX at every rel32 call from the PE's `.text`,
decoding backwards from the site to the last write of ECX (`local/ecx_census.py`). There are 625
call sites of `00BD2F10`, where `ghidra callers` lists 236 containing functions. 327 pass stream 1
and 214 pass stream 0. Of the other 84, 51 are register-derived (mostly `LEA ECX,[reg+1]`), 24 do
not decode within the 64-byte window, and 9 set ECX through an earlier call. Stream 0 is
concentrated in `004xxxxx` (95 of 214). Stream 1 is the gameplay stream: 183 sites in `009xxxxx`
(pilot bots, dive-bomb and torpedo tasks, the goaway re-roll) and every gunnery consumer.

| gunnery consumer | call site | stream |
| --- | --- | --- |
| visibility cache TTL `00864D90` | `00864F1E` | 1 |
| per-target record `00864880` | `00864A12`, `00864A31` | 1 |
| trigger / rearm `0072D2C0`, `0072D520` | `0072D3B3`, `0072D579` | 1 |
| shot decision `0072F6E0` | `0072F76D` | 1 |
| shot spawn `0072F830` | `0072FB6A` | 1 |
| projectile impact `0084BC60`, trace `0084BF00` | `0084BE4D`, `0084C19F` | 1 |
| gun bots `008FFA20`, `008FFF20`, `00902920`, `008FBEF0` | `008FFCD9`..`00902CF0`, `008FBF5C` | 1 |

**Who seeds stream 1, and from what.** There are six calls to `00BD2FD0`, all read:

| site | container | stream / seed |
| --- | --- | --- |
| `005CD1B0`, `005CD1BC` | `BSP_HudMovieScreen_EngageMovieInterface` | 1 = 12345 (`3039h`), 0 = 54321 (`D431h`) |
| `00798CAD`, `00798CB9` | `00798C80`, the movie camera's first step with dt > 0 (latch `+221h`) | 1 = 12345, 0 = 54321 |
| `0079D1AB` | `0079D020`, the movie camera's constructor | 1 = 123 (`EBX + 7Bh`, EBX zeroed at `0079D065`) |
| `0079A2FB` | `0079A260`, the movie camera's destructor | **1 = wall-clock milliseconds**: `[01090AB0]->vtable[20h]` gives ticks and frequency, times `1000.0` (`00CE47A0`), truncated |

So the gameplay stream is reseeded from the clock whenever the HUD movie camera is destroyed.
**Its seed is not determinable**, so two runs of the image do not draw the same sequence. There
is a second reason the host cannot reproduce the image's sequence: every one of the 327-plus
stream-1 sites draws from it, including many subsystems the host does not simulate.

**Decision: no `kRandomGeneratorBound`.** The algorithm is fully read, but binding MT19937 with a
fixed seed would claim a sequence the image never has. The host keeps its substitute: one shared
32-bit LCG (`1664525`, `1013904223`, seed `9E3779B9h`, 24-bit mantissa) for all gunnery draws. It
is now labelled precisely at `random_range_00bd2f10`. It preserves the image's coupling shape,
one stream shared by all gunnery consumers, and not the values.

## 2. The measurement option: `BSP_GUNNERY_RNG_STREAMS=1`

**It is a measurement substitution, never on in a reference run.** Unset, every gunnery draw goes
to the shared generator in the same order as before this packet. Set to `1`, each draw goes to
its own generator keyed by consumer and identity:

| consumer (`Impl::Draw`) | image site | key |
| --- | --- | --- |
| `visibility_ttl` | `00864D90` | (shooter unit, target unit) |
| `fire_stagger` | `0072D2C0` via the fire request | (gun index) |
| `hit_effect` | ship-hit fire/flood chance | (victim unit) |
| `hull_damage` | `00470510`'s base | (gun index, victim unit) |
| `blast_damage` | `0084BAD0`'s burst | (gun index) |

Each key's generator is the same LCG, started from a splitmix64 of the run seed (`9E3779B9h`)
and the key. So a key's k-th draw depends only on the key and k, and never on another key's
traffic. With the option on, the run also prints `summary mission gunnery per-consumer random
streams ON` and one `gunrow` line per gun (index, unit, platform, category, assigns, clears,
shots, trigger rises, angle refusals, and minimum-range skips), so pairs can be judged gun by gun.

**Proof (a): two identical runs are identical.** Option on, USN04, `--frames 4700
--press-start-frame 30 --menu-select USN04 --mission-frames 4500 --mission-frame-seconds 0.05`:
`local\str_a1.log` and `local\str_a2.log` differ on 0 of 50303 lines, with environment lines and
the run-varying `ship avoidance search` counter excluded. There are 726 gun rows.

The option changes the battle against option off, as a substitution must (`local\str_off.log`:
the same 9 deaths, 15970.7 against 15959.2 damage). It is a different random sequence, not a
fix, which is why it is never on in a reference run.

## 3. Proof (b)'s prediction, written before the treatment build

The treatment is the kind-5/6 minimum-air-range fix alone (`kAaMinRangeBound = true`, window and
armour off), option on, USN04 at 4700/4500. The control `local\b_ctl.log` counts, per gun, the
evaluations in which that skip would apply (`minrange_skips` on the `gunrow` lines). Exactly
eight guns are non-zero, **Yorktown-class01's eight FLAK mounts, gun rows 199-206**, with 12
each, 96 in all. No other gun of kind 5 or 6 ever holds a plane inside its `MinRange`.

- **Prediction:** gun rows 199-206 change (assigns, clears, shots, refusals); the other 718 gun rows
  are identical, and so is the per-unit table.
- **The risk named in advance:** those eight guns fire one round each in the control. If a changed
  round hits a different aircraft, or misses one it used to hit, the change spreads through that
  aircraft's flight and every gun that engages it. That spread would be real behaviour, not
  random-stream coupling.

### 3.1 Proof (b), measured: the prediction holds exactly

Treatment log `local\b_trt.log`, against the control `local\b_ctl.log`, option on, USN04
4700/4500:

| check | result |
| --- | --- |
| gun rows that differ | **8 of 726: rows 199-206, Yorktown-class01's FLAK mounts, the predicted set** |
| per-unit rows that differ | 1 of 71: Yorktown-class01, shots 70 -> 76 |
| hits, damage, deaths, first hit | identical |
| every other differing line | the per-step shot totals (+1 from 135 s), the projectile counters and the method call counts those extra rounds produce |

The eight rows:

| gun row | shots | trigger rises | angle refusals | min-range skips |
| --- | --- | --- | --- | --- |
| 199 | 1 -> 3 | 1 -> 3 | 1569 -> 1554 | 12 -> 20 |
| 200 | 1 -> 2 | 1 -> 2 | 1876 -> 1820 | 12 -> 20 |
| 201 | 0 -> 0 | 1 -> 3 | 1569 -> 1554 | 12 -> 20 |
| 202 | 1 -> 2 | 1 -> 2 | 1876 -> 1820 | 12 -> 20 |
| 203 | 0 -> 0 | 2 -> 2 | 1569 -> 1554 | 12 -> 20 |
| 204 | 1 -> 2 | 1 -> 2 | 1876 -> 1820 | 12 -> 20 |
| 205 | 1 -> 1 | 2 -> 2 | 1569 -> 1554 | 12 -> 20 |
| 206 | 1 -> 2 | 1 -> 2 | 1876 -> 1820 | 12 -> 20 |

Assignments and clears are unchanged (57 and 53 on every row). Only what the guns do with a
target moved. With the skip applied, a FLAK gun passes over a plane inside its `MinRange` and
takes the next candidate. That plane is further out and inside its window, so it settles and
fires more: 6 more rounds across the eight guns, none of which changes a hit. Because the skip
now shapes each walk, the observed skip count rises from 12 to 20 per gun. The named risk, a
changed round changing a hit, did not occur.

**The kind 5/6 minimum-air-range fix lands, default true.** Kind 6 (LIGHTARTILLERYFLAK) keeps a
zero minimum, because its second ammunition entry is not loaded. That is labelled at the switch.

### 3.2 The armour test on its own

Control `local\b_trt.log` (minimum range landed), treatment adding the AA gunner armour test,
`local\c_armour.log`. The predicted outcome was that nothing moves, because the counter showed 0
refusals: every AA round's best damage exceeds every aircraft's armour in USN04. **Result: 0
differing lines, and 0 of 726 gun rows.** It lands, default true. It is a faithful term with no
effect in this mission.

### 3.3 The fire window stays off

It is bound in the hull frame with the shared muzzle point, because the host builds no gun node
frame (`docs/GUN_MOUNT_POSITIONS.md`). A pair cannot make a substitution faithful, so it was not
landed, and not taken as its own pair here. The observe counter still reports its would-refuse
count (`window_rejects`).

### 3.4 What the landing does to a default run

With the option **off**, the same parameters, the landed build (`local\landed_off.log`)
against the pre-landing build (`local\str_off.log`):

| | before | landed |
| --- | --- | --- |
| `queued_hits` / `hull` | 116 / 72 | 122 / 72 |
| `total_damage` | 15970.7 | 16035.6 |
| `deaths` | 9 | 8 |
| per-unit rows that differ | | 27 of 71 |

Two deaths change. **Lexington-class01 lives on 24 hp** instead of dying at 220.36 s credited to
York-class02 (the friendly-fire death of `docs/ENTITY_DEAD_FLAG.md` section 5). D3A Val
#1.1|.-3 dies at 157.00 s to Lexington-class01 instead of at 155.95 s to Fletcher-class03.

These are not effects of the fix itself. The option-on pair (3.1) shows the fix touches eight
flak rows and no hit. In a default run those guns' extra rounds draw from the shared stream, and
every later gunnery draw shifts: the coupling `docs/AA_TARGETING.md` section 3 described. The
reference rows at difficulty 1 move with any change to gunnery draw counts; the integrator
should re-take them on the merged binary.

## 4. The skill line (item 4): nothing to feed yet

The image's gun bots read their skill row at `bot+34h`, set by `BSP_GunBot_Attach 008FBC80` from
`unit->vtable[12Ch]()` (`docs/PILOT_SKILL_LEVEL.md`). The row-dependent constants are the aim-error
envelope `006DEFF0` (`[[00E19990] + 1Ch*bot+34h + 0Ch/+10h]`, `docs/GUN_DISPERSION.md` section 7),
the tail-gunner block `[00E199A0] + 24h*bot+34h`, and the depth-charge level record
`[00E19988] + 14h*bot+34h`.

The host's gunnery calls one gun-bot routine, `bsp::gun_bot_slots_for_subtype_0072c6a0`, which
takes no level. It models no aim-error reroll, no `Throw` dispersion (read into the Lua flatten
and never applied) and no tail-gunner or depth-charge level record. **So no gun row reads a
skill-dependent constant, and feeding `units.skill_level()` would change nothing.** The
prediction is "no row moves" by construction, so there is nothing to measure or land. The
prerequisite is porting `006DEFF0` with its descriptor rows; the skill index is then one
argument. That is the next packet, not this one.

## 5. Friendly fire (read-only)

**Neither the image nor the host filters the firer's side.**
- **The blast `0084BAD0`.** Its gather `00904470` -> `0098C510` collects every collision node's
  owner in the radius. It skips only `node == excludeNode` (`0098C535`), which is the burst's
  own source (`docs/EXPLOSION_RADIAL_DAMAGE.md`). The per-record queue then skips only the source
  entity. The host's `apply_impact_blast` skips only `i == shooter`. Same rule.
- **The direct impact.** The segment sweep `0098ADD0` takes an `exclude_entity`, which is the
  firing unit, and nothing else. The host's `SegmentBinding` excludes only the shooter. The side
  skip in the host's projectile loop belongs to the torpedo closest-approach census and does not
  gate hits.
- **The credit `0077CE60`.** It writes `attacker_id`, side and class whenever the shot carries an
  owner id (`0077CF44`), with no side comparison. The host calls the same reconstruction and sets
  `last_attacker`. So a friendly hit does update the victim's credit, in both.

`docs/ENTITY_DEAD_FLAG.md` section 5's Lexington death, flak blasts and a direct impact from its
escort York-class02 with York credited, is therefore faithful in kind. **No `kFriendlyFireBound`
is added.**

## 6. Decisions

| item | decision |
| --- | --- |
| 1 the generator | fully read: MT19937, original 69069 seeding, two streams per thread, gameplay on stream 1, reseeded from wall-clock milliseconds by `0079A260`. Seed not determinable, so **no `kRandomGeneratorBound`**; the host's shared LCG stays as a labelled substitution. |
| 2 the option | `BSP_GUNNERY_RNG_STREAMS=1`, a measurement substitution; proofs (a) and (b) hold; the default is byte-identical to main |
| 3 the AA terms | minimum air range **landed**, armour **landed**, fire window **off** |
| 4 skill | nothing to feed: no host gun row reads a skill-dependent constant; the prerequisite is `006DEFF0` |
| 5 friendly fire | image and host agree (no side filter, and credit is written); no switch |

Ledger names added: `0079D020` BSP_HudMovieCamera_Construct, `0079A260`
BSP_HudMovieCamera_Destruct, `00798C80` BSP_HudMovieCamera_FirstStepReseed.

## 7. Open

- **Port the gun bot's aim-error envelope `006DEFF0`** and its descriptor rows. Then the skill
  index is one argument, and a skill pair can be predicted row by row.
- **Load kind 6's second ammunition** for `00729B90`.
- **Build the gun node frame**, so the fire window can be bound faithfully and measured as its own
  pair.
- **The census's 84 unresolved ECX sites** (51 register-derived, 24 undecoded, 9 after a call) were not resolved one by
  one. The stream-1 claim for gameplay rests on the 327 literal sites plus the gunnery table
  above.

## 8. The ship AI's torpedo draws (packet cc9_ship_torpedo_response)

The ship AI's torpedo response draws on stream 1 as well. That covers 009F0AD0's three admission
draws (predict, observation, speed error) and two of the brain constructor's timer seeds
(009F1316, 009F139E). The host routes them through `GameGunneryHost::ship_ai_draw(unit, lo, hi)`:

| option | generator |
| --- | --- |
| unset (default) | the shared gunnery generator, interleaved in call order with every gunnery draw, as in the image |
| `BSP_GUNNERY_RNG_STREAMS=1` | its own keyed generator, consumer `ship_ai_torpedo` = 7, key (unit, 0) |

With the option on, a treatment that builds torpedo tracks does not move any gunnery key's
sequence, so a pair isolates the ship's response. With it unset, the treatment shifts every later
gunnery draw, as it would in the image. The draws only happen when `kShipTorpedoResponseBound` is
on; with it off, a default run is byte-identical to before. See docs/SHIP_TORPEDO_RESPONSE.md.
