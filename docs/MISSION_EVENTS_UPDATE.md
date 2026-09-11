# Mission event director at 00987590 (packet `mission_events_update`)

Addresses: 00987590, 009870a0, 0098a020, 009763e0, 00974070, 00976f10, 0096ce70, 00980150,
0097b8c0, 00982540, 0096d540, 00968550, 00977990, 00977050, 009771e0, 00977370, 00977500,
00977690, 00977da0, 009781e0, 00978a70, 00986480, 00982c50, 0097e360, 00980380, 005b71d0,
005b5df0, 005bbc10, 005bbdc0, 0096c280, 00975c40, 00976f10

## What the subsystem actually is

The object the in-mission world tick runs at `game+21E0h` is **not** an objective / win-lose
director. It identifies itself: 009763e0 prints `"WarningManager Report Error"` and 009763e0
also prints `"WarningManager unknown message id: %s"`, and 009781e0 prints
`"---WarningManager invalid Message--- %d"`. The same object owns the critical section at
`+24h` that all three routines take, so the class name recovered from those literals is
**WarningManager**.

It is the in-mission **warning / radio-chatter director**. It does three things per frame:

1. Pumps the `input` event channel so mission-script event handlers bound to an input action
   fire (00982540).
2. Runs a 4-second periodic proximity scan over the world entity lists (00977990).
3. Runs the warning queues: retire one expired warning, then select the highest-priority
   ready warning and hand it to the voice manager (00974070).

Alongside the per-frame work it owns a `std::map` of **24 named event channels** that mission
Lua scripts subscribe to, and it is what turns a game-side notification (an entity killed, a
ship landed, a building captured, a mission failure) into a named Lua callback through
`BSP_MissionLuaHost_CallNamedThreadSafe` (00887e50, `docs/MISSION_LUA_HOST.md`).

Mission completion and failure are **not** written here. The `failure`, `failureshipyard` and
`failureairfield` channels only forward a notification that something already decided; the
routines that decide it are the callers listed under "Channel producers" below, in segments 61
and 62, and they are outside this packet. Nothing in 00987590's reachable set writes a mission
result or enqueues an end-of-mission request on the `docs/GAME_FRAME_CONTROL.md` deque.

The existing ledger name `BSP_MissionEvents_Update` is superseded by
`BSP_WarningManager_Update` for this reason; the old name is preserved in the ledger evidence.

## 00987590 - the per-frame director

`__thiscall(this, float scaledDelta)`, RET 4 (00987748). ECX is `game+21E0h`, from
`BSP_Game_OnMove` at 004e4e67 behind state 0Dh, `game+634h == 0`, a non-null `game+21E0h` and
`scaledDelta > 0.0f` (00d7a218). 136 listed instructions, no flow gaps.

Order, taken from the listing:

| Address | Call | ECX | Argument |
| --- | --- | --- | --- |
| 009875a2 | 00982540 | `this` | none |
| 009875d9 | 00977990 | `this` | none, only when the accumulator crossed |
| 009875e0 | 0096d540 | `this` | none |
| 009875e7 | 00968550 | `this` | none |
| 0098769a | 005b71d0 | `[[00e198c4]+A4h]` | `warning+14h` |
| 009876b6 / 009876c3 | virtual `+18h` | the warning | none |
| 0098766d | virtual `+1Ch` | the warning | none |
| 009876f3 | virtual `+0h` | the warning | `1` |
| 0098773d | 00974070 | `this` | the selected warning |

The accumulator at `+198h` is x87: `FLD [EBP+8]; FADD [EDI+198h]; FST [EDI+198h]`, then
`FLD [00ce3d34]; FXCH; FCOMIP` and `JBE`. 00ce3d34 is `00 00 80 40` = **4.0f**. The branch is
taken when `4.0f <= accumulator`, so the periodic pass runs on **strictly greater than 4.0f**,
and 009875d1 then stores `XORPS XMM0,XMM0` into `+198h`: the accumulator is **reset to zero,
not decremented**, so the period is a floor and the overshoot is discarded.

### Correction to `docs/GAME_WORLD_ENTITIES.md`

