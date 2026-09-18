# The eight world sets `00A2C450` walks, and the owned-group walk

Addresses: `00A2C450`, `00A2C489`, `00A2C491`, `008DDF90`, `008DF900`, `008DA160`, `004DF90F`,
`004DF911`, `004DF917`, `004DF919`, `004DF937`, `004DF949`, `004DF956`, `004DE20A`, `004DE234`,
`004D2C95`, `008CD440`, `008CDD60`, `008CE510`, `00A26510`, `00A265F0`, `00A1CB80`, `00A181A0`,
`00A18262`.

Packet `cc8_ai_world_sets`, read-only Ghidra analysis. Every descriptive name is a hypothesis, not a
recovered symbol. Host: `src/game_hosts_ai.cpp`. Report: `reports/ai_world_sets.json`.
Predecessor: `docs/AI_SQUADRON_SERVED.md`, which fixed the world-set answer and left the sets
themselves unread.

## 1. What the eight sets are

`00A2C450` indexes `[00E188A8] + index*4 + 21A4h` (`00A2C489 MOV ECX,[ECX+EBP*4+21A4h]`). That
array is **eight `SzurkeNyil` objects, one per player slot**, and it has exactly two producers.

### The allocator, `004DF90F`-`004DF959` in `BSP_Game_ConstructWorld`

```
004df90f  XOR EDI,EDI                 ; i = 0
004df911  LEA EBX,[ESI+21A4h]         ; &sets[0]
004df917  PUSH 30h
004df919  CALL 00BF681B               ; operator new(0x30)
004df934  PUSH EDI                    ; the loop index
004df935  MOV ECX,EAX
004df937  CALL 008DF900               ; the set constructor, __thiscall(set, int slot)
004df949  MOV [EBX],EAX               ; sets[i] = set
004df94b  CALL 008DA160
004df950  ADD EDI,1
004df953  ADD EBX,4
004df956  CMP EDI,8
004df959  JL  004df917                ; eight of them
```

`008DF900 BSP_ObjectiveSet_Construct` stores the vtable `00D1610C` at `+0h` (the class string
`"SzurkeNyil"` sits immediately before it at `00D16100`), zeroes `+4h`, `+8h`, `+0Ch` and the byte
at `+10h`, **stores the loop index at `+14h`**, and builds the container head at `+18h` through
`008DB7B0`, self-linking it and marking the nil byte at `+11h`. So each set knows its own slot.

### The other writer only nulls them

`004DE20A`-`004DE234` in `BSP_Game_ConstructActualStorage` is eight `MOV [ESI+21A4h+4k],EBX` with
`EBX` zero. That is the only other writer, and it writes nothing but null.

### Exhaustive census, with a positive control

`local/scan21a4.py` decodes every addressing form of the disp32 `21A4h`..`21C0h` in `.text` and
finds **50 sites**. The first run of that scan returned **zero**, which was a vacuous negative: the
displacement of a `MOV r/m32, r32` starts two bytes after the opcode, not six. The known site
`004DE20A` is the positive control that caught it. Corrected, the 50 break down as:

| kind | count | what |
| --- | --- | --- |
| `MOV [base+disp], reg` | 8 | the nulls at `004DE20A`..`004DE234` |
| `LEA reg, [base+disp]` | 2 | `004DF911` (the allocator above) and `004D2C95` in `BSP_Game_DestroyWorld` |
| `MOV reg, [base+disp]` | 40 | reads, every one of them |

**No code writes a set pointer except those two sites.** The array is filled once, at world
construction, and read everywhere else.

### What goes into a set, and who puts it there

The set's container holds `Objective*` records (`docs/OBJECTIVE_UNIT_LIST.md`,
`docs/MISSION_RESULT_DECISION.md`): `operator new(2Ch)`, name at `+4h`/`+8h`, kind at `+18h`, state
at `+1Ch`, and its own unit list at `+20h`..`+28h` whose elements are sixteen-byte records
`{Entity* unit, float x, float y, float z}`. `008DDF90 BSP_SzurkeNyil_ContainsUnit` is the
membership test over that structure.

