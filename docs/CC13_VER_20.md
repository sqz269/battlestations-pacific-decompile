# CC13 VER 20: AI plane weapon release, image against host (cc13-ver20)

Addresses: 006E4D50, 006E4F30, 006E4F91, 006E1F00, 006E1F4D, 006E0A70, 00999B70, 008568E0, 00855A90,
007BCC80, 009B7C90, 009C58D0, 009C5C9B

Read-only verification of the moment an AI aircraft lets go of its ordnance: the dive-bomb release, the
level-bomb release and the aerial torpedo drop. The host files `src/game_hosts_gunnery.cpp` and
`src/game_hosts_units.cpp` are leased by cc9 / cc13-ver17 and were not edited. Line numbers are this branch's,
merged with `origin/main` on 2026-10-13.

Status words: **reconstructed** (pure model in `src/plane_release_ver_cc13.cpp`, built), **verified equal**
(bytes read against the host code), **not game-validated** (nothing here was run).

## The finding in one paragraph

Every round a plane drops, bomb or torpedo, leaves through one routine, `006E4D50 BSP_MBombPlatform_Drop`. It
draws a cone, calls the round's launch hook `006E0A70` on a **copy of the round's matrix**, and the hook overwrites
that copy's forward row with the plane's world velocity (3.0 m/s off y) and re-orthonormalises it before
`006E1F00` turns the velocity inside it. So the cone is centred on the **velocity**, and a round with no cone
leaves with exactly `w - (0, 3, 0)`. The host centres the bomb's cone on the **plane's nose**, and gives the
torpedo the nose direction at the forward speed with no cone and no draws. Both differences put the round off
the impact point the bomber's own predictor (`007BCC80`, `009B7D4E`-`009B7EB5`, `009C7D71`) assumed.

## D-list

