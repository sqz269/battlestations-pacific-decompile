# Native render-target group ownership

Discovery only, based on `f9f9cd9`. The native group has a complete bounded
constructor, assignment and destruction contract. Closing the five-group
shadow-owner path still requires the real surface-wrapper lifetime and its
canonical pool. `D3D9FrameTargets` and `D3D9SurfaceBinding` alone do not provide
that lifetime. No retention adapter or surface destructor was invented here.

`reports/native_render_target_group_next.json` contains 50 installed-PE/Ghidra
byte comparisons, 16 source snapshots, proposed names, original ABIs, and the
next pool packet. Every live Ghidra query verifies project `bsp`, program
`/battlestationspacific.exe`, x86 image base `00400000` before analysis.
Names proposed here are descriptive hypotheses; this packet does not rename
Ghidra, change ledgers, add C++, or claim build/runtime validation.

## Object and constructor

`00B1FBB0..00B1FBF2` is a 66-byte constructor: ECX is caller-supplied storage,
EAX returns that same address, and it ends with plain RET. Shadow construction
allocates exactly `40h` with `00BF681B`; the group constructor allocates nothing
and has no EH frame. Its two established virtual entries at `00D5E600` are
`+00=00BD30E0` and `+04=00B1FCF0`.

| Offset | Established field and constructor action |
| --- | --- |
| `+00` | Write reference-base table `00CEB130`, then group table `00D5E600`. |
| `+04` | Intrusive signed 32-bit count, initialized to one. |
| `+08,+0C,+10,+14` | Four retained surface-wrapper identities, initialized null. |
| `+18` | Retained depth surface-wrapper identity, initialized null. |
| `+1C,+20,+24` | Raw 16-byte-record array pointer, signed count and capacity; all zero. |
| `+28,+2C,+30,+34` | Four independently owned COM interface references, initialized null. |
| `+38` | Independently owned depth COM interface reference, initialized null. |
| `+3C` | Exact sRGB-write byte, initialized zero. |
| `+3D..+3F` | Preserved allocation preimage; the constructor does not write these bytes. |

The constructor writes depth/array/depth-COM/sRGB fields first, then each
color-wrapper/COM pair. Zero-initializing the entire 40h allocation would erase
the preserved final three bytes. The cached COM fields are separate from the
COM reference held by each surface wrapper; a complete owner must release both.

## Assignment and device binding

`00B1FAB0..00B1FAF1` receives ECX=group and stack `(index, wrapper)`, RET8.
`00B1FB00..00B1FB3B` receives ECX=group and stack `(wrapper)`, RET4.
Both capture the old pointer, skip identical assignment, publish the new
pointer, interlocked-increment its count if nonnull, then interlocked-decrement
the captured old count. On zero they load the old object's current virtual
`+00` and invoke it. `00BD30E0` subsequently loads current virtual `+04` and
calls it with flag one. An old-owner callback therefore observes the new
published field, and a reentrant replacement survives the outer setter.

These setters do not update `+28..+38`, call COM, or bind a device. The getters
at `00B1F6D0` and `00B1F6E0` return borrowed wrapper pointers without retaining;
the former is RET4 with a stack index, the latter plain RET. Native color
indexing has no bounds check; a typed implementation should explicitly limit
its supported domain to indices zero through three.

`00B1F700` stores the low byte of its stack argument at `+3C` and RET4;
`00B1F710` returns that byte in AL, with no promise about the upper EAX bits.
`00B1F6F0` returns the address of the actual `+1C` array header.

The already reconstructed renderer binding path `00B24E70` consumes the live
wrapper getters and exact sRGB byte. Its color/depth helpers `00B23D80` and
`00B21690` read COM surface `wrapper+2C`, issue the real device operation, and
ignore native HRESULTs. They do not populate group `+28..+38`. This discovery
does not establish every possible writer of those caches, nor that they are
always null. A new cache-refresh operation in these setters would add behavior.

## Complete destruction, callbacks and EH

The complete body is `00B1FC00..00B1FCEF` (239 bytes), including the return
after the array `_free` that current Ghidra pseudocode omits. It installs the
group table, then processes these operations in order:

