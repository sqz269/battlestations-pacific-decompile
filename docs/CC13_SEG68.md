# Segment 68 (009F6090-009FE120): squadron free-attack targeting, the approach target ref, the plane gun

Packet `cc13-seg68`, worker `agent/cc13-seg68`, 2026-10-14. Static analysis of the installed
executable's bytes (Capstone over the PE on disk) and the exported pseudocode; no Ghidra writes.
Code: `include/bsp/cc13_seg68.hpp`, `src/cc13_seg68.cpp` (namespace `bsp::cc13_seg68`, in
`bsp_core`). Every name is a hypothesis, not a recovered symbol. The models are new C++
interfaces, not ABI-compatible replacements. They are build-tested only. Nothing is wired into
the game, and nothing is fixture- or game-validated. Every engine call the image makes is a named
host hook invoked at the same point.

Counts over the 65 functions with no reconstruction record: 57 reconstructed, 8 already hosted
(verified and recorded ledger-only), 0 skipped. Six vtable slots that are not Ghidra functions are
reconstructed alongside them: `009F6C10`, `009F7070`, `009F7080`, `009F70E0`, `009F7F40` and
`009F7B00`.

## 1. The squadron free-attack auto-target (40h, vtable 00D21C00)

`009F7590` builds it as `__thiscall(this, director, squadron)` with `RET 8`. Its layout:

| offset | contents |
| --- | --- |
| +0h | vtable 00D21C00 = {009F7620 deleting dtor, 009F8470 tick} |
| +4h / +8h | the director, the squadron (`[sq+3D0h]` is its leader, `[sq+35Ch]` its class) |
| +0Ch / +14h / +1Ch | filters A (00D21BC8), B (00D21B98), C (00D21BE4); each one's +4h is the owner |
| +24h | filter C's byte: `[[sq]+35Ch]->vtable[18h](13h)` (`009F7210`) |
| +28h | the previous "any rockets" answer (`009F7C90`) |
| +2Ch | selector {+0 owner, +4 filter, +8 target, +0Ch interval} |
| +3Ch / +3Dh / +3Eh | latches (`009F6DD0` clears them) |

The filter vtable slots are listed below. The root vtable is 00D21B50 = {009F6B80, purecall,
009F6B10, 009F7010, purecall}.

| slot | A (00D21BC8) | B (00D21B98) | C (00D21BE4) |
| --- | --- | --- | --- |
| +4 enabled | 009F6C10 true | 009F7070 false | 009F6C10 true |
| +8 score | 009F7BA0 | 009F7080 FLDZ | 009F7BA0 |
| +0Ch accept | 009F7010 | 009F7010 | 009F7F40 (guard circle) |
| +10h | 009F70E0 true | 009F7090 false | 009F7270 false |
| +14h weight | 009F9770 | none | 009F9770 |
| +18h distance | 009F7150 | none | 009F72A0 |

The functions:

- `009F8470` is the tick, `__thiscall(this, float dt)` with `RET 4`. It runs `009F6C80` (the
  filter pick, modelled in `band_009f_cc13.cpp`) and clears the target. Then `009F8470` calls
  the two hosted routines `009F8160` and `009F7C90`. With director state 1 it gates on the leader: party
  `[leader+1B0h]` must be 8 or `00927F10`-held, and `[leader+184h]` must be clear. Otherwise it
  needs director +3Ch and +3Dh set and `[sq+370h]` == 2. It then picks through the selector. A new
  target, accepted by `0071DF70` and different from the resolved command target, is sent as
  `007EEC50`'s class through `0071D980`. An ordnance-to-ordnance switch is skipped (`009F6D80`
  twice). A current command `00E08F88` first clears the director and issues a moveto to the
  squadron's own position. State 2 with +3Ch clear tail-jumps to `0071D9E0(2)`.
