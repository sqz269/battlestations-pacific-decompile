# The AI group think (the party commander's fixed step)

Addresses: 00A32D50 00A2E720 00A182C0 00A181A0 00A2E260 00A2DDE0 00A2DFA0 00A2DB80 00A2BD00
00A2BD90 00A2EEE0 00A2C8D0 00A2C5A0 00A2C6C0 00A2C660 00A2C450 00A22750 00A16B20 00A15970
00A184F0 00A18510 00A18520 00A0FC90 009FE080 009FE0F0 009FE120 009FFD80 009FFE50 00E0E34C
00E0E344 00E0E348 00F8A87C 00F8A89C 00F8A8BC 00F8A8C8 00F8A9E8 00F8AA48 00F8AA60 00F8AA6C
00F8AA78 00D22990 00D2306C 00D23084

Packet `cc2_ai_group_think`, worktree `agent/cc2-ai-group`. Ghidra was read-only for this packet:
no rename, comment, prototype or save. Every descriptive name below is a hypothesis, not a
recovered symbol. The two genuine recovered names are the Lua tuning keys **`AutoMerge_MergeDist`**
(`00D20180`-defaulted, record `+208h`) and **`Defend_ResourcePercent`** (`00D23500`), both string
literals in the image.

The reconstruction is `include/bsp/ai_group_think.hpp` and `src/ai_group_think.cpp`. It reuses no
type from `include/bsp/lua_binding_ai.hpp`, `ai_target_weights.hpp`, `command_classes.hpp` or
`entity_order_message.hpp`, and adds no top-level name that collides with them; every constant is
prefixed `kAiGroup`.

## Headline: two clocks, not one

The group think is two passes on two different clocks, and the packet's brief named the wrong
routines for both.

| Pass | Address | Rate | What it does |
| --- | --- | --- | --- |
| composition | `00A2E720` | every fixed step, 20 Hz | drain, evict, split, seed, auto-merge, per-member pass |
| party think | `00A182C0` -> `00A181A0` | per party, every 3 to 5 s | pick a planner for each unclaimed group, tick the planners |

Both hang off `00A32D50`, a twenty-byte routine **Ghidra has no function for**:

```
00a32d50: CMP byte ptr [00E0E34C],0
00a32d57: JE  00a32d63
00a32d59: CALL 00A2E720
00a32d5e: CALL 00A182C0
00a32d63: RET 4
```

`00A32D50` is slot `+8h` of the vtable `00D23168`, which the AI coordinator's constructor
`00A31730` installs on the tick-element node at `coordinator+170h` (`00A3177F`), registered into
fixed-step group 0 by `00875890 BSP_TickRegistration_Construct` at `00A31766`. Slot `+8h` is
"advance the simulation by one step" with a `float dt` of `0.05f` (`docs/TICK_ELEMENT_OVERRIDES.md`),
so the AI runs at 20 Hz. The `RET 4` pops that `dt` and **neither callee receives it**: both are
called with no pushes and with `ECX` untouched, so the AI is time-blind and uses the fixed-step
clock `00F876A4` directly.

`00E0E34C` is a `.data` dword whose low byte is the gate. Its absolute address occurs exactly once
in the whole image (`00A32D52`, the read), so nothing writes it; its static value is `1`
(`00E0E34C` = `01 00 00 00`), so the gate is always open in the shipped build.

## 1. The think as a rule table per step

### `00A2E720`, the composition pass (every step)

| Phase | Range | Rule |
| --- | --- | --- |
| 1 drain | `00A2E741`-`00A2E7E2` | while `[00F8AA80] > 0`: take the first node of the emptied list `00F8AA7C`; for every other group `g` in the global registry `00F8AA70` call `00A2B8F0(g+24h)(empty)`; unlink and free the node; then `empty->vtable[0h](1)`, the deleting destructor |
| 2 evict, split | `00A2E7E8`-`00A2E818` | for every group in `00F8AA70`: `00A2DDE0(group)` then `00A2E260(group)` |
| 3 seed | `00A2E835`-`00A2EA5A` | five entity collections hung off `world+19CCh` (`+64h`, `+13Ch` and three more): for each entity with `+5Ch` set, `+5Dh`/`+5Eh`/`+60h` clear, `+16Ch` null and `+54h < 2`, create one group per collection and add the rest to it |
| 4 auto-merge | `00A2EA5C`-`00A2EAF7` | for each of the eight party lists `00F8A9EC + p*12`: for each non-empty `A`, scan each non-empty `B`; if `00A2C8D0(A)(B)` then `00A2DB80(A)(B)` and restart the party's scan from the head |
| 5 proximity merge | `00A2EAFD`-`00A2EE7E` | only when `004BCA50(world) <= 3`: over the list at `00F8AA64`, for pairs with `009FFD70(A.leader) <= 009FFD70(B.leader)`, merge when the horizontal distance between the two leaders is under `AutoMerge_MergeDist` |
| 6 member pass | `00A2EE7E`-`00A2EEB5` | for party 0..7: publish the party in `00E0E344`, then `00A2C790(group)` for every group of that party; restore `-1` |

