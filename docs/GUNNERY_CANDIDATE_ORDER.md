# Gunnery candidate order

Addresses: `00865284`, `008657A3`, `00863990`, `00729BC0`, `00729B90`, `005459E0`,
`00727F10`, `00728000`, `00432480`, `007B8AD0`, `0063BCD0`, `00864D90`, `00864CA0`,
`0042B2F0`, `00414DB0`. All the call sites are inside `BSP_UnitGunneryAi_Tick`,
Ghidra body `00864FE0`-`008658B8`.

This document answers one question: **which end of the per-category candidate list
does a gun try first, and does the walk stop there.** It was opened because
`src/unit_gunnery_pass.cpp` carries the claim "both arms append unsorted, so their
entries sit at the end of the order array and are therefore tried first" and that
claim was treated as unverified after a wiring change moved the assign count from
2100 to 4989 and the hit rate from 69% to 14%.

**The claim is correct.** It is also already carried, with the same conclusion, by
`docs/UNIT_GUNNERY_PASS.md` (section 5, around the step 8.7 pseudocode). What
follows is an independent instruction-level derivation, plus the part that
document states but the packet brief assumed was absent: the walk does have range
gates, and they change what the ordering implies.

Reused rather than re-derived: `docs/UNIT_GUNNERY_PASS.md` for the tick's step
numbering, the `this+0CCh` / `DAT_00E19BF8` table pair and the six gun bot slots;
`docs/GUN_BOT_TICKS.md` for the bot-slot layout at `gun+390h`..`gun+3A4h`;
`docs/SHIP_SUB_ENTITY_LIST.md` for vtable slot `0FCh`; `docs/ENTITY_CLASS_IDS.md`
for the `0`..`60h` class-id space that makes `61h` a full table row.

## 1. The two frame arrays and their bound

`00864FE0` builds its prologue as `PUSH EBP / MOV EBP,ESP / AND ESP,0FFFFFFF8h /
SUB ESP,444h / PUSH EBX / PUSH ESI / PUSH EDI`, so the locals occupy
`ESP+0` .. `ESP+44Fh`. Two arrays live in that frame:

| Array | Base | Stride | Span | Entries |
| --- | --- | --- | --- | --- |
| order | `ESP+90h` | 4 (`[ESP + k*4 + 90h]`) | `90h`..`1CFh` = `140h` | `50h` |
| records | `ESP+1D0h` | 8 (`[ESP + i*8 + 1D0h]`) | `1D0h`..`44Fh` = `280h` | `50h` |

The record is **8 bytes**: `+0h` the entity pointer, `+4h` a float. Every access in
the body uses that stride - `MOV EAX,[ESP + EDI*8 + 1D0h]` at `008652A5` for the
pointer and `FLD [ESP + EDI*8 + 1D4h]` at `008652B8` for the float. **There is no
rank field in the record.** The rank is recomputed from a global table at every
comparison; section 3 shows the three instructions that do it.

`[ESP+10h]` is the count. It is reset to `0` at `008651AA`, at the head of each of
the twelve category iterations, so the list is per category and not per tick. The
only comparison of `[ESP+10h]` against a constant in the whole body is
`00865286 CMP dword ptr [ESP+10h],4`, which is the four-way unroll guard.
**Nothing bounds the count against `50h`.** A category that produced more than
`50h` accepted candidates would write records past `ESP+44Fh`, into the caller's
frame. This is a static observation, not an observed fault: `00863990`'s range
gate and `00864D90`'s visibility gate both filter, and no run in this packet
reached that count.

## 2. ABIs, from the stack cleanup

