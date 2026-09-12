# The command classes: a four-getter descriptor, not a behaviour object

Addresses: `00CFB378` `00CFB3A4` `00CFB518` `00CFB534` `00CFB554` `00CFB590` `00CFB5A4`
`00CFB5C0` `00CFB63C` `006F7F30` `006F7F40` `006F7F50` `006F7F90` `006F8010` `006F8020`
`006F8030` `006F8040` `006F8960` `00E08EF8` `00E08FC0` `00E19A7C`

Packet `cc2_director_commands`, companion to `docs/COMMAND_EXECUTION.md`. Descriptive names are
hypotheses; the 26 class names are string literals in the image
(`docs/SCENE_COMMAND_TYPES.md`, which owns the 26-row table and is not repeated here).

## Headline, and a correction to this packet's own brief

The packet was written expecting a base command class with `construct-from-descriptor`, `begin`,
`step`, `end`, `is done` and target accessors past `vtable[0Ch]`, and per-class execute/step
methods. **There are none.** Every command class vtable is five slots (six for `moveto` and
`moveonpath`), and every slot past the destructor is a two- to six-byte constant getter. There is
therefore no per-class begin/step/end to read, and no "execute method" for the 25 non-cruise
classes. The behaviour those methods would hold lives in the controller, which dispatches by
comparing the command pointer against the 26 singleton addresses; `docs/COMMAND_EXECUTION.md`
documents that engine and the reference map below shows where each class is named.

## The base vtable `00CFB378`

Read from `.rdata` directly (`ghidra xrefs` misses data references outside functions). The table is
five dwords and is followed at `00CFB38C` by the inline literal `undefined command name`, which is
what bounds it.

| Slot | Base | Body | Meaning |
| --- | --- | --- | --- |
| `+0h` | `006F7F90` | `MOV [ESI],00CFB378`; walks `00E19A70` for the node whose `+8h` is `this`; `CALL 00522B70`; `TEST byte ptr [ESP+8],1`; `CALL 00BF65B2`; `RET 4` | scalar deleting destructor: restores the base vtable, unlinks the object from the registry at `00E19A6C`, frees when bit 0 of the flag is set |
| `+4h` | `006F7F30` | `MOV EAX,00CFB38C; RET` | `const char* name()` |
| `+8h` | `006F7F40` | `XOR AL,AL; RET` | `bool requires_target()` |
| `+0Ch` | `00BF698E` | shared CRT-region stub | `int category()`, 0 for the base |
| `+10h` | `006F7F50` | `XOR EAX,EAX; RET` | a fourth `int` constant, 0 for the base and for 23 of the 26 classes |

`00CCE500 + 40h*k` is the constructor for class `k`: it increments `00E19A68`, stores the ordinal at
object `+4h`, appends to `00E19A6C` through `006F7D40`, then overwrites the vtable pointer with the
derived table. `006F7FE0` is a second, standalone copy of the same sequence for `settarget`
(`006F7FFF: CALL 006F7D40`, then `MOV [ESI],00CFB3A4`, `RET`); `docs/SCENE_COMMAND_TYPES.md`'s
evidence shows `00CCE500` running the sequence inline rather than calling it, so which of the two
the CRT table reaches is settled there, not here. The destructors at `00CDC6A0 + 40h*k` restore
`00CFB378`.

## Derived vtables, read from `.rdata` `00CFB378`..`00CFB69F`

Every derived table repeats the same five slots and is followed by its own name literal inline, so
the tables and the strings interleave. `settarget` (`00CFB3A4`) decodes as:

| Slot | Body | Bytes | Value |
| --- | --- | --- | --- |
| `+0h` | `006F91A0` | destructor thunk | |
| `+4h` | `006F8010` | `B8 0C 8B CF 00 C3` | `"settarget"` at `00CF8B0C` |
| `+8h` | `006F8040` | `B0 01 C3` | `requires_target = true` |
| `+0Ch` | `006F8020` | `B8 01 00 00 00 C3` | `category = 1` |
| `+10h` | `006F8030` | `33 C0 C3` | `0` |

