# Wake decomposition: whole-body Source readiness

The complete `00811180..00811812` body is ready for a **separate ordinary C++ leaf over actual entity storage**, using the existing production `0042B260` normalizer and genuine current CRT `_CIsqrt`. This audit implements nothing and leaves the semantic ship-wake ports unchanged. The full body resolves the previously open sign and exposes additional projection/rounding differences; it does not close membership or entity ownership.

## Actual storage and entry contract

The Original takes the entity in ECX and stack pointers to a three-float point, across output and along output, then returns with `RET 0Ch`. It preserves EBX/ESI/EDI/EBP; no semantic return value is established. It reads forty `18h`-byte samples at entity `+BD8h`, with head at `+F98h`: XYZ at sample `+0/+4/+8` and stored segment length at `+10h`. It reads no `written` count, heading, yaw rate, residual or owner/global projection.

The caller must supply actual initialized, retained addressed storage, normally head `0..39`, valid point words and writable outputs, and must own synchronization/lifetime. The body adds no null/range/empty-ring guard. It needs all eight x87 slots available, preserves the ambient control settings, and retains native status/exception effects. Mixed x87 comparisons and `COMISS` make an SSE-only or generic `long double` rewrite insufficient.

All input reads finish before the output stores. Across is written first at `811807`, then along by raw `MOVSS` at `811809`; aliased outputs therefore finish with along. Outputs can alias point/entity storage if it is writable. No whole-ring copy or invented view/owner is needed.

## Complete schedule

Let `R(j)` be the sample `j` steps back from head, using the native signed remainder schedule. `F(x)` below denotes an explicit native binary32 spill, not permission to rearrange the arithmetic.

1. **Nearest sample, `811180..81147C`.** Capture point XYZ through `MOVSS`; initialize best to `7F7FFFFFh` and nearest ordinal `k=-1`. Eight iterations each examine five samples, covering all forty from newest to oldest. Sample XYZ and each point-minus-sample delta pass through their individual `FLD/FSTP` float stores. Products/sums remain on x87 until one float squared-distance spill. Update only for ordered `distance < best`; equal minima keep the first sample. All unordered/Inf/FLT_MAX distances can leave `k=-1`; the actual exceptional path must remain, without substituting zero.
2. **Choose a neighboring line, `811482..811584`.** If `k=0`, select younger ordinal `n=0`; if `k=39`, select `n=38`. Otherwise compare the full 3D squared distances to `R(k-1)` and `R(k+1)`, with their native float stores. Newer distance `<=` older, **or unordered**, selects `n=k-1`; otherwise `n=k`. The segment endpoints are `A=R(n)` and `B=R(n+1)`. Preserve the signed `k=-1` path and actual rereads, including point Y/Z during neighbor selection.
3. **Direction, `811587..8115DE`.** Float-store each component of `A-B`, then call the complete registered normalizer on these same three local floats. Its result is `d`. This is the winning segment direction, not a heading or an interpolated wake direction.
4. **Projection, `8115E3..811660`.** Reload head, form componentwise `D=F(A-captured_point)`, then `q=F(dot(D,d))` in the exact x87 order. Keep an independent float copy of `q` for the later perpendicular residual.
5. **Along, `811664..811721`.** Start with `q` and add stored segment lengths for ordinals **0 through n inclusive**, float-storing after every addition. Preserve the four-at-a-time loop and head-read schedule; the negative-ordinal path skips the sum. There is no clamp and no call to the wake interpolator. Along can be negative.
6. **Sign, `811723..811760`.** Float-store `c=d.z*D.x-D.z*d.x`. An x87 `FCOMIP(0,c)` selects integer `-1` for ordered negative `c`; otherwise `COMISS(c,+0)/JA` selects `+1` only for ordered positive, and zero otherwise. Preserve those actual mixed instructions, including unordered, denormal/DAZ and exception behavior. The geometric sign agrees with the left-normal convention when both computations use the same planar direction; there is no universal sign inversion.
7. **Magnitude, `811768..8117EF`.** Float-store each product `q*d`, then each subtraction `r=F(D-F(q*d))`. Compute the full 3D squared residual with x87 products/sums and one float spill. Ordered `<=` binary64 `1e-10`, **or unordered**, selects positive-zero magnitude; otherwise call the genuine current `_CIsqrt` provider and preserve the native result spill/reload/store sequence. This measures distance to the selected line without clamping the projection to its segment.
8. **Outputs, `8117F1..811812`.** Convert the integer sign with `FILD`, multiply by the float magnitude and float-store across; then store accumulated along's raw bits. Negative sign times floored positive zero preserves negative zero.

