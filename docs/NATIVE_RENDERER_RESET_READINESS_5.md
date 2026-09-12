# Current native renderer reset readiness

At source base `d24bf877`, the complete actual-storage parents **B29670,
B262C0, B241C0 and B24BF0 remain unimplemented**. Most leaves that older
discovery reports called missing are now present. The principal shared barrier
is full logical-vertex ownership through base `B62010` and its independent
nonnull `+4C` pointee. General cache composition and recreation's separate
record callbacks also need explicit profile closure.

This is a read-only source/dependency audit. The accompanying
[report](../reports/native_renderer_reset_readiness_5.json) pins the inspected
source/header contracts, selected current ledger results, and six fresh saved
Ghidra/installed-PE comparisons totaling 2,963 bytes. Every supported live query
verified `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe` first. No source,
Ghidra, metadata, build, test or game change is part of this packet. Original
bytes and source availability are not parent composition, ABI or runtime proof.

## Current graph

All four native parents receive their actual receiver in ECX, no stack
arguments, and return with plain RET. B241C0 receives **renderer+34**, not the
renderer base. Descriptive names remain hypotheses rather than recovered symbols.

| Parent | Complete body | Current dependency boundary |
| --- | --- | --- |
| B24BF0 reset bindings | 453 bytes | B24710 texture, B24B00 index, B23F20 layout and B23D80 color binding are actual providers. B24840 and its B23710 assignment still lack complete logical vertex destruction. Preserve three independent guard scopes and current renderer-table reads. |
| B241C0 clear cache | 664 bytes | Only B29430's private all-null construction fragment exists. General release must compose the 26 current owner cells below, with release-before-clear and propagation before later writes. |
| B262C0 release resources | 468 bytes | Concrete surface/query/texture reset callbacks now exist. Complete parent still needs B24840 and general B241C0; its guard, ready-byte publication and current registry traversal are not supplied by typed fragments. |
| B29670 recreate device | 1,191 bytes | Requires both incomplete release parents above, actual checked-tree and registry composition, explicit CRT service binding, and concrete record+28 virtual+28/+2C profiles. Its many completed leaves do not implement the parent. |

Both B24BF0 and B262C0 issue twenty texture bindings followed by **four
vertex calls with stream index zero**, then null index/base zero. They do not
unbind vertex streams 0 through 3. B26920 is a different pipeline with a
different stream-index loop; its older description must not replace these calls.

## Cache owner composition

Offsets here are relative to the cache at renderer+34. Nonnull cells decrement
the captured owner's actual LONG+04. At zero, current slot0 runs before the cell
is cleared. A throwing terminal leaves that cell uncleared and prevents later
work. No new B23710 assignment or binder call should replace this schedule:
binders publish a replacement before releasing the old owner and may touch COM.

| Cache cells | Current actual providers and remaining qualification |
| --- | --- |
| +00, +08 | Renderer +34/+3C are established render/sampler state owners. D61A2C/D61A34 select BD30E0 then B422F0/B42310; full raw destruction exists in `native_material_pass_states`. The binding implementation demonstrates current-profile rereads and throwing deletion without a companion object. |
| +04 | Renderer +38 has a complete independent 130-byte native binder at B27B00. Its rows are stage/state/value and call complete B24510. D61A3C/B42330 third-state destruction exists, but this audit does not establish a positive producer/caller mapping from that profile into this exact cell. See the bounded candidate below. |
| +1780 | Renderer +17B4: D62AF4 hardware layout, BD30E0/current B60770 and complete raw layout/tree/declaration/pool lifetime through `native_hardware_layout_owner`. |
| +1784 | Renderer +17B8: D61DE0 logical index, BD30E0/current B4C1F0 and complete logical/physical lifetime through `native_logical_index_owner`; pooled/private physical profiles are explicit. |
| +1740+i*10, i=0..3 | Renderer +1774 stream cells. Known D61D6C enclosing stream profile leads to B4BF10/B4B5D0/B62010. The independent retained pointee at logical+4C still has no established nonnull writer/current profile/final-zero terminal. |
| +4D0+i*AC, i=0..15 | Renderer +504 texture cells. D61948/D61870/D618B0 select complete actual 2D/cube/volume owners and their concrete nested pool, retained-stream, surface and name services. Current texture binder shows the profile-to-provider route. |
| +18D4 | Renderer +1908: D5E600 frame-target group, BD30E0/current B1FCF0, with full actual frame-target/surface/vector lifetime. |

The reset visits sixteen texture owner cells; construction prepares twenty.
It preserves the last four owners, gamma cache+1938, planes and sparse gaps.
The general routine cannot call the private all-null fragment unless its caller
independently establishes that fragment's complete precondition.

The newer [geometry](LOGICAL_VERTEX_ALIAS_GEOMETRY.md) and
[factory](LOGICAL_VERTEX_ALIAS_FACTORY.md) reports add a clone mapping path and
four adjusted registry-header EH aliases. They leave the logical+4C profile
open. Their bounded negatives are provenance here, not freshly repeated scans
or proof that +4C is always null.

