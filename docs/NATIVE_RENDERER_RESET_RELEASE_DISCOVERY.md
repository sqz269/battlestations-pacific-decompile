# Renderer general cache/reset release discovery

Full B241C0 is 664 bytes, [B241C0,B24458); full B262C0 is 468 bytes,
[B262C0,B26494). Neither general body is source-ready. The remaining logical
vertex retained+4C lifetime is reached even by null unbinding, and the actual
production owner profile for renderer+38 is not established. This read-only
pass owns only those two addresses and this document/audit; no source, build,
runtime, original game or Ghidra changes were made.

50 fresh guarded spans / 3,994 bytes / 1,136 decoded instructions match the
original PE. The audit embeds complete bodies, missing alignment/free tails,
EH metadata, concrete profile words, positive writer chains, and frozen current
source/header pins. Stale descriptive ledger text is not treated as source status.

## General cache clear: 26 actual owner cells

ECX is cache=renderer+34, not the renderer base. The original returns RET and
has no EH registration or semantic result. For each cell it captures the owner;
null does not clear the cell. Nonnull decrements captured+4 with the real import;
zero reads the CURRENT captured profile/slot 0 and invokes ECX captured with no
stack flags. Only a returning terminal is followed by clearing the ORIGINAL
cell. A terminal callback's replacement of that cell is overwritten. Failure
leaves the current cell uncleared and stops later reset work, with earlier
changes intact. A noexcept companion is not an equivalent general terminal.

The following table lists every owner in execution order. Known profiles refer
to the admitted concrete source contracts, not arbitrary foreign profiles.

| Cache offset | Renderer offset | Actual role | Current profile domain |
| --- | --- | --- | --- |
| 0000 | 0034 | render-state block | 00d61a2c |
| 0004 | 0038 | block applying texture stage states; actual production owner identity unresolved | unresolved |
| 0008 | 003c | sampler-state block | 00d61a34 |
| 1780 | 17b4 | hardware vertex layout | 00d62af4 |
| 1784 | 17b8 | logical index stream | 00d61de0 |
| 1740 | 1774 | logical vertex stream 0 | 00d61d6c |
| 1750 | 1784 | logical vertex stream 1 | 00d61d6c |
| 1760 | 1794 | logical vertex stream 2 | 00d61d6c |
| 1770 | 17a4 | logical vertex stream 3 | 00d61d6c |
| 04d0 | 0504 | logical texture bank 0 | 00d61948, 00d61870, 00d618b0 |
| 057c | 05b0 | logical texture bank 1 | 00d61948, 00d61870, 00d618b0 |
| 0628 | 065c | logical texture bank 2 | 00d61948, 00d61870, 00d618b0 |
| 06d4 | 0708 | logical texture bank 3 | 00d61948, 00d61870, 00d618b0 |
| 0780 | 07b4 | logical texture bank 4 | 00d61948, 00d61870, 00d618b0 |
| 082c | 0860 | logical texture bank 5 | 00d61948, 00d61870, 00d618b0 |
| 08d8 | 090c | logical texture bank 6 | 00d61948, 00d61870, 00d618b0 |
| 0984 | 09b8 | logical texture bank 7 | 00d61948, 00d61870, 00d618b0 |
| 0a30 | 0a64 | logical texture bank 8 | 00d61948, 00d61870, 00d618b0 |
| 0adc | 0b10 | logical texture bank 9 | 00d61948, 00d61870, 00d618b0 |
| 0b88 | 0bbc | logical texture bank 10 | 00d61948, 00d61870, 00d618b0 |
| 0c34 | 0c68 | logical texture bank 11 | 00d61948, 00d61870, 00d618b0 |
| 0ce0 | 0d14 | logical texture bank 12 | 00d61948, 00d61870, 00d618b0 |
| 0d8c | 0dc0 | logical texture bank 13 | 00d61948, 00d61870, 00d618b0 |
| 0e38 | 0e6c | logical texture bank 14 | 00d61948, 00d61870, 00d618b0 |
| 0ee4 | 0f18 | logical texture bank 15 | 00d61948, 00d61870, 00d618b0 |
| 18d4 | 1908 | frame-target group | 00d5e600 |

Renderer176C/1770 are borrowed pixel/vertex shader cache identities, simply
zeroed at cache1738/173C. They are NOT among these 26 intrusive owners. The
first/third cells are positively traced from actual material pass 18/D61A2C and
pass 20/D61A34 via B4342C/B43437 and the full B27A80/B27B90 writers. Their complete
actual destructors and deleting wrappers already exist, with explicit valid
extent/shared heap/new-ABI boundaries.

A complete non-inventoried 130-byte body at B27B00 writes renderer 38 and applies
12-byte rows through full texture-stage setter B24510. This proves that writer's
behavior. It has no current Ghidra xrefs or literal DWORD pointer references.
D61A3C is positively constructed at material pass 1C, but matching row layout
does NOT prove a pass 1C-to-B27B00 production chain. That profile remains withheld.

The remaining known owners have complete actual context paths: hardware layout
D62AF4 -> B60770 and declaration D61D1C -> B48CA0; index D61DE0 -> B4C1F0 with
private/pooled physical profiles; the three texture kinds with their concrete
owner/pool services; frame group D5E600 -> B1FCF0 and actual surface lifetimes.
Each zero-reference BD30E0 action rereads current profile+4 and supplies flags 1.