1. For each color slot zero through three, capture the wrapper, decrement its
   count and invoke current virtual zero on zero; clear its field after the
   callback. Then freshly load the paired COM field, call COM virtual `+08`
   (`Release`) if nonnull, and clear that field after the call.
2. Perform the same wrapper-then-COM sequence for depth `+18/+38`.
3. Resize the actual raw record array to zero through `00B1F9F0`; free its
   current pointer through `00BF6989`; call `00BD30F0` to install reference-base
   table `00CEB130`; restore the EH frame and return.

Every later field load occurs after earlier callbacks. Do not snapshot all
attachments before releasing them. In particular, wrapper destruction can
change its paired cached COM field before that field is loaded. Clearing is
unconditional after a nonnull captured object's callback; repopulating that
same field during the callback does not prevent native clearing. The COM
`Release` return value is unused, and no COM error result stops later releases.

The destructor does not reset the group's count or sRGB byte. The record count
becomes zero; the freed record pointer and capacity are not cleared. Its
bounded behavior must not acquire extra implicit cleanup through C++ member
destructors or a second copied ownership structure.

The EH handler `00CBCCE3` selects FuncInfo `00DF5214`; its map at `00DF5204`
has state 0 -> `00CBCCD0` -> base destructor and state 1 -> `00CBCCD8` ->
array destructor `00B1FB90`, then state 0. The normal body enters state 1
before releasing attachments, changes to state 0 before array destruction,
and -1 before the base destructor. If an attachment callback unwinds, the
map cleans the array and base; it does not continue releasing all remaining
attachments. A host contract requiring nonthrowing terminal-release callbacks
should state that limit explicitly.

Deleting wrapper `00B1FCF0..00B1FD0E` (30 bytes) calls the complete destructor,
conditionally calls ordinary free `00BF65AC` for stack flag bit zero, returns
the original address in EAX, and RET4. It includes `ADD ESP,4` after free.
This group is not returned to the surface or texture pools.

## Raw record array

The meaning of each record's four DWORDs is unresolved. No record operation
retains or releases a pointer, and no element destructor runs when shrinking.

| Function | Complete range, ABI and established operation |
| --- | --- |
| `00B1F970` | Through `00B1F9F0`, 128 bytes; ECX=array, stack requested capacity, RET4. Signed minimum one; reserve only if larger than current capacity; allocate wrapped `capacity*16`, copy each live record as four ordered DWORD loads/stores, free old array, then publish pointer/capacity. |
| `00B1F9F0` | Through `00B1FA48`, 88 bytes; ECX=array, stack new count, RET4. Reserve when required, initialize added records to four zero DWORDs, decrement count repeatedly when shrinking, finally store requested count. |
| `00B1FA50` | Through `00B1FA99`, 73 bytes; ECX=array, stack pointer-to-record, RET4. If count equals capacity, signed wrapped doubling with minimum one; resolve destination after reserve, copy four DWORDs, increment count even if the wrapped destination compares null. |
| `00B1FB40` | Through `00B1FB8C`, 76 bytes; ECX=group, stack record pointer, RET4. Inlined append using that group's actual `+1C` array header. Currently not defined as a Ghidra function. |
| `00B1FB90` | Through `00B1FBA7`, 23 bytes; ECX=array, plain RET. Resize zero, reload and free pointer, return after `ADD ESP,4; POP ESI`. |

Reserve's omitted `00B1F9E1..00B1F9EB` continuation adjusts the stack, restores
EDI, publishes the allocated pointer and requested capacity, and restores EBP.
Its absence from pseudocode is not an early return. Valid typed storage must
have nonnegative count <= capacity and fit Win32 address/size arithmetic.
An append input pointing into storage that reserve frees is not a supported
safe source alias. None of these algorithms justifies inventing record meaning.

## Five groups in the shadow constructor

`00A8FBE0..00A8FC9A` captures the global target's color surface once through
`00A8FDA0`, allocates/constructs four groups at shadow `+4F0..+4FC`, then a
fifth at `+500`. Each group receives that captured color in slot zero. The
global target and its depth surface `00A8FDC0` are reloaded for each group,
and the group field is reloaded before depth assignment. No extra creator
release follows group publication: the newly initialized count-one reference
is the shadow's ownership.

