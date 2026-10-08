# Raw Vector2 tail facade

The Source-only `bsp::raw_length_2d_00414c60(const float*) noexcept`
entry now exposes the existing private `00414C60` math kernel through its
native register contract. It is a naked `__fastcall` function in the same
`src/vector_helpers.cpp` translation unit. Its complete compiled body is
**five bytes / one instruction**, `E9 00000000`, with exactly one
`IMAGE_REL_I386_REL32` relocation at +1 to the actual local `length_kernel`
symbol, COFF storage class 3. No anonymous-namespace extern is invented.

The facade adds no register, integer-stack, EFLAGS, x87, SSE or FP-environment
operation. It forwards ECX and the original return address directly to the
existing kernel. Callers must provide two readable float cells, valid input
lifetime/synchronization and three free local x87 slots; the genuine CRT's
private entry requirements remain external. The kernel returns the
float-spilled result in ST0 and its raw float bits in ECX, with plain RET and
no argument cleanup. **The incoming ECX pointer is not restored:** PUSH ECX
allocates a scratch word which the result stores overwrite before POP ECX.
The native greater and zero arms both exhibit this behavior. The facade
does not copy inputs, reset the FP environment or suppress native status effects.

This closes the 2D provider-binding prerequisite from
[the accepted append binding design](CC12_NATIVE_WAKE_APPEND_BINDING_DESIGN.md).
It adds no Original-address credit: `00414C60` was already reconstructed.
It does not implement or wire the append parent, geometry owner, reset cells
or host trail storage.

## Physical proof

The complete original 66-byte/26-instruction kernel was recovered from the
accepted raw-dependency archive and rechecked against the current original
image, SHA-256 `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
The compiled existing kernel matches all 58 non-relocated bytes; its two
four-byte operands are the genuine cutoff address and `_CIsqrt` call.
Whole-body CFG inspection establishes three maximum local x87 slots and
both ST0/ECX/RET return paths, treating the CRT as an external ST0-in/ST0-out
contract. No Original or new Source entry was executed.

Every pre-existing function in the complete `vector_helpers.obj` retains
its entire body bytes and normalized, ordered relocations:

| Existing function | Bytes | Instructions |
| --- | ---: | ---: |
| Public cdecl `length_2d_00414c60` bridge | 12 | 5 |
| `transform_point_copy_00414cd0` | 70 | 27 |
| Private `length_kernel` | 66 | 26 |
| `std::array<float,2>::data() const` | 3 | 2 |
| `std::array<float,3>::data()` | 3 | 2 |

The old public bridge's +8 REL32 and the new facade's +1 REL32 target the same
actual private kernel. Its +19 DIR32 resolves to the non-writable eight-byte
cutoff `bb bd d7 d9 df 7c db 3d`, and its +33 REL32 remains `__CIsqrt`.
The real x86 SDK `ucrt.lib`, selected by the normal link read log and retained
before and after the final build, has exactly one corresponding 60-byte
import member. Its I386 code import maps linker symbol `__CIsqrt` to DLL
name `_CIsqrt` in `api-ms-win-crt-math-l1-1-0.dll`.

All five baseline/first-build/final-build whole archives were physically
retained and parsed; each contains exactly one `vector_helpers.obj` member,
equal to its corresponding whole object. This proves the static archive
binding. No new application caller, forced-link probe, linked-function map
or loaded-module observation was added; application linkage and Original
CRT diagnostic/dispatch/exception-policy equivalence are not claimed.

## Build and evidence

Actual Source/header, consumed-header closure, selected command records,
generated projects/cache, configured compiler identity, compiler/tool
binaries, SDK import library, complete object and complete archive were
captured before normal builds and again after their checks. The final
75-header dependency set and bytes, selected compile command, relevant
Source/configuration inputs, five tool binaries and SDK library are unchanged
across the final build. Tools are physical configured files, not a claim
about loaded modules. Full phase pin lists are inside the sealed archive.

The first build and its snapshots are retained. Native scratch analysis
then corrected only the new comments' initial ECX-restoration claim, followed
by a second normal build with fresh pre/post captures. Both builds pass all
three existing checks, and all six compiled function bodies/relocations
match between the first and final build. The final capture ordering is:

| Event (UTC, 2026-10-08) | Interval |
| --- | --- |
| Final input capture | 14:42:45.887983–14:42:46.474040 |
| `./scripts/build.ps1` | 14:43:04.490135–14:43:35.672838 |
| Final output capture | 14:43:54.330708–14:43:55.035940 |

MSVC Win32 Release passes `reconstructed_math` (0.22 s),
`native_math_differential` (0.04 s) and `tool_tests` (7.87 s).
The existing LNK4006 duplicate `spawn_request_id_matches` warning remains.
No new test, fixture replay, probe, ad hoc link or new-entry execution was added.

The old archive predates imported main changes; the first normal build
increased its member count from 1,898 to 1,901. Aggregate project/log/object/
archive drift is retained and qualified separately from the complete-function
preservation proof. Whole-archive drift is not attributed solely to this facade.
The final comment build also changes object/archive identity despite identical
function bytes and relocations.

[The machine-readable report](../reports/cc12_raw_vector2_tail_facade.json)
records hashes, actual symbols, capture timing and the sealed local evidence
archive. Acceptance remains Source/static object/archive/build evidence;
new-entry runtime, ABI differential and gameplay validation are unclaimed.

Primary review independently verified all440 artifacts/all441 ZIP entries and five physical snapshot phases, all five preserved complete functions and six current compiled bodies, the same actual private66-byte kernel binding from both public entries, its readonly cutoff and the actual SDK unique60-byte CRT import member. Current combined MSVCWin32/all three existing checks pass. Root registered an additional raw tail-facade view with zero Original-function increment and preserved the existing Ghidra name. No facade execution, parent/class/game qualification. Receipt: `local/cc12_raw_vector2_tail_facade_primary_review/receipt.json`.

Final combined review after the incoming main type8 Source registration rechecked26 selected whole objects, all1424 complete functions,23 unique current core archive members and five complete linked bodies with20 relocated operands. Selected Source pins are current, and the normal MSVCWin32 build/all three existing checks pass. No new API/raw-entry execution or game proof is added. Receipt: `local/cc12_facade_callable_final_after_main/receipt.json`.