## Leaves that are already available

| Route formerly listed as missing | Current source contract |
| --- | --- |
| B237D0 dynamic release | Full actual renderer/wrapper fields and optional guard in `native_dynamic_buffer_device_release`. |
| B24460/B24610/B26170 cached/default state | Complete actual renderer validity/value fields and real COM in `native_renderer_cached_states`. B24510 texture-stage state is separately complete. |
| B23B10 resource restore | Complete actual default acquisition and texture/surface/query traversal in `native_renderer_resource_restore`, preserving local output-cell and callback-sensitive reads. |
| B49D00/B49DC0/B49F80/B4A040 | Complete actual logical buffer save/restore entries and their explicit physical/COM/allocation dependencies, as recorded in current ledger lookups. |
| B5E750/B5E810/B5E890/B5E8E0 | Complete actual shader bytecode save/release/restore in `native_shader_device_reset`. |
| B600B0/B60A10 | Complete device declaration release and full hardware-layout CreateIfMissing. The earlier CreateIfMissing-only-fragment boundary is stale. |
| B21960 gamma | Complete new source interface over actual renderer, actual float argument cell and full raw pow context in `native_renderer_gamma`. B29670 still must preserve its own x87 load/store into that callee cell. |
| C2F1C0/C2F1C6 XLive | Explicit stdcall forwarding to the caller-selected loaded `XLiveLibrary`; native ordinals 5005/5006, no synthesized success. This does not prove SDK internals, live IAT compatibility or SDK runtime. |
| +1974/+1978 auxiliary restore | Concrete producer evidence and four pooled/private profiles exist in `native_renderer_reset_readiness`. B49180/B492B0 are full physical recreation; B4B810/B4B9C0 are evidenced RET4 leaves. |

B29670's inlined auxiliary sequence at B2992C..B29965 freshly matches the
important field order already implemented by B1FD90: ready/lost gates, first
wrapper and device load, ready=1, first current profile/slot call, then second
wrapper/profile, current device, current slot. This is supporting composition
evidence, not promotion of an inlined B29670 fragment or its whole parent.

B262C0's depth/four-color wrappers call D619A0+3C -> B3D510. Its query list
calls D62AD0+18 -> B5FE20. Its texture list calls D61948+20 -> B3DD30, or
D61870/D618B0+20 -> the genuine B33F10 no-op. Registered surfaces also call
B3D510. All these actual callbacks now exist. Its +1A78 record loop performs
no owner load or callback; do not confuse it with B29670's distinct calls.

## Remaining composition boundaries and next bounded work

1. **Logical vertex terminal provenance.** Identify an actual nonnull writer
   and pointee profile for logical+4C before reconstructing B62010, B4B5D0,
   B4BF10, B23710 and complete B24840. Constructor nullness and the enclosing
   stream table do not supply this. No new ready owner implementation follows
   from the two latest alias reports.
2. **Recreation record callback profiles.** B29670 walks renderer +1A78/+1A7C,
   stride 2C, and calls current owner at record+28 through current virtual+28
   on release and +2C on restore. A next read-only packet can start at these
   producer/registration and callback edges. The existing B24E20/B188A0 shader
   texture-unload loop uses +1A9C/+1AA0 and cannot establish this profile merely
   because it also has 2C-byte records. This audit does not claim an exhaustive
   repository-wide absence of a matching owner provider.
3. **Cache+04 provenance / B27B00.** Fresh saved/PE identity confirms the
   independent 130-byte body through B27B81, ECX renderer, stack owner, RET4.
   It uses renderer+38, actual count+04 and 12-byte rows, invokes B24510 with
   row words 0/4/8 as stage/state/value, and increments +1B98 after returning
   work. Current saved analysis has no function start or xrefs at B27B00.
   Actual B24510 and B42330 providers make this a small bounded provenance and
   composition candidate, but no caller/profile proof was invented. A saved
   function repair would be a separately leased Ghidra mutation.
4. **CRT/service and full-parent integration.** The checked tree exposes an
   explicit potentially returning/throwing `SingletonLifetimeCallbacks`
   invalid-parameter service. The BF6713/BF66EF encoded-handler internals are
   still not reconstructed. Existing newer native vectors use the real SDK
   `_invalid_parameter_noinfo` under a documented source-CRT boundary. Choose
   and record the actual application binding when composing B29670; a no-op,
   unconditional throw, or terminate callback cannot silently replace it.

None of the four audited parents is an immediate source-ready packet under
these unresolved contracts. The independent provider candidates above are
bounded discovery work, not justification for generic destructor callbacks or
null-only full routines. B29670 also needs its precise tracked-lock depth,
current-lock normal release and optional-only EH cleanup: an exception does
not unwind the tracked section. Current import forwarding, source builds and
leaf fixtures do not demonstrate full device recreation or gameplay validation.