`follow` (`00CFB518`) is the same shape with `006F8840` returning 3 and `006F8860` returning true;
`cruise` (`00CFB554`), `stop` (`00CFB5A4`), `retreat` (`00CFB5C0`), `cleartarget` (`00CFB3B8`),
`clearorders` (`00CFB3CC`) and `attackmove` (`00CFB590`) likewise. `stop`, `retreat` and
`returntobase` do not override `+10h` at all: their tables carry the base's `006F7F50`.

**Two classes have a sixth slot.** `moveto` (`00CFB534`, `[14h] = 006F8960`) and `moveonpath`
(`00CFB63C`, `[14h] = 006F8F30`). `006F8960` is a Meyers-style lazy static: it tests bit 0 of
`00E19A8C`, and on the first call zeroes `00E19A7C`/`00E19A80`, writes the float at `00D7A24C`
(`1.0f`) into `00E19A84` and `00E19A88`, then returns `00E19A7C`. So slot `+14h` is
`const float* default_parameters()` over a four-float record `{0, 0, 1, 1}`. `00E19A7C` lies past
`.data`'s raw size, so the record is zero until that first call.

## Per-class behaviour: where each class is actually named

`ghidra xrefs` reports far fewer sites than exist (4 for `follow` against 19 in the image), because
the comparisons are `CMP reg,imm32` operands. The table below is a **complete** scan of `.text` for
the 4-byte little-endian singleton addresses (`local/cmdref_scan.py`), restricted to the controller's
own code (`0071B000`..`00723000` and `00810000`..`00838000`) and attributed to the containing Ghidra
function. `total` is the image-wide count including the two constructor and two destructor sites.

| Class | Object | total | Named in the controller at |
| --- | --- | --- | --- |
| `settarget` | `00E08EF8` | 18 | `00816FE5` (`00816E30`) |
| `cleartarget` | `00E08F00` | 12 | `00817019` (`00816E30`) |
| `clearorders` | `00E08F08` | 15 | `008171BF` (`00816E30`) |
| `artillery` | `00E08F10` | 8 | `00817229` (`00816E30`) |
| `torpedo` | `00E08F18` | 18 | `0071D721` (`0071D6D0`) |
| `divebomb` .. `dogfight` | `00E08F20`..`00E08F58` | 15-18 each | nowhere in the controller |
| `follow` | `00E08F60` | 19 | `00720D62` (`00720CD0`), `00816EA8`, `00835D5C` (`00835C70`), `00835ED0` (`00835E90`), `00836ADD` and `00836E39` (`00836920`) |
| `moveto` | `00E08F68` | 60 | `0071F67B` (`0071F600`), `00811FEC`, `00816F8B`, `00835BBA` (`00835B40`), `00836BA7` (`00836920`) |
| `cruise` | `00E08F70` | 18 | `0071DF08` (`0071DEE0`), `00811FD8`, `00816F67`, `00835D17`/`00835DA0`/`00835E19` (`00835C70`), `0083605E` (`00836040`), `00836E8C` |
| `attackmove` | `00E08F78` | 60 | `0071D75F`, `0071E837` (`0071E7F0`), `0072060A`, `00811FE4`, `00816FDA`/`00817221`/`00817239`, `00835BC1`, `00835DB0`, `00836B46`, `00836D69` |
| `moveonpath` | `00E08F80` | 42 | `0071BDFC`, `0071D740`, `0071D7A4` (`0071D780`), `0071DCB2`, `0071DF1D`, `0071E4C6`, `0071F6A6` (`0071F600`), `007200E7`, `007207D2`, `00721ACF`, `00812038`, `00816F7F`/`00816FB5`/`00817141`, `00836BF1`/`00836D2B` |
| `stop` | `00E08F88` | 30 | `0071DF01`, `00811FBA`, `00816F73`, `00835D1F`/`00835DA8`, `00836065`, `00836A8F`/`00836E74` |
| `retreat` | `00E08F90` | 18 | `00811FCC` |
| `returntobase` | `00E08F98` | 15 | nowhere in the controller |
| `land` | `00E08FA0` | 37 | `007205FA`, `00816FC0`, `00835EC8` (`00835E90`) |
| `closetoship` | `00E08FA8` | 9 | nowhere in the controller |
| `Leave` | `00E08FB0` | 8 | `0071DF0F` (`0071DEE0`), `00816EFE` |
| `disband` | `00E08FB8` | 8 | `0071DF16` (`0071DEE0`), `00816F43` |
| `tutorial` | `00E08FC0` | 4 | nowhere: only its constructor and destructor |