The producers are the mission Lua bindings **`008CD440 Objectives_Add`** and
**`008CDD60 Objectives_AddUnit`**, with `008CE510 Objectives_RemoveUnit` the inverse. The event is a
mission script call, and the index is the **player slot**, not a side, a team or a class.

So `00A2C450(group, brain+24h)` asks: **does this group contain a unit that is an objective for
that brain's player slot.** `00A18269 JNZ` then sends such a group to the first planner
(`brain+0h`) and every other group with a groupable combatant to the fourth (`brain+0Ch`).

## 2. Why the answer in this process is `false`, and stays `false`

`008CD440 Objectives_Add` is **unimplemented here**. Every run log carries

```
host MissionLuaNative::Objectives_Add [008cd440] UNIMPLEMENTED, returning a neutral value
```

together with `Objectives_Completed [008bd340]` and `Objectives_Failed [008bd900]`. No objective is
ever created, so no objective carries a unit, so every one of the eight sets is empty, so
`008DDF90` finds nothing for any member and `00A2C450` takes its `00A2C4B4` walk-ended arm for
every group.

That is the same answer `docs/AI_SQUADRON_SERVED.md` installed, and this packet replaces its
reasoning ("this process builds no entity set at `world+21A4h`") with the accurate one: **the sets
exist and are constructed correctly; they are empty because their Lua producer is not bound.**

The host no longer hardcodes it. `GameAiCoordinatorHost::Impl::objective_sets` is the eight-slot
table, the predicate walks the group's members against the indexed set exactly as `00A2C486`/
`00A2C491` do, and a squadron answers for its flight leader because that is the unit an objective
would name. Filling the table is one assignment away once `Objectives_Add` is bound. The census
line reports how often the test was asked, how often it hit, and how many units the eight sets hold.

## 3. Only the first owned group is ordered, and that is native

`docs/AI_COMMAND_LIFETIME.md`'s follow-up `ai_planner_owned_group_walk` asked whether a planner
that owns several groups orders only the first natively. **It does.** Both mode planners, read to
their `RET`:

`00A26510 BSP_AiPlanner_SiegeThink`, body `00A26510`-`00A265E8`:

```
00a2652b  CMP [ESI+28h],0 / JNZ 00a265a5     ; owned count; zero takes the spawn-tag arm
00a265ac  MOV EDX,[ESI+24h]                  ; the owned-group list sentinel
00a265b0  MOV EDI,[EDX]                      ; the FIRST node
00a265b5  CMP EDI,EDX / JNZ 00a265c2         ; empty-list guard
00a265ca  MOV ECX,[EDI+8]                    ; that node's group
00a265d3  CALL 00A1CB80
00a265d8..00a265e8                           ; epilogue, RET
```

`00A265F0 BSP_AiPlanner_CompetitiveThink`, body `00A265F0`-`00A266B7`, is the same shape at
`00A26685`-`00A266A2`, ending at `00A266B7 RET`.

**Neither has a back edge.** There is no loop over `[ESI+24h]`: the first node's value is taken,
handed to `00A1CB80` once, and the routine returns. The host's `ai_mode_planner_tick`, which passes
`in.first_owned_group` and nothing else, already matches. `coverage: complete` for both bodies.

Two arguments differ from the host's and are recorded rather than changed: the weight factor is
`FLD1`, a literal `1.0f`, in both (`00A265C6`, `00A26694`), and the reset-target flag is
`00A265B2 SETZ CL` over `CMP [00F8A9E0],3` in Siege but the immediate `0` at `00A26699` in
Competitive. `contract: unread` for `00F8A9E0`.

## 4. USN01's gunnery census

Asked again whether USN01's zero hits are a consequence of the squadron work. **The census does not
read zero on any build this worker can run**, including the current `main` merged into this branch.
Measured directly:

| Build | shots | projectiles | entity impacts | water | hits | damage | first hit |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `f5400e43f`, before the squadron packet | - | - | - | - | 23 | 220.0 | 4.70 s |
| `7c1036d5f`, the squadron packet | - | - | - | - | 36 | 241.0 | 4.70 s |
| `2eb86d8c4`, plus the served fix | 14676 | 14676 | 45 | 42 | 45 | 563.5 | 4.70 s |
| `8f3370237`, main alone | 13699 | 13699 | 10 | 42 | 10 | 313.4 | 58.60 s |
| `7c2f34c12`, current main merged here | 13699 | 13699 | 10 | 42 | 10 | 313.4 | 58.60 s |

Read the squadron packets' own column: 23 hits become 36 and then 45, and 220.0 damage becomes
563.5. **Neither packet removed a hit from that mission.** The fall to 10 reproduces on **main
alone**, without this branch, and the current main reproduces it byte for byte.

Its shape says which of the two explanations holds. Shots fall 7%, from 14676 to 13699, while
entity impacts fall 78%, from 45 to 10, and water impacts are unchanged at 42. First hit slips from
4.70 s to 58.60 s, the mission's one kill disappears (`deaths=1 kill_credits=1` becomes
`deaths=0 kill_credits=0`) and the contact census's `dead=2840` becomes `dead=0` because nothing
dies. Guns are still firing; the shells are not landing on anything.

**Verdict: a regression in what the shells can hit, on `main`, from something other than these
packets.** It is not the artefact reading (aircraft flying attack tasks instead of the paths that
produced strafing hits), because that would cut the shot count and it does not. Locating it needs a
bisect of `main` between `f5400e43f` and `8f3370237`, which this packet did not do.

## 5. Host methods

| Site | In | Callee | Host method | this / args | ret |
| --- | --- | --- | --- | --- | --- |
| `00A18262` | `00A181A0` | `00A2C450` | `group_has_member_in_world_set` | group; brain `+24h` | bool |
| `00A2C491` | `00A2C450` | `008DDF90` | the same method's member loop | set; member | bool |
| `004DF937` | `004DE610` | `008DF900` | (not modelled: the sets are host-side) | set; loop index | void |
| `004DF919` | `004DE610` | `00BF681B` | (not modelled) | `30h` | block |
| `00A265D3` | `00A26510` | `00A1CB80` | `ai_mode_planner_tick`'s single call | planner; group, `1.0f`, flag | void |
| `00A266A2` | `00A265F0` | `00A1CB80` | the same | planner; group, `1.0f`, `0` | void |

### Substitutions

* **The eight sets' contents.** `008CD440 Objectives_Add` and `008CDD60 Objectives_AddUnit` are
  unimplemented, so `objective_sets` is empty and the predicate answers `00A2C450`'s walk-ended
  arm. The table and the walk are real; only the producer is missing, and it is named.
* **`00A1CB80`'s weight factor and reset flag** keep the host's existing values rather than the
  `1.0f` and the two different flags the listing shows. Recorded in section 3, not changed, because
  `00F8A9E0` is unread.

## 6. Corrections

Appended, not rewritten, in the documents concerned.

* `docs/AI_COMMAND_LIFETIME.md`'s follow-up row `ai_planner_owned_group_walk` is **answered**: both
  mode planners order the first owned group only, with no loop. Section 3 has the listing.
* `docs/AI_SQUADRON_SERVED.md` says of `00A2C450` that "this process builds no entity set at
  `world+21A4h`". It does build them, eight of them, at `004DF917`; they are empty because
  `Objectives_Add` is unimplemented. The answer that document installed is correct and unchanged.

## 7. Validation

`./tools/run_game.ps1`, 3200 frames, `--mission-frames 3000 --mission-frame-seconds 0.05`.
"Before" is this branch with current `main` merged (`7c2f34c12`); "after" adds this
packet's change. **Every number is identical**, which is the expected result and the
evidence that the change is behaviour-preserving: the predicate answers `00A2C450`'s
walk-ended arm either way, because every objective set is empty.

