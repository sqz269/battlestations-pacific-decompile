# The entity's local 4x4 and the vtable slot 88h setter

Addresses: 006E00A0, 00431410, 0042B9A0, 0042D700, 0042D770, 00414DB0, 004134F0, 00413920,
0042ED50, 00CFC3D0 (data), 00D7A24C (data).

Packet `cc_entity_matrix`, worktree `agent/cc-entity-matrix`. Ghidra was **read-only** for this
packet: no renames, comments, prototypes or saves. Every descriptive name below is a hypothesis,
not a recovered symbol. This packet takes the `entity_local_matrix_slot` follow-up of
`docs/WORLD_ENTITY_UPDATE.md`, which left slot `88h`'s body unread.

The reconstruction is in `include/bsp/entity_local_matrix.hpp` and `src/entity_local_matrix.cpp`;
machine-readable facts are in `reports/entity_matrix.json`.

## There are exactly two setters

Slot `88h` is `void __thiscall(entity, const float* matrix)`, `RET 4`. Sixteen vtables carry this
interface (the ones whose slot `0D8h` is `00955970`), and between them they hold two distinct
implementations. Neither has a Ghidra function; both were read from the disk listing.

| Slot 88h body | Vtables | Extra step |
| --- | --- | --- |
| `00431410` | 00CF8C08, 00CFB028, 00CFCD60, 00CFF3F8, 00CFFDE0, 00D0B770, 00D1A698 | none |
| `006E00A0` | 00CF90B0, 00CFA778, 00CFB738, **00CFC3D0**, 00CFFA30, 00D01630, 00D09678, 00D0BF80, 00D0C648 | the owner notification |

`00CFC3D0` is `MDestroyer`'s (`docs/GAME_WORLD_ENTITIES.md`), so a unit instance takes the
`006E00A0` path. The vtable list is the xref set of each body: `006E00A0` is referenced from nine
data addresses, every one of them a vtable's `+88h` slot, and `00955970` from sixteen, every one a
`+0D8h` slot, the nine being a subset of the sixteen.

`00431410`, body `00431410..00431444`:

```
00431410  EAX = [ESP+4]                 ; the source matrix
00431418  ECX = ESI + 74h
0043141B  004134F0(&entity->local74, EAX)   ; BSP_Matrix_Copy4x4X87, 16 floats
00431420  byte [ESI+0C8h] = 0
00431427  byte [ESI+10Ch] = 0
0043142E  ESI = [ESI+48h]               ; first child
00431435  loop: 0042ED50(child)         ; BSP_SceneNode_InvalidateSubtreePose
0043143C  ESI = [ESI+44h]               ; next sibling
00431444  RET 4
```

`006E00A0`, body `006E00A0..006E00E7`, is the same nine steps with EDI as `this` and one tail:

```
006E00D4  EDX = [EDI+310h]
006E00DA  EAX = [EDX+0Ch]
006E00DD  ECX = EDI + 310h              ; the sub-object's address, not [EDI+310h]
006E00E3  CALL EAX                      ; (entity+310h)->vtable[0Ch]()
006E00E7  RET 4
```

`ECX` is `EDI+310h` while the vtable came from `[EDI+310h]`, so `entity+310h` is an embedded
sub-object whose first dword is its own vptr. The notification is unconditional and the pointer is
never null-checked. What the notified object does is unread; the doc's host method carries the
address, not a verb (checklist rule 1).

### The invalidation is done twice per interpolator record

`docs/WORLD_TIMED_ATTACHMENTS.md` step 7 reads `00904AEF..00904B13` as the interpolator pass
invalidating the pose cache after the setter call at `00904AE3`. That block is correct, and it is
also **the setter's own block, repeated**: `00904AF7`/`00904AFE` clear the same two bytes and
`00904B09` walks the same `+48h`/`+44h` child list through the same `0042ED50`. The pass therefore
clears the flags and walks the children twice for every record it applies. Nothing depends on the
second pass; it is redundant work in the shipped binary, not a second effect.

