# CC13 seg59a: segment 59 scoring and awards band (packet `cc13-seg59a`)

Grant: the cc9 gameplay lead opened segment 59 (008EE670-00922240) to cc13 on 2026-10-17 only for
its mission scoring and awards band from 00905300, for functions reached by gameplay or the
end-of-mission flow. Code: `src/cc13_seg59a.cpp`, `include/bsp/cc13_seg59a.hpp`. Nothing is
wired into bsp_game; no game runs. Status of every function below: reconstructed, build-checked;
not fixture-tested, not ABI-compatible, not game-validated. Names are hypotheses.

## Where the band ends

The band runs from 00905300 to the segment end 00922240. Its subject is the scoring object
`*(game+21A0h)`: eight 284h-byte player records (base `scoring + slot*284h`, with a +4h offset
in the Lua binding accessors) and a tunables block at +1424h..+14B0h. Most functions in it are
STL instantiations over the record's containers (tagged `stl_*`, bound, not reconstructed). The
game routines that were not yet reconstructed or recorded are listed under "Left" below.

## Model

Every routine takes the original objects as raw bytes and reaches the engine through
`Seg59aHost`. The record containers are bound STL member instantiations, invoked as host methods
named by address: `004C8B80` string->int find, `005070C0` string->int index, `00625900`
int->int index, `0090E740` int->float index, `009119E0` int->(int->float map) index, `00667EB0` /
`0065CD40` / `0058A3A0` ushort->int index / find / erase, `00725020` / `00725050` / `00722180` /
`007265E0` ushort vector begin / end / iterator equality / push_back, `0090D9D0` the 8-byte
drop-record vector append. The checked-iterator failure call `00BF6713` is made at the binary's
points. NativeString (8 bytes: +0 length, +4 data) passed by value is modeled as `NativeStr`
by value; the callee releases it with the inline `00419CC0`/`00BD1510` pair, as the image does.
The `00419CC0` singleton is called without its three pushes (they are consumed by `00BD1510`).

## Functions

