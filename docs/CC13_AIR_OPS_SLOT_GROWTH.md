# Air-ops slot growth: 006C7210 and the player's launch

Reviewer: cc13-airops1 (read-only on cc9's files). Status: **exported/listing-read**; the proposed
host change below is **build-tested** in agent/cc13-airops1 and then reverted, not committed and not
game-validated. Every address was read from the disk image with `bsp.py disasm-raw` (Capstone over
`.text`), not from a decompile. Caller censuses are rel32 (E8/E9) plus absolute-dword scans of the
whole image (`local/ao1_xref.py` in this tree), each with a positive control.

## Verdict

- `006C7210` **grows the deck unconditionally** whenever no slot is in state 1 or 5. It has no
  argument and no flag; nothing it reads can refuse. cc13-mission1's reading is right.
- `006C7210` has **exactly one caller**, `006CC6D6` inside `006CC690`, whose one caller is
  `0089E5E9` inside `0089E3C0`, the Lua binding `LaunchSquadron` (table entry `00E0BB7C`, name
  pointer `00E0BB78` -> `00D10954 "LaunchSquadron"`). The Support Manager screen never reaches it.
- **There is no 4-slot limit on the deck.** The count is whatever the scene authored (`NumSlots`,
  docs/AIROPS_LOAD_FROM_SCENE.md) plus whatever `LaunchSquadron` appends. The "four" is a property of
  the **screen**, which has four slot rows and is written as if a deck had four slots (below).
- The host's `"no such slot"` refusal is **faithful in outcome but not in mechanism**. The player's
  launch never grows the deck and never launches into a slot that does not exist; when there is no
  launchable slot the native button press simply does nothing (`0067A56D` -> `0067A664`). But the
  host gets there by the wrong route: it borrows `006C7210`'s pick (an AI-only routine), and it
  accepts state 1 where the screen accepts only state 5. Diff in section 4.

## 1. 006C7210, full control flow

`__thiscall`, ECX = deck block, no stack arguments (`RET` at `006C748B` has no immediate). Returns
the slot index in EAX.

| Address | What happens |
| --- | --- |
| `006C722E`-`006C7252` | EBP = block+0Ch; if non-null, lock it (`CALL [00CE2218]`) and `+18h += 1`. |
| `006C7256`-`006C7264` | ESI = block+50h (count); EAX = 0. Count 0 -> straight to the append path (`006C7299`). |
| `006C7270`-`006C7286` | Walk slots at block+4Ch, stride 58h: read slot+2Ch; `== 1` or `== 5` -> `006C728A`; else next. |
| `006C7288` | Walk exhausted -> `006C7295` -> append path. |
| `006C728A`-`006C728F` | ESI = found index; `CMP EAX,-1 / JNE 006C745E` (always taken: EAX is a valid index). **Early return path 1: returns the first slot in state 1 or 5.** |
| `006C7299`-`006C730F` | Build a template 58h record on the stack at ESP+30h (layout below). |
| `006C7316`-`006C732B` | If count != capacity (block+54h) -> `006C73B7`. |
| `006C7331`-`006C73B5` | Capacity full: new capacity `2n+2` stored at block+54h, `operator new(cap*58h)` (`00BF55BE`), copy-construct each old record (`0065CAF0`), destroy it (vtable+0, arg 0), free the old buffer (`00BF6989`), store the new one at block+4Ch. |
| `006C73B7`-`006C73D7` | Placement copy-construct the template at `base + count*58h` (`0065CAF0`; null check from the `ADD` flags at `006C73BD`), `count += 1` (`006C73D7`). |
| `006C73DB`-`006C7451` | Destroy the template's two sub-objects (`+38h`: vtable `00CF64AC`, unregister via `006952A0` if `+4Ch` set, `00695870`; `+14h`: vtable `00CF6494`, same with `+28h`). |
| `006C7456`-`006C745C` | ESI = block+50h - 1. **Return path 2: the index of the record just appended.** |
| `006C745E`-`006C748B` | Unlock block+0Ch if held (`+18h -= 1`, `CALL [00CE2210]`), return ESI. |

There is no third path. The function never returns -1 or the old count, and never refuses.

Appended record (offsets from the record base, template at ESP+30h):

| Offset | Value | Note |
| --- | --- | --- |
| +00h | vtable `00CF6564` | |
| +04h | 0 | class (006CC6F5 writes it) |
| +08h | 0 | count (006CC70F) |
| +0Ch | 3 | |
| +10h | 0 | arm / class+134h default (006CC704, 006CC712) |
| +14h | vtable `00CF6514` | observer sub-object; its pointer is +28h |
| +18h, +1Ch, +20h | 0 | |
| +24h | byte 1 | |
| +28h | 0 | squadron |
| +2Ch | **1** | state (`006C72D5`) |
| +30h | 0.0f | timer (`MOVSS` of a zeroed XMM0) |
| +34h | byte 0 | |
| +38h | vtable `00CF652C` | second observer sub-object; pointer +4Ch |
| +3Ch, +40h, +44h | 0 | |
| +48h | byte 1 | |
| +4Ch | 0 | |
| +50h | 9 | |
| +54h | the deck block (EDI) | |

So the new slot is born in state 1, which is one of the two states the next call's walk accepts: the
array grows only when **every** existing slot is in state 2, 3 or 4.

`006CC690` (the only caller) then writes class/arm/count on that index and either starts the launch
(`006CC733` -> `006C7490`) or, when block+38h is non-zero, queues it (`006CA770`, `006CA640`). It
returns the index; `0089E618` adds 1 for Lua. Note `0089E564` calls `006BF620` (the readiness test)
and **discards its result** (the next instruction is `PUSH 1`; AL is never tested), so
`LaunchSquadron` launches whatever the deck's readiness.

The host's `air_ops_pick_launch_slot_006c7210` + `air_ops_launch_squadron_006cc690`
(src/air_operations.cpp:378, :599-608) already model this: pick returns `size()` and the launch
resizes. That part is faithful (growth by one rather than `2n+2` capacity, which is unobservable).

## 2. Callers

Census by rel32 + absolute dword over all sections. Positive controls in the same run: `006C7490`
-> `006CA8FA`, `006CC733` (known), `006CD7CD`; `006C0F00` -> 7 calls + the `006C4780` thunk jump;
`006C4780` -> `00674848`, `0067A5A7` (known), `00896145`.

| Target | Callers |
| --- | --- |
| `006C7210` | `006CC6D6` (in `006CC690`) only. No absolute dword (no vtable or table entry). |
| `006CC690` | `0089E5E9` (in `0089E3C0`) only. |
| `0089E3C0` | absolute dword at `00E0BB7C` (Lua registration, name `LaunchSquadron`). |
| `0065CA50` | `00660463`, `006604BD`, `00678BF2`, `00679C19`, `0067A560`, `0067A888`, `0067A9B9`, `0067AE4C`: all Support Manager screen code. It is a slot-record **assignment** into a stack local (`0065CA5A`.. copies +04h..+10h, rebinds the +14h observer), so the screen only reads the deck through copies. |
| `006C0E70` | `006C483E` (in `006C47F0`) and an unreferenced thunk at `006C4770`; `006C47F0` is called only from `006CD6E1` (in `006CD6C0`). |

**AI / scripts:** `LaunchSquadron` -> `006CC690` -> `006C7210`. Grows the deck.

**Player (Support Manager screen, 00675C40):** `0067A54A`-`0067A5E6`, reached when screen+5Ch == 0
and screen+1F4h != 0, with the selected unit at screen+300h (class 9 or 45h; deck = unit+72Ch for
45h, unit+1188h otherwise):

1. `0067A54A`: slot index = **screen+324h** (no +320h is added), record copied to a local via `0065CA50`.
2. `0067A565`: local+2Ch **== 5**, else `JNE 0067A664` (nothing happens).
3. `0067A57D`: `006BF310(class)` stock; `<= 0` -> `0067A664`.
4. `0067A584`-`0067A592`: `min(stock, screen+2B4h)`.
5. `0067A5A7`: `006C4780` (thunk -> `006C0F00`) fills **that** slot.
6. `0067A5D1`/`0067A5E6`: message 82h {slot, order 3, target, game+18ECh} routed by `0077C2A0`.

No `006C7210`, no growth. The slot is always an existing one because the cursor only ever holds an
index produced from the deck's own walk (section 3).

## 3. Where "four" lives: the screen, not the deck

- **Initial cursor, `00660190`.** Clears four row widgets at screen+CCh (`0066019E` loop of 4), then
  walks `i = 0 .. block+50h` (`0066044D`) for the first slot that is "out" (`00658530`: +28h set and
  state 3 or 4) or in state 2 or 5. If that index is **> 3**, `0066048A`-`00660498` stores
  `+320h = i - 3` and **`+324h = 3`**; else `+324h = i`, `+320h = 0`. With no match the loop exits
  at `0066044F` and +324h keeps its value (the constructor zeroes it at `0066FA11`).
- **+320h is write-only.** Byte scan of `89/8B/3B/39/C7/83/03/FF ?? 20 03 00 00`: only the writes
  `006601E8`, `00660492`, `006604AA`, `0066FA0B` (positive control: these four are the stores).
  So the scroll offset is never applied, and the launch at `0067A54A` with a first selectable slot
  at index >= 4 acts on slot **3**.
- **Row widgets are indexed by the slot index.** `[screen + idx*4 + DCh]` at `00678CA8`,
  `00679C7C`, `006604E8` etc. The next field, screen+ECh, is a separate widget (`0065DD43`), so the
  per-slot row array is four entries.
- **Cursor steps assume four slots.** Forward (`00679BF2`-`00679C61`): `(i+1) mod block+50h`,
  skipping slots that are neither out nor state 2/5. Backward (`00678BC2`-`00678C9C`):
  `(i+3) mod block+50h`, which is "previous" only when the count is 4.
- **What the player sees at the limit.** Nothing is greyed or refused by a slot-count test, because
  there is none. A slot that is not in state 5, or a class with no stock, makes the launch press a
  no-op (`0067A56D`, `0067A58A`). The per-launch group is capped by screen+2B4h (constant 3,
  `0066EC0A`) and by `006C0F00`'s own stock / plane-room clamp (docs/AIROPS_LAUNCH_TICK.md).

So a deck that `LaunchSquadron` has grown past four slots displays wrongly on the native screen
(row widget overrun into +ECh on a forward step, a wrong "previous", launches pinned to slot 3); the
native does not defend against it. cc9-ships43's "4-slot limit is native" is right about the screen
and wrong about the deck.

## 4. Host: what matches the image

`GameScriptOrdersHost::player_air_ops_launch` (src/game_hosts_script_orders.cpp:1571-1578):

- slot 0 calls `air_ops_pick_launch_slot_006c7210`: not native for the player; should be
  `00660190`'s initial cursor (with its clamp to 3);
- the state test accepts 1 or 5, citing `006C0E70`; the screen tests `== 5` (`0067A565`), and
  `006C0E70` belongs to `006C47F0` on the `006CD6C0` path, not to the screen;
- the `"no such slot"` refusal itself can stay: an explicit `slot_number` past the end has no
  native equivalent, and with the cursor rule below slot 0 can never produce one on a non-empty deck.

Proposed change (applied in agent/cc13-airops1, `./scripts/build.ps1` result below, then reverted;
cc9 owns the file):

```diff
@@ player_air_ops_launch
     int slot = slot_number - 1;
-    if (slot_number <= 0) slot = bsp::air_ops_pick_launch_slot_006c7210(*deck);
+    if (slot_number <= 0) {
+        // The screen never calls 006C7210 (its one caller is 006CC690, LaunchSquadron),
+        // so the player's launch never grows the deck. Slot 0 stands for the cursor
+        // 00660190 places when the screen opens: the first slot whose squadron is out
+        // (00658530: +28h set, state 3 or 4) or whose state is 2 or 5; none leaves the
+        // constructor's 0 (0066FA11). Past row 3 the cursor is clamped to 3
+        // (0066048A..00660498; the scroll it stores at +320h is never read), and
+        // 0067A54A launches slot +324h itself, so a deck of more than four slots
+        // launches slot 3. docs/CC13_AIR_OPS_SLOT_GROWTH.md.
+        slot = 0;
+        for (std::size_t i = 0; i < deck->slots.size(); ++i) {
+            const bsp::AirOpsSlot& s = deck->slots[i];
+            const bool out_and_away = s.launched_squadron != 0u
+                && (s.state == bsp::AirOpsSlotState::kLaunched
+                    || s.state == bsp::AirOpsSlotState::kRecalled);
+            if (out_and_away || s.state == bsp::AirOpsSlotState::kLaunching
+                || s.state == bsp::AirOpsSlotState::kReady) {
+                slot = i > 3u ? 3 : static_cast<int>(i);
+                break;
+            }
+        }
+    }
     if (slot < 0 || static_cast<std::size_t>(slot) >= deck->slots.size())
         return refuse("no such slot");
     const bsp::AirOpsSlotState state = deck->slots[static_cast<std::size_t>(slot)].state;
-    // 0067A565 and 006C0E70: the screen fills only a slot in state 1 or 5.
-    if (state != bsp::AirOpsSlotState::kCooldown && state != bsp::AirOpsSlotState::kReady)
-        return refuse("the slot is not in state 1 or 5");
+    // 0067A565: the screen fills only a slot in state 5; any other state and the
+    // button does nothing (0067A56D). 006C0E70's "1 or 5" belongs to 006C47F0,
+    // reached from 006CD6C0 (006CD6E1), not to the screen.
+    if (state != bsp::AirOpsSlotState::kReady)
+        return refuse("the slot is not in state 5");
```

Behavioural consequence for cc9 to weigh: a harness `launch` on a slot in state 1 is refused after
this change (the native player cannot launch from state 1; only the AI's `006C7210` takes state 1).
If the harness `launch` line is meant to stand for `LaunchSquadron` rather than the screen, the
right host call is `air_ops_launch_squadron_006cc690`, which already grows.

Build check: with the diff applied, `./scripts/build.ps1` in J:\PROG\battlestations-pacific-decompile-cc13-airops1 completed with exit code 0 (2/2 CTest suites passed); the file was then restored with `git checkout`.

## Uncertainty

- The two lock imports `00CE2218`/`00CE2210` are read as lock/unlock from their pairing with the
  `+18h` counter; not resolved by name here.
- `006C47F0` / `006CD6C0` is described only as "the message path"; which message and which peer
  (network client vs local) was not established.
- The screen gate conditions before `0067A54A` (screen+5Ch, +1F4h, `0067A433` byte tests) were read
  for reachability only.