The five phases run unconditionally in that order. Phase 3 is the only creator of groups outside
the Lua binding `AICreateGroup`, and the `+54h < 2` gate (`CMP [ESI+54h],EBP` with `EBP = 2` at
`00A2E82E`) means only teams 0 and 1 are auto-grouped.

### `00A182C0`, the party pass (every step, but each party is rate-limited)

`__cdecl void()`, eight iterations: `EBP` runs `0xF8A8C8` to `< 0xF8A9A8` with `ADD EBP,0x1C`, so
there are exactly **eight party slots**. `00A16B20` bounds the brain array independently with
`CMP EDI,0xF8A8BC` (`00A16B46`), which agrees: `00F8A89C + p*4` for `p` in 0..7.

Per party, in order:

1. `[00E0E344] = party` (`00A18309`) before anything reads a record.
2. If the brain `[00F8A89C + p*4]` is null, or `00A15970(brain)` is false, gate on the clock:
   skip the party unless `[00F8A87C + p*4] <= [00F876A4]`.
3. Reschedule: `[00F8A87C + p*4] = now + BSP_Random_UniformFloatRange(3.0f, 5.0f)`. `00CE3854` is
   `3.0f` and `00CE3850` is `5.0f`; `00BD2F10` is already `BSP_Random_UniformFloatRange` in the
   ledger. **This is the AI commander's real tick rate: three to five seconds, jittered.** The
   reschedule happens before the enable test, so a disabled party still burns its timer.
4. If the record's enabled byte (`+0h`) is clear, or `009FFE50(party)` is false: `00A16490(brain)`,
   `00BF65AC(brain)`, then `[00F8A89C + p*4] = 0`.
5. Otherwise allocate `28h` and construct with `00A15A70(party)` if the slot is empty, then
   `00A181A0(brain)`.
6. After the loop, `[00E0E344] = -1`.

`00A15970` is the immediate-think override: for modes 4 through 7 it asks the matching
mode-specific planner (`brain+10h`..`+1Ch`) through `vtable[+30h]`, and a `true` bypasses the
timer entirely.

`009FFE50(party)`, on the arm where `world+61Ch` is set: **modes above 3 enable every party, modes
0 through 3 enable only party 0 and party 4.** Those two are the two sides' commander slots.

### `00A181A0`, one party brain's think

`__thiscall(brain)`, no stack arguments, tail-jumps to `00A179E0`. The brain is `28h` bytes:

| Offset | Meaning |
| --- | --- |
| `+0h`, `+4h`, `+8h`, `+0Ch` | four planners, all four ticked through `vtable[+20h]` every think |
| `+10h`, `+14h`, `+18h`, `+1Ch` | mode-specific planners for modes 4, 5, 6, 7 |
| `+20h` | the party slot, the key into `00F8A9E8 + p*12` |
| `+24h` | an index into the world's entity-set array at `world+21A4h + k*4` |

`mode = 004BCA50([00E188A8])`. For mode 4, 5, 6 or 7 the think is one call,
`brain[10h + (mode-4)*4]->vtable[+20h]()`, and the group walk is **skipped entirely**. Otherwise:

- For every group of this party with a non-zero population:
  - `00A2E260(group)` unconditionally, even for a group another planner owns;
  - if `group+5654h` is null, choose a planner and claim the group with `00A22750`:

| `00A2C5A0(group)` | `00A2C450(group, brain+24h)` | planner |
| --- | --- | --- |
| false | not evaluated | `brain+0h` |
| true | true | `brain+0h` |
| true | false | `brain+0Ch` |

- Then tick `brain+0h`, `+4h`, `+8h`, `+0Ch` through `vtable[+20h]`, in that order.
- Tail-call `00A179E0(brain)`, the brain's own planning pass (`contract: partial`, body
  `00A179E0`-`00A18195` not read by this packet; it starts from `brain+20h` through `008EA0C0` with
  the container at `00F88C30` and uses `brain+24h`).

`00A22750 planner_claim_group`, `__thiscall(planner)(group)`, `RET 4`: `00A2C600(group)`, then
`00A1C8B0(planner)(group)`; when that answers false it appends the group to the planner's own list
at `planner+20h`, sets `group+5654h = planner`, calls `00694A60 BSP_Observer_RegisterPair(group,
planner)` and sets `planner+2Ch = 1`. The claim is what makes `group+5654h` non-null, which is why
a claimed group is skipped on later thinks.

### The three member predicates

All three walk `group+563Ch` and answer "does any member satisfy":

| Routine | Member test | Class ids |
| --- | --- | --- |
| `00A2C6C0` | `009FE120` = `IsKindOf(06h)` | the ship base, "units" |
| `00A2C660` | `009FE0F0` = `IsKindOf(0Fh) or IsKindOf(18h)` | the plane base or `PlaneSquadronGen` |
| `00A2C5A0` | `009FE080` = `IsKindOf(18h) ? !007EDA90(e) : IsKindOf(06h)` | a ship, or a squadron `007EDA90` rejects |