## The classes read in full

### `settarget` (`00E08EF8`, category 1, requires a target)

`coverage: complete` for the class; its execution is `00816E30`'s arm at `00816FE5`
(`docs/CRUISE_COMMAND.md`, contract). Within the controller it is never named: a category-1 command
reaches the fire-target path generically. `008358D0` forces the fire target for **every** category-1
or category-2 command, `settarget` included, whenever the session mode is 0 or 1.

* begin: no special arm in `0071F600` or `00835C70`; `CommandAcceptsTarget` demands a resolvable
  target whose `+5Dh` byte is clear, because `requires_target` is true and the category is 1.
* step: no arm in `00836920`; only the generic arrival test applies.
* end: `00720850` with index 0.

### `cleartarget` (`00E08F00`, category 0, no target) and `clearorders` (`00E08F08`, category 0)

`coverage: complete`. Both are category 0 with `requires_target = false` and `vtable[10h] = 0`, and
neither is named anywhere in the controller. They are handled entirely on the entry side, in
`00816E30` at `00817019` and `008171BF` (contract). They never occupy a command slot: no controller
code compares a slot against either address.

### `attackmove` (`00E08F78`, category 2, requires a target)

`coverage: complete` for the controller's arms; `00816E30`'s three sites are contract.

* begin (`00835C70`, `00835DB0`): when `FUN_00521E70(0)` passes or the current fire target is null,
  and the current fire target differs from the resolved command target, call `00835860` at
  `00835E07` to set the fire target. No stage call, so the command stays at stage 0.
* the override guard (`0071E7F0`, `0071E837`): a new override command is refused unless it resolves
  to the same object the `attackmove` queue head targets.
* step (`00836920`, `00836B46`): target gone means stage 2. A target passing `vtable[5Ch](41h)` is
  left alone. A target passing `vtable[5Ch](1Ch)` that is flagged at `+5Eh` or shares the session's
  side is converted: `00465080(target, 0)` builds a parameter record and `0071ECF0` issues `moveto`,
  then stage 2. A live visible unit that fails `FUN_005457C0(target+54h)` also goes to stage 2.
* `CommandAcceptsTarget` (`0071D6D0`, `0071D75F`) resolves the target for `attackmove` before
  returning true; the resolved value is discarded at the call site.

### `moveto` (`00E08F68`, category 3, requires a target)

`coverage: complete` for the controller's arms.

* It is the **conversion target** of two rules: `0071F600` at `0071F67B` rewrites the queue head to
  `moveto` when the head's target passes `vtable[5Ch](1Ch)`, the head's category is 1 or 2 and the
  target's `+54h` equals the session endpoint's `+54h`; `00836920` at `00836BA7` reissues `moveto`
  for the `attackmove` friendly-target case.
* `00835B40 BSP_WeaponDirector_RetargetCommandSlot` (contract) names it at `00835BBA`.
* begin: no arm; step: no arm of its own beyond the generic arrival test. Its only class-specific
  datum is the sixth vtable slot's `{0, 0, 1, 1}` default parameter record.
