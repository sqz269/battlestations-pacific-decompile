# Circle tangent selection and circle intersections

Packet `orch6_ship_ai_circle_tangent`, worker `agent/orch6-circle-tangent`.
Addresses: `004F3560`, `004F3970`, `004F4520`, `004F47B0`, `009D6550`, `009D68B0`.
Names are hypotheses. Ghidra access was read-only against
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`; the repository
wrappers verified that target before each live batch. No Ghidra annotations,
function definitions, saves, or changes to the installed image were made.

`004F47B0` intersects two circles. `009D68B0` supplies a circle centered on the
query with radius **clearance**, and the target circle, after its selected
tangent/chord endpoint is closer to the query than clearance. It ignores the
intersection status. This closes the two geometry host contracts previously
unread in `SHIP_AI_APPROACH_UPDATE.md` without duplicating `009D68B0` or the
existing `004F3970`/`009D6550` projections.

## Coverage and interfaces

| Routine | Native ABI and body | Coverage | Implementation |
| --- | --- | --- | --- |
| `004F3560` | two stack floats, AL flag, `RET 8` at `004F361C/004F3624`; body `004F3560-004F3626` | complete | `circle_relative_equal_004f3560`, original x87/SSE schedule |
| `004F4520` | ECX/EDX center pointers; two float radii and two output pointers on stack; EAX status, `RET 10h` at `004F45A9/45D3/4631/46FF/47AB`; body `004F4520-004F47AD` | complete | `circle_intersections_004f4520`, explicit borrowed CRT added |
| `004F47B0` | ECX/EDX three-float circles, two output pointers, EAX status, `RET 8` at `004F47CF`; body `004F47B0-004F47D1` | complete | `circle_intersections_004f47b0` |
| `004F3970` | ECX circle; point/base/half-chord pointers, AL flag, `RET 0Ch`; body `004F3970-004F3B98` | complete review; existing semantic projection corrected | `ship_ai_path_tangent_chord_004f3970` |
| `009D6550` | ECX circle; point/first/second pointers, AL flag, `RET 0Ch`; body `009D6550-009D65D0` | complete review; reused unchanged | `ship_ai_path_tangent_points_009d6550` |
| `009D68B0` | ECX output, EDX circle; point, clearance, byte-valued side in three stack slots, EAX output; `RET 0Ch` at `009D6A3A/6ABF`; body `009D68B0-009D6AC1` | complete control flow; native indeterminate scratch is an explicit projection deviation | existing `ship_ai_circle_tangent_009d68b0`, plus concrete `ShipAiCircleGeometryHost` |

All six routines are already defined in Ghidra; `no_ghidra_function` is empty.
These interfaces are not binary replacements. The circle layout is reused from
`ship_ai_approach_update.hpp`; `009F37D0/37E4/37EA` produces its x/z/radius fields,
and `009D6A60/6A71/6A84` produces the local clearance circle. No new object offsets
or fabricated global state are introduced.

## `004F3560`: relative equality

For ordinary finite inputs it tests
`abs(float(a-b)) < max(1, abs(a), abs(b)) * float(0.0001)`.
`00E0830C` is `17 B7 D1 38`, binary32 0.0001. The multiplication remains in x87
until the strict comparison at `004F360E`; it is not rounded to a float first.
Absolute values use ordered-positive selection and `-0 - value`, not a sign-bit
mask. The larger magnitude selection chooses the second operand on unordered,
the floor chooses 1 only on ordered less, and the final unordered compare returns
false. `00D7A208` was checked live as `00 00 00 80` (negative zero). The instruction
schedule preserves these details and signed-zero handling.

All call sites were checked: `004F36A5` compares a segment denominator with zero;
`004F45C2` compares the two radii; `004F46C7` compares distance with the float-rounded
radius sum. Each passes two floats and the callee's `RET 8` cleans them up.

## `004F4520`: result and output writes

Let `d` be the center distance, `a`/`b` the first/second radius. Differences and
the squared-distance sum spill to binary32; the sum must be ordered greater than
double `1e-10` (`00CE3820`) before the CRT sqrt is called, otherwise `d = +0`.

| EAX | Condition in execution order | Outputs |
| --- | --- | --- |
| 0 | `d > a+b` (`004F4598/459C`) | neither written |
| -2 | `d < 0.0001f` and relative-equal radii (`004F45B2`, `004F45C2`) | neither written |
| -1 | selected max radius minus selected min radius greater than d (`004F4621`) | neither written |
| 1 | remaining geometry and relative-equal `d` and float-rounded `a+b` (`004F46C7`) | both receive the base |
| 2 | remaining geometry | first = base - offset, second = base + offset |

The initial radius sum is compared in x87 and also saved as binary64; only the
later equality call rounds it to binary32 (`004F46B4-46C0`). The axial distance
`t = (d*d - b*b + a*a)/(2*d)` spills at `004F464C`. The height is
`sqrt(float(a*a - t*t))`, computed even before the external tangency test.
The base uses `float(t/d)` times each original center difference, each product
stored before adding the first center. The offset uses height times the
perpendicular normalized center difference. The reciprocal-length callee
`00419260` rounds each square separately; it is the existing native kernel,
not the squared-distance cutoff kernel used earlier in this routine.

Internal tangency returns 2 with equal outputs. No absolute-radius conversion,
radicand clamping, finite-input rejection, or near-tangent repair is added.
Unordered branches and exceptional arithmetic retain the assembly behavior.

The new kernel saves EBX to hold explicit `CameraAxesCrtAccess`, adding four
bytes to native argument offsets, and adds a final CRT argument (`RET 14h`).
Original locals and x87 stack instructions retain their offsets and order;
only the two native temporary `SUB ESP,8` argument setup spans need additional
offset accounting. CRT sqrt and reciprocal-length implementations are reused.

## `009D68B0`: selection, fallback, and corrections

The complete listing establishes EBX=output at `009D68C5`, ESI=circle at
`009D68CA`, EDI=query at `009D68BD`, and EBP=`SETZ` result at `009D68E4`.
`CMP byte [ESP+3Ch],AL` at `009D68B7` tests only the low side byte: 0 selects
the second tangent, any nonzero byte the first. The sole caller at `009F3829`
pushes 1, with ECX output, EDX the produced circle, query and clearance on stack.

False from `009D6550` retains the query as chosen. If center distance is less
than 1 (or its sum fails the ordered `1e-10` cutoff), the routine draws from
stream 1 over `[0, 6.2831854820251465f]`, executes hardware FCOS and FSIN, and
returns center plus `{radius*sin(angle), radius*cos(angle)}`. Both trigonometric
results **and both scaled offsets** spill to binary32 before the additions
(`009D6A16/6A1E`). The corrected projection retains those stores and x87 operations.

Otherwise, when clearance is ordered greater than chosen-query separation, the
routine calls `004F47B0`. Baseline ESP here means ESP at `009D6A48`: input
clearance is `+40h`, computed separation is `+44h`. The push at `009D6A6C` means
`[ESP+44h]` at `009D6A77` still reads **input clearance**, and after the second
push `009D6A84 [ESP+38h]` stores it at baseline `+30h`, the temporary radius.

The first output aliases chosen at baseline `+18h/+1Ch`; it must retain chosen
on a non-writing status. The second output at `+20h/+24h` has no earlier write
in the entire listing. Its preexisting stack bits are now an explicit
`second_intersection_seed`. The default `{0,0}` preserves the old C++ call shape
and is a defined projection deviation, not evidence of native initialization.
No arbitrary invalid-output fix is applied to the geometry kernels.

The reused `004F3970` also needed a correction: `COMISS` at `004F3A36` followed
by `JBE` at `004F3A3B` **continues** on unordered. A NaN radius can therefore
reach the inside-chord arm and yield valid NaN outputs. Its earlier C++ negated
`<=` condition incorrectly returned false. Ordinary finite geometry is unchanged.

## Validation and limits

`reports/ship_ai_nav_circle_tangent.json` carries every native call-site pair,
the containing function, routine coverage, and correction evidence. The wrapper
exported all six routines after target verification. `verify-seeds` matched all
eight installed-image seed ranges. `./scripts/build.ps1` completed for MSVC Win32
Release with both existing CTest checks passing (`reconstructed_math` and
`native_math_differential`). `python tools/verify_report_calls.py
reports/ship_ai_nav_circle_tangent.json` checked 18 call rows with zero failures.

The ignored, manifested `local/circle_geometry_probe.cpp`, compiled and run by
`./local/run_circle_probe.ps1`, passed 22 checks: all five status values,
non-writing output retention, pair ordering, internal tangency, strict epsilon,
unordered radius, low-byte side truncation, explicit scratch retention, and one
stream-1 random fallback. Logs: `local/circle-build-final.log` and
`local/circle-probe.log`. This probe is fixture evidence; the existing native
differential test covers its original seed routines, not these new kernels.

No runtime frame claim or game integration is made. The existing tangent/chord
projection still uses double intermediates for some native x87 register spans,
with its documented division rounding limit; the concrete host reuses it rather
than silently promising exact 80-bit parity. The new intersection and relative
comparison kernels preserve the instruction schedules. Hardware x87 exception
state, unusual output aliasing, and original RNG/CRT process effects have not been
game-validated; callers supply the real borrowed RNG/CRT dependencies.