That doc reads `event+10h` as the duration and the virtual at `+1Ch` as the start time. The
listing shows the opposite roles. 00974070 writes the clock `00f876a4` into `warning+10h`
(009740f9..00974108) when a warning is applied, so `+10h` is the **apply timestamp** and the
virtual at `+1Ch` returns the **pending lifetime**. The comparison at 00987677 is
`warning+10h <= now - lifetime`, which is numerically the same test, so the reconstruction in
`src/world_entities.cpp` still computes the right answer; only the two field names there are
swapped. `include/bsp/world_entities.hpp:260` is owned by another packet and was left alone.

### The walk

`+E0h` is an MSVC `_SECURE_SCL` `std::list` (`_Myfirstiter +E0h`, `_Myhead +E4h`,
`_Mysize +E8h`) whose node value at `node+8h` is a `Warning*`. The body returns immediately
when `+E8h` is zero.

For each node, with `now = 00f876a4`:

- `warning+10h <= now - warning->vtable[7]()` retires it: `warning->vtable[0](1)` (the scalar
  deleting destructor), `node->_Prev->_Next = node->_Next`, `node->_Next->_Prev = node->_Prev`,
  `_free(node)`, `--*(+E8h)`, and **`RET` immediately** (0098772b jumps back to the loop head
  but `ESI` has been freed; the pseudocode's `return` at 009876fc..00987727 is the real exit).
  At most one warning is retired per frame, and a candidate selected earlier in the walk is
  discarded unapplied.
- Otherwise `005b71d0` decides readiness; when ready, the warning replaces the held candidate
  if none is held or if `held->vtable[6]() < candidate->vtable[6]()` (009876c9 `FCOMIP` then
  `JBE` back to the loop head, so **strictly greater** priority wins and ties keep the earlier
  entry).

On reaching the head sentinel, `00974070(candidate)` runs when a candidate was held.

## The WarningManager object

`operator_new(0x1B0)` at 004dc8b3, constructor 0098a020 at 004dc8cf, `game+21E0h = EAX` at
004dc8df, then `009870a0` with ECX = the object; all inside `BSP_Game_ConstructGlobalSubsystems`
(004dc6a0). Size **1B0h**. Vtable `00d1b968`, whose slot 0 is the scalar deleting destructor
0098a2a0. 004d2bb0 `BSP_Game_DestroyWorld` clears the field.

| Offset | Meaning | Evidence |
| --- | --- | --- |
| `+0h` | vtable `00d1b968` | 0098a020 |
| `+4h`..`+20h` | eight dwords zeroed by Init | tail of 009870a0 |
| `+24h` | `CRITICAL_SECTION*`, `BSP_CriticalSection_Create` | 0098a020, taken by every report path |
| `+2Ch` | container built by 0096b260 | 0098a020 |
| `+38h`..`+D4h` | 13 native strings, `_eh_vector_constructor_iterator_(this+38h, 8, 13h, ...)` | 0098a020 |
| `+D0h` | byte, warnings-suppressed flag, cleared by Init | 009870a0, tested at 00977050 |
| `+D8h` | list head from 0096b960, sentinel flag at `+15h` | 0098a020 |
| `+E0h` | `std::list<Warning*>` pending queue | 00987590, 009763e0 |
| `+ECh` | `std::list<Warning*>` applied queue | 00974070, 009763e0 |
| `+F8h` | `std::map<NativeString, std::list<Subscription>>` event channels, case-insensitive | 00982540 (`LEA ECX,[EDI+F8h]`), 00980150 |
| `+104h`, `+105h` | two bytes cleared by each channel dispatch | 00986480 |
| `+108h` | table built by 0097f570 from the Lua `messages` table | 009870a0 |
| `+10Ch` | list head from 008e5cf0, sentinel flag at `+25h` | 0098a020 |
| `+11Ch` | table built by 00979990 from the Lua `entity` table | 009870a0 |
| `+130h` | table built by 00979990 from `playerunit_section` | 009870a0 |
| `+148h`, `+154h` | list heads from 005826b0 and 00443e20 | 0098a020 |
| `+160h`, `+16Ch` | list heads from 0096baf0 and 0096bb80 | 0098a020 |
| `+174h` / `+178h` | float deadline / active flag, `airRaidSoundExpires` | 00968550, 00980380 |
| `+17Ch` / `+180h` | float deadline / active flag, `collisionSoundDisableTime` | 00968550, 00980380 |
| `+188h` | list head from 0096bbd0 | 0098a020 |
| `+190h`, `+194h` | two refcounted effect handles from `BSP_EffectHandle_AcquireByName` | 009870a0 |
| `+198h` | float periodic accumulator, zeroed by Init | 00987590, 009870a0 |
| `+19Ch` | native string, pending Lua callback for input action 3 | 0096d540 |
| `+1A8h` | list head from 005a0220 | 0098a020 |

`009870a0` also stores the object into the singleton `00f8a0c4` (39 read sites across the
image) and installs two hook function pointers, `[00f8bbcc+230h] = 00987080` and
`00f8bf4c = 00987090`.

### Init (009870a0), `__fastcall(this)`

Constructs its own `LuaStateOwner`, opens it, runs `Scripts/datatables/Warnings.lua`, then
reads from the globals table: `Warnings`, and inside it `escapecharacters`, `entity`,
`playerunit_section` and `messages`. `messages` becomes the table at `+108h` that 009763e0
checks a warning's message id against. Two effect handles are acquired by name (the two names
are 11h and 12h bytes long; their literals were not read in this packet). The decompiler drops
six unreachable blocks in this body, all SEH unwind funclet tails.