## The matrix layout

`00414DB0` `BSP_EntityPose_RefreshWorld` is the producer of the cached matrix at `entity+0CCh`, and
it settles the storage order of both matrices in one expression (`00414DD2..00414DF1`):

```
if (byte [ESI+0C8h] != 0) return;
if ([ESI+3Ch]) 00414DB0(parent);                    ; recurse to the root first
if ([ESI+3Ch]) { PUSH parent+0CCh; PUSH &temp; ECX = ESI+74h; 00413920 }   ; temp = local * parentWorld
else           { EAX = ESI+74h }
004134F0(&entity->world_CC, EAX);
byte [ESI+0C8h] = 1;  byte [ESI+10Ch] = 0;
```

`00413920`'s convention is `ECX = left`, stack `(dst, right)`, established byte-exactly in
`docs/NATIVE_CAMERA_MATRIX_MATH.md`. So the composition is `world = local * parentWorld`, a
child-times-parent product. That is the row-vector convention: row-major storage with the
translation in the **last row**. The column-vector convention would need `parent * local`.

The translation row is corroborated from a second, independent producer:
`include/bsp/mission_entity_lua_attach.hpp` records `kEntityPoseXOffset = 0FCh`,
`kEntityPoseYOffset = 100h`, `kEntityPoseZOffset = 104h` from `00928DA4`, `00928D9C` and
`00928DB4`. `0FCh` is `0CCh + 30h`, the fourth row of the world matrix. By the same stride the
local matrix's translation is at `74h + 30h = 0A4h`.

| Entity offset | Bytes | Field | Evidence |
| --- | --- | --- | --- |
| `+74h` | 64 | local 4x4, row-major | `004134F0` at `0043141B`; `REP MOVSD` of `10h` dwords at `009559D4` |
| `+0A4h` | 12 | local translation row | `74h + 30h`, the stride of `0CCh -> 0FCh` |
| `+0C8h` | 1 | world matrix valid | set at `00414DF6`, cleared at `00431420` |
| `+0CCh` | 64 | cached world 4x4 | written by `004134F0` at `00414DF1` |
| `+0FCh` | 12 | world translation row | `00928D9C`, `00928DA4`, `00928DB4` |
| `+10Ch` | 1 | second cache byte, meaning unresolved | cleared at `00431427`, `00414DFD`, `0042ED64` |
| `+3Ch` | 4 | transform parent | `00414DBF`, `00955986` |
| `+44h`/`+48h` | 4 | next sibling / first child | `0043143C` / `0043142E` |
| `+310h` | — | notified sub-object | `006E00D4` |

`+10Ch` is cleared everywhere the world matrix is invalidated *and* everywhere it is refreshed
(`00414DFD` clears it right after setting `+0C8h`), so it is a cache of something derived from the
world matrix, not a second copy of the same validity bit. `00414E10`
`BSP_EntityPose_GetDerivedAffineInverse` is the likeliest owner; that is not established here.

## The producers

| Producer | Site | What reaches `+74h` | Owner |
| --- | --- | --- | --- |
| matrix interpolator pass `00904600` | `00904AE3` | the composed interpolated matrix, four chained `00413920` | `docs/WORLD_TIMED_ATTACHMENTS.md` |
| a spawn/placement routine with **no Ghidra function** | `006E5D03` | a stack matrix built from the `00D7A24C` identity constant, after the slot `98h` placement at `006E5CFF` | unowned; see the follow-up table |
| unit attach `00925CE0` | `009259C4` | identity twice (`00925DCA`, `00925EA0`), then the creator's `localFrame` | `docs/UNIT_INSTANCE_LAYOUT.md` (packet `cc2-*`) |
| ship motion `00825F20` | — | **nothing**: see below | packet `cc_controlled_unit` |