Class ids from `docs/ENTITY_CLASS_IDS.md`. `007EDA90` was not read (`contract: unread`).

`00A2C450`, `__thiscall(group)(int setIndex)`, `RET 4`: any member for which
`008DDF90([00E188A8] + 21A4h + setIndex*4)(member)` is true.

## 2. `00A2BD90` is not a fan-out

The brief expected `00A2BD90 BSP_AiGroup_ForwardCommand` to turn a group command into per-member
commands with formation offsets and a leader. **It does no such thing.** Its seven instructions
(`docs/ENTITY_ORDER_MESSAGE.md` already transcribed them) forward both stack arguments to
`group+564Ch`'s `vtable[+24h]` and return. `group+564Ch` is not a command class of
`docs/COMMAND_CLASSES.md` -- those vtables are five slots of constant getters -- it is an **AI
command object**, an eight-byte instance whose vtable has ten slots.

`00A0FC90`, the base class's `vtable[+24h]`, is **`RET 8`** -- a routine Ghidra has no function for,
and a no-op. `00A0FC90` occurs sixteen times in `.rdata`, all inside the AI-command vtable block
`00D2298C`-`00D22C68`, so it is the `+24h` slot (or equivalent tail slot) of every class in that
block. `00A2BD90` is therefore a **notification hook** that lets a group's current AI command see
that one of its members was given a direct order; in the three classes the group constructor can
install it discards the notification. Whether any class in the block overrides the slot with a real
body was not settled instruction by instruction (`coverage: partial`).

Both callers are outside this packet: `0077D600 BSP_Entity_IssueCommand` at `0077D7A7` and
`0071ECF0 BSP_WeaponDirector_IssueCommand`, with arguments `(command, SceneCommandTarget*)`.

The real fan-out of a group command is **`00A2DB80`**, the merge, which the brief called the think.

### `00A2DB80`, the merge and the command retarget

`__thiscall(into)(from)`, `RET 4`. Returns at once when `from+5644h` is zero.

Phase A, `00A2DBC1`-`00A2DD0E`. For every group `g` in the global registry `00F8AA6C`:

1. skip when `g+5644h` is zero;
2. skip unless `g+564Ch->vtable[+8h](6)` is true, a family test on the command object;
3. skip unless `g+564Ch->+1Ch == from`, i.e. the command is aimed at the group being absorbed;
4. `cls = g+564Ch->vtable[+4h]()`, the command's class id, and allocate a replacement:

| `cls` | size | constructor | arguments |
| --- | --- | --- | --- |
| 7 | `20h` | `00A10890` | `(g, into)` |
| 8 | `2Ch` | `00A109B0` | `(g, into)` |
| 9 | `20h` | `00A10AE0` | `(g, into)` |
| other | - | - | the command is left alone |

5. `00A2BD00(g)(replacement)`, which deletes the old command through `vtable[0h]` with flag 1 and
   stores the new pointer.

Phase B, `00A2DD13`-`00A2DDD6`. While `from+5644h` is non-zero: `0077BEA0(from+563Ch)` removes the
first member, `006956A0(member, from+10h)` drops the observer pair, the emptied `from` is pushed
onto `00F8AA78`, and `00A2D8E0(into)(member)` adds the member to `into`.

So a merge **preserves the class of every command that pointed at the absorbed group and rebinds it
to the absorber**, then moves the members one at a time.

`00A2C8D0 can_auto_merge`, `__thiscall(into)(from)`, `RET 4`, is the gate. It answers false when
`from` is null or equal to `into`, when either `+5648h` grouping-enabled byte is clear, or when
either population is zero. Then, with `HasShip = 00A2C6C0` and `HasAir = 00A2C660`:

- `HasShip(into) and HasAir(from)` -> false;
- `not HasShip(into) and HasAir(into) and HasShip(from)` -> false;
- otherwise the decision is delegated to `from+564Ch->vtable[+14h](into)`.

**A ship group and an air group never merge.** Everything else is the absorbed group's command's
call.

### `00A2D8E0`, the sorted insert that defines the leader

`__thiscall(group)(entity)`, `RET 4`. `009FE0B0(entity)`; `entity+16Ch = group`;
`00694A60 BSP_Observer_RegisterPair(entity, group ? group+10h : 0)`; then `v = 009FFD80(entity)` and
the member list `group+563Ch` is walked, inserting before the first member whose `009FFD80` is
lower. **The member list is kept sorted descending by `009FFD80`, so the head is the leader** --
which is why `00A2EEE0`'s first Lua field is `"leader"` and why phase 5 of the composition pass
takes the first member as the group's position. `009FFD80` is a per-class weight: `[00D7A24C]` by
default, `[00CE3D08]` for class `06h`, and further arms for `1Bh` onward (`coverage: partial`,
`009FFD80`-`009FFDDE` read only for the first two arms).

## 3. The resource and ratio rules

