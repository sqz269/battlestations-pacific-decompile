# Selected avoid-zone arc clipping

`avoid_zone_arc.hpp` provides the complete native instruction schedules for
`00415970..00415D6E` and `004F3BA0..004F3D90`. Names are hypotheses. These are new
Win32 C++ interfaces with explicit borrowed CRT access, not drop-in original ABI
replacements. No runtime adapter was changed by this packet.

| Function | Native ABI | Coverage |
| --- | --- | --- |
| `00415970` | ECX=head-word address; stack center, radius, start bearing, end-bearing pointer; EAX=last writing segment; `RET10h` at `00415D6C` | complete instruction/branch schedule |
| `004F3BA0` | ECX=center, EDX=segment start; stack radius, end, output; EAX=count; `RET0Ch` at `004F3D64`, `004F3D79`, `004F3D8E` | complete instruction/branch schedule; native solver-failure ambient-stack limitation below |

The selected list reuses `AvoidZoneSelectedSegment`, the actual 20h allocation
produced by `00415190/00417630`. Its start/end pairs are at +0/+8, next/previous
links at +10h/+14h, next-run at +18h, and run-closing byte at +1Ch. The arc walker
uses next unless the closing byte is nonzero or next is null; otherwise it takes
next-run. It writes only the caller's end-bearing reference; links and padding
are unchanged unless the caller explicitly aliases that output with their bytes.
These are cyclic edge runs with explicit continuations, not a spatial tree.

`004F3BA0` first spills end-start, its squared length, and the resulting length
to binary32. Squared lengths below/equal to native double `1e-10`, and unordered
comparisons, select length zero. Division still occurs with that length: no
degenerate-segment fallback is inserted. The normalized direction and its
perpendicular feed the existing `native_segment_parameters_004f3630`, solving
`start + t*direction = center + u*perpendicular`.

For an ordered `-radius < u < radius`, the helper spills and square-roots
`radius*radius-u*u`, then computes the two parameters `t-root` and `t+root`.
Only parameters strictly between zero and the segment length produce outputs,
in that order. Thus exact tangency and exact segment-endpoint intersections are
excluded. Radius is not replaced by its magnitude. Count-zero leaves all output
words untouched; count-one leaves the second pair untouched. Products spill to
float before endpoint addition, retaining the native rounding sequence.

The parameter solver's success flag is ignored. Its t output aliases the
initialized delta-x local, but u at native local +18h has no initializer. If the
solver rejects, that u read is ambient stack data. A finite delta whose squared
length overflows can normalize to a zero direction and reach this path. The
source preserves the read and branch schedule; no deterministic output promise,
zero substitute or retry is made for that path. The typed adapter has a different
stack context, so ambient bits need not agree across separate native/source
calls. Ordinary zero/short segments and NaN inputs included in the fixture take
their observed deterministic rejection paths instead.

The arc walker squares radius once, then skips a segment unless at least one
endpoint's squared distance is ordered-greater-or-equal to that value. It
computes the nearest point by the native normalized-direction projection,
including float spills and the original unordered branches. A cutoff distance
at or below radius permits the circle helper call. The caller supplies absolute
start and end bearings; the end reference is only written when a candidate
passes the directional tests.

For every segment with intersections, the walker captures
`SubtractWrappedAngle(end,start)` once. Each point becomes the native compass
bearing: host CRT atan2 result spilled to float, subtracted from widened
float(pi/2), spilled again, with widened float(2*pi) added once when negative.
`SubtractWrappedAngle(candidate,start)` must have the same strict direction and
lie strictly inside the captured interval. A passing candidate writes
`AddWrappedAngle(start,candidate_delta)` and replaces the returned segment.
**The captured interval is not updated between the two points of one segment.**
Consequently the second point may overwrite a nearer first point. For the chord
`(-10,3)->(10,3)`, center `(0,0)`, radius 5, start -1.3 and end 1.3, the final
bearing is `0.927295208` (`3F6D6338`), despite an earlier approximately -0.927295
intersection. Reversing the segment reverses candidate order. This ordering is
preserved instead of adding a nearest-hit policy.

Read-after-write aliases also retain native behavior. For example, when the
helper's output starts at its input start pair for that chord, the first point
is `(-4,3)` and the second becomes `(10,3)` because it reloads the now-modified
start. An arc end-bearing reference that aliases center.x can similarly affect
the later candidate's atan2 input. The fixture compares both aliases.

The implementation reuses the actual parameter solver and wrapped-angle
functions, and `native_crt_sqrt_st0_00bf7030` with `CameraAxesCrtAccess`. Atan2 is
the existing host `_CIatan2` library boundary, as in `geometry_helpers.cpp`; this
packet does not claim an independently recovered VS2005 atan2 implementation.
No generic geometry library, standard-math replacement, vtable or return table
is introduced. The private circle kernel appends the CRT pointer after the
three original stack arguments and returns `RET10h`. The private arc kernel
saves its added EDX CRT pointer before the native frame, moves only original
argument references by four bytes, and leaves local spill offsets unchanged.
Existing C++ angle functions are cdecl, so their calls explicitly clean eight
bytes without another FP spill.

Both live owned bodies have zero gaps: 308 and 172 instructions. All five helper
call sites and both arc call sites were checked, including both helper calls in
`00416270`. The complete direct-call table is in `reports/avoid_zone_arc.json`.
No worker Ghidra writes or function repairs were needed; names are proposed in
the ledger for root annotation.

Validation: Win32 Release and both existing tests pass. One ignored original-byte
fixture verifies six live/installed spans (the two owned routines, parameter
solver, relative comparison and two angle wrappers), relocates their actual
absolute operands, and executes them against the Release library. **21 helper
and 22 arc cases pass**, with equal 148-word records and exact returned segment
identity checks. Cases cover output counts/preservation, tangency/endpoints,
direction reversal, zero/negative/infinite radius, selected NaN/degenerate
inputs, projection clamps, signed sweep writes, cyclic runs, and aliases.
The fixture binds both sides to the same recovered sqrt service with bypass=1
and to the same actual host atan2; it proves surrounding geometry and ordering,
not independent CRT-library parity. Exceptions are masked, precision is 53 bits,
rounding is nearest. Other control modes, solver-rejected ambient u, CRT error
handler effects and gameplay are not fixture claims. Infinite or very large
angles that reach existing unbounded wrapping may not terminate.

Reproduce with `python local/prepare_arc_probe.py`, then
`cmd /c local\build_arc_native_probe.cmd`, then `local\arc_native_probe.exe`.
The preparation wrapper verifies `C:/Users/sqz269/bsp.gpr` and the configured
program before every live query. All source, image, relocation, record and log
hashes are retained in the report. An initial optimized fixture adapter was
inlined around an opaque asm call, after which MSVC incorrectly reused clobbered
XMM0/XMM1 as the source arguments. The saved optimized listing establishes this;
marking the fixture adapter `noinline` fixes the `/O2 /fp:strict` differential.
This was a fixture boundary correction, not a production algorithm change.