| Routine | ABI | Cleanup evidence | Coverage |
| --- | --- | --- | --- |
| `00863990` | `__thiscall bool(AiNode*, int category, Entity* contact, float* outKey)` | `RET 0Ch` at `008639B3` and `00863A7C`; three dwords | complete, `00863990`-`00863A7E` |
| `00729BC0` | `__thiscall bool(Gun*, Entity* target)` | `RET 4` at `00729BFD`, `00729D13`, `00729D25` | complete, `00729BC0`-`00729D27` |
| `00729B90` | `__thiscall float(Gun*)`, result in ST0 | `RET` with no immediate at `00729BA8`, `00729BB2` | complete, `00729B90`-`00729BB2` |
| `005459E0` | `__thiscall bool(Gun*)` | `RET` with no immediate at `005459F8`, `005459FE` | complete, `005459E0`-`005459FE` |
| `00727F10` | `__thiscall void(Gun*, Entity* target, int designated)` | `RET 8` at `00727FE9`, `00727FFC`; two dwords | complete, `00727F10`-`00727FFE` |
| `00728000` | `__thiscall void(Gun*)` | tail not read past `0072806A` | partial: `0072806A`-`00728073` unread |
| `00432480` | `__thiscall void(Entity*, Container* out)` | `RET 4` at `00432492`; one dword | complete, `00432480`-`00432492` |
| `007B8AD0` | `__thiscall bool(Entity*)` | `RET` with no immediate at `007B8ADB` | complete, `007B8AD0`-`007B8ADB` |
| `0063BCD0` | `__thiscall void(Container*)` | head only | partial: `0063BCE5`-`0063BD09` unread |
| `00864D90` | `__thiscall bool(VisCache*, Entity*)` | `RET` not read | body not read in this packet; name is prior art |
| `00864CA0` | `__thiscall void(AiNode*, Entity*, Gun*)` | `RET` not read | body not read in this packet; name is prior art |

The argument order at `00863990` is fixed by its three call sites, all of which
push right to left: `00865230` (`PUSH EDX` = `&[ESP+2Ch]`, `PUSH EAX` = contact,
`PUSH ESI` = category), `008654D4` and `00865626`. Inside the callee,
`MOV EBP,[ESP+14h]` after `SUB ESP,0Ch / PUSH EBP` reads `entry_esp+4`, the first
argument, and that value is later used as `FLD [ESI + EBP*4 + 430h]` - a category
index, not a pointer.

## 3. What "rank" is, and where it comes from

`00865272`..`00865284`, for the new contact:

```
0086526A  MOV ECX,[ESP+20h]            ; the contact
0086526E  FLD  float ptr [ESP+2Ch]     ; ST0 = the key 00863990 wrote
00865272  IMUL ESI,ESI,61h             ; ESI = category * 61h
00865275  MOV EDX,[ECX + 0C4h]         ; the contact's class id
0086527B  ADD EDX,ESI
0086527D  MOV EDX,[EDX*4 + 0E19BF8h]   ; EDX = rank
```

and the identical three instructions for the candidate already in the array, at
`0086529F`..`008652A7`, `008652D9`..`008652E1`, `00865313`..`0086531B`,
`0086534D`..`00865355` and `0086539B`..`008653A3`. `61h` is 97 and the class-id
space is `0`..`60h`, so `DAT_00E19BF8` is one dword per (category, class id).

Two consequences:

- The rank is a **pure function of (category, class id)**, read fresh at every
  comparison. Caching it in the record, as `GunneryCandidate::rank` in
  `include/bsp/unit_gunnery_pass.hpp` does, is behaviourally equivalent but is
  not the native record layout. See section 8.
- Distance does not participate in the rank. It participates only as the
  tie-break, and only through the key `00863990` wrote.

**Does a higher rank sort earlier or later?** The array is built descending in
rank (section 4) and consumed back to front (section 5), so the gun sees
**ascending rank: the numerically smallest rank first**. A lower number is the
higher priority. `docs/UNIT_GUNNERY_PASS.md` section on `00862820` records that a
rank of `0` is a rejection, which is consistent: `0` is not "best", it is "never".

## 4. The insert, `00865284`..`0086542B`: the array is descending

The scan runs **front to back** over the order array, four positions per
iteration plus a scalar tail. Position 0 of the unrolled body:

```
00865291  MOV EDI,[ESP + ECX*4 + 90h]   ; EDI = order[ECX]
00865298  MOV EAX,[ESP + EDI*8 + 1D0h]  ; the candidate already there
0086529F..008652A7                       ; EAX = its rank (section 3)
008652AE  CMP EDX,EAX                    ; new rank vs existing rank
008652B0  JL  008652CB                   ; new < existing -> advance
008652B2  JG  008653D5                   ; new > existing -> STOP, insert here
008652B8  FLD float ptr [ESP+EDI*8+1D4h] ; equal rank: existing key
008652BF  FXCH                           ; ST0 = new key, ST1 = existing key
008652C1  FCOMI ST0,ST1
008652C3  FSTP ST1                       ; pop the existing key, keep the new one
008652C5  JA  008653D5                   ; new key > existing -> STOP, insert here
```