- `009F7810` is the selector pick. Its interval is `((selector address >> 4) & FFh) * 0.001f +
  0.9f`: the image derives it from the object's address. It walks the recon slot list
  `[008053C0([sq+54h])+DE8h]` (node +4h next, +8h item, item +4h entity). It keeps the strictly
  highest score. A held target survives unless the new best exceeds its score + 0.05.
- `009F7480`, the candidate gate, rejects these: kind 0Fh; a kind-6 entity outside the map whose
  command blocks auto action; a kind-18h squadron outside the map and detached; an entity whose
  vtable+140h object has +1D4h set; and a kind-1Bh entity whose +140h object is kind 45h or 46h.
  It then defers to the filter's accept slot.
- `009F7BA0`, the score: it takes a squadron target's leader (`[t+3D0h]`) and applies the accept
  slot, the sensors (`00804350` from the own leader, mask FFh) and the AttackLists rank. The
  result is `0.5 / max(1.0f, distance) + rank`.
- `009F9770` maps the class `007EEC50` chooses to a list in the `004D6CE0` block.
  `009F7B00` ranks the entity: the list length minus the index of the first kind it matches.
- `009F7FE0` is the guard circle the hosted `009F8160` asks, with scale 1.25f (00CF29A8).
  `009F7F40` is filter C's accept: it is inside `GuardDist` (tuning +594h) of the director's path
  point, or there is no path point.

### The AttackLists loader `009F89F0`

`__fastcall(record)`, called from `004D4A20` (`seg10_cc13.cpp` names it as a host hook). It runs
`Scripts\global\luaMW_init.lua` and then `Scripts\datatables\SquadronFreeAttackTargets.lua` in a
fresh Lua state opened with 41h. For each of eight lists in `AttackLists` it reads string keys
`"0"`..`"23"`, formatted by `009F6F40`, which is `vsprintf_s` with a 4-byte buffer. A value that is
a string is copied unbounded into a 64-byte local and replaced by its index in the 97 class names
at 00E0CD80, or by -1 when no name matches. Any other value is read as an integer. Each value is
appended with `009F8960`. The image order and the vectors: DepthCharge +4h, Strike +74h, DiveBomb
+14h, LevelBomb +24h, TorpedoBomb +34h, NonFighterGun +54h, FighterGun +44h, Kamikaze +64h.

## 2. The weapon director's auto-target constructor

- `009F6A20` is `__thiscall(selector, director)` with `RET 4`. It sets +4h to the director, the
  vtable to 00D21B48, and +8h to `[director+24Ch]`. It calls `009F69C0`, then sets +34h = 1.0f,
  +38h = `-00BD2F10(ECX=1, 0.0f, 1.0f)`, +0Ch to the director and +3Ch = FLT_MAX.
- `009F69C0` clears the search vector and range, stores the owner at search+14h, clears the best,
  and calls `009F65E0`. That routine is modelled in `bot_fire_target.cpp`, and this model calls it.
- `009F60C0` is the destructor body.

## 3. The approach target ref (vtable 00D21CB4)

`NativeApproachTargetRef` mirrors the 74h layout over the native observer storage
(`observer_lifetime.hpp`). The archive keys name the fields: `precision` is +48h (the spread),
`bullpos` +54h (the centre), `error` +34h (the bias), `projtime` +44h, `vehicle` +14h.
`009FACC0` writes them through `writer->vtable[0Ch](0, key, type, value)` with types 5, 5, 5, 2
and 4. Type 5 passes the field's address, type 2 the float's bits and type 4 the pointer.
`009FB3E0` reads them back through `reader->vtable[10h]`. It reads `vehicle` through the
by-value ref's own `vtable[10h]` with an 8-byte `{4, &slot}` descriptor. It then binds the
vehicle as the target (`009FB190`) and runs `009FADA0(0.0f)`. Observer edges use
`register_observer_pair_00694a60` and `unregister_pair_006952a0` (ECX = subject, EDX = ref).
`009FB190` registers only +18h. The +14h it derives (the target when IsKindOf(5)) is not
registered. `009FAF90` transforms the stored world point +1Ch by the dying target's matrix
(`004142E0` with ECX = ref+1Ch, not +28h) before it clears the ref. `009FB640` and `009FB670` have
byte-identical bodies. So do `009FB0F0` and `009FB6C0`, except that `009FB6C0` returns `this`.
`009FCF90` has the same shape for the opportunistic-rocket state's ref at +44h. `009FCFD0` is the
land-state observer vector's push_back, written over `native_land_state_lifetime.hpp`.