Derived-shadow EH states one and two free the current raw 40h allocation,
then continue to state zero's complete shadow destructor. The group constructor
itself has no throwing operations. The report retains the exact funclets and
the derived unwind map; no broad shadow-constructor implementation is claimed.

## Existing code and the production surface gap

`RenderCommandReference` provides actual intrusive host ownership and current
terminal-release dispatch. It is not proof that an arbitrary projected surface
has its native destructor. `D3D9FrameTargets` uses shared ownership of real COM
bindings for existing renderer use, but lacks this raw array, independent COM
caches and native group lifetime. `D3D9ResetTexture2D` owns a COM texture while
its cached level wrappers are borrowed. Its reset callbacks do not implement
retained `00B3FD80` surface returns or wrapper pool deletion.

For actual `00D619A0` surfaces, zero count reaches `00B3F5B0 -> 00B3F4E0`:
install the surface table; remove its pointer from the current renderer's
borrowed reset array; ensure the real resource-support singleton; freshly load
and release COM `+2C`; inspect current flags `+28` and decrement tracking global
`0108DAFC` for `10h/100h`; return diagnostic string storage; install the base
table. Flag-one deletion then returns the actual slot to pool `0108DB00` via
`00B3D860`. A COM-only deleter omits those operations.

The resource-support getter `00B3E730` is a real singleton lifecycle side effect,
not surface registration. `docs/SURFACE_REGISTRATION_AUDIT.md` distinguishes
explicit factory registration, generic singleton ownership and allocation
pooling; the corresponding current native bytes were rechecked here.

`00B3FD80` first searches the texture's eight-byte cached level records and
retains a hit. On a miss it calls real COM `GetSurfaceLevel`, allocates from
`0108DB00`, constructs the surface with `00B3F630`, releases the temporary COM
reference, and, unless texture flag bit zero is set, retains a separate cache
reference and appends the level/wrapper pair. Native HRESULTs are ignored;
there is no sound failure fallback inferred from the uninitialized output path.
These operations remain separate from target-group implementation.

## Approved next packet: surface allocation pool only

The independent pool packet owns eleven functions: `00B3EC60` initialize,
`00B3ED40` allocate, `00B3D120` slab construct, `00B3D860` return slot,
`00B3E390` virtual-zero trim, `00B3E2B0` destroy, `00B3D370` unwind table free,
`00B3F2A0` allocation thunk, `00B3DCC0` ECX-slot return thunk, `00CD7B40`
static initializer and `00CE0C90` static destructor. EH spans `00CBEE00..28`
and `00DF764C..7688` remain evidence only; the root owns Ghidra/ledger edits.

Use exactly four new files: `include/bsp/d3d9_surface_pool.hpp`,
`src/d3d9_surface_pool.cpp`, `docs/D3D9_SURFACE_POOL.md`, and
`reports/d3d9_surface_pool_audit.json`. Bind the actual canonical pool
`0108DB00` and the same `AllocatorListDomain`/`00E188B4` head already used by
the directional-light pool. The pool profile is `00D6193C`, virtual zero
`00B3E390`; no second registry or allocator list is needed.

Its `38h` storage has allocator links `+00..08`, real critical section `+0C`,
recursion `+24`, slab table/count/capacity `+28/+2C/+30`, first-free index
`+34`. Initial table capacity is 32. Each `744h` slab holds 32 slots of `38h`;
the actual surface occupies `34h`, with slot-to-slab index in the trailing
DWORD `+34`. Free indices occupy slab `+700..+73F`, free count `+740`.
Initialization preserves the other slot bytes.

Trimming an empty slab frees it, swaps in the last slab, decrements count,
updates all 32 moved slot IDs, and revisits the index before scanning again.
It then recomputes first-free. All of this follows an omitted free-call tail.
Allocation likewise continues after `00B3EDD1` through stack adjustment and
table publication. Destruction frees every slab and the table, drains the
current critical-section recursion, deletes that section, and unlinks the
actual allocator-list node. It does not invoke any surface destructor.

One focused native differential fixture should cover real stride/ID/slab
geometry, movement and update of all 32 IDs, returned slots, and cleanup tails.
Full surface construction, COM operations, reset/texture ownership and target
group C++ remain later work. This discovery ran no build or runtime test.
