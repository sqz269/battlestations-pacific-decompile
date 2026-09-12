# Tracer parameter curve loading and lifetime

Addresses: `00BABFE0`, `00BAC5E0`, `00BAC670`, `00869E40`, `00BACB10`,
`00BACBB0`. Descriptive names are hypotheses. The implementation uses the
existing `TracerParameterCurveStorage` and `NativeLuaObjectStorage` directly.
There is no second owner, reference count, storage domain, or registry wrapper.

| Routine | Original ABI and inclusive body | Coverage |
| --- | --- | --- |
| reserve `BABFE0` | ECX owner+08, stack signed capacity; `BABFE0..BAC061`; final `BAC05F RET4`, length 3 | complete instruction-path behavior, including disk post-free tail |
| resize `BAC5E0` | ECX owner+08, stack signed size; `BAC5E0..BAC636`; final `BAC634 RET4`, length 3 | complete |
| append `BAC670` | ECX actual owner, stack binary32 coordinate/value; `BAC670..BAC729`; final `BAC727 RET8`, length 3 | complete, exact slope x87 operations |
| load `869E40` | ECX unused, stack Lua-object pointer/output-owner pointer/float scale; `869E40..869FCB`; final `869FC9 RET0C`, length 3 | complete normal-return and C++ allocation-failure behavior; original SEH/nonlocal Lua error ABI excluded |
| destroy `BACB10` | ECX actual owner, no stack arguments; `BACB10..BACB6C`; final `BACB6C RET`, length 1 | complete normal-return behavior, including disk tail; original SEH/unwind ABI excluded |
| scalar destroy `BACBB0` | ECX actual owner, stack flags, EAX same address; `BACBB0..BACBCD`; final `BACBCB RET4`, length 3 | complete normal-return behavior, including disk stack cleanup |

## Record production and failure state

The 1Ch owner and its 0Ch coordinate/value/slope records are declared only in
`tracer_parameter_curve.hpp`. Constructor `BACAA0` establishes refcount 1,
table identity `D63FFC`, and zero record pointer/size/capacity/point count/cache.
The new loader calls that existing initializer on its actual allocated owner.

Reserve clamps requested capacity to at least one, then returns if current
capacity already suffices. Its allocation size is the low 32 bits of
`capacity*12`. It copies `storage_size` records as raw words, frees the old
allocation, publishes the new pointer, then publishes capacity. Size and point
count do not change. The free callback sees the old pointer and capacity.
Resize first reserves only if necessary, zeros each newly exposed record,
decrements size when shrinking, and writes the requested size. It leaves the
evaluation count/cache unchanged and does not free capacity when shrinking.

Append uses the evaluation point count to find the previous record. It computes
coordinate delta with x87 FSUB and **FSTP32**, reloads that rounded delta, loads
threshold bits `3A83126F` (approximately 0.001), then executes FCOMIP/JBE.
Ordered deltas below the threshold write positive-zero slope. Equality and
unordered comparisons enter the value-delta/division path; NaN is not treated
as the zero-slope branch. That path retains FSUB/FDIVRP/FSTP32 under the caller's
control word. The coordinate/value payloads for the appended record are copied
without floating-point arithmetic and its slope is positive zero.

The previous slope changes **before** growth. If allocation throws, the old
pointer/capacity/size/point count remain, but that slope update remains too.
When size equals capacity, capacity doubles using 32-bit wrap, with results
<=1 replaced by 1. The record is then written, storage size increments, and
point count increments. No sorting, deduplication, repaired counts, or cache
reset is added. Native malformed-storage/count preconditions remain unchecked.

## Actual Lua loading and ownership

`load_tracer_parameter_curve_00869e40` takes the actual native stack-tracked
Lua object, a reference to the caller's owner pointer, and the binary32 scale.
If the output is null it allocates exactly 1Ch bytes, constructs the owner, and
publishes it before reading rows. Existing owners are appended to. The captured
owner remains the target throughout the call, matching EDI in the original.
The loader scans integer rows beginning at 1 and stops at the first nil.
Each row's first value comes through existing `B66270`, then x87 multiplication
by scale and FSTP32. Its second value comes through the same getter and a
binary32 spill. It releases both number temporaries before appending, and the
row temporary before fetching the next row. The two original unused default
Lua objects and their cleanup order are preserved. The nil test applies the
complete `B65FB0` rule to actual kind/state/index fields, using linked Lua5.1.1.

These bindings reuse `B65F50`, `B67720`, `B66270`, and `B67700` in
`native_lua_objects.cpp`; they use neither `GuiLuaRef` nor synthetic rows.
The five callers in `86D180` were inspected at `86D829`, `86D8DA`, `86D98B`,
`86DA3C`, and `86DC46`. Each pushes scale, output pointer, then Lua-object
pointer; the callee's RET0C confirms the three stack arguments. The first four
load the scale from a local float; the fifth uses FLD1.

