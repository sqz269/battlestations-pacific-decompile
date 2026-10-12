# Function-split listing repair (cc13-splits1)

Packet cc13-splits1 repaired Ghidra function splits that the band passes found, and scanned the image for
more of the same shape. A split is a Ghidra function whose entry lies inside another function's
instruction stream: the parent's body stops one byte before it, and the tail is listed as a function of
its own. No C++ changed; nothing was wired; no game was run.

## What the splits were

None of the repaired splits came from a no-return flag. The typed flow metadata
(`bsp.py ghidra typed-flow`, captures in `local/sp1_tf*.json`) showed every parent and every callee with
`no_return = false`, and no call-site flow override. The parent bodies stop exactly where a function had been
created at a mid-stream address; Ghidra bodies do not overlap, so the parent ended there. Three shapes:

- **Fall-through tails**: the split starts at the instruction after a non-terminating instruction of the
  parent (`cmp eax,ebx` or `cmp al,bl` after the SEH prologue, or after a `CALL`). Examples: `0084DDC1`, `00953DC4`,
  `00953EC4`, `00A9E428`, `0084A5D0`, `0072D5D1`, the 5- and 18-byte heads of the terrain grids.
- **Branch-target tails**: the split follows a `RET` of the parent without INT3 padding and is the target of
  a conditional jump inside the parent (`006F2A5E JNE`, `0078D6EB JE`, `009CFB90`).
- **Undefined heads**: the real function start (after INT3 padding) was not a Ghidra function, and only the
  tail was: `00729FD0` (tail `00729FF4`), `0072A0D0` (tail `0072A0F4`), `0074A8D0` (tail `0074A8E4`, reached by
  `JE` at `0074A8DE`).

`0077A4B0` and `0077A510` from the brief are not splits: each starts after an INT3 run, has no reference, and
ends in `RET` before INT3 padding. They were left unchanged.

## Checks before each repair

For every range (`local/sp1_plan.py`, `local/sp1_probe.py`, `local/sp1_refs.py`):
1. Linear Capstone disassembly from the parent start lands exactly on the split address.
2. The split address has no CALL, JMP or absolute reference except the parent's own branch.
3. The range end is the instruction after the tail's last `RET`, followed by INT3 padding (or the next
   function), and no other function starts strictly inside the range.
4. `bsp.py lease check` on every parent and tail address: unleased.
5. Prototypes were default `undefined f(void)` everywhere (`local/sp1_protos_before.txt`), so nothing was lost by
   re-creation.

## Repair

`tools/ghidra_define_function.py <start> <end> ... --recreate --delete-inside --record
reports/cc13_splits1_defined.json`, under the Ghidra write lock: the mid-body function is deleted, the parent is
deleted (its plate comment kept) and re-created from the existing instructions with `disassemble_first=false`,
taking its ledger name. 'Non-Returning Functions - Discovered' stayed disabled (checked with
`list_analyzers`), and no analysis pass was started. Every event, with the previous function text, is in
the record file.

| parent | name | old body | split deleted | new body |
| --- | --- | --- | --- | --- |
| `006F2A50` | BSP_CommandBuilding_TakeLandingPad | 006F2A50-006F2A66 | `006F2A67` FUN_006f2a67 | 006F2A50-006F2BC6 |
| `0084DDA0` | BSP_PlaneSquadronCommandBlock_BeginCommand | 0084DDA0-0084DDC0 | `0084DDC1` FUN_0084ddc1 | 0084DDA0-0084E00C |
| `00953DA0` | BSP_Unit_SetInferiorFailureProperty | 00953DA0-00953DC3 | `00953DC4` ..._SplitTail | 00953DA0-00953E52 |
| `00953EA0` | BSP_Unit_ClearInferiorFailureProperty | 00953EA0-00953EC3 | `00953EC4` ..._SplitTail | 00953EA0-00953F52 |
| `00A9E400` | BSP_GuiListbox_ReadProperties | 00A9E400-00A9E427 | `00A9E428` ..._SplitTail | 00A9E400-00A9E7D1 |
| `00729FD0` | (unnamed, was undefined) | - | `00729FF4` FUN_00729ff4 | 00729FD0-0072A082 |
| `0072A0D0` | (unnamed, was undefined) | - | `0072A0F4` FUN_0072a0f4 | 0072A0D0-0072A182 |
| `0074A8D0` | (unnamed, was undefined) | - | `0074A8E4` FUN_0074a8e4 | 0074A8D0-0074A918, 0074A920-0074A98E |
| `0078D6C0` | FUN_0078d6c0 | 0078D6C0-0078D784 | `0078D785` FUN_0078d785 | 0078D6C0-0078D81D |
| `00849F70` | BSP_Shipyard_LoadFromProperties | 00849F70-0084A5CF | `0084A5D0` FUN_0084a5d0 | 00849F70-0084ACAD |
| `0072D5B0` | BSP_Gun_SelectFireMode | 0072D5B0-0072D5D0 | `0072D5D1` BSP_Gun_SelectBulletRecord | 0072D5B0-0072D6DB |
| `009CFB60` | BSP_TakeoffStateSlow_ShouldLeave | 009CFB60-009CFBA3 | `009CFBA4` BSP_Unit_Send7AhMessage_009cfba4 | 009CFB60-009CFC64 |
| `00ADAC90` | BSP_TerrainRenderObject_LoadFoliageDefault | 00ADAC90-00ADAC94 | `00ADAC95` BSP_TerrainRenderObject_LoadFoliage | 00ADAC90-00ADACB4 |
| `00ADACF0` | BSP_TerrainGridU32_Resize | 00ADACF0-00ADAD01 | `00ADAD02` ..._Resize_Tail | 00ADACF0-00ADADB6 |
| `00ADAF50` | BSP_TerrainGridU16_Resize | 00ADAF50-00ADAF61 | `00ADAF62` ..._Resize_Tail | 00ADAF50-00ADB015 |
| `00AED2D0` | BSP_TerrainGridF32_Resize | 00AED2D0-00AED2E1 | `00AED2E2` ..._Resize_Tail | 00AED2D0-00AED428 |
| `00AF0990` | BSP_GraphicsSingleton_00f8c278_Get | 00AF0990-00AF09A9 | `00AF09AA` ..._Get_Tail | 00AF0990-00AF0A5D |