| # | image | host | effect on hit probability | predicted mission effect |
| --- | --- | --- | --- | --- |
| D1 | `006E1F1F`-`006E1F7C` with `006E0A70` (`006E0AE7`-`006E0AF4`): rows of the turn = velocity frame | `game_hosts_gunnery.cpp:13836`, `velocity[i] = vmag * (dx*right + dy*up + forward)` with the plane pose rows | bombs leave 2-4 degrees above the velocity line the impact predictor assumed (130.4's measured turn); about 15-30 m off the solution on a dive, an along-track overshoot on a level run | more dive-bomb hits wherever bombs fall today: USN04 and JM08 dive strikes, PRCP01's Vals; USN01's ScoutDauntless pair should regain the Convoy1 hit that GUNNERY 130.4 lost. No RNG change |
| D2 | the torpedo takes the same `006E4D50` path: 4 stream-1 draws, `006E0A70`, `006E1F00`, drift | `game_hosts_gunnery.cpp:13620`-`13622`, `:13638`: `forward * dot(v, forward)`, no 3.0, no draws; `game_hosts_units.cpp:27297`-`27316` routes 2Bh away from the rack arm | small: the entry point moves by fall time x speed, up to about 0.5-0.8 s x 80 m/s with a few degrees of angle of attack; the MaxFall break-up height moves by 0.5 m | USN04 / JM08 Avenger and PRCP01 Kate drops: entry 40-60 m nearer the release than now (estimate, no run); 4 stream-1 draws per drop in the image that the host does not take |
| D3 | `006E4F38`-`006E4F72`: ThrowMul only for a plane with a bot (`+DF4h`) that is slot 8 or AI-held (`00927F10`) | `game_hosts_gunnery.cpp:13800`-`13803` multiplies for every plane | none on AI rows | none on the harness rows; a player-flown, botless plane would scatter by the bare Throw |
| D4 | `00999B70` picks the ThrowMul row by the fire record's kind (`+8h`): 0Ah torpedo `+34h`, 0Bh depth charge `+E0h`, 12h strike `+12Ch`, then level `+C8h` / dive `+78h` | `game_hosts_gunnery.cpp:13802` level/dive only | none for bombs; for a torpedo the extent is 0 anyway (Throw = 0 on every torpedo platform) | none, unless a bomb rack's record kind is 0Ah/0Bh/12h (not checked) |

### D1 in detail: the bomb's direction

The image, read from the bytes (`local/` disassembly, disk bytes):

1. `006E1F1F`-`006E1F2E`: `REP MOVSD` copies the round's world matrix `round+CCh` to `[ESP+4Ch]`.
2. `006E1F30`-`006E1F4D`: the owner is `00922E90(5)` from the round; when it exists, `owner->vtable[190h]`, which
   is `006E0A70` in both `MBomb` (`00CF95C8`) and `MTorpedo` (`00D0C578`), runs with `(owner, &copy)`.
3. `006E0A70`: for an owner answering `vtable[5Ch](0Fh)` whose `vtable[38h]` exceeds `1.3888888` (`00CF8AAC`,
   `FCOMI`/`JBE` at `006E0A9F`): `w = vtable[34h]`, `w.y -= 3.0` (`[00E08E54]`), `round+318h..320h = w`, and
   **`copy` row 2 (`+20h..+28h`) = w** (`006E0AE7`-`006E0AEF`, ECX = `[ESP+1Ch]`, the second argument), then
   `0085DC80 BSP_Matrix_OrthonormalizeBasisRows` on it (`006E0AF4`).
4. `006E1F7C`: `004134F0` copies `copy` to `round+74h`.
5. `006E2062`-`006E2181`: the turn reads rows at `EBP-29Ch`, `-28Ch`, `-27Ch` (`round+74h`, `+84h`, `+94h`,
   EBP = round+310h) and writes `v = |w| (Dx r0 + Dy r1 + Dz r2)`.
6. `D` is `(t cos a, -t sin a, 1)` from `006E4D50` (`006E4FD7`-`006E5045`), so with `t = 0` the round keeps `w`.

The host (`release_bomb_drop`, `game_hosts_gunnery.cpp:13776`-`13837`) takes `w` correctly
(kBombDropVelocityBound, `:13793`), then rebuilds the direction from `h.unit_pose`'s `right/up/forward`, which is
the frame step 3 replaced. GUNNERY_OPEN_ITEMS 130.4 measured this replacement as a 2-4 degree turn from the
`5do` velocity and 130.6 adopted it, reading the rows as "plane pose x rack local". That read is right for the
matrix the round **hangs** with and wrong for the matrix the turn **uses**: it misses the `vtable[190h]` call
between the copy and the turn. Correction recorded here; the GUNNERY doc is cc9's to amend.

Adoption note (cc9, `game_hosts_gunnery.cpp`, leased): behind a new switch, build the frame as
`plane_release_ver_cc13::rack_round_release_velocity_006e4d50` does (row 2 = `normalize(w')`, rows 0/1 by
`0085DC80` from the pose's row 1), or equivalently turn the cone about `w'` instead of `forward`. With
`Throw * ThrowMul == 0` (SPVeteran/Elite dive bombers) the result is simply `w'`. The draw count and order do not
change. Predict: bombs land on the predicted impact `009C7D71` already logs (`predicted impact` in the
`gunnery: bomb drop` line); the `bomb drop scatter ... turn=` diagnostic falls to the cone angle alone.

### D2 in detail: the torpedo is a bomb at release

- `006E4D50` walks the rack's children answering `vtable[5Ch](2Ah)` (`006E4EC5`). `MTorpedo` (2Bh) has parent 2Ah
  (`src/entity_class_ids.cpp`), so the torpedo is one of them.
- Both projectiles' `vtable[108h]` is `006E27C0 BSP_BombProjectile_BulletBlock` (`00CF9540`, `00D0C4F0`), and the
  block's `vtable[34h]` is `006E1F00` for both (`00CF9614`, `00D0C3AC`); `006E529C` calls it with `D` and the drift.
- So the torpedo leaves with `w' = w - (0, 3, 0)` (Throw is 0 on every torpedo platform in this installation's
  `deviceclasses.lua`, e.g. `Torpedo platform US weak` line 3933), after four stream-1 draws (`006E4F91`,
  `006E4FB1`, `006E513C`, `006E51AC`) and a Wind 0.01 drift.
- The host spawns it at `forward * dot(v, forward)` (`:13620`-`13622`), dropping the 3.0 and any angle between nose
  and velocity, and `game_hosts_units.cpp:27297`-`27316` sends 2Bh to that spawn as "not through 006E4D50's bomb
  arm", which the class tree contradicts.

Arithmetic, level release at 12 m (`SPNormal` TorpReleaseAlt), 80 m/s, nose 3 degrees above the flight path:
host `vy0 = +4.2`, fall 2.05 s; image `vy0 = -3.0`, fall 1.29 s; entry vertical speed 15.9 against 15.6 m/s,
entry speed equal to 0.1 m/s. With no angle of attack the gap is the 3.0 alone: fall 1.56 s against 1.29 s.
`MaxWaterHitVel` (100 for bullets 63/64/69) and the MaxFall test are unaffected at 12 m; the fall break-up starts
at 49.5 m instead of 50 m (Mk24, Type 91) because of the extra 3 m/s.

Adoption note: route a torpedo drop through `release_bomb_drop`'s velocity and draw code (the `bomb_scatter`
keyed stream keeps other streams aligned), keep the torpedo's swim hand-off. One switch, OFF first.

## Verified equal

- **Water entry `008568E0`.** `|shot+8h| > MaxWaterHitVel` (`+DCh`) or `vy < -classDesc+ECh` breaks the round;
  host `game_hosts_gunnery.cpp:9698`-`9731` with kTorpedoMaxFallBreakupBound ON. `00855A90` builds
  `+ECh = sqrt(2 * MaxFall * 9.8100004196)` (`00855AAE`-`00855AD3`, double `00CF9058`) and returns false for
  `sqrt <= 0` (`00855AE2`, `[00D7A218]` = 0.0), so a class with MaxFall 0 is refused at load and the host's
  `max_fall > 0` guard is equivalent. The host's `9.81` literal differs in the 8th digit.
- **Fall time `007BCC80`.** `t = (sqrt(vy^2 + 2 g h) + vy) / g`, `vy = unit vy - 3.0`, `h <= 0 -> 0`, both `g` the
  double at `00CF9058`; `dive_bomb_task.cpp:251` matches, float roundings included.
- **Level-bomb impact prediction `009B7D4E`-`009B7EB5`.** `+A4h = 007BCC80(-dy) + 0.15`, impact =
  position + velocity x (`+A8h` + `+A4h`), y replaced by the aim point's (`009B7EC8`); the paratrooper branch
  `009B7D7D` uses `007BCCF0` instead. `plane_level_bomb_task.cpp:52`-`64` matches.
- **Aimdive aim error `009C5BEE`-`009C5C9B`.** Two `00419010` calls over `x0 = +A8h + bias`, `x1 = +ACh + +50h`,
  `x = height above target`: lead `(0 .. row+5Ch)`, gain `(1 .. row+60h)`; error =
  `gain * (cos(bearing) * planar - lead)` (`FSUBR` at `009C5C4E`). The steep gate `unit+C64h > [00CEC728]`
  (`009C5BDB`, `JA` over the block) leaves the latched planar distance in `[ESP+5Ch]`. `dive_bomb_task.cpp:298`
  and `game_hosts_units.cpp:5035`-`5040` match.
- **Cone constants.** `a = U(0, pi)` (`[00D7A264]` = 3.1415927f), `t = tan(U(0, s))` (`00412E20`, FSINCOS/FDIVP);
  `game_hosts_gunnery.cpp:13805`-`13808` matches, including stream 1.

## Reconstructed (built, not run)

| native | host symbol | file |
| --- | --- | --- |
| `00999B70` | `pilot_bot_throw_mul_row_00999b70` | `src/plane_release_ver_cc13.cpp` |
| `006E4F0D`-`006E4F76` | `rack_throw_extent_006e4f30` | same |
| `006E4F7A`-`006E5045` | `rack_drop_cone_006e4f91` | same |
| `006E1F1F`-`006E1F7C` + `006E0A70` + turn | `rack_round_release_velocity_006e4d50` (on `gameplay_seg53_cc13`'s `006E0A70`/`006E1F00` model) | same |
| host comparison | `host_bomb_release_velocity`, `host_torpedo_release_velocity` | same |

Ledger: `config/reconstruction/00990000.jsonl` (`00999b70`), `config/reconstruction/006e0000.jsonl`
(`006e4f30`, `006e4f91`, `006e1f4d`).

## Uncertainty

- D1's size depends on the host's angle of attack at release; 130.4's 2-4 degree turn is the only measurement.
  The 15-30 m figure is slant range x angle for a 400-500 m release, not a run.
- `0085DC80`'s row rebuild is taken from its ledger record and `gameplay_seg53_cc13`'s model, not re-read here;
  with no cone only row 2 matters and it is `normalize(w')` either way.
- The owner `00922E90(5)` is assumed to be the plane for every rack round (the round hangs under the rack entity
  under the plane, 130.6).
- D2's mission figures are arithmetic, not runs. No game run was made (RUN SLOTS rule).

## Handoff

Done: D1-D4 with addresses and adoption notes; four ledger records; one source/header pair. Open: the level-bomb
aim tick's probe `007F0280` and the torpedo aim tick were not re-read (cc13-ver6 verified the latter's gates); the
bomb rack fire record's kind at `[+3F8h]+34h`+8h for bombs (D4's caveat) was not checked.