`00825F20` `BSP_UnitInstance_UpdateShipMotion`, body `00825F20..00826D6B`, is the routine the
packet brief expected to build the local matrix from the position and heading. It does not write
it. Filtering its whole Ghidra listing for a store into `[reg+74h]` through `[reg+0B3h]` returns
nothing; the ten `LEA ECX,[ESI+74h]` sites are all the *left operand* of a `00413920` call inside
an inlined copy of `00414DB0` (`008264A8`, `00826503`, `00826567`, `008265C5`, `008267E5`,
`00826838`, `008268E5`, `00826BB5`, `00826C12`, `00826CA5`, each followed by the
`byte +0C8h = 1; byte +10Ch = 0` pair). The routine reads the local matrix nine times and
invalidates the pose once, at `00826798..008267B6`, with the setter's own clear-and-walk block
inlined but **without** the `+310h` notification. The write it invalidates for is in one of its
callees; which one is not established here and is left to `cc_controlled_unit`.

## Host methods, in native call order

| # | Call site | Native | Host method | Owner |
| --- | --- | --- | --- | --- |
| 1 | 00904AE3 | vtable `88h` | `entity_set_local_matrix` | `docs/WORLD_TIMED_ATTACHMENTS.md` |
| 2 | 0043141B | 004134F0 | `copy_local_matrix` | this packet |
| 3 | 00431420 | — | `clear_pose_cache_flags` (`+0C8h`, `+10Ch`) | this packet |
| 4 | 0043142E | — | `first_child` = `entity+48h` | this packet |
| 5 | 00431437 | 0042ED50 | `invalidate_subtree_pose` | `docs/WORLD_ENTITY_UPDATE.md` |
| 6 | 0043143C | — | `next_sibling` = `child+44h` | this packet |
| 7 | 006E00E3 | vtable `0Ch` of `entity+310h` | `notify_local_matrix_owner`, contract unread | this packet |

Steps 2 through 6 are `00431410`'s whole body and are `006E00A0`'s first five steps at
`006E00AC`, `006E00B6`/`006E00BD`, `006E00B1`, `006E00C8` and `006E00CD`.

## Reconstruction

`set_entity_local_matrix_00431410` and `set_entity_local_matrix_006e00a0` are sequence routines
over `EntityLocalMatrixHost`, one pure virtual per native call site. `write_identity_matrix_0042d700`
and `entity_has_derived_local_matrix_0042b9a0` are the two trivial slot bodies as pure rules. None
is a drop-in binary replacement: the natives are `__thiscall` members on partly recovered layouts.

| Routine | Coverage |
| --- | --- |
| `00431410` | complete |
| `006E00A0` | complete |
| `0042B9A0` | complete |
| `0042D700` | complete |
| `00414DB0` | complete, already reconstructed as `refresh_pose_00414db0` (`bsp/pose_refresh.hpp`) |
| `0042D770` | partial: only `0042D770..0042D77F` read, enough to see it is a second identity writer of the same shape. Slot `94h` is not otherwise touched. |

## Corrections

### To `docs/WORLD_TIMED_ATTACHMENTS.md`

Step 7 describes `00904AEF..00904B13` as the pass invalidating the pose cache. That is right, but
it is not additional work: the same three effects already happened inside the slot `88h` call at
`00904AE3`. The pass performs the clear and the child walk twice per applied record.

### To `include/bsp/unit_instance.hpp` and `docs/UNIT_INSTANCE_LAYOUT.md`

`kUnitOffPoseBlock = 0x0CC` is described as a "pose block" with sub-fields `kUnitOffPoseLateral`
at `0F0h` ("`[pose+24h]`") and `kUnitOffPoseBase` at `100h`. The producer `00414DB0` shows what it
actually is: a 64-byte row-major **world matrix**, `local * parentWorld`, spanning `0CCh..10Bh`.
The two named fields are elements of it, not fields of a record: `0F0h` is row 2 column 1 and
`100h` is row 3 column 1, the translation's Y. The constants are correct; their meanings are not.
`bsp/mission_entity_lua_attach.hpp`'s `0FCh`/`100h`/`104h` X, Y, Z reading is the consistent one.

