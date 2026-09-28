# USN02: why DeRuyter is not driven below 15% (packet `cc9_usn02_deruyter_fire`)

Worker cc9-ships2, on main `32f3d4f74`. Ghidra was read-only.

## 1. The script's orders

From `usn_2_java.lua`:
- **DRGrp** (DeRuyter, Java, Kortenaer, Electra), `:205..236`: skill `SKILL_STUN`, no torpedo
  evasion, no repair, `JoinFormation` on DeRuyter, `SetInvincible(unit, 0.1)`, then
  `NavigatorMoveToRange(DeRuyter, DRGoTo)`.
- **DRKillers** (Haguro, Jintsu, Yudachi, Samidare, Murasame, Harusame), `:300..330`: no repair,
  `SetInvincible(unit, 0.5)`. **Odd indices (Haguro, Yudachi, Murasame) target `NLCL[1]` =
  DeRuyter; even indices (Jintsu, Samidare, Harusame) target `NLCL[2]` = Java.** Each gets
  `NavigatorAttackMove(unit, trg)` and then `luaSetScriptTarget(unit, trg)`.
- **EnemyDestroya** (Yamakaze, Minegumo, Asagumo, Yukikaze, Kawakaze, Tokitsukaze, Amatsukaze,
  Hatsukaze), `:332..372`: torpedoes enabled at difficulty 1 and 2. The first four attack
  `luaPickRnd(HoustonGrp)` and the rest `luaPickRnd(ExeterGrp)`, through the same two calls.
- **`luaSetScriptTarget`** (`commandhelpers.lua:2941`) calls **`SetFireTarget(entity, target)`**
  for a ship or a submarine.

Phase 1 ends at `:531` when `GetHpPercentage(DeRuyter) < 0.15` or all eight EnemyDestroya are dead.

## 2. What the floor-ON host does (cc9-gunnery3's `local\IF_ON_USN02.log`, copied as `local\g3_if_on_usn02.log`)

- **`SetFireTarget` (`0089A8B0`) is UNIMPLEMENTED.** Every DRKiller's guns follow the automatic
  selector.
- **DeRuyter** takes 2420 and ends at 1700 health, about 41% (HP 4120), far from 15%. Java ends
  at 2068.
- **The DRKillers spread their fire:** Yudachi deals 3693 and sinks Encounter and Jupiter.
  Samidare deals 1975, Murasame 1938, Jintsu 1790, Harusame 1104 and Haguro 760.
- **Six of the eight EnemyDestroya die.** Yukikaze never engages: 0 taken, 0 hits, nearest
  540 m, 6136 m range on the table. So the all-dead arm never completes either.
- Exeter is sunk at 385.68 s by Yukikaze, and line 521 ends the run at 386.13 s in phase 1.

## 3. The image

- **`0089A8B0 SetFireTarget`.**
  - It reads argument 0 through `00888AA0`.
  - For argument 1: nil gives a null target; an entity table gives that entity through
    `00888AA0`; a Vector3 gives a fresh dummy entity (`operator new(1E4h)`, `004E5980`,
    positioned).
  - It then calls `entity->vtable[114h]()`, the weapon director, and
    **`00835860(target, force = 1)`**.
- **`00835860`** routes a kind 5Eh message when `force` is set, or `+23Ch` is clear, or `+238h`
  is null.
- **Its receiver `00836240`** (through `00721BD8`..`00721BF1`) stores
  **`director+23Ch = force`** and, on a change, **`director+238h = target`**, with the observer
  pair.
- **The selector `009F5DA0`, step 15**, keeps its hands off a locked target:

```
009F5F01  MOV ECX,[ESI+0Ch]           ; the director
009F5F06  MOV EDX,[EAX+2Ch] / CALL EDX ; does it report a current target
009F5F0D  JE  009F5F1B                 ; none -> set
009F5F12  CMP byte [EAX+23Ch],0
009F5F19  JNE 009F5F26                 ; locked -> skip
009F5F1E  PUSH 0 / PUSH EDI            ; 00835860(chosen, force = 0)
```

So in the image the three odd-index DRKillers keep their guns on DeRuyter, and the three even ones
on Java, for as long as the targets live. The gunnery host's step 8.7 engages
`director+238h` first (docs/BOT_FIRE_TARGET.md).

**Torpedoes:** a destroyer's torpedo target is the ship AI's approach target. It comes from the
same `NavigatorAttackMove` order, which the host already issues, so no torpedo difference is
claimed here.

## 4. The binding

`kScriptFireTargetBound`, in `include/bsp/game_hosts_ship_ai.hpp`, is committed OFF.
- `GameShipAiHost::store_fire_target_00836240` applies `00836240`'s gate and store to the
  controller's `fire_target` and a new `fire_target_locked`, and names the row's fire target,
  which the gunnery host engages.
- The selector's `director_target_locked` answers the lock. `scan_party_list` leaves the row's
  target alone while a live locked target is held.
- The script-orders host binds `SetFireTarget` (`0x0089a8b0`) and resolves the arguments through
  `GameUnitsHost::ship_ai()`.
- A new summary line counts sets and releases: `summary mission ship ai script fire target`.

**Labelled substitutions:**
- A locked target that dies is released: `+238h` is cleared, as the observer pair implies, and
  the lock byte stays.
- The Vector3 dummy target is not modelled.

## 5. Predictions, before any run

- **USN02 9200/9000:**
  - SetFireTarget runs 14 calls: the six DRKillers and the eight EnemyDestroya.
  - Haguro, Yudachi and Murasame hold DeRuyter; Jintsu, Samidare and Harusame hold Java.
  - DeRuyter is driven down to her 10% floor (about 412 of 4120). **Phase 1 ends when she passes
    15%, predicted between 120 s and 330 s (mission frames 2400 to 6600).** Its arm is `:531`,
    then the `Blackout` callback `luaMoveToPh2`.
  - Phase 2 is reached before Exeter's 385.68 s loss, so the run's end moves.
  - Yudachi no longer sinks Encounter and Jupiter.
  - EnemyDestroya also lock their scripted Houston-group and Exeter-group targets, so Houston
    and Exeter take more fire early.
  - Exeter's fate is uncertain. Deaths among party 0's escorts fall, and hit records rise on the
    flagships. Exit 3.
- **USN01 3200/3000 and USN04 4700/4500: identical (exit 0 or 1).** Neither script calls
  `luaSetScriptTarget` or `SetFireTarget`.
- **JM06 3200/3000: identical.** `jm06.lua` calls `luaSetScriptTarget` at `:1021`, `:1025`,
  `:1115` and `:1196`, but none is reached in `rb6_jm06`'s 3000 frames (0 native calls).
