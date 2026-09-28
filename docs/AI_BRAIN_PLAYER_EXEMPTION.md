# Does the AI party brain leave the player's ship alone? (packet `cc9_ai_brain_player_exemption`)

Worker cc9-ships2, on main `861f62c49`. Ghidra was read-only, and every body named here has a
Ghidra function. Names are hypotheses.

## Answer

**No. The image has no player exemption on the brain's order path.**
- **Houston is a member of the party-0 AI groups.** USN02's scene authors no `OwnerPlayer`, so
  every unit keeps `unit+180h = 9`, and in single player `009FFD20` gives a team-0 unit slot 0.
- **The close-attack pass `00A13B60` serves it.** Its member gates test only class traits.
- **The command lands through `0077D600`** with the same flags a HUD order uses, replacing the
  player's.
- **No role test, no selected-unit read and no "recent player order" test** exists on that path.

The host re-tasking Houston 12 s after the `--helm-orders` moveto is therefore the image's rule,
not a host gap. **No switch is bound.**

What does protect a player's ship in the image is the **helm, not the brain**.
- A player who takes role 1 sets `unit+184h`.
- `009F3DF3` then forces that unit's ship AI into `cruise` whatever its director holds
  (docs/SCRIPTED_HELM.md sections 1 and 6, docs/CONTROLLED_UNIT_HELM.md).
- So an AI `attackmove` lands on the director but cannot steer a ship whose helm the player
  holds.
- A HUD `moveto`, which is not a helm, is exactly as replaceable as the harness order was.

## 1. Which units the brain may task

**Group membership.** The compose pass `00A2E720` seeds groups at phase 3 (`00A2E835`-`00A2EA5A`)
from the world collections. The seed gate is:
- `+5Ch` set;
- `+5Dh`, `+5Eh` and `+60h` clear;
- `+16Ch` null;
- `+54h < 2`.

Eviction, `00A2DDE0` (body `00A2DDE0`-`00A2DEE7`), keeps a member only while
`009FFD20(member) == group+5634h` and `member+54h == group+5638h` (docs/AI_GROUP_THINK.md). The
party key is `009FFD20`, listed in full:

```
009FFD20  MOV EAX,[ECX+180h]        ; the owner-policy slot
009FFD26  CMP EAX,8 / JE 009FFD59   ; 8 -> -1: no party, no brain
009FFD2B  CMP EAX,7 / JBE 009FFD5C  ; 0..7 -> that slot
009FFD30  MOV EAX,[00E188A8]
009FFD35  CMP [EAX+1FE4h],0 / JNE 009FFD59   ; multiplayer -> -1
009FFD3E  EDX = [game+18CCh + 4*[game+18ECh]] ; the local player's record
009FFD4B  MOV EAX,[ECX+54h] / SUB EAX,[EDX+28h] / NEG / SBB / AND EAX,4
009FFD58  RET                        ; single player: 0 on the local player's team, else 4
```

**Who writes `unit+180h`.** It is written only through `vtable[144h]` (`0077F2D0` into `009278A0`,
docs/SHIP_AI_ROLE_OWNERS.md). A census of `MOV r32,[r32+144h]` followed by `CALL r32`
(`local\cc9-ships2-vcall.py`) finds nine sites. The control, the same scan for `148h`, finds the
SetRoleAvailable binding's `008ABA47` and `008ABA69` among 19. Of the nine:
- `0077F1F9` in `0077F0E0` activation, fed by the `OwnerPlayer` scene property (string
  `00CF882C`), or **9 when it is absent** (`0077F1F1 MOV EAX,9`);
- `0095AC28` in `0095ABE0` Unit_HandleMessage, a message arm that takes the record's `+28h`;
- `0077FFC9`, `007800A3` and `00780193`, the session dispatchers;
- `006F50D2` and `006F5148` in `006F4D10`;
- two sites outside game logic, `00A9D106` (GUI) and `00B21BA9` (renderer).

