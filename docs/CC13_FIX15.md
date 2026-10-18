# CC13 fix15: ArtilleryEnable 0089C590

Packet cc13-fix15, from the CC13_VERIFY16 R6 finding ("ArtilleryEnable unanswered"). Switch:
`kLuaArtilleryEnableBound` (`include/bsp/game_hosts_gunnery.hpp`), committed **false**. Status:
reconstructed and build-tested. Not game-validated: no game runs were made.

## Send side

`0089C590 BSP_LuaBinding_ArtilleryEnable` works in four steps:
1. It reads argument 0 through `00888AA0`.
2. It gets the weapon director from the entity's `vtable[114h]` (the load is at `0089C6A8`).
3. It reads argument 1 through `00B66250`.
4. Only for a non-null director (the test at `0089C6E9`), it passes the flag to `0071DFD0` with
   `ECX` = the director (`0089C6F2`). The image
has no null check on the entity itself before the `vtable[114h]` call. The host skips a nil entity
instead (MINOR: an error path).

`0071DFD0 BSP_WeaponEnable_SendArtilleryMessage`, `__thiscall(director)(bool)`, `RET 4`. It builds
a message on the stack with `BSP_SessionMessage_ConstructBase(5Ah)` (`0075B430`) and sets vtable
`00CFD9C4`. It writes sub-kind **3** at msg+20h (`0071E01A`) and the flag at msg+24h (`0071E022`).
It then routes the message with `0077C2A0(msg, 7, 0)` on `[director+34h]`. The AAEnable twin
`0071E050` writes sub-kind **4** (`0071E09A`), not 2 as `docs/CC13_VERIFY16.md` R6 says. Sub-kind
2 is the dword at director+1CCh.

## Receive side

| Step | Address | What |
| --- | --- | --- |
| route | `0077C2A0` | session route, channel 7 |
| dispatch | `00721A40`, arm at `00721A7E`..`00721A93` | `msg->vtable[0Ch](5Ah)` passes, then `director->vtable[38h](msg)` |
| slot | `00D09EC0`+38h (`00D09EF8`) = `00835610` (`JMP 0071C1E0`); `00835640` sits at `00D09F90` in another table and tails to `0071C1E0` at `0083568A` | `00D09EC0` is the vtable `008363E0` stores, the director a LandFort gets from `00743E90` |
| store | `0071C225`..`0071C231` | sub-kind 3: `[ECX+220h] = (msg+24h != 0)` |

The field is director+220h. The reflection visitor `008360C0` names it `artilleryEnabled` (string
`00D09E0C`).

**Default.** `00720180 BSP_CommandControllerBase_Construct` stores 1 at `007202FD` (and 1 at
+221h..+223h). The other writers are the scene pass B (`007219DB`, from a ship's bag; the host
already loads it into the per-name table) and the network snapshot in (`0072199B`). So before any
script call the byte is 1, unless a ship's scene bag says otherwise. The ON host takes the
pass-B entry when there is one and `{1,1,1,1}` otherwise. That matches the image.

## Readers of +220h (displacement scan)

I scanned every executable section of the PE for a memory operand with displacement 220h
(`local/fix15_scan.py`, Capstone). Byte accesses on a director:

| Site | Function | Kind |
| --- | --- | --- |
| `0071C231` | `0071C1E0` | write, sub-kind 3 |
| `007202FD` | `00720180` ctor | write 1 |
| `0072199B`, `007219DB` | snapshot in / pass B | write |
| `007213E4`, `007214D4` | snapshot out | read |
| `008360F5` | `008360C0` reflection visitor | read |
| `0086258B` | `008624C0` weapon-panel stance push | **read, fire path** |
| `009F2B18`, `009F2E20` | `009F1BC0` ship AI | **read**, cached at AI +12B9h |
| `00720DA0` / `00720DE8` | getter / four-byte setter | no rel32 or absolute reference anywhere in the image |
| `00861993` | `00861990` bridge getter | no rel32 or absolute reference anywhere in the image |

The scan's other hits (`00568C46` in a UI screen, `00A99F9D`/`00A9A01B` indexed, and the `ADC` hits
that are data decoded as code) do not access a director. `ghidra callers` was not relied on: the
unreferenced getters were checked with a rel32 and absolute-dword scan (`local/fix15_dword.py`).