## The Warning record

Concrete warnings are `operator_new(0x84)` (00977050 and its siblings), so the base plus the
common subclass is **84h** bytes. The fields the director and the report path touch:

| Offset | Meaning | Evidence |
| --- | --- | --- |
| `+0h` | vtable | everywhere |
| `+4h` | message id size | 009763e0 `param_2[1] == 0` rejects an empty id |
| `+8h` | message id `char*` | `"WarningManager unknown message id: %s"` prints `warning+8h`, falling back to `00f8a0c8` |
| `+Ch` | state, `1` queued, `2` applied | 00974070 writes 2; 0096ce70 unlinks only for 1 or 2 |
| `+10h` | float apply timestamp | 00974070 stores `00f876a4` |
| `+14h` | `std::vector<Clip12>` of sound clips (`_Myproxy +14h`, `_Myfirst +18h`, `_Mylast +1Ch`, `_Myend +20h`) | element size 0Ch from the `2AAAAAABh` / `SAR EDX,1` divide at 0097408D and 005B721C |
| `+38h` | dword passed as 005bbc10's second stack argument | 009740d5 |
| `+50h` | speaking entity for the positional path; zero selects the flat path | 00974078, 009763e0 `piVar2[14h]` |
| `+68h` | speaking entity for the flat path | 009740d2 |
| `+80h` | id compared in 00976f10's cancel-by-id walk | 00976f10 `piVar2[20h]` |

Virtual slots established by use:

| Slot | Offset | Signature | Use |
| --- | --- | --- | --- |
| 0 | `+0h` | `void(int flags)` | scalar deleting destructor, always called with 1 |
| 3 | `+Ch` | `void()` | called on accept, before the queue insert (009763e0) |
| 4 | `+10h` | `int()` | kind id; 00976f10 matches kind 4 |
| 5 | `+14h` | `bool(Warning* other)` | duplicate test used by both report walks |
| 6 | `+18h` | `float()` | priority |
| 7 | `+1Ch` | `float()` | pending lifetime, used on the `+E0h` queue |
| 8 | `+20h` | `float()` | applied lifetime, used on the `+ECh` queue |

Slots 1, 2 and 9 upward were not observed.

## 009763e0 - report, `__thiscall(this, Warning*)`

Takes `+24h`, then:

1. `warning+4h == 0` prints `"WarningManager Report Error"` and returns, leaking nothing
   because the caller owns the object until the queue takes it.
2. Walks `+E0h`. An entry whose `now - entry->vtable[7]() >= entry+10h` is destroyed, unlinked
   and freed, and the routine **returns** - the same one-per-call rule as the director. An
   entry whose `vtable[5](incoming)` is true refreshes its own `+10h` to `now`, destroys the
   **incoming** warning and returns: duplicate suppression keeps the older entry alive.