The x87 state after the projection spill and through segment summation is, from ST0, `D.z, d.z, D.x, d.x, D.y`. Immediately before cross-product `FSUBP` it is `D.z*d.x, d.z*D.x, D.z, d.z, D.x, d.x, D.y`. The normalizer call enters with an empty stack. The report retains every instruction, spill and branch address.

The normalizer divides by a float-spilled square root unless squared length is `<=1e-10` or unordered, when its divisor is float-spilled `1e-5`. Thus tiny/degenerate segments need not produce a unit vector. Preserve the literal residual formula on those paths; do not add another normalization or assume an ideal mathematical projection.

## Concrete differences from the semantic port

Current `ship_ai_wake_decompose_00811180` accepts a projected `ShipAiWakeTrail`, returns a validity flag and has a host-only `written==0` early exit. Its nearest ordinal starts at zero, and its production body uses `SUBSS/MULSS/ADDSS`; the Original has different x87 product/sum round trips. It omits the neighbor decision and projection term, sums only the legs before the nearest step, then calls `00810630`'s semantic sampler and measures a horizontal left-normal projection at that interpolated point. The Original uses the selected line and a signed full 3D magnitude, with its explicit cutoff and comparison behavior.

The existing header's assertion that the image always returns zero along beyond the head is contradicted by the complete body. For a straight segment `A=(0,0,0)`, `B=(0,0,-1)`, head length zero and point `(0,0,2)`, the native schedule gives `q=-2`, along `-2`; the semantic nearest-head sum gives zero. With point `(2,3,0)` and both directions assumed `(0,0,1)`, native magnitude is `sqrt(13)` with negative sign, while the semantic horizontal projection is `-2`. These are algebraic examples, not executed fixtures or bitwise runtime comparisons.

## Production providers and next packet

`normalize_camera_basis_0042b260` is registered in `bsp_core`. Its current complete COFF body is 138 bytes/47 instructions and equals the Original body after masking only its three relocation operands. The two resolved read-only payloads equal Original binary64 `1e-10` (`3DDB7CDFD9D7BDBBh`) and `1e-5` (`3EE4F8B588E368F1h`). Its call relocation names `__CIsqrt`; the current primary executable imports `_CIsqrt` from `api-ms-win-crt-math-l1-1-0.dll`. This is the genuine current CRT contract already used by the production normalizer; Original CRT dispatch globals and diagnostic policy remain outside the claim.

The existing semantic decomposition is separately retained as 438 bytes/132 instructions, with its interpolation-call relocation. Worker and primary code bytes match for both selected functions. Only the normalizer's build-path-specific anonymous-namespace relocation names differ; corresponding constant payloads were independently resolved and matched. Both exact physical members in each existing `bsp_core.lib` equal their respective retained objects. These are inspected existing artifacts, not a new build or live-loaded provider check.

The smallest useful Source packet is a new `native_unit_wake_decomposition.hpp/.cpp` entry, for example:

```cpp
void decompose_native_unit_wake_00811180(
    const void* actual_entity, const float* point, float* across, float* along);
```

It should preserve the complete x87/SSE/read/store schedule behind this ordinary interface, call the actual existing normalizer/current CRT, and use only the verified read-only constant bits. A private assembly worker is a possible implementation technique. Add no mutable world globals, host callbacks, `written` field, validity result, interpolator or null/empty repair. Leave both legacy semantic ports unchanged. Primary registration and proportionate validation belong to that future Source packet; the public interface does not claim the Original register entry ABI.

The established `0070ED30` contract is sufficient context: it supplies the transformed member point, calls at `70EEA6`, then stores across into record `+10h` and along into `+20h`. Its parent body was not expanded here, and this new leaf would not establish the actual group/settings/entity producer graph.

## Evidence boundary

The whole wake body is 1,683 bytes/523 instructions, gap-free and identical to the live listing addresses and installed Original bytes. The complete normalizer and three constants are also pinned. Pre/post bytes, frozen Source, production objects/library members and three direct-call rows are checked in `reports/cc12_wake_decomposition_Source_readiness.json`; artifacts are in `local/cc12_wake_decomposition_Source_readiness_evidence/` and its ZIP. No Source/Ghidra edit, build/test replay, native/SDK/game execution or runtime/ABI/gameplay claim occurred.