| Mission | | `served` | `attackmove` | `scored` | commands | groups | members | hits | entity impacts | world-set queries | hits | objective units |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| IJN01 | before | 2450 | 2250 | 465500 | 2515 | 3 | 221 | 17 | 17 | - | - | - |
| IJN01 | after | 2450 | 2250 | 465500 | 2515 | 3 | 221 | 17 | 17 | 1 | 0 | 0 |
| USN01 | before | 0 | 0 | 0 | 475 | 4 | 109 | 10 | 10 | - | - | - |
| USN01 | after | 0 | 0 | 0 | 475 | 4 | 109 | 10 | 10 | 1 | 0 | 0 |
| USN02 | before | 616 | 559 | 4004 | 635 | 2 | 50 | 168 | 168 | - | - | - |
| USN02 | after | 616 | 559 | 4004 | 635 | 2 | 50 | 168 | 168 | 1 | 0 | 0 |

The world-set test is asked **once** per mission, which is one unclaimed group carrying a
groupable combatant reaching `00A18262`, and answers false every time with zero units in
the eight sets. The squadron census is unchanged too: IJN01 keeps its 33 squadrons and
USN01 its 20, with `007EDA90` true for none of them.

### Attribution

Nothing moved. The one line this packet changed replaces a hardcoded `false` with the
native walk over a table that is empty for the reason section 2 names, so no group moves
between planners and no order changes. What the packet delivers is the evidence for why
that `false` is right, the producer that would make it sometimes true, and the settled
answer to the owned-group walk in section 3.


### Correction to section 4: the USN01 fall is not a regression

Appended after the lead's bisect closed it. Section 4's verdict ("a regression in what the shells
can hit") is **wrong**, and so is the follow-up row that asks for a bisect. The cause is tree
ancestry, not a defect: the 23/36/45 column was measured on branches cut **before** `fb8c0ff76`,
the plane-physics packets that first made aircraft move, and every tree showing 10 contains it.
Frozen aircraft were the targets those shells were hitting. `docs/GAME_EXECUTABLE.md`, section
"The USN01 gunnery bisect", carries the bisect table and the ancestry checks, and it records the
same mission falling from 23 hits to 1 on the morning those packets landed. The shape reading in
section 4 (shots nearly unchanged, impacts collapsing) is correct as a description and points at
exactly this: the shells still fly, and the things they used to hit have moved away.

### Correction to section 2: the producer is bound now

Appended by packet `cc8_mission_objectives`. Section 2 says the eight sets are empty "because
their Lua producer is not bound". `008CD440 Objectives_Add`, `008CDD60 Objectives_AddUnit` and
`008CE510 Objectives_RemoveUnit` **are bound now**, and the sets fill: three objective records on
IJN01, two on USN01, four on USN02, all at slot mask `0x01`.

They still hold **no units**, and `00A2C450` still answers its walk-ended arm, for a different and
sharper reason: every `Objectives_Add` call on these three missions carries `argc=6`, which is
arguments 0 through 5 and no target block, and `Objectives_AddUnit` is never called at all. That is
a property of the authored mission scripts, not of the host. `docs/MISSION_OBJECTIVES.md` has the
argument order, the census and the fifteen other readers the negative arm hides.

## 8. Follow-up packets

* `008CD440 Objectives_Add` and `008CDD60 Objectives_AddUnit`: binding them fills the eight sets and
  turns `00A2C450` into a real test, which is the only thing standing between this host and the
  native planner split.
* `008DA160`, called on each set right after construction, and `008DDF00`/`008DD310`, the set find.
* `00F8A9E0`, the global `00A265B2` compares against `3` for the reset-target flag.
* `00A179E0 BSP_AiPartyBrain_EngagementPass`, body `00A179E0`-`00A18195`, still unread.
* A bisect of `main` between `f5400e43f` and `8f3370237` for USN01's impact regression.