3. Walks `+ECh` the same way, using virtual `+20h` for the lifetime instead of `+1Ch`.
4. On reaching the end of both, looks the message id up in the `+108h` table
   (`STL_inst_0096dbb0(warning+4h)`). A miss prints
   `"WarningManager unknown message id: %s"` and destroys the warning.
5. On a hit: `warning->vtable[3]()`, insert into the queue through 009730c0, and when
   `warning+50h` is non-null and `005b71d0` says ready, apply it at once through 00974070
   rather than waiting for the next frame.

`ghidra flow` reports two gaps: 6 bytes after a `JMP` at 00976469 (not after a call, left
alone) and **12 bytes after `CALL 00bf65ac` (`_free`) at 00976530**. The gap is not repaired
here; the orchestrator should decide.

## 00974070 - apply, `__thiscall(this, Warning*)`, RET 4

ECX is the manager, the stack argument the warning.

- `warning+50h != 0`: bounds-checks `warning+14h`, builds a 12-byte value
  `{00cf0dd0, first[1], first[2]}` on the stack from the vector's first element and calls
  `005bbdc0(ECX = [[00e198c4]+A4h], thatValue, warning+50h)`. 005bbdc0 refreshes the camera
  transform (`game+19FCh`), takes `cameraX - entityX` through 0042b2f0 to a distance, forms
  `(2000.0 - distance) / 2000.0` with 00cf0dd8 = **2000.0** and plays only when the result is
  at least 00ce3868 = **0.25f** and above zero, i.e. inside **1500 m**. It then calls 005bbc10
  with a null speaker.
