# Canonical GUI frame, Section emission and PointLight ownership

Addresses: 00aa9730, 00aa8320, 00ac73e0, 00ac7260, 00ab8ee0, 00ab6a30,
00a9e070, 00aa87b0, 00aa6a40, 00a9a380, 00a9e120, 00abe6e0, 00abe7b0,
00abe960, 00abf420, 00abeda0, 00abf770, 00abe630, 00b7c710, 00b7c770,
00b7c850, 00b7c740, 00cd81a0, 00b7abd0, 00b7b690, 00b7b810, 00b7b1d0,
00b7b770, 00b7b110, 00b7bd30, 00b7b600, 00cd8060, 00ce0ea0, 00b7c4c0,
00b7c5b0, 00aa6870, 00aa68f0, 00aa6980, 00aaa3e0, 00a9e110.

This batch connects frame updates and Section geometry to the existing canonical
widget owner, adds the complete normal PointLight owner/pool lifetime, and shares
the base widget teardown between the actual Group and Text companions. The source
uses the existing Model, mesh, material, timed-entry and physical backlink owners.
No new surrogate resource graph or silent unresolved callback was introduced.

The implementation and ABI limits are detailed in `GUI_WIDGET_BASE_LIFETIME.md`,
`GUI_WIDGET_FRAME_RUNTIME.md`, `GUI_SECTION_RUNTIME.md` and
`NATIVE_POINT_LIGHT_OWNER.md`. Names are descriptive hypotheses, not recovered
symbols. `reports/orch5_section_frame_light_batch.json` records the source hashes,
worker commits, independent reviews, fixture evidence and exact annotation list.

## Integrated behavior

AA9730 now shares the normal base tail for Group/Text deletion. Child current20
precedes primary release; each live front child receives current4(1) while still
attached, then detaches during its own base tail. The same timed allocation drains
before container and companion disposal. Group flags0 retains the detached C++
wrapper; flags1 frees it. Native GUI slab/list allocators, vtable ABI and SEH are
outside this projection. The focused lifetime fixture exercises Groups; mixed
Text teardown has source review and build evidence only.

AA87B0 updates eligible actual children before the owner's timed entries, then
uses the current listener and actual mouse state. A single borrowed frame runtime
registers with the same widget registry. Retirement checks active operations;
update preflight occurs before entering an active frame. Standalone AA6A40 hit
testing protects its owner during current64. Screen/Group/Text/ClipBox/Section/
FrameBox current40 profiles are established. Icon40 remains a different body.
Listener adapters are mandatory; fixture observers do not prove native listener
implementations. The frame runtime must be destroyed before its widget registry.

Section17 is accepted by the real child factory with explicit Section services.
The D5C8E8 table establishes its inherited color/alpha/type/clip profiles. Current74
and current78 compose the canonical constructor/load paths. ABE7B0 reloads the
same entry+08 after current5C, then reaches the actual Section setter and emitter.
The staged ABF770 writer handles all three geometry modes, retaining native
mapping/publication order and x87/SSE stores. Constructor/load may enter emission;
emission reentry and active retirement are rejected. Exceptions leave a terminal
failed Section state, with preceding stores retained and no automatic unmap,
rollback or retry. Borrowed renderer and mapped-resource owners must stay live.

PointLight construction, type ancestry, physical backlinks and final deletion
use the same actual 200h slot and +04 reference count. The +1FC slab ID and
unwritten +1EC..1FB bytes remain distinct from the backlink tail. Pool growth and
trimming update moved slab IDs. Flags0 tears down without returning the slot;
flags1 returns it before unbinding the companion, with no post-free owner reads.
The shared Light destructor aliases the existing Point/Directional storage.

## Evidence and verification

The primary verified the existing `bsp.gpr` and `/battlestationspacific.exe` before
Ghidra batches. Matching installed bytes support new definitions for ABEDA0,
ABF770, ABE630, A9A380 and CD8060. Point pool local false-free fall-through gaps and
the B7C5B0 tail were repaired; the latter now contains B7C693 -> B6F440 and ends at
B7C6A9. Global CRT no-return flags were not changed. Repair reports preserve events,
and the annotation archive preserves preceding names/comments. Affected exports
are refreshed after annotations and the project is saved.

The combined MSVC Win32 Release build and both existing tests passed. The Group,
frame and PointLight source probes were relinked against the final combined
`bsp_core.lib`; all passed. The Section probe was regenerated from the exact
current numeric kernel and passed modes0/1/2 in both directions at fill0.25,
including segment count, positions, winding, white vertices and balanced x87
TOP/tag. It does not execute the full Section owner or renderer. No permanent
tests were added. Numeric call-row verification is recorded separately and does
not establish indirect profile or gameplay equivalence by itself.

## Follow-up packets

- Recover Section AC0280 property/texture loading, ABF5B0 copy and native Section
  scalar deletion/pool transport, preserving the same canonical resources.
- Recover Icon current40 AB1150 and concrete GUI listener subobjects before
  extending frame dispatch beyond the established profiles.
- Configure the canonical runtime from the game executable with actual service
  owners, then validate complete Section mapping/emission and visible GUI output.
- Extend common widget deletion to other actual derived profiles and native GUI
  allocation contracts when their independent resource tails are established.

This is exported, reconstructed, build-tested and scoped source-fixture-tested
work. It is not a drop-in binary replacement or a gameplay-validated game rebuild.
