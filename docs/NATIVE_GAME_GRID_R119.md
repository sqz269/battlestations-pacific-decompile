# Native game grid CPU routines, R119

Addresses: `00709C90`, `00709CF0`, `00709760`. Analysis and saved flow repair also cover `0070BD70`, `0070B330`, `0070B220`, `0070B280`; their source implementation remains pending.

## Result

Three complete ordinary bodies, 714 original bytes, now have source implementations in `src/native_game_grid.cpp`. The actual owner is 84h bytes, with dimensions at +3C/+40, positions at +08, and normals at +1C. “Grid” is a descriptive hypothesis, not a recovered class name. This packet does not identify the enclosing simulation.

| Entry | Native contract | Reconstructed behavior |
|---|---|---|
| 709C90, 91 bytes | ECX destination, stacked source, EAX destination, RET4 | Copy two DWORDs then twelve floats through sequential x87 FLD/FSTP32 pairs. Preserve forward overlap and signaling-NaN quieting. |
| 709CF0, 500 bytes | ECX actual owner, RET | Calculate interior central differences, spill each component to binary32, execute the original extended-precision cross-product stack schedule, call existing 42B260 normalization, then copy neighboring normals into the boundaries. |
| 709760, 123 bytes | ECX actual owner, RET | Free the six current nonnull pointers at +08 through +1C in order; clear all six fields only after every call has returned. |

The normal calculation reloads dimensions and the destination pointer after its cross stores. Row edges are copied before the top/bottom edges. Even boundary copies use x87 rather than integer copies. Unsigned address arithmetic preserves native low-DWORD products and differences; signed loop comparisons preserve the original branch conditions. There are no new dimension, finite-number, or alias guards.

The cleanup body calls native CRT `free` at BF6989, verified from the callee. Its default source service uses the source CRT's `free`; caller allocations must belong to that domain. Later pointer fields are loaded after earlier calls. Source exceptions retain partial effects; no rollback or replay protocol is added to this leaf. The source interfaces do not reproduce original binary calling conventions or incidental EAX values from the void routines.

## Verification

The evidence collector verified the existing project `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, and 2,945 live bytes against the PE's mapped image, including the larger pending constructor closure and its constants. This includes the PE's zero-filled BSS view at F87574. The report records exact body boundaries, original bytes, instructions, and hashes.

One local differential probe copied the three original bodies, 714 bytes, and relocated seven CALLs. Both normal lanes call the existing concrete 42B260 implementation and its UCRT square-root boundary; this is not a new independent comparison of that dependency. The probe used `/MD /W4 /WX /O2 /fp:strict` and an embedded manifest.

- 432 descriptor cases: nine source/destination offsets, four bit-pattern rotations, and all 12 x87 precision/rounding combinations.
- 2,160 normal cases: 15 dimension pairs including rectangular, small and nonpositive dimensions with accessible padded storage; separate and aliased position/normal arrays; six data patterns including infinities, signaling/quiet NaNs and denormals; all 12 x87 precision/rounding combinations.
- 1,792 cleanup cases: every six-pointer null mask, four callback mutation schedules and seven injected-failure settings. The observer compared pointers, complete owner snapshots at each call, exceptions and final bytes. These are controlled foreign-call observations, not an execution of the original game's allocator.
- One source check allocated six blocks through the actual source CRT and released them with the default service.

All 4,384 paired cases passed: **71,737,472 compared bytes**. The floating-point comparisons also checked the control word, exception flags and balanced x87 stack. The strict Win32 build and all three existing CTests passed. No permanent test cases were added.

The saved Ghidra repair removes six false CALL_RETURN overrides after BF6989 in 709760 and restores their three-byte ADD ESP,4 instructions. The same recorded batch repairs the returning BF65AC call in pending scalar destructor 70B280. The report distinguishes the seven analysis repairs from the three implemented functions. Names and evidence comments preserve prior values; the project is saved, comments read back and affected exports refreshed.

## Follow-up packet

Complete 70BD70 construction, 70B330 initialization, 70B220 destruction and 70B280 scalar deletion, then bind the three 4DDB90 call sites. The complete captured listings are in the evidence archive. The initializer calls these CPU routines and the existing actual declaration/vertex/index factory and mapping services.

Assembly findings to retain:

- The fresh 38h constructor descriptor leaves two DWORDs at +20/+24 uninitialized. They are copied into owner +5C/+60 and remain observable during all six allocations before being overwritten. The source constructor needs an explicit private-frame preimage contract.
- CPU arrays use saturated `count * 12` allocation sizes, with the current owner count reloaded before each allocation.
- The back-face index at 70B706 uses the **row** count; the other quad indices use the column count. The initial GPU vertex position uses quotient/remainder by rows, while UVs use columns. Preserve the rectangular-grid behavior.
- Renderer declaration lookup uses literal CFD4E4, `gunvc.mvfm`, through current slot +38. The renderer captured later at 70B5C1 is retained for both stream factories. Vertex flags are 1000h; index format is 65h. Existing streams are released before replacement and fields are cleared after terminal callbacks.
- Use the existing renderer cache, physical factories, canonical owner companions and mapping implementations. The historical mesh-buffer fixture setup is available under the older `orch4-20260910` worktree; its old successful HAL result must be revalidated.

The game constructor is unchanged by this packet and still requires 70BD70. Renderer-backed initialization, native hardware faults, original FH3/SEH, arbitrary private-stack aliases, concurrency, drop-in ABI and gameplay are not validated here. The ordinary windowed application was not rerun.