- `warning+50h == 0`: `005bbc10(ECX = [[00e198c4]+A4h], warning+14h, warning+38h, warning+68h)`.
  005bbc10 classifies the speaker into 0..4 from `game+18CCh + 4*game+18ECh` (`+28h`, the local
  player's side) against `speaker+54h`, with `2` accepted unconditionally and four virtual
  `+5Ch` probes selecting 1, 2 or 3, and 4 when the side does not match. The index picks a slot
  from `manager+A0h`.
- Both paths then set `warning+Ch = 2`, `warning+10h = 00f876a4`, remove the warning from the
  `+E0h` list by value through 0096c280, and push it onto the `+ECh` list through 0096b510.

## 005b71d0 - readiness, `__thiscall(this, const vector<Clip12>*)`, RET 4

ECX is `[[00e198c4]+A4h]`, the voice manager; the stack argument is `warning+14h`. Returns
false unless all of:

- `[[00e188a8]+21E4h]+34h == 0` **and** `+24h == 0`. That object is the panel / cutscene
  sequence manager (constructor 00452660, segment 4 keywords `suppressinterruptmsg`,
  `panelstates`, documented in `docs/MISSION_STATE_ENTRY.md`), so radio lines are suppressed
  while a panel sequence is running.
- `voice+74h == 0`.
- For each clip in the vector, at least one of the **two** playback slots at `voice+8h` and
  `voice+20h` (stride 18h) reports free through 007027b0. The slot set does not change inside
  the loop, so the outer iteration is redundant in effect.
- `voice+6Ch == 0`, which is the value actually returned (`SETZ AL` at 005b727e).

Clip count comes from 005b5df0, `(_Mylast - _Myfirst) / 0Ch`.

## 0096d540 and 00968550 - the two polls

`0096d540(this)`: when `this+19Ch` holds a non-empty callback name and input action **3** is
pressed this frame (`BSP_InputAction_WasPressedThisFrame`, 004c43c0), it raises the HUD flag
`[game+1ED4h]->vtable[5Ch](0x54)` then `[game+1ED4h]+390h = 1`, calls
`BSP_MissionLuaHost_CallNamedThreadSafe(0, this+19Ch, 0, 0, 0FFFFFFFFh)` and clears the name
with the empty literal at 00ce3a0c. This is the "press the action to answer the prompt" path.

`00968550(this)`: two deadlines against `00f876a4`. `+174h`/`+178h` is `airRaidSoundExpires`
and only clears its flag. `+17Ch`/`+180h` is `collisionSoundDisableTime` and additionally calls
`[[[00e198c4]+C4h]+60h]->vtable[34h](0)`. Both use `<`, so the flag clears on the first frame
strictly past the deadline.

## 00977990 - the 4-second proximity scan

No `this`. Walks the world list at `[game+19CCh]+64h`; for each entity whose `+8h` payload has
`+5Ch` set and `+5Dh`, `+5Eh`, `+60h` clear and a non-empty array at `+348h`/`+34Ch` with more
than one element, it walks the second world list at `[game+19CCh]+13Ch` looking for an entity
of a different side (`+54h`) that passes 00803ce0 == 1 and whose transform is within
`00d09fe8` = **4.0e6**, i.e. **2000 m**, of it. 00975d00 returns the per-entity record; a hit
binds the other entity into `record+8h` and sets `record+4h = now + 2.0` (00d7a308 is the
double `2.0`), a miss past the deadline releases the reference and sets `other+9h = 1`. The
same 2.0-second constant is the cancel cooldown in 00976f10.

## The named event channels

`00980150` is `std::map<NativeString, Channel>::operator[]` with a case-insensitive compare
(`BSP_NativeString_LessCaseInsensitive`, 00443d00) that returns `node+14h`. The map node holds
the key at `+0Ch` and the value at `+14h`; the value is itself a `std::list` base, so
`channel+4h` is `_Myhead` and `channel+8h` is `_Mysize`.

Each channel node carries a subscription object pointer at `node+14h`. `0097b8c0(channel,
params)` walks the list, calls `subscription->vtable[3](params)` and, on true, appends the
subscription's callback name at `subscription+4h` to an output vector. `00982540` uses the same
subscription objects with a nested list at `subscription+10h` whose entries hold an input action
id at `+0Ch`, matched against the 30h-byte input records at `inputSingleton+4h` with the
`docs/GAME_FRAME_CONTROL.md` edge test (`record+28h` set, `record+20h` clear).

`0097e360` is the parser that turns a Lua event block into a subscription; its 26 callees are
the per-kind condition factories. It names the kinds it accepts:

`ammoType`, `command`, `entityKilled`, `exitzone`, `failure`, `generate`, `hpEvent`, `input`,
`musicOver`, `player`, `recon`, `repair`, `shipLanded`, `stock`, `surrender`, `target`.

### Channel producers

Each dispatcher takes `+24h`, looks its channel up, boxes its parameters, evaluates the
subscriptions and calls `BSP_MissionLuaHost_CallNamedThreadSafe`. 00986480 is the worked
example: it boxes a 2-byte field from `entity+174h` behind the vtable `00d1af24` and a copied
string, evaluates, frees the boxes and calls the host.

| Channel | Dispatcher | Native producers |
| --- | --- | --- |
| `recon` | 00980e50 | none in the image |
| `kill`, `globals.warn_uslost` | 009813a0 | 007f3b10, 00959450 |
| `exitzone` | 00982120 | 007f31a0 |
| `input` | 00982540 | the director itself, every frame |
| `surrender` | 00982990 | 004d87b0 `BSP_Game_CheckMultiplayerPlayerCount` |
| `failure`, `failureshipyard`, `failureairfield` | 00982c50 | 0093bed0, 0093c300, 0095abe0 |
| `leak` | 009832f0 | 0093a4f0 |
| `fire` | 00983780 | 0093a470 |
| `repair` | 00983c10 | 0064a270, 0064a2f0 |
| `zone` | 00983f50 | none in the image |
| `command` | 00984300 | 0071f600, 00836920, 0084e010 |
| `target` | 00984800 | 0071f600 |
| `ammoType` | 00984ba0 | none in the image |
| `stock` | 00984eb0 | 006c7b10, 006c7d10, 006ca770, 00846d90 |
| `gui` | 00985250 | none in the image |
| `generate` | 00985590 | 0077fad0 |
| `player` | 00985920 | 004d0fc0, 00770750 |
| `musicOver` | 00985c50 | none in the image |
| `chat` | 00985f30 | 005d0050 |
| `entityKilled` | 00986480 | 0077ce60 |
| `shipLanded` | 00986820 | 0074ad90 |
| `damage`, `repair`, `hpEvent` | 00986b00 | 00879070 |
| `hit` | 00988510 | 0077ce60 |

Four channels have no native producer, so they are fired from script or from a path this
packet did not reach.

## Message ids produced natively

The thirteen segment-64 report helpers name the warnings they raise. Each follows 00977050's
shape: take `+24h`, test the guards, `operator_new(0x84)`, `BSP_NativeString_Assign(<id>)`,
build the warning through 00974150 and call 009763e0.

| Routine | Message ids |
| --- | --- |
| 00977050 | `submarineairlow` |
| 009771e0 | `submarineaircritical` |
| 00977370 | `submarinedepthdamage` |
| 00977500 | `submarineperiscopebroken` |
| 00977690 | `torpedo` |
| 00977da0 | `base`, `own`, `nmy`, `supply`, `captureship`, `captureshiprange` |
| 009781e0 | `cap`, `base`, `own`, `player`, `nmy`, `supply`, `neut`, `last`, `oneleft`, `neutralize` |
| 00978a70 | `commandbuildinglast`, `commandbuildingplayer`, `navalsupply`, `hitweareunderattack` |

00977050's guard set, taken as the template: `this+D0h == 0`, `0 <= game+18ECh < 8`,
`00f876a4 > 4.0f` (the 4.0f at 00ce3d34 doubles as a mission-start grace period),
`entity+5Ch` set, `entity+5Dh`, `entity+5Eh`, `entity+60h` clear, `0077edf0(entity)` false and
`00965810()` true.

## Constants

| Address | Bytes | Value | Use |
| --- | --- | --- | --- |
| 00ce3d34 | `00 00 80 40` | 4.0f | periodic period and the report grace |
| 00d7a218 | `00 00 00 00` | 0.0f | the caller's delta gate |
| 00d7a308 | `00 00 00 00 00 00 00 40` | 2.0 (double) | proximity hold and cancel cooldown |
| 00d09fe8 | `00 00 00 00 80 84 4E 41` | 4.0e6 (double) | squared proximity radius, 2000 m |
| 00cf0dd8 | `00 00 00 00 00 40 9F 40` | 2000.0 (double) | voice attenuation range |
| 00ce3868 | `00 00 80 3E` | 0.25f | minimum attenuation to play |
| 00ce3a0c | `00 00 00 00` | `""` | clears the pending prompt name |
| 00f876a4 | runtime | mission clock | every deadline in this packet |

## Reconstruction

`include/bsp/mission_events.hpp` and `src/mission_events.cpp` add `bsp::WarningManagerState`,
`bsp::WarningRecord`, `bsp::WarningSubscription` and the pure rules this packet established:
the expiry and priority comparisons, the periodic accumulator, the two deadline flags, the
input prompt, the voice readiness gate, the voice attenuation, the report path with its
duplicate suppression and unknown-id rejection, and the channel dispatch. The per-frame walk of
the `+E0h` queue is **not** duplicated: `bsp::update_mission_events_00987590` in
`include/bsp/world_entities.hpp` already owns it, and `bsp::kMissionEventTickPeriod` is reused
from that header rather than redeclared.

`bsp::run_warning_manager_update` sequences the four per-frame calls over a
`bsp::WarningManagerHost` with one method per native call site, in the style of
`bsp::run_application_frame`.

Nothing here is a drop-in binary replacement. The native routines are `__thiscall` on objects
whose full 1B0h and 84h layouts are not recovered, and the reconstruction takes projections of
the fields this packet establishes.

## Uncertainties

- The subscription object's own class and full layout are not recovered. `+4h` as the callback
  name and `+10h` as the nested per-action list come from two use sites only.
- The channel `std::list` node value size is not pinned. The head sentinel flag byte sits at
  `+19h` (00980150), which places the value at 11h bytes or more; `node+14h` is inside it.
- `005bbc10`'s speaker classification indexes `manager+A0h` by 0..4. What the five slots are was
  not read.
- The two effect names acquired in Init (11h and 12h bytes) were not read.
- 0098a020 is tagged `cg_array_ctor_helper`; it is a real constructor, not a compiler helper,
  and the tag is stale.
- 00977990's `00975d00` record type and 00867b10's role are inferred from the surrounding
  reference counting, not read.

## Flow gaps found, not repaired

| Function | Gap |
| --- | --- |
| 009763e0 | 12 bytes at 00976535..00976541 after `CALL 00bf65ac` at 00976530 |
| 00976f10 | 12 bytes at 00976fb2..00976fbe after `CALL 00bf65ac` at 00976fad |
| 00982540 | 9 bytes at 009828d5 after `CALL 00bf6989`; 7 bytes at 009828f8, 7 at 0098291b and 3 at 00982974, each after `CALL 00bf65ac` |

00987590 and 00974070 list cleanly.

## Follow-up packets

| Packet | Addresses | Files | Contract |
| --- | --- | --- | --- |
| `warning_message_table` | 009870a0, 0097f570, 00979990, 0096dbb0 | docs/WARNING_MESSAGE_TABLE.md, include/bsp/warning_messages.hpp | The `Scripts/datatables/Warnings.lua` load: the `messages`, `entity` and `playerunit_section` tables at `+108h`, `+11Ch` and `+130h`, their record layout and the id lookup 009763e0 performs |
| `warning_event_subscriptions` | 0097e360, 00980380, 00980c10, 0097b8c0, 0097f0f0 | docs/WARNING_EVENT_SUBSCRIPTIONS.md, include/bsp/warning_subscriptions.hpp | The 26 condition factories behind the 16 parsed event kinds, the subscription layout and how a mission script registers one |
| `voice_line_playback` | 005bbc10, 005bbdc0, 005b71d0, 005babb0, 007027b0, 005b9760 | docs/VOICE_LINE_PLAYBACK.md, include/bsp/voice_playback.hpp | The voice manager at `[00e198c4]+A4h`: the two playback slots, the clip vector element, the speaker classification table at `+A0h` and the attenuation curve |
| `warning_producers` | 00977050, 009771e0, 00977370, 00977500, 00977690, 00977da0, 009781e0, 00978a70, 00974150 | docs/WARNING_PRODUCERS.md, include/bsp/warning_producers.hpp | The thirteen native report helpers, their guards and the Warning constructor 00974150 with its subclass vtables |
| `mission_failure_decision` | 0093bed0, 0093c300, 0095abe0, 007f3b10, 00959450 | docs/MISSION_FAILURE_DECISION.md | What actually decides mission failure and defeat before it reaches the `failure` and `kill` channels, and whether it writes a mission result or enqueues an end-of-mission request |

## State per address

| Address | Name recorded | State |
| --- | --- | --- |
| 00987590 | `BSP_WarningManager_Update` | exported, analyzed, reconstructed, build-tested |
| 009870a0 | `BSP_WarningManager_Init` | exported, analyzed |
| 0098a020 | `BSP_WarningManager_Construct` | exported, analyzed |
| 009763e0 | `BSP_WarningManager_Report` | exported, analyzed, reconstructed, build-tested |
| 00974070 | `BSP_WarningManager_ApplyWarning` | exported, analyzed, reconstructed, build-tested |
| 00976f10 | `BSP_WarningManager_CancelByTarget` | exported, analyzed |
| 0096ce70 | `BSP_WarningManager_Withdraw` | exported, analyzed |
| 00980150 | `BSP_WarningManager_ChannelIndex` | exported, analyzed |
| 0097b8c0 | `BSP_WarningChannel_Evaluate` | exported, analyzed, reconstructed, build-tested |
| 00982540 | `BSP_WarningManager_PumpInputChannel` | exported, analyzed |
| 0096d540 | `BSP_WarningManager_PollPrompt` | exported, analyzed, reconstructed, build-tested |
| 00968550 | `BSP_WarningManager_UpdateDeadlines` | exported, analyzed, reconstructed, build-tested |
| 00977990 | `BSP_WarningManager_ScanProximity` | exported, analyzed |
| 005b71d0 | `BSP_VoiceManager_CanPlay` | exported, analyzed, reconstructed, build-tested |
| 005b5df0 | not renamed, library-shaped | exported, analyzed |
| 005bbc10 | `BSP_VoiceManager_PlayLine` | exported, analyzed |
| 005bbdc0 | `BSP_VoiceManager_PlayPositionalLine` | exported, analyzed, reconstructed, build-tested |
| 0096c280 | `BSP_WarningList_RemoveByValue` | exported, analyzed |
| 00986480 | `BSP_WarningManager_FireEntityKilled` | exported, analyzed |
| 00982c50 | `BSP_WarningManager_FireFailure` | exported, analyzed |
| 0097e360 | `BSP_WarningManager_ParseEventBlock` | exported, analyzed |
| 00980380 | `BSP_WarningManager_LoadEventTable` | exported, analyzed |
| 00977050 | `BSP_WarningManager_ReportSubmarineAirLow` | exported, analyzed |
| 009771e0 | `BSP_WarningManager_ReportSubmarineAirCritical` | exported, analyzed |
| 00977370 | `BSP_WarningManager_ReportSubmarineDepthDamage` | exported, analyzed |
| 00977500 | `BSP_WarningManager_ReportSubmarinePeriscopeBroken` | exported, analyzed |
| 00977690 | `BSP_WarningManager_ReportTorpedo` | exported, analyzed |
| 00977da0 | `BSP_WarningManager_ReportCaptureShip` | exported, analyzed |
| 009781e0 | `BSP_WarningManager_ReportCapturePoint` | exported, analyzed |
| 00978a70 | `BSP_WarningManager_ReportCommandBuilding` | exported, analyzed |

## Correction from docs/MISSION_RESULT_DECISION.md

The five addresses proposed above as the mission failure decision are not one: two are a component-failure hazard roll, one a unit message handler, and two are unit destruction reporting the kill warning. Nothing in the image decides a win or loss natively; the mission script writes objectives (0x2C records in eight per-slot sets at `game+21A4h`, kind at +18h, state at +1Ch) and calls `Scoring_SetMissionCompleted`, and `BSP_Game_CheckMissionCompletion` enqueues request 0Fh once the `PlayBinkMovie` object at `game+7188h` has +21h set; 0Fh runs `GGame::EndScene` (`004d7970`), which commits the score record into `game+6B4h` keyed by the mission id at `game+2198h` and enqueues the teardown request 10h. A player death raises interface 34h (`GUI_limbo`, the respawn screen) after a 1.0f grace period.

## Correction from docs/WARNING_MESSAGE_TABLE.md and docs/VOICE_LINE_PLAYBACK.md

The warning table packet establishes that entity and playerunit_section are
children of Warnings.escapecharacters; messages is directly under Warnings.
The +108h map holds vectors of alternative strings. The +11Ch/+130h escape
sections each hold a prefix string and a replacement map; +114h accumulates
both sections' prefixes. The installed-data fixture found 122 message IDs,
36 entity replacements and 5 section replacements. Full Init is still an
external sequence around the reconstructed table-loading fragment.

The voice loop uses CMP ESI,1/JL at 005B7251/54: only the slot at manager+8 is
polled. 007027B0 can stop/release/reset that slot, and readiness repeats it for
each clip before reading manager+6Ch. The old two-slot static readiness helper
and standalone attenuation helpers have been retired in favor of the complete
sequences in voice_playback.hpp/.cpp.

Speaker classification performs five virtual probes (8, 1Ah, 1Bh, 19h, 6).
005BBDC0 accepts a Clip12 by value plus an entity pointer. The clip holds a
record pointer whose +8 signed sound ID is tested, and an opaque word copied
unchanged. Positional attenuation is an admission gate only: it is not passed
as volume. The native JA/JNC pair also permits unordered values with masked
FP exceptions. The double-distance host interface does not promise bit-exact
x87 intermediates. No audible playback or original-game validation is claimed.

The older counts, static-readiness assertion, four-probe description and
broader source-level implications above are superseded by these findings.
Combined verification is recorded in reports/warning_voice_order_integration.json.