### `Defend_ResourcePercent` -- and the overlap question is answered

`docs/LUA_BINDING_AI.md` and `docs/AI_GLOBALS_AND_TARGET_WEIGHTS.md` both flagged that
`00F8A8BC + slot*4` and the `1Ch` records at `00F8A8C8` "are not disjoint for slots 3 and above".
Two independent bounds settle the layout: `00A16B20` ends the brain array at `0xF8A8BC` and
`00A182C0` starts the party records at `0xF8A8C8`, so **the array is exactly three dwords wide**.

The globals loader guards it. At `00A360F5`:

```
00a360f5: CMP EDI,0x3
00a360f8: JGE 00a3613e          ; skip the store for modes 3..6
00a36121: CALL 00B66330         ; GetFloatOrDefault("Defend_ResourcePercent", [00CF6560])
00a36126: FSTP dword ptr [EDI*0x4 + 0xf8a8bc]
```

`EDI` is the loader's mode counter: the only writes to `EDI` before this store are `XOR EDI,EDI` at
`00A3374B` and `ADD EDI,0x1` at `00A370D4` (the whole-listing filter for `EDI` finds nothing else
until `00A36B8F`, after the store). The key string at `00D23500` is `Defend_ResourcePercent`. So the
three slots are the three difficulty modes `IslandCaptureParams_Rookie`, `_Regular` and `_Veteran`,
and **the loader never overflows**.

The Lua setter does. `00A37E4B` stores at `[ESI*4 + 0xF8A8BC]` with `ESI = 009FFC80()`, the game
mode 0..6, and no bound check. Calling `AISetDefendResourcePercent` in Duel, Escort, Siege or
Competitive writes into party 0's record: mode 3 hits the enabled byte, 4 the attack ratio, 5 the
aggressive ratio and 6 the quick-spawn-valid byte. None of the 299 installed mission scripts that
use the binding runs in those modes, per `docs/LUA_BINDING_AI.md`'s script survey, so the fault is
latent rather than reached.

Two accessors read the array, both routines **Ghidra has no function for**:

- `00A18510`: `FLD [ECX*4 + 0xF8A8BC]; RET` -- the defend share.
- `00A18520`: the same load, then `FLD1; FSUBRP; FSTP [ESP]; FLD [ESP]` -- `1 - percent`, the attack
  share, rounded to float by the store and reload.

### The two ratios

| Field | Address | Writers | Readers |
| --- | --- | --- | --- |
| attack ratio `+4h` | `00F8A8CC` | `00A163D6` (init), `00A32F29` (reset to `[00CE3800]` = 0.5), `AIEnable`'s table branch | **none found** |
| aggressive ratio `+8h` | `00F8A8D0` | the same three | `00A18510`-style accessor `00A184F0`, read at `00A1A73E` |

A full-image displacement scan for `00F8A8CC` finds three sites, all writes, and the scan for the
record base `00F8A8C8` finds four, none of which reads `+4h`. So **the attack ratio is written and
never read**; the aggressive ratio is the one the AI uses.

`00A184F0` is the accessor: `LEA EAX,[ECX*8]; SUB EAX,ECX; FLD [EAX*4 + 0xF8A8D0]; RET`, i.e.
`party*1Ch` -- third confirmation of the stride. Its one reader is `00A1A720`:

```
00a1a725: MOV EAX,[ESI+0x16c]            ; the entity's group
00a1a72b: TEST EAX,EAX; JZ 00a1a757      ; ungrouped entities take the other arm
00a1a72f: MOV ECX,[ECX+0x1c]             ; planner+1Ch = the brain
00a1a732: MOV ECX,[ECX+0x20]             ; brain+20h = the party
00a1a73e: FLD [EDX*4 + 0xf8a8d0]         ; the party's aggressive ratio
00a1a74e: CALL 00A2CBD0                  ; (group, ratio)
```

So the aggressive ratio reaches the group as a scalar argument of `00A2CBD0`
(`__thiscall(planner)(group, float aggressive)`, `contract: unread`). The ungrouped arm at
`00A1A757` goes through `00A2C600` instead.

## 4. Quick-spawn

`00A25B90` is the one consumer, at `00A25C06`:

```
00a25c06: MOV ECX,[0x00E0E344]                 ; the ambient party, published by the think
00a25c0c: LEA EAX,[ECX*8]; SUB EAX,ECX
00a25c15: ADD EAX,EAX; ADD EAX,EAX             ; party*1Ch
00a25c19: XOR EDX,EDX
00a25c1b: CMP byte ptr [EAX + 0xF8A8D4],BL     ; the quick-spawn-valid byte, +0Ch
00a25c21: JZ  00a25c29                         ; leave EDX null when it is clear
00a25c23: LEA EDX,[EAX + 0xF8A8D8]             ; else EDX = &position, +10h
00a25c3e: CALL 00A25A30                        ; (&position or null, 1.0f, ..., 1)
```