| Address | C++ | ABI | Behaviour |
|---|---|---|---|
| 0090C000 | `score_table_is_badge_earned_0090c000` | thiscall(scoring unused, NativeString by value), RET 8, AL | `00553C80(&key, p, 0)` for "BU", "BW", "BM", "BO" (00D1872C..00D18720) in that order; true on the first hit; releases the key. |
| 0090EDE0 | `scoring_grant_award_0090ede0` | thiscall(scoring, slot, const NativeString*, value, byte accumulate), RET 10h | See "Award grant" below. |
| 00914270 | `award_registry_score_for_name_00914270` | thiscall(scoring, NativeString by value), RET 8, EAX | 0090C000 on a copy (copy tests the source length); when accepted returns `[0050FC30([00E19900]+10h, &key) + 3Ch]`, else 0; releases the key on both paths. |
| 0090F5A0 | `scoring_grant_ga_ae_once_0090f5a0` | thiscall(scoring, slot), RET 4 | Inline literal "GA_AE" (Resize 5, memcpy 6); when absent from record+70h grants it through a fresh 0041E870 string, (slot, &s, 1, 0). |
| 0090F6C0 | `scoring_plane_achievement_0090f6c0` | thiscall(scoring, plane, kind), RET 8 | slot = [plane+1ACh] (no-op above 7); map record+1B4h keyed by the word plane+174h. kind 0 stores map[id] = slot; then find; kind 4 erases a present id; kind 1 grants "SA_IM" when present; kind 3 grants "SA_OC" always. |
| 0090F860 | `scoring_capture_awards_0090f860` | thiscall(scoring, slot, a, b), RET 0Ch | Slot above 7: nothing. Else record+178h += 1; grants "GA_DN" when absent and a == 0Ch, then "GA_AI" when absent and b == 31h. Caller 006F6760 (command building capture tick). |
| 00913AB0 | `scoring_spawn_awards_00913ab0` | thiscall(scoring, slot, kind), RET 8 | kind 0Fh bumps "Counter_RUA_MU"; then grants "RUA_MU" when the counter equals `RUA_MU` record's first threshold (0050FC30, int vector +48h, checked [0]); grants "GA_SL" when absent. |
| 00908500 | `find_ushort_00908500` | ECX = result, EDX = &value, stack (first, last iterators), RET 10h | std::find over a ushort range; reconstructed under rule 2b because of its register ABI at the 0090EC40 call site. |
| 0090EC40 | `scoring_record_unique_kill_0090ec40` | thiscall(scoring, unit, unused), RET 8 | Skips clients (mode 2), units whose +170h slot 0 says 0Ch and slots [unit+1B0h] above 7. A new unit id (+174h) is appended to record+130h and record+0DCh[category] += 1. |
| 0090ED40 | `scoring_add_unit_float_0090ed40` | thiscall(scoring, slot, unit, float), RET 0Ch | Skips clients; record+118h[category] += amount (one FADD). |
| 0090ED90 | `scoring_add_party_loss_0090ed90` | thiscall(scoring, party, const NativeString*, amount), RET 0Ch | For each of 8 records: party 0 adds to the map at +1F0h, party 1 at +1FCh (scoring-relative), others skip. |
| 0090EB80 | `scoring_count_unit_event_0090eb80` | thiscall(scoring, slot, unit, byte), RET 0Ch | Slot <= 7: record+0F4h (or +10Ch when the byte is set)[category] += 1. |
| 0090EBF0 | `scoring_count_owner_event_0090ebf0` | thiscall(scoring, unit), RET 4 | Slot [unit+180h] <= 7: record+0E8h[category] += 1. |
| 0090E670 | `scoring_record_drop_interval_0090e670` | thiscall(scoring, slot, unit, float), RET 0Ch | Slot <= 7: appends {word unit+174h, float(CVTTSS2SI seconds)} to record+160h via 0090D9D0. |
| 00905350 | `scoring_set_slot_ranking_00905350` | thiscall(scoring, slot, ranking), RET 8 | record+8 = ranking. |
| 0090BDA0 | `score_record_set_condition_message_0090bda0` | thiscall(record, NativeString by value), RET 8 | Assign into +254h, release the argument. |
| 0090BE30 | `score_record_set_victory_message_0090be30` | same | +25Ch. |
| 0090BEC0 | `score_record_set_message_264_0090bec0` | same | +264h (callers 008BC0C0 and 0091C650). |
| 0090BFC0 | `scoring_set_final_function_name_0090bfc0` | thiscall(scoring, const NativeString*), RET 4 | Assign into +147Ch, self-assignment skipped. |
| 009064D0 | `scoring_map_usage_points_009064d0` | thiscall(scoring, float), RET 4 | FISTP(float([1468h] * s)). |
| 00910500 | `scoring_remaining_unit_points_00910500` | thiscall(scoring, category, index), RET 8 | FISTP(float(config[index] * float([1464h] * map1428[category]))). |
| 00910570 | `scoring_suicide_points_00910570` | thiscall(scoring, category, index, count), RET 0Ch | count 0 -> 1; FISTP(float(config[index] * float(float([146Ch] * count) * map1428[category]))). |
| 00911C00 | `scoring_kill_points_00911c00` | thiscall(scoring, selector, by, target, float kills, index, byte flag), RET 18h | v = map1434[by][target]; selector 0 / 2 scales by [144Ch] / [1458h]; s = [[game+21A0h]+1478h] if flag else +1474h; FISTP(float(float(config[index] * float(v * kills)) * s)). |
| 00911CE0 | `scoring_damage_points_00911ce0` | thiscall(scoring, by, target, selector, int damage, index, byte flag), RET 18h | v = float(damage * map1440[by][target]); selector 0 / 2 scales by [1454h] / [1460h]; FISTP(float(float(config[index] * v) * s)). |
| 0091C560 | `scoring_reset_for_new_mission_0091c560` | thiscall(scoring), tail JMP 0090FA40 | Clears +147Ch (Resize 0,0; memcpy from 00CE3A0C), 00916980, per slot record+254h = i, record+0Ch = 2 in a session else [game+6ACh], byte +14B0h+i = 0; 00994310 on [00F8A2FC]; +1494h = +14ACh, +14A4h/+14A5h = 0, +149Ch = +14A8h, +1424h = local slot (0 offline), +1498h = 1, +14A0h = 0.0; then 0090FA40 through the host. |