The x87 stack is exact: `0086526E` pushed the new key and nothing pops it until
`008653D9 FSTP ST0`; `FSTP ST1` is the keep-the-original idiom, so every one of
the five copies compares `new` against `existing` in that direction. The three
early-reject jumps into `0086542F` (`0086523E`, `0086524F`, `00865264`) all
happen before `0086526E`, so no path leaks an x87 register.

So the scan **advances** while the new candidate is worse and **stops** at the
first slot it beats, where "beats" is a strictly greater rank, or an equal rank
with a strictly greater key. Everything before the insert point therefore has a
greater-or-equal (rank, key). **The order array is sorted descending in
(rank, key).**

The commit:

```
008653D5  MOV EDX,[ESP+10h]                       ; count
008653E7  MOV [ESP + EDX*8 + 1D0h],EAX            ; record[count].entity
008653EE  MOVSS [ESP + EDX*8 + 1D4h],XMM0         ; record[count].key
008653F9  JLE 00865415                            ; nothing to shift
00865400  MOV ESI,[ESP + EAX*4 + 8Ch]             ; order[EAX-1]
00865407  MOV [ESP + EAX*4 + 90h],ESI             ; -> order[EAX]
00865413  JG  00865400
00865421  MOV [ESP + ECX*4 + 90h],EDX             ; order[at] = count
0086542B  MOV [ESP+10h],EDX                       ; count+1
```

The **record is always appended at `count`**; only the order array is sorted.
`8Ch` is `90h - 4`, so the shift copies `order[i-1]` to `order[i]` downward from
`count`, which needs the order array to hold `count+1` entries during the shift.

## 5. The walk, `00865773`..`00865871`: back to front, first survivor wins

```
00865777  MOV EAX,[EDX + 398h]     ; the gun list head; EDX = unit + category*0Ch
00865789  MOV ECX,[ESP+10h]
0086578D  ADD ECX,-1
00865790  MOV [ESP+40h],ECX        ; the cursor seed = count - 1
0086579A  MOV EBX,[ESP+40h]        ; reseeded for EVERY gun
008657A0  MOV EDI,[EAX + 8h]       ; the gun
008657A3  JL  0086586A             ; count == 0 -> ClearBotFireTarget
008657B0  MOV ESI,[ESP + EBX*4 + 90h]
008657B9  LEA ESI,[ESP + ESI*8 + 1D0h]   ; &record[order[EBX]]
   ... gates ...
00865822  SUB EBX,1
00865825  JNS 008657B0             ; next candidate
00865827  JMP 0086586A             ; exhausted -> ClearBotFireTarget
```

`00865790` seeds the cursor at `count-1` and `00865822` decrements it, so **step 0
of the walk is `order[count-1]`, the far end of the sorted array**. Combined with
section 4 that is ascending (rank, key): the lowest rank number first, nearest
first inside a rank.

The exit is the decisive half. When a candidate passes every gate the code reaches
`00865829`, calls `00727F10` at `00865833`, optionally `00864CA0` at `00865860`,
and then falls into `00865871`, which advances the **gun** list and jumps back to
`00865796`. **No instruction between `00865829` and `00865871` jumps back to
`008657B0`.** The gun takes the first candidate that survives and never examines
another. Ordering is therefore not a preference, it is the whole decision.

`0086586A CALL 00728000` (clear the target on all six bot slots) is reached from
`008657A3` when the list is empty and from `00865827` when every candidate was
skipped.

## 6. Step 8.7 really is unranked, and it really does land at the far end

Both arms are the same shape. Arm B, the command target, first:

```
008654AC  if ([ESP+38h] == [ESP+18h]) goto 00865609   ; same as arm A -> skip
008654B6  if ([ESP+38h] == 0)         goto 00865609
008654D4  if (!00863990(ai, category, [ESP+38h], &[ESP+6Ch])) goto 00865609
008654E5  ref = 00427EB0(owner)                        ; the owner's position
00865516  0063BCD0(ai + 0BCh)                          ; reset the scratch list
00865521  [[ESP+38h]]->vtable[0FCh](ai + 0BCh)         ; the sub-entity list
00865531  begin = 0042EE00(ai + 0BCh)
0086553B  end   = 0042EDD0(ai + 0BCh)
0086554A  for (e : that range):
00865584    if (!00864D90([ai+68h], e)) continue
0086558D    record[EBX].entity = e
008655D5    record[EBX].key    = 0042B2F0(ref - e->pos)   ; a raw length
008655E1    order[EBX] = EBX
008655E8    EBX = EBX + 1
```

