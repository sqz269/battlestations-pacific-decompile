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