## 4. The plane gun object and leg plumbing

- `009FAAD0` constructs the gun (task+314h). It sets the wander at +4h to {0.5, 0.02, 0}, calls
  `009FA620`, sets +28h = -1.0, and +44h = 999999.0.
- `009FAB60` sets accel, sets scale = max(1e-5, s), sets radius = min(reach / scale, 0.9), calls
  `009FA620`, and sets +3Ch.
- `009FABE0` is `__thiscall(gun, heading, pitch)` with `RET 8`. It computes a = pi/2 - heading,
  plus 2pi when a is negative, and +68h = normalised (cos a, tan pitch, sin a). The hosts record
  it and do not model it.
- `009FA510` is the world-edge overrun. `game_hosts_units.cpp:29153` sets `near_edge = false`
  instead.
- `009F9B60` and `009FA430` grow and append the 10h-byte array: capacity * 2 + 2.
  `009FA9F0`, `009FB010` and `009FB120` construct, get lazily and destroy the leg lock singleton
  at 00F8A724. `009FAA40` and `009FAA60` do the same for the leg manager at 00F8A720.
  `009F9A70` and `009F9AD0` are the small resets.
- `009F9C80` returns `(sin a, cos a)` with `a = U1(0, 2pi)`. `009F9BE0` and `009F9BF0` are the unit
  +460h accumulator getters. `009FE060` returns a squadron's member count +3CCh, or 1.

## 5. x87 sites (operation order and stores)

Main-thread gameplay runs at 24-bit precision and the frame-job workers at 53-bit
(`docs/CC13_FPU_PRECISION.md`). These models compute in float, which matches the 24-bit word. A
53-bit thread would round the unstored intermediates marked "ext" to double instead.