`config` is the float vector at `00432650()+2Ch`, read with the checked `[index]` (00BF6713 when
unallocated or index >= size, unsigned).

### Award grant (0090EDE0)

1. `report = true`. mode = [game+1FE4h]. Modes 0 and 1 run 0090C000 on an inline copy of the
   token and discard the result (0090EE65). When the slot is not the local one [game+18ECh],
   [game+5D4h] == 0Dh and mode == 1, a session message 21 is constructed (00764340) on the stack
   ([ESP+24h], 18Ch bytes): +18h = 14h, +1Ch = slot, +188h = value, +7Ch = unbounded copy of the
   token text (00F899A0 when null); sent with `00772AF0(ECX = game+1EF0h, &msg, [game+18CCh+slot*4])`;
   `report = false`; skip to step 3.
2. Find the token in record+70h (checked iterator). Absent: 0090C000 on a 00426060 copy; a badge
   goes to the HUD ticker `00648AB0(ECX = [[00E198C4]+40h], copy)` and clears `report`. Present:
   clears `report`.
3. `stored = accumulate ? map[token] + value : value; map[token] = stored` (two 005070C0 calls when
   accumulating).
4. When `report`: 0090C000 on a fresh copy; stop when it is a badge or the slot is not local; stop
   when the token has the "RANK" prefix and length < 6, or the "Counter" prefix; id =
   `006B8DA0([00E19900], token)`; when id-1 <= 62h: `00A410A0([00F8ABE8], id)` then
   `007FBE20(game+650h, token, map[token])`.

## Precision

The point formulas assume the main thread's 24-bit precision control (callers are the rewards
screen builders and the bot scheduler, docs/CC13_FPU_PRECISION.md): each x87 product rounds to
float, modeled as float multiplications in the image's order. FIMUL/FILD products are formed in
double (exact for |count|, |damage| < 2^29) and rounded once. Under 53-bit control the chained
products in 00910570, 00911C00 and 00911CE0 would round once at the FSTP instead (possible
1-ulp differences before FISTP). FISTP uses round-to-nearest-even; unrepresentable values give
80000000h.

## Existing implementations recorded (ledger only)

| Address | Where | Verified |
|---|---|---|
| 0090C800 / 0090C870 / 0090C8E0 / 0090C950 | `scoring_get_slot_keyed_score` (src/scoring_bindings.cpp) with `kScoringMapOffsetAction/Ship/Plane/Command` = 24h/30h/3Ch/48h | LEA ESI,[EAX+24h/30h/3Ch/48h] after `this+slot*284h+4`, 004C8B80 find, RET 8 (0090C84C, 0090C8BC, 0090C92C, 0090C99C). |

## Left in the band (not reconstructed here)

- 0090FA40 (the reset tail, about 740h bytes: session message 21 sends of the profile counters
  Counter_RUA_DU / Counter_RUA_MU / Counter_RUA_DBU on a client, grants on a host).
- 00915760 (record clear, 7C0h), 009150A0 (force refresh), 0091C650 (session message 15h
  handler), 009105F0 (16h handler), 0090ABB0 (score row send), 0091D780 (2880h), 0091A080,
  00920690, 00906430: large or session/persistence-side; not started.
- 00915F20 (kamikaze direct hit delivery), 00913D80 / 00914100 (failure statistics / repair
  notify), 00911E80 / 00912A60 (the side-AI scheduler think passes): gunnery or AI subjects
  hosted by cc9 (game_hosts_gunnery.cpp, game_hosts_mission_frame.cpp); left to cc9.
- The remaining `FUN_` entries between 00906430 and 00911470 and 0091CC40..00922210 are STL
  instantiations or unreferenced (no callers, no data references in the index).