The quick-spawn record is therefore an **optional position hint**: a null pointer when the valid
byte is clear, and the address of the three floats otherwise, passed with the weight `1.0f` and the
flag `1` into `00A25A30` (`contract: unread`). Note that the consumer takes the party from
`00E0E344` rather than from an argument, so it is only correct while the think owns the call stack;
outside a think the global is `-1` and the index is negative.

## 5. The group object's full field table

Everything `00A2DFA0` writes, plus the fields other routines settle. `__thiscall(block)(entity)`,
`RET 4`, body `00A2DFA0`-`00A2E1D3`, read end to end.

| Offset | Value or producer | Meaning |
| --- | --- | --- |
| `+0h` | `00D23084` | primary vtable; slot 0 is `00A2D8C0`, the deleting destructor, which calls `00A2D440` |
| `+4h`, `+8h`, `+0Ch` | 0 | unread |
| `+10h` | `00CE3CD4` then `00D2306C` | **the observer subobject**, passed as the observer to `00694A60` at `00A2D906` and to `006956A0` at `00A2E3BE`. Its vtable is five slots: `00A2D570`, `00A2DA60`, `00A2BD40`, `00A2DB50`, `0042B140` |
| `+14h`, `+18h`, `+1Ch` | 0 | unread |
| `+20h` | byte 0 | unread |
| `+24h`..`+5623h` | **written by nobody in the constructor** | `00BF681B` does not zero and the constructor stores nothing here; `0x5600` bytes with no producer |
| `+5624h` | 0 | unread |
| `+5628h` | `-1` | unread |
| `+562Ch` | 1 | unread |
| `+5630h` | `this`, at `00A2E1BC` | self pointer |
| `+563Ch` | `std::list<Entity*>` object | the member list |
| `+5640h` | `004C1630(+563Ch)` | the list's sentinel node; `[+5640h]` is the first member's node and `node+8h` the entity |
| `+5644h` | 0 | the member count; every pass gates on it |
| `+5648h` | byte 1 | grouping-enabled; `00A2C8D0` rejects a merge when either side has it clear |
| `+564Ch` | a new 8-byte AI command | vtable `00D22990` when `+5634h < 0`, `00D229E0` when `009FFE50(+5634h)` is true, `00D22990` otherwise; `+4h` back-points to the group |
| `+5650h` | `0.0f` | unread |
| `+5654h` | 0 | the claiming planner, set by `00A22750` |
| `+5658h` | `[00E0E348]`, then the counter increments and wraps to 1 at 1000 | group id, 1..999 |
| `+565Ch` | byte 0 | unread |
| `+5634h` | `009FFD20(entity)` | the AI party slot, `-1` for slot 8; the key into `00F8A9E8 + p*12` |
| `+5638h` | `entity+54h` | the team id; the key into `00F8AA48 + team*12` |
| size | `5660h` | `00A38CAE` and `00A2E40D` both `PUSH 0x5660` |

`009FFD20` is already `BSP_Unit_LossCountingSlot` in the ledger and its recorded evidence matches:
`entity+180h` when 7 or below, `-1` when exactly 8, `-1` in multiplayer, and 0 or 4 in single player
by team match. The AI reuses that slot as the group's party key, and `00A2DDE0` evicts any member
whose slot no longer equals `group+5634h`.

### The four registries a group is linked into

| Base | Root | Key | Linked at |
| --- | --- | --- | --- |
| `00F8A9E8 + p*12` | `+4h` | `group+5634h`, the party | `00A2E06E` |
| `00F8AA48 + t*12` | `+4h` | `group+5638h`, the team | `00A2E0AD` |
| `00F8AA6C` | `00F8AA70` | none, every group | `00A2E0E0` |
| `00F8AA78` | `00F8AA7C`, size `00F8AA80` | emptied groups, pending delete | `00A2E3E7`, `00A2DEB6`, `00A2DD8F` |

A fifth list at `00F8AA60` (root `00F8AA64`) is the one phase 5 of the composition pass walks; what
puts a group into it was not established (`contract: unread`).

### The eviction and split passes

`00A2DDE0`, `__thiscall(group)`, body `00A2DDE0`-`00A2DEE7`: returns at once on a zero population,
then a member keeps its place only while `+5Ch` is set, `+5Dh`/`+5Eh`/`+60h` are clear,
`009FFD20(member) == group+5634h` and `member+54h == group+5638h`. Otherwise
`0077BEA0(group+563Ch)` removes it, `member+16Ch` is cleared when it still points at this group, and
`006956A0(member, group+10h)` drops the observer pair. An emptied group goes onto `00F8AA78`.

`00A2E260`, `__thiscall(group)`, body `00A2E260`-`00A2E49E`: builds a local list of every member for
which `009FE080` is true, and splits them into a **new** group only when that subset is non-empty
and strictly smaller than the population (`00A2E334` `JBE`, `00A2E342` `JNC`, both unsigned). The
first split member creates the new group with `00A2DFA0`; the rest are added with `00A2D8E0`. So a
group whose members all pass `009FE080`, or none of whose members do, is left alone.

### `00A2EEE0` is the Lua info fill, not a think