| function | sites |
| --- | --- |
| `009F6A20` | 009F6A8F RNG stream 1 (0,1), 009F6A94 FCHS, 009F6A9E FSTP float +38h |
| `009F7150` | 009F717F/018F/019F FSUB then FSTP float; 009F71BC..009F71CA (dx*dx + dy*dy) + dz*dz ext, 009F71CC FSTP float; 009F71DA FCOMI vs double 1e-10 (JBE: equal or NaN gives 0); 009F71E0 sqrt, FSTP float |
| `009F72A0` | the same twice (009F72E8..009F734A, 009F73DF..009F7454); 009F745E FMUL double 0.5, 009F7464 FADD d1, FSTP float: d2*0.5 + d1 |
| `009F7810` | 009F7828 FILD, FMUL double 0.001f, FADD double 0.9f, FSTP float; 009F78B2 FCOMPI score > best (JBE); 009F78E8 FADD double 0.05f on the slot-8 result ext; 009F78F2 FCOMPI best > that |
| `009F7BA0` | 009F7C37 FSTP float d; 00415550 (1.0 > d ? 1.0 : d); 009F7C56 FDIVR double 0.5, FSTP float; 009F7C66 FIADD rank, FSTP float |
| `009F7F40` | 009F7F7E/7F8C FSUB, FSTP float; 009F7FA8 FMUL ST0,ST0 (DC C8), FMULP ST2, FADDP: dx*dx + dz*dz, FSTP float; 009F7FBE g2*g1 ext; 009F7FC5 FCOMPI r2 > d2 (JA) |
| `009F7FE0` | 009F807D/8089 FSUB, FSTP float; 009F80A5..009F80B1 (dx*dx + 0*0) + dz*dz ext, FSTP float; 009F80BB..009F80D0 ((g1*g2)*s)*s ext; 009F80D2 FCOMPI r2 > d2 |
| `009F9BF0` | (+474h + +464h) + +478h, FSTP float |
| `009F9C80` | RNG stream 1 (0, 2pi float); FCOS, FSTP float; FSIN, FSTP float |
| `009FA510` | lo = x_min + m (009FA52F FADDP ext); d = lo - x FSTP float; hi = x_max - m (D8 E1 ext); d = x - hi via DE E2 FSUBRP ST2 = ST0 - ST2, FSTP float; the same for z-max (009FA5A2, 009FA5AE) and z-min (009FA5DA FADDP, 009FA5E2); every max is FCOMI worst > d (JA) or keep d |
| `009FAB60` | FCOMPI double 1e-5f vs scale; 009FAB9A FDIV float, FSTP float; FCOMPI double 0.9f |
| `009FABE0` | 009FABE8 FSUBR double pi/2, FSTP float; +2pi double, FSTP float; FCOS/FSIN FSTP float; 00412E20 FSINCOS + FDIVP (DE F9 = sin/cos) FSTP float; 00419440 length; FLD1 FDIVRP (DE F1 = 1/len) FSTP float; three products FSTP float |
| `009FB190`, `009FB3E0` | RNG stream 1 (0, 2.0f), FSTP float +60h (009FB3E0 draws twice, plus `009FB190`'s draw when the stream holds a vehicle) |

The DC-form scan (`tools/x87_dc_form_scan.py --range 009f6090-009fe130`) finds one site in this
packet's set, 009FBEB4 DC E9 in the hosted `009FBB20`. It reads FSUB ST(1),ST(0), which is
ratio - 1, the host's value.

## 6. Hosted functions (verified, recorded ledger-only)

| address | where | verdict |
| --- | --- | --- |
| `009F65B0` | `bot_fire_target.cpp` `push_kind`, inside the `009F65E0` model | faithful: `__thiscall(vector, byte, dword)` RET 8 pushes {flag, kind} |
| `009F6D80` | `attack_commands.cpp` `attack_command_is_ordnance_class` | faithful (seven compares, RET 4) |
| `009FA260` | `approach_target_ref.cpp` `approach_target_ref_pick_009fa260` | faithful for its two slot +100h implementations; args +48h, +54h, +64h..+70h; pick + bias stored float |
| `009FBB20` | `game_hosts_units.cpp:33116` `circle_steer_009fbb20` | faithful in order. It computes in double and rounds at the image's float stores, which matches 53-bit, not the main thread's 24-bit (see 5) |
| `009FE0F0`, `009FE120` | `game_hosts_ai.cpp:1846`, `:1838` inline IsKindOf tests | faithful |
| `009F7C90`, `009F8160` | `game_hosts_units.cpp:15702` `run_spent_ordnance_rtb_0084e010` | fused and partial; see the adoption notes |

## 7. Adoption notes (for cc9)

1. **Ship auto-target phase (LIVE, shared RNG).** `009F6A20` sets the think countdown to
   `-00BD2F10(ECX=1, 0.0f, 1.0f)` (009F6A8F). That is one stream-1 draw per ship director.
   `game_hosts_ship_ai.cpp:11983` uses `-(index % 20) * 0.05f` and draws nothing. The countdown
   decides which frames each ship's once-a-second target think falls on (`009F5DA0`, run every
   frame by `step_auto_target`). Adopting the image moves every ship's think phase and advances
   stream 1 by one draw per director, which shifts every later stream-1 draw.
2. **AI weight offset for dogfight and strafe.** `009F9770` (009F9887..009F98E5) uses FighterGun
   (+44h) when the squadron leader is IsKindOf(13h) or passes `009F6BE0` (IsKindOf(17h) and the
   byte at unit+C24h). Otherwise it uses NonFighterGun (+54h). The row table in
   `attack_commands.cpp` gives 54h for both, so `attack_command_ai_weight_offset` is wrong for
   fighters. Not live: nothing calls it.
