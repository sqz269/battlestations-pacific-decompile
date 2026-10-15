# Actual scene-record map iterator decrement

| Native body | Bytes | Coverage | Original interface |
| --- | ---: | --- | --- |
| `004678F0..00467978` | 137 | complete Source | ECX actual eight-byte iterator; RET; result undefined |
| `00468200..0046820B` | 12 | complete Source | ECX iterator; EAX same iterator; RET |
| `004688E0..004688EB` | 12 | complete Source | ECX iterator; EAX same iterator; RET |

The descriptive scene-map role is provisional. The iterator has owner+0 and
node+4; nodes use left+0, parent+4, right+8 and sentinel byte+19h. The container
producer and insertion layouts are documented in `SCENE_RECORD_MAP.md`.
This Source borrows actual raw storage, without a container or iterator overlay.

The initial zero-owner check at `004678F3` invokes the existing concrete
`00BF6713` Source provider. It may return: the iterator's node is loaded only
afterward. For a sentinel current node, publish its right child first; a
sentinel right child invokes validation and returns without another write.
For a non-sentinel left child, walk its rightmost non-sentinel descendant and
publish once. Otherwise walk parents while the current iterator node equals
the parent's left child, publishing each intermediate parent before advancing.
After this ascent, check the *current* iterator node's sentinel byte. Failure
invokes validation and returns without publishing the captured final parent.

The native invalid exits at `00467913` and `0046796F` are tail jumps. The new
Source calls the accepted real current-SDK CRT provider then returns; it does
not reproduce the original private frame, flags, fault address or CRT policy.
No noreturn/noexcept promise, callback, guard, rollback or invented error result
is introduced. Source getters/stores are volatile physical-word/byte accesses;
operand reads in the ascent are explicitly sequenced in listing order.

Both 12-byte wrappers call decrement and return their captured input pointer.
They have no live callers. The third live decrement caller is the insertion
routine: `0046B3EF` in `0046B340` sets ECX to its local eight-byte iterator at
ESP+14h. Its post-call node reload at `0046B3F4` observes intermediate writes.
The two remaining call sites are `00468203` and `004688E3`.

`reports/goal_oct15_scene_map_iterator.json` records byte comparison, call
verification and build qualification. The existing seed/math/tool tests do
not execute these three new entries. Native binary ABI, private CRT/FH3,
specific original/source runtime differential and gameplay remain unproven.

The combined Release Win32 `scripts/build.ps1` passed at
`026da884ef927effb85f118488357a83634eff48`, with all three configured existing
CTest entries passing. The four new translation units in this batch each
occur once in the actual `bsp_core.lib`. The transfer verifier accepts all
six typed edges here: four CALLs and two tail JMPs.