The brief named `00A2DB80` and `00A2EEE0` as "the group's THINK". `00A2EEE0` is
`AIGetGroupInfo`'s table writer, as `docs/LUA_BINDING_AI.md` already said: its 551 instructions call
`00B675D0`, `00B67580`, `00B67700`, `00B67800`, `00B666C0` and `00B673A0`, all Lua-object setters,
plus the string builders `0041E870`, `0041DD40` and `00BF7680`. Its only caller is
`00A378C0 BSP_LuaBinding_AIGetGroupInfo`. It has no call site in any think path. `coverage:
partial` -- the callee set was enumerated and the head read, the field list was not transcribed.

## 6. What a unit asks its group

`entity+16Ch` readers established by evidence in this packet and its neighbours:

| Site | Containing | What it asks |
| --- | --- | --- |
| `00A1A725` | `00A1A720` | whether the entity is grouped at all, then routes the party's aggressive ratio to the group |
| `00A2E850` | `00A2E720` phase 3 | "already grouped", the seeding gate |
| `00A2DE76`, `00A2E3A7` | eviction, split | whether the back-pointer still names this group before clearing it |
| `00A37277` | `00A37250` | the group argument of five Lua bindings |
| `0077D7A7` | `0077D600` | the group to notify through `00A2BD90` |

The displacement `16Ch` is shared by unrelated classes -- a wildcard scan for
`MOV reg,[reg+16Ch]` returns more than 110 sites across the front end, HUD and scoring -- so a
global scan cannot be attributed and this list is `coverage: partial`.

## Host table

`native` is the callee and `address` the call site; `containing` is the function whose Ghidra body
holds the site.

| Host method | address | native | containing | contract |
| --- | --- | --- | --- | --- |
| `high_level_ai_enabled` | `00A32D50` | `00E0E34C` read | `00A32D50` (no Ghidra function) | data read, not a call |
| composition pass | `00A32D59` | `00A2E720` | `00A32D50` | body read; six phases, `coverage: complete` for the phase rules |
| party pass | `00A32D5E` | `00A182C0` | `00A32D50` | body read in full |
| `release_group_reference` | `00A2E784` | `00A2B8F0` | `00A2E720` | `__thiscall(group+24h)(emptyGroup)`. contract: unread |
| `unlink_and_free_emptied_node` | `00A2E7BF` | `00BF65AC` | `00A2E720` | the CRT free; the list-size decrement sits in the run Ghidra omits after it |
| `destroy_group` | `00A2E7D9` | `vtable[0h](1)` | `00A2E720` | slot 0 of `00D23084` is `00A2D8C0`, which calls `00A2D440`, the unregister. contract: partial |
| `evict_invalid_members` | `00A2E7FD` | `00A2DDE0` | `00A2E720` | body read in full |
| `split_detached_members` | `00A2E804`, `00A18243` | `00A2E260` | `00A2E720`, `00A181A0` | body read in full |
| `create_group` | `00A2E881`, `00A2E8FC`, `00A2E967`, `00A2E9D8`, `00A2EA49`, `00A2E42A` | `00A2DFA0` | `00A2E720`, `00A2E260` | body read in full |
| `add_group_member` | `00A2E447`, `00A2DDB2` | `00A2D8E0` | `00A2E260`, `00A2DB80` | body read in full; sorted insert by `009FFD80` |
| `can_auto_merge` | `00A2EAA7` | `00A2C8D0` | `00A2E720` | body read in full; delegates to `from+564Ch` `vtable[+14h]`, which is unread |
| `merge_group` | `00A2EAC3`, `00A2EE3E` | `00A2DB80` | `00A2E720` | body read; `coverage: partial`, phase B transcribed from the call filter |
| `group_member_pass` | `00A2EE97` | `00A2C790` | `00A2E720` | head read only: walks members and calls `member->vtable[+114h]`. contract: partial |
| `auto_merge_dist` | `00A2EDCE` | `00A371A0` | `00A2E720` | already read by `docs/AI_GLOBALS_AND_TARGET_WEIGHTS.md`; `+208h` is `AutoMerge_MergeDist` |
| `game_mode` | `00A2EB03`, `00A181A9`, `009FFE62` | `004BCA50` | `00A2E720`, `00A181A0`, `009FFE50` | `__thiscall(world)`, int. contract: unread |
| `group_leader_order_key` | `00A2EB97`, `00A2EBA2` | `009FFD70` | `00A2E720` | adjustor thunk: `ECX = entity+C4h`, `JMP 009FDF30`, double in ST0. contract: unread |
| `random_think_interval` | `00A1834A`, `00A16405` | `00BD2F10` | `00A182C0`, `00A163D0` | already `BSP_Random_UniformFloatRange`; `RET 8`, `(3.0f, 5.0f)` |
| `brain_wants_immediate_think` | `00A18311` | `00A15970` | `00A182C0` | body read; four mode arms onto `brain+10h`..`+1Ch` `vtable[+30h]` |
| `party_ai_enabled` | `00A18361`, `00A2E124` | `009FFE50` | `00A182C0`, `00A2DFA0` | body read for the `world+61Ch`-set arm; the other arm `00A2FE84` onward is unread. contract: partial |
| `destroy_party_brain` | `00A183BF`, `00A16B2F` | `00A16490` | `00A182C0`, `00A16B20` | contract: unread |
| `create_party_brain` | `00A1838C` | `00A15A70` | `00A182C0` | `__thiscall(block)(party)` on a `28h` allocation. contract: unread |
| `party_brain_think` | `00A183AB` | `00A181A0` | `00A182C0` | body read in full |
| `planner_tick` | `00A181BB`, `00A181D2`, `00A181E9`, `00A18200`, `00A1828F`, `00A18299`, `00A182A3`, `00A182AD` | `vtable[+20h]` | `00A181A0` | eight sites, one per brain slot. contract: unread |
| `group_has_groupable_combatant` | `00A18253` | `00A2C5A0` | `00A181A0` | body read in full |
| `group_has_member_in_world_set` | `00A18262` | `00A2C450` | `00A181A0` | body read in full |
| `planner_claim_group` | `00A18273` | `00A22750` | `00A181A0` | body read in full |
| `party_brain_plan_tail` | `00A181C0`, `00A181D7`, `00A181EE`, `00A18205`, `00A182B3` | `00A179E0` | `00A181A0` | five tail jumps. contract: unread, body `00A179E0`-`00A18195` |
| set command | `00A2DCF5`, `00A37ADE` | `00A2BD00` | `00A2DB80`, `00A37A00` | body read in full, `RET 4` |
| notify command | `0077D7A7` | `00A2BD90` | `0077D600` | body read in full; `vtable[+24h]` is `00A0FC90`, `RET 8` |
| observer register | `00A2D906`, `00A227A3` | `00694A60` | `00A2D8E0`, `00A22750` | already `BSP_Observer_RegisterPair` |
| observer unregister | `00A2DE8D`, `00A2E3BE` | `006956A0` | `00A2DDE0`, `00A2E260` | contract: unread |
| member list erase | `00A2DE71`, `00A2E3A2`, `00A2DD4A` | `0077BEA0` | `00A2DDE0`, `00A2E260`, `00A2DB80` | contract: unread |