3. **`009F7C90` / `009F8160` (LIVE when `kSquadronSpentOrdnanceRtbBound`).** No RNG. Beyond the
   substitutions the host already labels, these differ:
   - At 009F81F6..009F81FC, a fighter-class squadron (`[sq+35Ch]->vtable[18h](13h)`) with
     no ordnance left sets +3Dh, where a non-fighter sets +3Ch. The host does nothing for fighters.
     `009F7C90`'s +3Dh arm then ends the command once `007EE720` agrees.
   - The `00E08FA0` arm (009F8295..009F82D5) sets +3Ch in three cases: the target is gone; the
     target has +5Dh or +5Eh set; or `00927C90(squadron, 0)` is true and the target's +54h
     differs from the squadron's. It is not hosted.
   - Dogfight (009F8319..009F83A8) ends the command (`0071E430(dogfight, 1)`) in two cases. One is
     a squadron target whose vtable+174h answers 00E08F90 and whose members are all outside the
     map; a squadron with no members counts. The other is a kind-0Fh target outside the map whose
     +9D4h owner answers 00E08F90. Not hosted.
   - The guard arm (009F83AD..009F8407): an attack-category command that is not the director's
     +54h, with `0080DC70` false, and the director's +18Ch target outside 1.25 GuardDist of the
     path point (`009F7FE0`). It raises the override stage to 2 and runs the selector pick
     (`009F7810` on this+2Ch, dt 9999.0f). Not hosted.
   - +3Eh set forces +3Dh (009F840C). Not hosted.
   - The host re-tests only while +3Ch is clear (`!st.latch_3c`). The image re-evaluates the
     ordnance arm every tick. The result is the same latch, since the arm only ever sets it.
4. **The free-attack tick `009F8470` is not hosted** (no source models it; only comments name
   it). The image's per-tick re-selection for an AI-held squadron in director state 1 does not
   run in the host: no AttackLists ranking, no override command to the best target, no state-2
   override raise. LIVE: every squadron's command block builds one (0084D8C1 in `0084D810` calls `009F7590`), and
   the controller update calls it through `[+38h]->vtable[4]`
   (`include/bsp/squadron_spent_ordnance.hpp`). No RNG: the selector interval is address-derived
   (`009F7810`).
5. **The world-edge overrun `009FA510`** (fly-to solver). `game_hosts_units.cpp:29153` forces
   `near_edge = false`. Live only for aircraft within the margin of the map edge. No RNG.
6. **`009FABE0`, the gun direction.** The hosts record it at 009C4E27, 009C280E, and in the
   moveto, land-standby and land-line states, and do not model it. `+68h..+70h` stays unset. No
   RNG.
7. **`opportunistic_rocket_cc13.cpp:224`** stores the found rocket directly. The image sets it
   through `009FCF90` (009FD2F0), which registers and unregisters the observer edge on the
   rocket. This is a cc13 model, not a cc9 host. No RNG.

## 8. Uncertainties

- `009FB3E0` leaves +18h, +41h and the projtime-derived state unwritten when the stream holds no
  vehicle. The model's defaults (null, 0) stand in for whatever the allocation held.
- `009F7BA0` passes `009F70E0`'s whole EAX (only AL is set) as `007EEC50`'s flag. The model passes
  the byte. Likewise `009F8470` passes director+3Dh in a dword whose upper bytes come from the dt
  slot.
- The `009F7B00` checked-iterator traps (00BF6713) and `009F9B60`'s placement-new null test are
  not reproduced. They cannot fire on valid state.
- The 64-byte string copy in `009F89F0` is unbounded in both the image and the model.
- `009F6BE0`'s Ghidra name says +824h; the bytes read +C24h (009F6BF0).