Arm A, the fire target, is `00865680`..`0086574B` and differs only in the target
(`[ESP+18h]`, resolved at `00865447` from `[ai+5Ch]->vtable[4h]()`), the position
reference (the owner's cached `+0FCh`/`+100h`/`+104h` triple after a `00414DB0`
refresh, rather than the `00427EB0` accessor) and the stack slots.

Three facts follow, all from `008655E1` and `0086574B`:

1. **No rank is read.** Neither arm touches `+0C4h` or `DAT_00E19BF8`. The store
   is literally `MOV dword ptr [ESP + EBX*4 + 90h],EBX`.
2. **No shift is performed**, so the entry stays at position `count`, the far end.
3. **No key bias is applied.** `0042B2F0` returns the raw length; the `+100.0`
   of `00863A6B` is only on the recon path.

Both arms run after the recon sweep (`00865442` is the sweep's exit) and arm A
runs after arm B, so the final order array is, front to back:

```
[ recon contacts, descending (rank, key) ][ arm B sub-entities ][ arm A sub-entities ]
```

and the walk, back to front, visits **arm A's sub-entities in reverse insertion
order first, then arm B's in reverse insertion order, then the recon contacts
ascending**. The comment in `src/unit_gunnery_pass.cpp` is right, and the reason
is the back-to-front walk of section 5, not the append on its own.

`00432480` is the base `0FCh`: `PUSH ECX / MOV [ESP],ECX / MOV ECX,[ESP+8] /
PUSH EAX / CALL 004323D0 / RET 4` - it pushes the entity itself, so an entity that
does not override the slot contributes exactly one candidate.

## 7. The gates, and what they do not cover

Four distance tests exist. Three of them are real range gates and one is a
minimum.

| # | Site | Rule | Applies to |
| --- | --- | --- | --- |
| 1 | `00863A34`..`00863A41` | `key < owner[category*4 + 430h]`, strict (`JC`) | every recon contact, and each arm's **target**, not its sub-entities |
| 2 | `008657D8`..`008657F0` | skip when `00729B90(gun) > key`, a **minimum** | only when `005459E0(gun)` (kind `5` or `6`) and the candidate answers `vfn 5Ch(0Fh)` |
| 3 | `00729C81`..`00729C8A` | reject when `distance > [proj+60h]`, strict (`JA`) | **every** candidate, recon and step 8.7 alike |
| 4 | `00729C00`..`00729C13` | reject when `vfn 5Ch(5)` and `[t+54h] == 2` | every candidate |

Gate 3 is the one the packet brief assumed was absent. `00729BC0` recomputes the
distance itself at `00729C7A` from the two refreshed positions - it does not
trust the record's key - and compares it against `+60h` of the record selected by
`[gun+3F4h]+74h + (kind == 6 ? 48h : 0) + 34h`. So **a gun does not accept a
target outside its ammunition's range**: it falls through to the next candidate.

What gate 3 is not: it is a nominal ballistic reach, not a hit-probability gate.
Nothing in the walk asks whether a firing solution exists, whether the flight time
is survivable, or whether the target is closing. A candidate at 95% of `[proj+60h]`
is accepted exactly as readily as one at 5%.

Gate 1 is the interesting hole. Each arm calls `00863990` on the **target**, which
applies the per-category acquisition range, and then enumerates that target's
sub-entities and appends each one with only `00864D90` (visibility) in the way. A
sub-entity is by construction near its parent, so the hole is narrow, but it is
real: the per-category range is enforced on the parent's centre only.

Gate 2 inverts the usual reading. `00729B90` returns `+58h` of the same record
whose `+60h` gate 3 uses, and `008657F0 JA` skips when that value **exceeds** the
candidate distance. It is a "too close" rule, and it fires only for a kind-5/6
mount against a class-`0Fh` candidate.

One more skip, not a distance test: `00865809`..`00865820`. For category 7 only,
a candidate that **is** the arm-A target is skipped unless `[ai+7Dh]` is set. The
torpedo category therefore prefers the command target over the fire target, the
opposite of the order the array gives it. Category 7 also never runs the recon
sweep (`008651F5 CMP ESI,7 / JZ 00865442`), so a category-7 gun has no candidates
at all except the two step-8.7 arms.

## 8. The recon key is biased, not a distance

`00863A21`..`00863A71`:

```
00863A21  CALL 0042B2F0                 ; |owner.pos - contact.pos|
00863A32  FST  float ptr [EBX]          ; *outKey = the raw length
00863A34  FLD  float ptr [ESI + EBP*4 + 430h]
00863A3B  FXCH                          ; ST0 = length, ST1 = range
00863A3D  FCOMIP ST0,ST1
00863A41  JC   00863A4F                 ; length < range -> continue
00863A46  XOR  AL,AL                    ; otherwise reject
00863A4F  if (contact->vfn 5Ch(0Fh) && 007B8AD0(contact))
00863A69     *outKey = *outKey + [00D7A220]
```

`00D7A220` holds `00 00 00 00 00 00 59 40`, the double `100.0`.
`007B8AD0` is `XOR EAX,EAX / CMP [ECX+9D8h],EAX / SETZ AL / RET`, so it is true
when the contact's `+9D8h` slot is null. The stored key is therefore
`distance + 100` for a class-`0Fh` contact with a null `+9D8h`, and plain distance
otherwise - a de-prioritising bias inside the tie-break, applied **after** the
range gate reads the unbiased length.

The step-8.7 arms never apply it, so a recon contact and an arm sub-entity at the
same true distance can carry keys 100 apart. Since the two groups never compare
against each other (section 6), that only matters within the recon group.

## 9. What the target arms resolve to

Arm A, `00865447`: `ECX = [ai+5Ch]` (the fire-target policy object),
`CALL [[ECX] + 4h]`, result to `[ESP+18h]`.

Arm B, `00865456`..`008654A8`: `unit->vtable[140h]()` guarded three times for
null, then `->vtable[114h]()`, then `0071EBF0`, then `00521EA0`, result to
`[ESP+38h]`. `docs/UNIT_GUNNERY_PASS.md` records that `vtable[140h]` is `0047F320`,
`MOV EAX,ECX / RET` - identity - in the base case.

`[ESP+18h]` is also what `00865804` compares each candidate against to compute the
`designated` byte at `[ESP+30h]`, which `00727F10` forwards to bot slot `+39Ch`
alone (`00727F52`, `vtable[3Ch]`). Note that `0086580E` writes `[ESP+30h]` as a
**byte** and `00865829` reads it as a **dword**; the upper three bytes are stale
frame contents. The callee only tests the low byte, so this is a native quirk, not
a defect to reproduce faithfully.

## 10. Proven vs. assumed

Proven from the listing in this packet:

- The record is 8 bytes, `{entity, key}`, with no rank field; capacity `50h` for
  both arrays; no bound check on the count.
- The order array is descending in (rank, key); the insert stops at the first slot
  the new candidate strictly beats.
- The walk is back to front from `count-1`, reseeded per gun.
- The walk stops at the first survivor; no back edge to `008657B0` after
  `00865829`.
- Step 8.7's two arms append at `order[count] = count` with no rank read and no
  shift, so **the director's targets are tried before every recon contact, and the
  fire target before the command target**.
- Gate 3 (`00729C81`) rejects any candidate beyond `[proj+60h]`, so a gun does not
  lock a target outside its ammunition range.
- `100.0` at `00D7A220`; `61h`-wide rank table at `0E19BF8`; `00432480` appends
  the entity itself.

Assumed or unread:

- `00864D90` and `00864CA0` keep their prior Ghidra names; their bodies were not
  read here. `00728000`'s tail after `0072806A` and `0063BCD0`'s body after
  `0063BCE5` were not read.
- The meaning of the class queries `vfn 5Ch(0Fh)`, `(0Eh)`, `(5)` and of the kind
  enum `[gun+3F4h]+80h` values `2,3,4,5,6,9` is taken from
  `docs/UNIT_GUNNERY_PASS.md` and `docs/WEAPON_DIRECTOR.md`, not re-derived.
- No run was made in this packet. Section 11's attribution of a measured hit-rate
  change to this ordering is a mechanism argument, not run evidence
  (`docs/WORKER_VERIFICATION_CHECKLIST.md` rule 6).

Corrections and clarifications to existing modules (not applied here; the
integrator folds them in):

| Where | Was | Is | Evidence |
| --- | --- | --- | --- |
| `include/bsp/unit_gunnery_pass.hpp` `GunneryCandidate` | a three-field record with `rank` | the native record is 8 bytes, `{void*, float}`; the rank is recomputed per comparison | stride 8 at `008652A5`/`008652B8`, rank reload at `0086529F`..`008652A7` |
| `src/unit_gunnery_pass.cpp` step 8.7 comment | "append unsorted, so ... therefore tried first" | true, but the reason is the back-to-front walk seeded at `00865790`; appending at the end alone would make them **last** | `00865790` / `00865822` |
| `src/unit_gunnery_pass.cpp` `candidates[count].rank = 0` | a stored rank of zero | an unobservable placeholder: the arms run after the last insert, so no comparison ever reads it. A rank of `0` is a rejection elsewhere (`00862820` step 3), so the value is misleading | `008655E1`, `0086574B` |

## 11. What this means for the assignment regression

The ordering is native-correct. That is the load-bearing point: wiring vtable slot
`0FCh` so that step 8.7 contributes candidates does not introduce a bug, it
restores the native preference. Every gun in every category whose two director
targets resolve now tries those targets, in full, before any recon contact, and
locks onto the first one that clears gates 2, 3 and 4.

The mechanism is sufficient to produce a large assign and shot increase with a
falling hit rate:

- Categories that previously produced nothing now produce something. Category 7
  never sweeps recon at all (`008651F5`), so before the change every category-7
  gun took `0086586A` and cleared its target on every tick.
- Guns that previously took the nearest high-priority recon contact now take the
  director's target instead, whatever its distance up to `[proj+60h]`.
- `[proj+60h]` is a ballistic reach. A target at 90% of it is accepted with the
  same indifference as one at 10%, and the walk never reconsiders.

The mechanism does **not** support "the gun accepts a target it cannot reach".
Gate 3 forbids that. If the observed misses are at distances beyond `[proj+60h]`,
the cause is elsewhere: either the reconstruction is not applying gate 3, or the
two arms are resolving to something the native would not resolve to, or the
`0FCh` override the reconstruction installs returns a different set than the
native ship class does. Those are the three places to look, and the first is
cheapest to check.

A useful discriminator, and the one this packet cannot run: log, per accepted
assignment, the candidate's source (recon, arm A, arm B) and its distance against
`[proj+60h]`. If the water and expiry shells concentrate in arm A/B assignments
below `[proj+60h]`, the ordering is native and the reconstruction's aim or lead
solution is what diverges. If any accepted assignment sits above `[proj+60h]`,
gate 3 is missing from the reconstruction.

## Follow-up packets

1. **Gate 3 in the reconstruction.** Confirm `00729BC0`'s `distance > [proj+60h]`
   rejection at `00729C81` is present in `src/game_hosts_gunnery.cpp`'s accept
   host and that it recomputes the distance rather than reusing the record key,
   which is what `00729C7A` does. Owned files: the gunnery host module.
2. **`00864D90`, the visibility cache test.** `00864D90`-`00864FC9`, the only gate
   a step-8.7 sub-entity faces before it enters the list. Its `+68h` cache is a
   `24h`-byte record aged at `0086505D`; the aging and the `-1.0f` seed at
   `00864C0F` are unread.
3. **The kind enum `[gun+3F4h]+80h`.** Values `2,3,4,5,6,9` are branched on at
   `005459EC`, `00729B96`, `00729CEF` and `00865844` with three different
   groupings. Recovering the enum would settle whether gate 2 is a torpedo
   minimum-arming range or an AA minimum.
4. **The category acquisition ranges `owner + category*4 + 430h`.** Twelve floats,
   the only per-category distance limit. Their producer is unidentified.
5. **The unbounded count.** Establish whether any reachable configuration can
   push a single category past `50h` accepted candidates, or prove the upstream
   filters bound it.
