# Native general-convex intersection

Addresses: 00C51EF0, 00C53010, 00C535E0, 00C48BE0, 00C51C20.

R140 supplies the five complete normal bodies behind the general-convex
dispatcher, using the existing R139 simplex reducer and R135 convex support
methods. `NativeDynGeneralConvexRuntime` supplies the complete one-slot source
table for `DynGeneralConvexIntersectStorage`. Descriptive names are hypotheses,
not recovered symbols. Application context admission and gameplay remain open.

## Bodies and original ABI

| Entry | Inclusive end | Bytes | Original contract |
| --- | --- | ---: | --- |
| C51EF0 | C5300D | 4382 | ECX work; status EAX 0/1/2; RET |
| C53010 | C535D1 | 1474 | ECX matrix A, EDX shape B, EAX matrix B; stack work/result/shape A; AL hit; RET 0C |
| C535E0 | C53623 | 68 | ECX owner; stack result/shape A/matrix A/shape B/matrix B; AL hit; RET 14 |
| C48BE0 | C48E2C | 589 | ESI work; RET |
| C51C20 | C51EE6 | 711 | EDI work; RET |

The five bodies total **7,224 bytes and 2,137 instructions**. Current Ghidra
and installed PE bytes agree. The native table cell D7A1A8 points to C535E0.
No listing repair was needed in these bodies.

The actual 1F8h scratch record contains shape pointers at +0/+4, composed
transforms at +0C/+3C, a double threshold at +70, result pointer at +78,
direction at +80, four difference points at +98, paired witness arrays at
+F8/+158, simplex count at +1B8, iteration count at +1BC and selected witnesses
at +1C0/+1D8. The dispatcher supplies the owner's 26-direction array at +1F0
and critical-section pointer at +1F4. These bodies do not acquire that lock.

## Recovered behavior

C51EF0 derives a direction from the composed AABB centers, obtains opposite
support points through each actual shape's double-support slot +0C, and
preserves the native separation, plateau, duplicate and small-direction
predicates. It invokes C3CC30 to reduce the difference simplex, retains the
native ten-iteration limit and computes one-, two- or three-point witnesses
with the original ordered x87 expressions and determinant guard.

C51C20 transforms the two body centers, scans the constructor's 26 directions,
filters them by the center-delta dot product, and retains the strictly smaller
support projection and corresponding witnesses. Ties keep the previous choice.

C53010 composes the body and shape matrices, including the native approximately
0.01 X perturbation of transform A. It runs the search with the original
approximately 0.02 threshold. Early separation returns AL=0. Other statuses
select the witness direction or fallback scan, preserve both native shifted
searches and the 0.25 additions, undo their witness shifts, and project one
contact. Upper EAX bits are not the hit contract. C48BE0 writes count=1 and
nine floats: body-local witness A, body-local witness B and world normal.
Untouched result bytes retain their incoming values.

C535E0 preserves the native eight-byte stack alignment, allocates the 1F8h
scratch record, and invokes C53010 using its register/stack contract. The source
table thunk accepts the actual owner in ECX and five stack arguments. Its
stable, noncopyable runtime owns the table and a copy of `CameraAxesCrtAccess`;
the CRT pointees remain borrowed. Scratch and invocation context are local.

## Source arithmetic and context boundary

Private kernels retain every original non-relocated instruction byte. Calls,
branches and data addresses are rebound to source code/storage. Thirteen
double constants retain their exact native bit patterns, including the values
that only approximate decimal 0.01, 0.02 and 0.00001.

An explicit extra stack argument carries the CRT context in the result, search,
intersection and dispatcher kernels. Their adjusted cleanup is RET 4, RET 4,
RET 10 and RET 18 respectively. The ESI/ECX reducer adapter and two small call
shims preserve the original arithmetic; the sqrt shim enters the existing
recovered ST0 service without adding an FP spill or operation. Public C++
interfaces and the complete virtual slot are callable source interfaces, not
a claim of binary replacement or native exception metadata compatibility.

The COFF audit checks **2,137 original instructions, 43 branch destinations**,
call/constant relocation symbols, extra-context offsets, cleanup adapters and
all 13 constant bit patterns. The native reference chain retains its own
reducer and math helpers; it does not redirect those calls into the new kernels.

## Validation

Strict MSVC Win32 build and **all three existing CTests pass**. No permanent
tests were added. Two ignored probes use /MD, /fp:strict and embedded manifests.

* **14,040 pairs; 12,214,800 exactly matching bytes:** 585 translated/rotated
  cube configurations in each of 12 x87 precision/rounding modes, through both
  the explicit complete intersection interface and production dispatcher table.
  Comparisons include AL hit, result bytes, support seeds, CW/SW/tag/MXCSR, and
  the complete explicit work record. The dispatcher's private stack scratch is
  not observed. No data or pointer normalization is applied in this probe.
* **192 scene pairs; 111,079,056 matching comparison bytes:** the existing
  complete narrow-phase task consumes actual cube geometry through the source
  general-convex table, and is compared with the copied native task and complete
  native geometry chain. The corpus exercises filtering, separated and
  overlapping bodies, shape chains, event flags and task ranges in all 12 FP
  modes. It records **27,492 service events and 384 snapshots** across the
  paired cases (counts reported per side). Real pool critical-section operations
  and the event spinlock execute; final event unlock is checked.

The scene trace normalizes only task-table and shape-table/mesh pointer words;
the pool's 24 OS-managed critical-section bytes and FP instruction pointers are
excluded. Memory service calls, native sites, recursion depth, allocated records,
manifolds, events, result state and final support seeds are compared. The old
controlled-dispatch counter and replacement mask are unused in this geometry
probe and establish no coverage. This is serial scene evidence, not a race test.

Both probes initialize the requested FP mode before constructing each side's
inputs. An initial scene-fixture mismatch on a rounding-mode transition exposed
the missing setup reset; correcting the fixture made all 192 pairs agree.
The direct probe was updated to use the same setup rule and rerun successfully.

Primary collection verifies 16,965 bytes, including existing reducer/math,
constants and tables. The scene union re-verifies **20,663 bytes in 35 spans**;
these totals overlap and must not be added. Both sides share the existing shape
support and constructor implementations plus the recovered CRT sqrt service.
The fixture borrows shape tables without invoking shape-lifetime slots.

## Remaining work

Valid shapes, meshes, transforms, owner lifetime and borrowed CRT state are
required. The corpus does not prove arbitrary geometry, all predicate outcomes,
unmasked FP traps, malformed pointers, private-stack aliases, native exception
ABI or concurrent lifetime behavior. The other collision dispatchers, remaining
world tasks and application context wiring still need reconstruction and
integration before an ordinary game run can validate this path.

See `reports/native_dyn_general_convex_r140.json` for call rows, native byte and
instruction evidence, Ghidra annotation receipts, fixture hashes and immutable
tested/integrated artifact archives.