## Corrections to earlier docs

1. **`docs/LUA_BINDING_AI.md`, `group+10h`.** Was: "member list head", because `00A2D8E0` links the
   entity there. Is: the group's observer subobject, vtable `00D2306C`. `00A2DFA0` stores
   `00CE3CD4` then `00D2306C` at `+10h` (`00A2DFCD`, `00A2DFEB`), and `00A2D8E0` passes
   `group+10h`, not the list, to `00694A60 BSP_Observer_RegisterPair` (`00A2D8FD`: `LEA EDX,[ESI+0x10]`).
   The member list is `+563Ch`, its sentinel `+5640h`, its size `+5644h`.
2. **`docs/LUA_BINDING_AI.md`, the `00F8A8BC`/`00F8A8C8` overlap.** Was: flagged as an unresolved
   overlap for slots 3 and above. Is: the `Defend_ResourcePercent` array is three dwords, indexed by
   difficulty, and the loader guards the index at `00A360F5` (`CMP EDI,3; JGE`). Only the Lua setter
   `00A37E4B` is unguarded, so the collision is reachable from script in modes 3 through 6 and from
   nowhere else.
3. **This packet's own brief, "the group's THINK is `00A2DB80` and `00A2EEE0`".** Is: `00A2DB80` is
   the merge and `00A2EEE0` is `AIGetGroupInfo`'s Lua-table writer. Neither is on a think path. The
   think is `00A32D50` -> `00A2E720` and `00A182C0` -> `00A181A0`.
4. **This packet's own brief, "`00A2BD90` fans a command out".** Is: it forwards two arguments to
   the group's current AI command's `vtable[+24h]`, which is `00A0FC90`, `RET 8`, a no-op in every
   class the constructor installs.
5. **`docs/LUA_BINDING_AI.md`, `group+5644h` "population; zero makes both a no-op".** Refined, not
   wrong: `+5644h` is the `std::list` size field of the member list at `+563Ch`, written to 0 by the
   constructor as `[EBX+8]` with `EBX = ESI+563Ch` (`00A2E027`), and the zero test gates eight
   routines, not two.

## Open questions

- The `0x5600` bytes of `group+24h`..`+5623h` have no producer in the constructor and no consumer
  this packet found. Either a base-class constructor was inlined without stores, or a large member
  array is filled lazily somewhere unread.
- What puts a group into the list at `00F8AA60`/`00F8AA64` that the proximity merge walks.
- The AI command classes: sixteen vtables in `00D2298C`-`00D22C68`, three constructors named
  (`00A10890`, `00A109B0`, `00A10AE0`) and three installed by the group constructor. The class-id
  space (`vtable[+4h]`, values 7, 8, 9 seen) and slot `+14h`'s merge decision are unread.