* `requires_target` is true while three quarters of the authored `Moveto` records name no target;
  `docs/SCENE_COMMAND_TYPES.md` already flags that reading as provisional and this packet found
  nothing to settle it.

### `stop` (`00E08F88`, category 3, no target)

`coverage: complete`.

* begin (`00835C70`, `00835D1F`/`00835DA8`): `0071D810(1)` — `stop` enters **stage 1**. Skipped
  entirely, returning 1, when the owning unit's `+184h` byte is set.
* step (`00836920`, `00836A8F`): stage 2 when more than one command is queued, or `unit+184h` is
  set, or `COMISS XMM0, dword ptr [00D7A218]` on the motion controller's speed at
  `*(unit+73Ch) + 28h` does not set the carry flag. The constant is `0.0f`, so the `JC` at
  `00836AC8` skips completion only for a negative or unordered speed: `stop` completes as soon as
  the unit is not making sternway.
* `0071DEE0` answers false for a `stop` slot, so a queued `stop` is not treated as an active order.
* `00836040 BSP_WeaponDirector_NormalizeSelfTarget` names it at `00836065` (contract).

### `follow` (`00E08F60`, category 3, requires a target)

`coverage: complete`.

* It is the command `00720CD0` issues after clearing the queue: `00720D61: PUSH 0xe08f60`.
* begin (`00835C70`, `00835D5C`): when the target does not resolve, or the unit is not in a group,
  or the target's group differs from the unit's, the routine's result becomes
  `BSP_Entity_ControllerBelongsToAnother()` rather than 1.
* step (`00836920`, `00836ADD`): stage 2 unless the unit is in a group (`unit+284h`), the group
  leader from `007788D0` exists and is not the unit, the command's target is that leader, and fewer
  than two commands are queued.
* `00835E90` (contract) names it at `00835ED0` alongside `land`.

### `retreat` (`00E08F90`, category 3, no target)

`coverage: partial`. The only controller site is `00811FCC`, inside `00811F50`, a routine outside
this packet's lease that also names `stop`, `cruise`, `attackmove`, `moveto` and `moveonpath` at
`00811FBA`..`00812038` — the shape of a name-to-singleton table rather than an execution arm. There
is no `retreat` arm in `0071F600`, `00835C70`, `00836920` or `0071DEE0`, so a queued `retreat` takes
the generic path: pushed like any other, begun without a stage call, completed only by the generic
arrival test or by an explicit clear. `contract: unread` for `00811F50`.

### `clearorders`, `cruise`

`cruise` is `docs/CRUISE_COMMAND.md`'s and is not re-read here; the controller arms that name it are
listed in the reference table above, and `docs/COMMAND_EXECUTION.md` records that it enters stage 1
at `00835DA0` exactly as `stop` does.

## `contract: unread`

`artillery`, `torpedo`, `divebomb`, `levelbomb`, `dropkamikaze`, `depthcharge`, `strafe`, `rocket`,
`kamikaze`, `dogfight`, `moveonpath`, `returntobase`, `land`, `closetoship`, `Leave`, `disband`,
`tutorial`. For each of these the class descriptor itself is complete (name, category,
`requires_target` and the `+10h` constant come from the vtable dump above and
`docs/SCENE_COMMAND_TYPES.md`); what is unread is the behaviour at the controller sites listed in the
reference table. `moveonpath` is the largest gap: 16 controller sites, including the whole path
machinery at `0071BDE0`, `0071DC80`, `0071E4C0`, `0071FDE0`, `007207C0` and the `50h` path objects
at controller `+1A4h`.

## Open questions

* `006F88D0` has the same lazy-static shape as `moveto`'s `+14h` getter and sits in the `follow`
  block, but `follow`'s vtable is five slots and nothing in `.rdata` points at it.
* Whether `vtable[10h]`, zero for 23 classes, is ever non-zero. `00CFB590`'s `006F8B90` and the
  `moveonpath`/`land`/`closetoship` tables were not decoded byte by byte.
