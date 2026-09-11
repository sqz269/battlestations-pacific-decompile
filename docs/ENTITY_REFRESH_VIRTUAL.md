# The entity refresh virtual, vtable slot 0D8h

Addresses: 00955970, 0042B9A0, 0042D700, 00414DB0, 004134F0, 00413920, 00CFC3D0 (data),
007BB4B8 (tail jump).

Packet `cc_entity_matrix`, worktree `agent/cc-entity-matrix`. Ghidra was **read-only**. Every
descriptive name is a hypothesis, not a recovered symbol. This packet takes the
`entity_refresh_virtual` follow-up of `docs/WORLD_ENTITY_UPDATE.md`.

`00955970` has **no Ghidra function**. It was read from the disk listing with `disasm-raw`; its
boundaries and their evidence are in the `no_ghidra_function` table below. The reconstruction is
`refresh_entity_00955970` in `include/bsp/entity_local_matrix.hpp` and
`src/entity_local_matrix.cpp`.

## The routine

`void __thiscall(entity)`, no stack arguments, plain `RET` on all three exits. Body
`00955970..00955A37`. It is vtable slot `0D8h` in sixteen vtables, `MDestroyer`'s `00CFC3D0`
among them, and it is what the matrix-interpolator pass calls at `00904B2A` to close each record.

```
00955970  SUB ESP,0C0h ; PUSH EBX ; EBX = this
00955979  if ([EBX+4A4h] == 0) goto tail                     ; no scene node: do nothing
00955986  if ([EBX+3Ch] != 0) {                              ; parented
0095598C      if (byte [EBX+0C8h] == 0) 00414DB0(this)       ; refresh the cached world matrix
0095599A      node = [EBX+4A4h]
009559A2      node->vtable[34h](EBX + 0CCh)                  ; publish the cached world matrix
009559B5      return
          }
009559B6  if (!this->vtable[8Ch]()) {                        ; root without a derived local
00955A1F      node = [EBX+4A4h]
00955A2A      node->vtable[38h](EBX + 74h)                    ; publish the local matrix itself
00955A30  tail: ADD ESP,0C0h ; RET
          }
009559C4  local = *(Matrix4x4*)(EBX+74h)                     ; REP MOVSD, 10h dwords
009559DA  PUSH &local ; PUSH &product                        ; 00413920's two stack arguments
009559ED  PUSH &derived ; ECX = EBX ; CALL vtable[90h]        ; returns &derived in EAX
009559F4  ECX = EAX ; CALL 00413920                          ; product = derived * local
009559FA  004134F0(&local, EAX)                              ; local = product
00955A03  node = [EBX+4A4h]
00955A0E  node->vtable[38h](&local)
00955A1E  return
```

### The three pushes are not three arguments

`009559DA`, `009559DF` and `009559ED` push `&local`, `&product` and `&derived`, and then one call
is made. It looks like a three-argument virtual. It is not: every one of the sixteen vtables holds
`0042D700` in slot `90h`, and `0042D700` ends `RET 4`. A three-argument `__thiscall` would need
`RET 0Ch` and the stack would be left 8 bytes short at `009559F4`.

`0042D700` is the MSVC "return a struct by value" shape: it takes the caller's return buffer as its
only stack argument, writes an identity 4x4 into it and hands the buffer back in `EAX`. So slot
`90h` is `Matrix4x4 __thiscall()` with a hidden buffer, and the first two pushes belong to the
`00413920` call that follows, whose arguments are already in place when it is reached. `00413920`
reads them at `[ESP+44h]` and `[ESP+48h]` after its own `SUB ESP,40h`, giving `dst = &product` and
`right = &local` with `ECX = left = &derived`; the convention `ECX = left`, stack `(dst, right)` is
the byte-exact one of `docs/NATIVE_CAMERA_MATRIX_MATH.md`.

### The derived branch is unreachable in the shipped image

The gate at `009559B6` is `this->vtable[8Ch]()`, a `bool`. Reading `+8Ch` out of all sixteen
vtables that carry slot `0D8h` gives `0042B9A0` in every one, and `0042B9A0` is `XOR AL,AL; RET`.
Slot `90h` is likewise `0042D700` in all sixteen and slot `94h` is `0042D770` in all sixteen.

So `009559C4..00955A1E` never executes in this image, and `refresh_entity_00955970` marks it
`kDerivedLocal` but no shipped class reaches it. It is reconstructed because the listing was read,
not because a call site reaches it. A class that overrode slot `8Ch` would compose
`derived * local` and publish that instead of the bare local.

### The two scene-node slots differ

A parented entity publishes through slot `34h` and passes the **address** of the entity's cached
world matrix (`LEA EAX,[EBX+0CCh]`), so the node holds a pointer into the entity. A root entity
publishes through slot `38h` and passes a matrix the node must copy: `EBX+74h` on the plain path,
a stack temporary on the derived path. The two slots are not interchangeable and neither body was
read; both host methods are named by their slot.

### The null-scene-node exit

`00955979` is the only test that can skip everything. It shares the epilogue at `00955A30` with the
plain-local exit, which is why the `ADD ESP,0C0h` appears at three addresses in two shapes: the
`009559B5` and `00955A1E` exits pop the registers they pushed (`EBX` alone, and `EBX`/`ESI`/`EDI`
respectively), and the shared tail pops only `EBX` because `ESI` and `EDI` are pushed at
`009559C6`/`009559C7`, after the branch that reaches it.

