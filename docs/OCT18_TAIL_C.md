# Lane oct18-tC, 2026-10-18

The packet is `codex_tail_C.txt` from the cc16-progress worktree: 106 exact
addresses, leased as `oct18-tC-tail`. No host wiring or game execution is part
of this packet. Descriptive names remain hypotheses. The scalar accessors use
MSVC Win32 naked functions to preserve FLD/RET without an intermediate float
spill; numerical units and ownership lifetimes are not inferred.

## Batch 1: raw leaves and compiler boundary

All 14 bodies below are in `src/engine_seg11_35_66_cc13.cpp`, namespace
`bsp::oct18_tail_c`, declared in `include/bsp/oct18_tail_c.hpp`. The existing
engine TU is separately leased and already registered in the build. This avoids
touching another owner's CMake lease. Searches of `include/` and `src/` found
call sites, virtual service declarations or typed projections, but no existing
standalone raw bodies for these leaves.

Evidence: saved per-address Ghidra pseudocode and listings under
`exports/bsp/functions/<address>/`. The live connection was verified against
the configured `bsp` project and `/battlestationspacific.exe` before annotation.

| Address | Behavior | Original ABI |
|---|---|---|
| 00A3E460 | Borrow owner+32h, accepted-invite block | ECX owner, EAX address, RET |
| 00A90ED0 | Empty input enumeration slot | ECX owner, no result, RET |
| 00AA38E0 | Load visibility byte at F5h | ECX layer, AL byte, RET |
| 00AA6BE0 | Store reader to actual F8BC84 cell | ECX reader, RET |
| 00AAB5E0 | Store callback to actual F8BCFC cell | ECX callback, RET |
| 00AD56D0 | Load terrain/foliage field at 3Ch | ECX owner, EAX word, RET |
| 00AD5720 | Load scalar at 0Ch | ECX owner, ST0 float32 extended, RET |
| 00AD57D0 | Load bush texture at 9Ch | ECX owner, EAX word, RET |
| 00AD57E0 | Load bush normal texture at A0h | ECX owner, EAX word, RET |
| 00BBDD30 | Load atmosphere scalar at 20h | ECX owner, ST0 float32 extended, RET |
| 00BBDD40 | Load atmosphere scalar at 28h | ECX owner, ST0 float32 extended, RET |
| 00BBDD60 | Borrow wind direction at 2Ch | ECX owner, EAX address, RET |
| 00BBDD70 | Load atmosphere scalar at 38h | ECX owner, ST0 float32 extended, RET |
| 00BBDDB0 | Load skybox field at 18h | ECX owner, EAX word, RET |

The two callback setters expose the actual cell as an explicit reference, hence
use a new C++ ABI. The other leaves preserve their observed register conventions,
but have no native replacement or application/game validation claim. The layer
visibility method also exists as a typed GUI fragment; the new body reads the
actual raw byte and returns it without converting arbitrary byte values to bool.

`00B83137` is a compiler-generated catch funclet, not a game function: it consumes
the parent's EBP+14h work pointer, calls `_free` at BF65AC and rethrows through
`__CxxThrowException@8` at BF6885. The seven-instruction listing ends at B8314B.
The existing `SceneRegistryVectorInsertFreeCatchBoundary` source projection stays
intact; the function is classified `library_instantiation_out_of_scope` rather
than adding another body or claiming native FH3 unwinding.

Validation: scripts/build.ps1 passed the Win32 Release build and both existing
CTests (reconstructed_math and tool_tests). No new tests were added. This is
source/build evidence; no original-body differential or game run was performed.

## Batch 2: complete ordinary storage bodies

Fifteen additional bodies use the same registered TU and header. Apart from the
two naked entries, their new C++ interfaces accept mandatory, address-labelled
callee bindings. They are implementations over actual borrowed storage, not new
host stand-ins. The dependency contracts come from the pseudocode, call-site
listing and existing named APIs. Checked-iterator failure BF6713 is allowed to
return. Library code remains a dependency; no CRT body is reconstructed.

| Address | Source body | Original ABI / evidence detail |
|---|---|---|
| 00A3EAC0 | active_user_slot | ECX owner, EAX word at 11Ch, RET |
| 00B6DAB0 | node_local_position | ECX node, XYZ stack; exact 12 instructions; replace argument by node+B0h and JMP captured virtual38 |
| 00A2C720 | ai_group_contains_entity | ECX group, entity stack, AL bool, RET4; checked sentinel walk |
| 00A11AF0 | ai_enable_torpedoes | ECX command, RET; retain group identity checks and entity capture across virtual5C |
| 00A15490 | ai_close_attack_tick | ECX command; target leader point, target pass, reloaded landing target, tail A11AF0 |
| 00A15500 | ai_defend_position_tick | ECX command, RET; followers, formation, own leader point, null target |
| 00A10D50 | ai_merge_strength | ECX command, other stack, EAX bool, RET4; own ST0 spills double before other's unspilled ST0 |
| 00A10C60 | ai_merge_distance | ECX command, other stack, EAX bool, RET4; preserve x87 float spills, operand order and unordered comparison |
| 00A07E40 | ai_build_entity_records | ECX group, EDX output vector, class stack, RET4; actual 20h caller temporary |
| 00A7A980 | sound_dsp_resize | ECX array header, signed count stack, RET4; reserve/grow/null-fill/shrink, publish count last |
| 00AC2090 | gui_line_describe | ECX Line, visitor stack, RET4; two-word tagged pairs by value; actual Points literal bound explicitly |
| 00AC2950 | gui_line_reset | ECX Line, RET; retain guards/copy binding, reset count, ensure4, geometry0/element0 resets |
| 00B8F100 | group_world_sphere | ECX Group, EAX group+13Ch, RET; both dynamic uncached aggregation and static affine branches |
| 00BD91F0 | release_whole_file | receiver unused, buffer stack, RET4; explicit BF6989 free binding |
| 00BE7AB0 | cache_file_from_vfs | ECX store, name/flags stack, RET8; original name and flags, memory-view/add, memory/source release order |

The group-sphere body closes a real gap in the pre-existing
`native_point_light_supported_world_sphere` projection, whose dynamic cache-miss
branch remains partial. Historical records are preserved. The new body binds
the actual aggregation callee instead of assuming a static or cached group.

`00AD3A60` and `00ABF590` are classified as compiler-generated MSVC scalar
deleting destructor wrappers. Both test flag bit0, conditionally free, and
return the original receiver. AD3A60 resets the base vtable then frees; ABF590
calls the separate game destructor ABF4F0 then the Section pool free ABEC40.
Their existing ownership projections remain. This classification does not
classify the underlying game destructors as vendor code.

Validation: Win32 Release and both existing CTests passed. Assembly names `in`,
`out` and `dx` initially collided with MASM reserved words; the local identifiers
were corrected and the build rerun successfully. No added tests or host wiring.
Source normal control flow is complete for these bodies. Native FH3/SEH, fault
transport, private call-stack identity and gameplay remain unproved.