Every new body was read back with `typed-flow` (`local/sp1_after*.json`): each former split address now
reports its parent as the containing function, and no new split appeared.

## Ledger

- The ten named tails keep their ledger records and names, with evidence rewritten to "Interior label of
  <parent>, not a function start" followed by the earlier record (the convention of the interior labels made
  with `tools/ghidra_label_interior.py`). Ghidra kept each deleted function's symbol as a label at the same
  address, so the labels are in place; the label tool answered "already exists" for all ten.
- The thirteen named parents got an appended note recording the body repair.
- The unnamed tails (`FUN_` names) had no ledger, tag or reconstruction record. The new heads `00729FD0`,
  `0072A0D0`, `0074A8D0` and `0078D6C0` stay unnamed.
- `ghidra_annotate.py --apply` re-applied the thirteen parents' names and evidence comments. The tails' plate
  comments went with the deleted functions; their evidence remains in the ledger.

`BSP_Gun_SelectFireMode`'s own evidence already describes the whole body (the fire-mode switch that the tail
`BSP_Gun_SelectBulletRecord` performs); `BSP_TakeoffStateSlow_ShouldLeave`'s already gives the body as
009CFB60..009CFC65, which the repair now matches.

## Exports

`bsp.py ghidra export ... --force` refreshed the seventeen parents (without `--force` the command keeps the
old files). The stale export directories of the deleted tails (for example `exports/bsp/functions/0084a5d0`)
were left in place. No snapshot was taken: Ghidra's function count fell by 13 (17 deleted, 4 heads defined),
so the next `bsp.py snapshot` will re-export the list.

## Image scan for more splits (`local/sp1_scan.py`)

Every Ghidra function start not preceded by INT3/NOP was tested: shape A (linear disassembly from the
preceding function lands on it and the previous instruction falls through) and shape B (the previous
instruction is `RET`/`JMP` and the preceding function branches to it). 95 shape-A and 11 shape-B hits;
most shape-A hits are desynchronised data or jump tables (implausible previous instructions such as
`sbb bl,[esi]`), CRT (`00BFxxxx`) and Lua library code, and EH continuations after catch funclets. The real
candidates beyond the brief, and what was done:

| split | parent | state |
| --- | --- | --- |
| `0072D5D1` | `0072D5B0` | repaired |
| `009CFBA4` | `009CFB60` | repaired (this was a cc13-gp99 name for the tail) |
| `00ADAC95`, `00ADAD02`, `00ADAF62`, `00AED2E2`, `00AF09AA` | terrain grids / graphics singleton | repaired (cc13-mopup1 had named the tails) |
| `007A7DF0` | `007A7DE0` BSP_MovCam_SetViktorMode | **not repaired: leased to cc13-gp7a** |
| `007F64EC` | `007F6190` BSP_GunPlatform_SnapHeadingToFireWindow | **not repaired: leased to cc13-gp7a** |
| `0085F7C0` | `0085F6D0` | **not repaired: leased to cc13-gp85** |
| `008D97A0` | head `008D9750` (undefined) | **not repaired: leased to cc13-gp85** |
| `005220FA` | `005220C0` | **not repaired: leased to cc13-gp41** |
| `009EB4D1` | `009EAFC0` BSP_ShipAi_NeighbourAvoidBoxRefresh | **not repaired: leased to cc13-gp41**; its ledger evidence already says it is not a function |
| `00BF3ACC` | `00BF3A80` | left: its ledger evidence asks to preserve the saved split |
| `00685070` | - | not a split: called from `006857D2` and jumped to from two places |
| `00412070` | - | not a split: the bytes before it are data |

## For cc9

The gameplay parents repaired here are `BSP_CommandBuilding_TakeLandingPad` (006F2A50),
`BSP_PlaneSquadronCommandBlock_BeginCommand` (0084DDA0), `BSP_Unit_SetInferiorFailureProperty` /
`BSP_Unit_ClearInferiorFailureProperty` (00953DA0 / 00953EA0), `BSP_Shipyard_LoadFromProperties`
(00849F70), `BSP_Gun_SelectFireMode` (0072D5B0), `BSP_TakeoffStateSlow_ShouldLeave` (009CFB60),
`FUN_0078d6c0` (water crossing / height sampling, 0078D6C0) and the unnamed `00729FD0`, `0072A0D0`
(the same property-setter shape as 00953DA0) and `0074A8D0`. Their decompiles now cover the whole body;
host comments that cite the old split addresses still point at valid code, now interior labels.