### The tail jump at 007BB4B8

`JMP 00955970` at `007BB4B8` is a tail call from inside an unnamed function. Ghidra reports no
references to `007BB4B8`, so it is reached by fall-through within its own body, not by a call or a
vtable dword. This packet did not identify the containing routine.

## Host methods, in native call order

| # | Call site | Native | Host method | Contract |
| --- | --- | --- | --- | --- |
| 1 | 00904B2A | vtable `0D8h` = 00955970 | `entity_refresh` | this packet |
| 2 | 00955979 | — | `scene_node` = `entity+4A4h` | `kUnitOffSceneNode` |
| 3 | 00955986 | — | `parent` = `entity+3Ch` | this packet |
| 4 | 0095598C | — | `world_matrix_valid` = byte `entity+0C8h` | `kUnitOffPoseValid` |
| 5 | 00955995 | 00414DB0 | `refresh_world_matrix` | `bsp/pose_refresh.hpp` |
| 6 | 009559AC | node vtable `34h` | `scene_node_set_pose`, contract unread | this packet |
| 7 | 009559BE | vtable `8Ch` = 0042B9A0 | `has_derived_local_matrix`, always false | this packet |
| 8 | 009559D4 | — | `read_local_matrix` = 16 dwords at `entity+74h` | `docs/ENTITY_LOCAL_MATRIX.md` |
| 9 | 009559F0 | vtable `90h` = 0042D700 | `derived_local_matrix`, by-value return | this packet |
| 10 | 009559F4 | 00413920 | `multiply_matrices` | `docs/NATIVE_CAMERA_MATRIX_MATH.md` |
| 11 | 009559FE | 004134F0 | folded into the reconstruction's copy loop | `docs/NATIVE_CAMERA_MATRIX_MATH.md` |
| 12 | 00955A13, 00955A2E | node vtable `38h` | `scene_node_set_world_matrix`, contract unread | this packet |

## Reconstruction

| Routine | Coverage |
| --- | --- |
| `00955970` | complete: all three exits and both root branches are modelled |
| `0042B9A0` | complete |
| `0042D700` | complete |

`refresh_entity_00955970` returns which exit it took so a host can assert the branch. It is not a
drop-in binary replacement.

## Corrections

### To `docs/WORLD_ENTITY_UPDATE.md`

Its follow-up row for this packet says `00955970` "reads `entity+4A4h`, `entity+3Ch` and the pose
block at `+0CCh` and calls the scene node's vtable `34h`". That is the parented branch only. The
routine has two further exits, and `+0CCh` is a world matrix, not a pose record
(`docs/ENTITY_LOCAL_MATRIX.md`).

### To the natural reading of `009559DA..009559F0`

Nothing in the repository asserted it, but the obvious reading of the three pushes is a
three-argument slot `90h`. The `RET 4` in `0042D700` rules it out. Recorded here so the next reader
does not have to re-derive it.

## Follow-up packets

| Packet | Addresses | Files | Contract |
| --- | --- | --- | --- |
| `scene_node_pose_slots` | scene-node vtable `34h` and `38h` | docs/SCENE_NODE_POSE_SLOTS.md | The two publish paths: slot `34h` takes a pointer into the entity, slot `38h` takes a matrix to copy. Neither body is read. Scene-graph code is Codex-owned |
| `entity_refresh_tail_jump` | 007BB4B8 and its containing function | — | Which routine tail-jumps into `00955970`; Ghidra has no function covering the jump |
| `entity_derived_local_override` | vtable `8Ch` across all vtables | — | Whether any class outside the sixteen overrides slot `8Ch`, which would make `009559C4..00955A1E` live |

## no_ghidra_function

| Start | Inclusive end | Evidence for each boundary |
| --- | --- | --- |
| 00955970 | 00955A37 | Start: `SUB ESP,0C0h` and the target of sixteen vtable `+0D8h` dwords; `FUN_00955830`'s Ghidra body ends at `00955965` and `00955966..0095596F` is INT3 padding. End: `RET` (`C3`) at `00955A37` after `ADD ESP,0C0h`, then INT3 `00955A38..00955A3F` and `FUN_00955A40` |

`0042B9A0` and `0042D700`, also without Ghidra functions, are bounded in
`docs/ENTITY_LOCAL_MATRIX.md`.

## State per address

| Address | Name recorded | State |
| --- | --- | --- |
| 00955970 | BSP_Entity_RefreshSceneNodeMatrix | analyzed, reconstructed, build-tested |

Nothing in this packet is fixture-tested, ABI-compatible or game-validated.

## Uncertainties

* The scene node's vtable slots `34h` and `38h` are read from their call sites only.
* No run-time evidence: `bsp_game.exe` does not yet reach this path with a live entity, so the
  branch counts are static (checklist rule 6 is not satisfiable here yet). When it does, the
  expected outcome for a parented `MDestroyer` is `kParentedPose` and for a root one
  `kPlainLocal`, never `kDerivedLocal`.
* The vtable-slot values were read from the sixteen vtables that carry slot `0D8h`. A class outside
  that set could override slot `8Ch`; nothing was found, but the search was over these sixteen.