**The gate point.** `008624C0` (step 4, `0086258B`) calls `00861CD0(byte)`. That function walks the
artillery category list at `00E0A4F8` and ORs or clears bit 1 (`00861CF2` / `00861CF7`) of each
category's mask at ai+80h. `008633D0` then refuses a non-plane target when bit 1 is clear. Bit 0
(may engage a plane) is untouched, so a disabled artillery category can still be given an air
target. `008624C0` runs from `00864FE0` step 5 through the bridge that `00863A80` builds for any unit
whose `vtable[114h]` director exists.

`009F1BC0` writes `+12B9h = [director+220h] && 00863840(...)` (`009F2B18`..`009F2B3D`) and plainly
at `009F2E20`. The host models it as `director_artillery_enable_0220()` in
`src/ship_ai_approach_update.cpp`, and that reads the same per-name table.

## Host change

- `src/game_hosts_script_orders.cpp`: a binding row for `ArtilleryEnable` at `0x0089c590u`, with
  `handles()` answering `kLuaArtilleryEnableBound`. The native's arm calls
  `record_director_artillery_enable_0071c231(name, flag)`, the named accessor. It copies the
  unit's table entry (or the constructor's four 1s), sets `artillery`, and stores it back.
  - A plane is skipped: its `vtable[114h]` is `0047F180`, which returns null, so the image never
    sends.
  - SUBSTITUTION (labelled): the flag is stored at the call, not routed through `0077C2A0` and
    delivered by the session.
  - OFF: `handles()` answers false, so the row stays UNIMPLEMENTED and nothing is written.
- `src/game_hosts_gunnery.cpp`, `apply_director_stance_008624c0`, a separate `if constexpr` block
  after the existing ship read. Ships already read the whole entry under `kShipDirectorEnablesBound`.
  The new block lets any other non-plane unit take `artillery` from its entry. It is not inside any
  cc9-gunnery51 block (`2a3c92f93`, `1e7a81506`, `376d089f6`, `5c5d57b05`); blame on the
  surrounding lines shows `453041b29`, `53bb947f2` and `8f205baad`.

No RNG draw: `0089C590`, `0071DFD0`, `0071C1E0`, `00861CD0` and `008624C0` do not call `00BD2F10`.
Indirectly, a unit that stops engaging surface targets fires fewer rounds. Any per-shot draw that
the gunnery path makes (`0072D579` under cc9-gunnery51's switch) is then consumed less, so with
the shared stream every later draw shifts.

## Missions reached (this installation's Lua)

Single-player calls, non-PRCP paths:

- `COTP-USN/usn_09_leyte.lua` `:9`-`:11`: Fortress1-3 (LandFort, Japanese) disabled at load;
  `:589` disables and `:2137` re-enables.
- `ijn/JM/jm01.lua` and `COTP-IJN/jm01.lua` `:213` Monaghan off, `:3270`/`:3274` on.
- `jm09.lua` `:182`/`:194` (StageTrg) off, `:1589` on.
- `COTP-USN/usn_04_defend_guadalcanal.lua:1990`: USN_DummyCruiser[1] off.
- `jm03`, `usn_03` (3), `usn_11`, `bsm_02` (5), and the PRCPUS copies of usn_05 (12), usn_09
  (11) and usn_16.
- `scripts/global/commandhelpers.lua` (5).
- Multiplayer: siege03 (20), siege07 (5) and others. These are not on the host path.

## Predicted ON effect (not run)

- **usn_09.** The three fortress towers stop engaging ships with their artillery categories. They
  can still engage planes if a gun's mask keeps bit 0.
- **jm01.** The Monaghan holds surface fire until `:3270`.
- **usn_04.** The dummy cruiser stops shooting ships.

For a ship, `009F1BC0`'s +12B9h also turns false. The ship AI's ring query then drops its
artillery gate (`q.allow_artillery`), so the approach logic can move as well as the guns. Labelled:
the host applies the store in the same frame as the call, whereas the image delivers it through
the session. OFF output is unchanged by construction: the native is unhandled and the gunnery block
is `if constexpr`.

## Divergences (labelled)

1. Store at the call instead of the session delivery (same as TorpedoEnable and AAEnable).
2. A nil entity is skipped. The image would call through a null vtable.
3. "Has a director" is approximated as "not a plane" (kind 0Fh), as TorpedoEnable does. A unit
   kind whose `vtable[114h]` is null and that is not a plane would get an entry the image never
   writes.