### To this packet's own brief

The brief expected the local matrix to be built from position and heading inside `00825F20`. It is
not; see The producers above.

## Follow-up packets

| Packet | Addresses | Files | Contract |
| --- | --- | --- | --- |
| `entity_local_matrix_owner` | entity+310h, its vtable slot `0Ch` | docs/ENTITY_LOCAL_MATRIX_OWNER.md | What the `006E00A0` tail notifies. The sub-object's vptr is `[entity+310h]` and `ECX` is `entity+310h`; neither the class nor slot `0Ch` was read |
| `entity_derived_cache_10c` | entity+10Ch, 00414E10 | docs/ENTITY_DERIVED_CACHE.md | What `+10Ch` guards. It is cleared by every invalidator and also by `00414DFD` immediately after `+0C8h` is set |
| `unit_local_matrix_writer` | 00825F20's callees, 0092E5B0, 00933BB0, 00953CC0 | — | Which callee of the ship motion writes `entity+74h`; `00825F20` itself only reads it and invalidates. Belongs to `cc_controlled_unit` |
| `entity_spawn_local_frame` | 006E5D03 and its containing routine | — | The slot `88h` producer at `006E5D03` sits in a region with no Ghidra function. `FUN_006E5A80`'s body is `006E5A80..006E5AAB` and does **not** contain it, so the containing routine is unresolved and the site is cited by address only |

## no_ghidra_function

| Start | Inclusive end | Evidence for each boundary |
| --- | --- | --- |
| 00431410 | 00431446 | Start: `MOV EAX,[ESP+4]` after INT3 padding; the previous function's slot-`88h` neighbourhood is unclaimed and `00431410` is the target of seven vtable dwords. End: `RET 4` (`C2 04 00`) at `00431444`, then INT3 `00431447..0043144F` and `FUN_00431450` |
| 006E00A0 | 006E00E9 | Start: `MOV EAX,[ESP+4]`; `FUN_006E0060`'s Ghidra body ends at `006E0095`, INT3 `006E0096..006E009F`. End: `RET 4` at `006E00E7`, INT3 `006E00EA..006E00EF`, then `BSP_BattleshipClass_ReadLuaFields` at `006E00F0` |
| 0042B9A0 | 0042B9A2 | Start: target of sixteen vtable `+8Ch` dwords. End: `XOR AL,AL` (`30 C0`) then `RET` (`C3`), INT3 `0042B9A3..0042B9AF`, then `XOR EAX,EAX; RET` at `0042B9B0` |
| 0042D700 | 0042D760 | Start: `MOV EAX,[ESP+4]` after INT3 `0042D6F5..0042D6FF`. End: `RET 4` at `0042D75E`, INT3 `0042D761..` |

`0042D770` is read but not named or bounded by this packet.

## State per address

| Address | Name recorded | State |
| --- | --- | --- |
| 006E00A0 | BSP_Entity_SetLocalMatrixAndNotify | analyzed, reconstructed, build-tested |
| 00431410 | BSP_Entity_SetLocalMatrix | analyzed, reconstructed, build-tested |
| 0042B9A0 | BSP_Entity_HasDerivedLocalMatrixStub | analyzed, reconstructed, build-tested |
| 0042D700 | BSP_Matrix_ReturnIdentity4x4 | analyzed, reconstructed, build-tested |

Nothing in this packet is fixture-tested, ABI-compatible or game-validated.

## Uncertainties

* `entity+310h`'s class and its vtable slot `0Ch` are unread. The host method is named by address.
* `entity+10Ch`'s meaning is unresolved; only its write sites are established.
* The `006E00A0` vs `00431410` split is by vtable, not by a read class hierarchy: no RTTI ships in
  this image (`docs/GAME_WORLD_ENTITIES.md`), so "base" and "derived" here are inferences from one
  body being a prefix of the other.
* `0042D770` (slot `94h`) was read for four instructions only.