`SetRoleAvailable` (`008AB850`) writes the `+188h` table through `vtable[148h]` and `00927D20`,
not `+180h`.

**`universe/scenes/missions/usn/usn_2_java.scn`** (this installation, mtime 2024-07-13 08:27)
contains no `OwnerPlayer` bytes. Every USN02 unit therefore has `+180h = 9`. Houston, on the
local team, has slot 0. It is seeded into a party-0 group and never evicted.

**The close-attack member gates, `00A143A0`-`00A14444`** (docs/AI_CLOSE_ATTACK_TICK.md):

```
00A143ED  PUSH 18h / CALL [vt+5Ch]        ; PlaneSquadronGen?
00A14402  PUSH 17h ... 00A1440C CMP byte [EDI+0C24h],0   ; the carrier link (007EDA90's shape)
00A14427  PUSH 6 / CALL [vt+5Ch]           ; a ship?
00A14435  MOV ECX,[ESI+538h]               ; the vehicle CLASS descriptor
00A1443D  MOV EAX,[EDX+2Ch] / CALL EAX     ; a class trait
00A14444  JNE 00A14D4D                     ; true -> skip
```

`unit+538h` is the class descriptor (docs/ATTACK_CAPABILITY_INPUTS.md, docs/ATTACK_GATE_TAILS.md),
so the only non-squadron gate is a class property. The order is issued at `00A14A4A`-`00A14A6E`
with `PUSH 1`, the same flags as the HUD moveto `005F9B56`, into `0077D600`.

**Negatives, each with its positive control:**
- **The selected unit.** `D8 88 E1 00` (`[00E188D8]`) appears nowhere in `00A00000`-`00A38000`.
  The same scan over `005F9000`-`005FD000` finds `005F9597` and more.
- **The role queries.** Of the rel32 calls to `00927F10` and `00927F30`, none lies in the AI
  segment. The only one near it is `009F84BA`, in the ship AI.
- **The field displacements.** The `AC 01 00 00`, `84 01 00 00` and `80 01 00 00` hits in
  `00A13B60` are stack slots (`00A13BF2 MOV [ESP+184h],ESI`) or misaligned. The other
  `+1ACh`/`+180h` hits are in the planners and `00A2F6F0`, which issue no member order on this
  path.
- **The command hops.** The decompiled `0077D600`, `00816E30`, `0071ECF0` and `00721A40` read none
  of `+1ACh`, `+184h`, `+180h`, `00927F10`/`00927F30`, `[00E188D8]` or `game+18ECh`.
  `SetCommand 008358D0` and `PushCommandSlot 0071E6C0` (both listed in full in the ledger) test
  only the slot queue and target.

**Scope not covered:** which planner commands a given group in the image (CLOSEATTACK or
another) is the planners' question, not the member gate's. This packet does not establish that
the image's party-0 planner chooses CLOSEATTACK for Houston's group at 168.8 s. It establishes
only that no member gate would spare Houston when a planner does.

## 2. The host

`src/game_hosts_ai.cpp` has the same shape.
- `GameAiCoordinatorHost::Impl`'s `close_member_*` methods answer the squadron, ship and
  `excluded_007eda90` tests.
- `close_member_controller_busy` stands in for `00A1443D`'s class-trait call and answers false.
  Its name reads as "busy", but the image calls it on the class descriptor, so it is not a
  player gate either.
- Houston passed because it is a party-0 ship, exactly as in the image.

## 3. Consequence for validation

A player order that must hold beyond one AI tick needs the helm path: role 1, `unit+184h`, and
the `009F3DF3` cruise force, as `BSP_PLAYER_HELM` does (docs/SCRIPTED_HELM.md section 7). A
`moveto` alone does not hold. Two routes to USN02's primary 2 are open:
- a helm drive, which is throttle and rudder rather than a point;
- a `--helm-orders` line re-issued after each AI tick, which a real player's repeated clicks
  would also do.