The default memory bindings implement the observed `BF681B` malloc/new-handler
retry/bad_alloc policy and matching free. `BF55BE` is its JMP thunk;
`BF6989 -> BF65AC -> BF9DC8` are the returning free path. Optional custom
bindings must provide the same nonnull-or-throw allocation and nonthrowing
matching free contract, with the same bindings used through the owner's life.
An owner-allocation exception leaves output unchanged. A later record exception
leaves the constructed output published and releases active Lua temporaries
during C++ unwinding; prior appended points remain. Empty tables create a valid
owned empty curve. Original CRT global handler/EH object identity is not replaced.

Destroy resizes storage to zero, frees its record pointer, then calls the
existing base destructor behavior `BD30F0`, which writes table identity `CEB130`.
It does **not** clear the freed pointer, capacity, point count, cache, or refcount.
Scalar destroy executes that body, frees the owner only when flags bit 0 is
set, and returns the same address even after free. Neither routine decrements
the reference count. Native table `D63FFC` has slot0 `BD30E0`, slot4 `BACBB0`,
and slot8 `BA9DA0`. Those DWORDs are original-image identities, never callable
host pointers. A current-slot binding can use the actual scalar destructor or
existing evaluator for those entries; unrelated current slots require their
own bindings. The consumer remains responsible for its actual reference-count
transition and current-table dispatch.

## Read-only Ghidra repair evidence

Every live query/export used `bsp.py ghidra`, which verifies existing
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, x86 language and image
base. Seven fixture ranges were byte-identical between disk and live Ghidra;
hashes are in the report. No functions, names, prototypes, comments, or project
save state were changed by this packet.

* `BABFE0` needs instructions restored at `BAC053..BAC05C`: ADD ESP,4; POP EDI;
  MOV [ESI],EBP; MOV [ESI+8],EBX; POP EBP. Its existing min/max end is correct.
* `BACB10` currently ends at `BACB49`, the free CALL's last byte. Restore
  `BACB4A..BACB6C` and extend its body through inclusive `BACB6C`. This includes
  the base destructor CALL at `BACB57`, FS restoration and the final RET.
* `BACBB0` needs `BACBC5..BACBC7` ADD ESP,4 restored after its free CALL. Its
  existing inclusive end `BACBCD` is correct. `869EDD..869EDF` is skipped
  alignment padding, not a missing executable path.

## Verification and limits

The ignored `local/curve_loading_probe/` extracts original instructions, checks
live/disk equality, relocates their exact native calls to the same allocator
and actual Lua bindings, and executes original/rebuilt paths separately.
3,072 append/evaluate/destroy pairs passed over 12 x87 precision/rounding words,
including threshold neighbors, signed zeros, subnormals, infinities, and quiet
and signaling NaNs. The comparison checks all owner fields except allocation
addresses, every active record byte, evaluation result/cache, x87 status and
an existing stack sentinel. Growth, reserve, resize, publication order, and
the pre-growth slope update on allocation failure also passed.

84 original/rebuilt loading comparisons executed linked Lua5.1.1 with actual
tracked objects across the same 12 control words. They cover scaling (including
subnormal/NaN scale), first-nil termination, null/existing outputs, point bytes,
stack top/tracking counts, actual evaluation and owner-plus-record scalar free.
Focused rebuilt checks cover empty-table ownership, flags2 retaining the owner,
owner-allocation failure and post-publication record-allocation failure cleanup.
Original loader/destructor SEH frames returned normally in the fixture; their
native exception handlers were not exercised or reconstructed.

The source and ignored probe compile under MSVC Win32 `/W4 /WX /O2 /MD /EHsc`
with `/link /MANIFEST:EMBED`. `scripts/build.ps1` passed Release Win32 and the
one configured existing CTest, `reconstructed_math`. This packet does not edit
CMake; its source is compiled directly by the strict fixture pending integrator
registration. `verify_report_calls.py` identifies the expected missing live
`BACB57 -> BD30F0` call row; all other reported call/tail rows pass.
The game's current application flow is not wired to this new default-curve
binding. Loaded-curve fixture evidence is not gameplay or rendered-tracer proof.

## AK saved-analysis and combined-build integration

The seven-module AK batch is registered in bsp_core. Strict MSVC Win32
compilation and both seeded CTests passed with explicit `--parallel 1`; the
standard parallel script hit environment MSB3491 before compiling C++.
The report records saved Ghidra name/signature preimages, prior-comment
preservation and readback, original-byte fixture coverage and exact call checks.
Reported returning-free continuations and missing definitions are now repaired
and saved; worker-era pending-integration notes above describe the earlier snapshot.
New C++ interfaces and required real runtime bindings remain as documented.
Successful full construction, native EH compatibility and gameplay are not implied.