After the first three owners, reset clears 210 validity bytes, the borrowed
shader cells, layout/index, and base-vertex cache1788=-1. Four stream records
clear 04/08/0C after owner release. Sixteen ACh banks clear eightDWORDs/onebyte
BEFORE each owner release; four additional constructed bank owners are preserved.
Twenty 48h records clear only DWORD0/4/8 and WORD C. Target release precedes 24
MOVSS zero stores in the original order (18D8,18E8,18DC,18E0,18E4,18EC..1934).
Plane storage178C..18CF,18D0, gamma1938 and unlisted gaps/value bytes survive.
The existing all-null fragment has a private constructor proof only.

## Full resource-release order and EH

B262C0 receives raw renderer in ECX and returns RET. It conditionally enters
the mutable0108D6DC guard. The ready1D8B CMP precedes state 0 arming; the false
path still disarms/leaves but does no resource/cache work. The true path clears
ready BEFORE any virtual calls. Current renderer profile/selector is reloaded
for each of 20 texture calls `(index,0)`, FOUR vertex calls all `(0,0)`, then one
index call `(0,0)`. D5F0A8 +130/+134/+138 are B24710/B24840/B24B00. Texture/index
are full raw providers; vertex is still only semantic and blocked in raw storage.

Current depth198C and four colors197C..1988 are null-tested and receive their
current +3C calls. Query list19A0/19A4 uses signed live count, reloaded base and
current indexed owner/slot 18. Texture1B00/1B04 and surface1B0C/1B10 loops retain
the old raw cursor across calls, then reread count FIRST and base second before
increment/end comparison. This is not a snapshot or safe indexed traversal.
The final 2Ch arithmetic-only loop reads count/base for its initial check, then
recaptures count once for the fixed end; it has no callback or element access.
Only then does full B241C0 run on renderer+34.

Surface D619A0/+3C=B3D510, query D62AD0/+18=B5FE20, 2D D61948/+20=B3DD30 and
cube/volume D61870/D618B0/+20=B33F10 are complete existing actual providers.
B33F10 is the genuine one-byte RET, not an invented callback. Dynamic release
B237D0, resource restore B23B10 and dynamic restore B1FD90 are also already full
raw source; they are not rediscovered blockers. The broader recreation graph's
hardware iterator/create and shader device-save/restore providers are complete.

FH3 handler CBD128 loads FuncInfoDF581C and jumps BF6B43. One unwind entry at
DF5814 is state -1/actionCBD120; that action takes guard at EBP-14 and jumps full
B21110. Cleanup reads current mode before any guard data and zero-extends its
byte before leave. Normal exits read current mode BEFORE disarm, then push the
saved fullDWORD (only AL byte was initialized), load saved renderer and call
B33B00. A normal-leave exception is not retried by this frame. Skipped entry
leaves native guard data uninitialized; no mode-transition repair is justified.

## Remaining lifetime chain and bounded next work

The four logical vertex owners use D61D6C slot 0 BD30E0 -> current slot 4 B4BF10
-> B4B5D0 -> B62010. The base captures retained+4C, decrements pointee+4, calls
its CURRENT slot 0 at zero and clears+4C only after return. Raw+50 allocation/free
is separate. The nonnull+4C writer, pointee profile, deleting terminal, allocation
domain and lifetime/EH remain unknown. Existing physical/declaration removal,
owner and pool-return providers do not establish that missing identity.

Null input to B24840 can release a nonnull old object. Its four reset calls all
target stream 0 and do not prove stream 1..3 null for final B241C0. Neither an
all-null reset, a generic callback framework, nor an available retained-memory
class chosen without a writer/profile chain closes the general release.

A separate source-ready 252-byte packet is proposed: full B27A80[122] and
B27B90[130], in four new native_renderer_material_state_binding files. Both have
positive actual producer/profile chains and complete existing lifetime/cached
setter providers. The new raw context would borrow sync globals and the concrete
two-word profile views, dispatch full actual deleting providers directly, and
preserve publish-before-retain/release, current row-base/count reloads and final
current counters. It would close two cache writers, not general reset. B27B00
is excluded until its production input profile is grounded. No new address
lease or implementation for this proposal belongs to this discovery commit.

For the primary lifetime blocker, a future finite pass can select at most 12 new
genuinely decoded non-stack writes/copies from the existing frozen 498-candidate
input, explicitly excluding prior 31 and subsequent 12 identities. It must prove
containment and receiver/escaped producer identity, stop at 12, and retain the
bounded-negative limit. This pass did not repeat those scans. A separate small
renderer 38 pass could resolve E8/E9/adjusted-alias callers and trace at most 6
concrete inputs; current negative xrefs do not establish dead code or lifetime.

Frozen discovery and source pins are under ignored
`local/renderer_reset_release_discovery/`. No build/runtime result is claimed.


Primary verified all 232 sealed worker files, 112 additional report pins and 50 freshly guarded spans (3,994 bytes). General clear/reset still lacks actual texture-stage profile provenance and logical-vertex retained4C owner terminal/lifetime. Four reset vertex calls all use(0,0). Shader cells176C/1770 are borrowed zeros. B27A80/B27B90 source work is separate. Immutable discovery evidence: `local/reset_release_discovery_primary/`. No source, original-body execution or gameplay claim is added by this review.