- `00A179E0`, the brain's planning tail, and `00A15A70`/`00A16490`, the brain's constructor and
  destructor, so the four planners at `brain+0h`..`+0Ch` are named only by slot.
- Whether the attack ratio at record `+4h` is read anywhere; no read site exists in the image via
  an absolute-address load.

## Flow gaps

`ghidra disasm` omitted four instruction runs in `00A182C0`, all after calls. Only one carries
behaviour:

| Gap | Call site before it | Content |
| --- | --- | --- |
| `00A183CA`-`00A183D3` | `00A183C5` `00BF65AC` (free) | `ADD ESP,4` then `MOV [0xF8A89C + ESI*4],EBX` -- **the freed brain pointer is cleared**, so the destroy path leaves no dangling slot |
| `00A182F8`-`00A182FF` | - | loop alignment padding |
| `00A1832B`-`00A1832E` | - | branch alignment padding |
| `00A183E2`-`00A183E5` | - | branch alignment padding |

`00A2E720` has the same shape at `00A2E7C4`-`00A2E7CC`, after `00A2E7BF` `00BF65AC`, which must hold
the emptied-list size decrement; it was not transcribed.

## Routines with no Ghidra function

| Address | End (inclusive) | Name |
| --- | --- | --- |
| `00A32D50` | `00A32D65` | the AI coordinator's fixed-step advance |
| `00A184F0` | `00A18500` | the party's aggressive-ratio accessor |
| `00A18510` | `00A18517` | the `Defend_ResourcePercent` accessor |
| `00A18520` | `00A18533` | `1 - Defend_ResourcePercent`, the attack share |
| `00A0FC90` | `00A0FC90` | the AI command base's `vtable[+24h]`, `RET 8` |

`00A18410`, `00A18420` and `00A184D0` sit in the same unclaimed block and are trivial accessors of
the planner class; they are outside this packet.

## Correction from docs/AI_PLANNERS.md (packet cc2_ai_planners)

- **Was:** the brain is 28h bytes: +0h..+0Ch four planners, all ticked through vtable[+20h] every think; +10h..+1Ch mode-specific planners for modes 4..7
  **Is:** the brain is 28h bytes but only one of the two sets exists at a time. 00A15A70 branches on 004BCA50 and for modes 4..7 constructs exactly one mode planner, leaving +0h..+0Ch null; for any other mode it constructs the first four and leaves +10h..+1Ch null.
  **Evidence:** 00A15A70 body 00a15a70-00a15ce3, the eight arms at 00A15ADA, 00A15B31, 00A15B88, 00A15BDF, 00A15C31, 00A15C61, 00A15C92, 00A15CC3, each guarded by the CMP against 004BCA50's result at 00A15AB7
- **Was:** 00A22750 planner_claim_group ... appends the group to planner+20h
  **Is:** the std::list object is at planner+20h and the node is spliced against the root at planner+24h; planner+28h is the size. LEA EDI,[ESI+20h] at 00A22773 is the container's ECX, MOV EBX,[ESI+24h] at 00A2276C the root.
  **Evidence:** 00A22750 listing 00a2276b-00a2279b, and the base constructor 00A1EE8D-00A1EEA3 which builds the same three fields
- **Was:** then 00A1C8B0(planner)(group); when that answers false it appends the group
  **Is:** 00A1C8B0 is a list-membership test over planner+24h, not an accept or scoring test. Body read in full, 00a1c8b0-00a1c8fe.
  **Evidence:** 00A1C8B0 walks the root at param_1+24h comparing node+8h against the argument and returns 1 on a hit
- **Was:** the aggressive ratio reaches the group as a scalar argument of 00A2CBD0 (__thiscall(planner)(group, float aggressive), contract: unread)
  **Is:** 00A2CBD0 is __thiscall(AiGroup* group)(AiGroup* target, float aggressive), RET 8. ECX is the group being ordered and the first stack argument is the chosen target group, not the group.
  **Evidence:** 00A1CEED MOV ECX,EBX with EBX = [ESP+40h] set at 00A1CB89 (the group), 00A1CEEC PUSH ESI (the chosen candidate) and 00A1CEE4/00A1CEE9 pushing the float; the body then reads group+5644h, group+564Ch and group+5640h off ECX
- **Was:** What puts a group into the list at 00F8AA60/00F8AA64 that the proximity merge walks (open question)
  **Is:** 00F8AA60 is element 2 of std::list<AiGroup*> g_aiGroupsByTeam[3] at 00F8AA48 with stride 0Ch, and BSP_AiGroup_Construct appends every group to element group+5638h unconditionally. include/bsp/ai_group_think.hpp already declares the array as kAiGroupPerTeamListBase, so the doc's open question is the stale half.
  **Evidence:** CG_static_init_00CE04B0 calls 00BF7C6E(0xF8AA48, 0xC, 3, 0xA16E00); the constructor's insert is at 00A2E086-00A2E0BD keyed on param_1[0x158e] = param_2+54h
