# The LCP solver's per-constraint impulse math

Addresses: 00403720, 00C4F040, 00C31C30, 00C4DE40, 00C42BA0, 00C42530, 00C42230, 00C37B50,
00C35020, 00C41AE9..00C41B4A; read as structure 00403850, 00C5C7A0, 00C5C710, 00C431D0,
00C42ED0, 00C4F140, 00C437D0, 00C35160, 00C37C40, 00C350C0.

Packet `cc_dyn_constraints`, 2026-09-11. Reconstructed in `include/bsp/dyn_lcp_impulse_math.hpp`
and `src/dyn_lcp_impulse_math.cpp`; semantic C++ interfaces for MSVC Win32, not drop-in binary
replacements. Descriptive names are hypotheses, not recovered symbols. The saved project is
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`. This worker made no Ghidra
mutation; the ledger records the new names.

This is the `dyn_lcp_impulse_math` follow-up `docs/DYN_CONTACT_SOLVER.md` opened, which called
this "the last thing between that reconstruction and a contact response that matches the game".
The producer of the records it reads is `docs/DYN_COLLISION_PASS.md`.

## Which routines the ten iterations actually run

`docs/DYN_CONTACT_SOLVER.md` says "Each task runs, per group in its range, `00C5C7A0`
(`SolverPreStep`) then `00C5C710` (`SolveConstraints`)". Both of those have exactly one caller
and it is `Dyn_Scene_LCPSolver2Task_vslot0` (`00403850`), the class selected only by
`world+10h == 1`. The shipped world stores `0` there, so the task that runs is
`Dyn_Scene_LCPSolverTask_vslot0` (`00403720`), and its body is a different chain. See
Corrections.

`00403720`, `00403784..004037DB`, with the solver context at `[ESP+10h]` (a `0EAACh`-byte stack
object the `__alloca_probe` at `00403733` reserves) and the group at `[[task+8h]+44Ch] + i*0Ch`:

```
for (i = task+0Ch; i <= task+10h; ++i) {
    00C4F040(ECX = context, EAX = group[i], dt pushed);   // 0040379A, RET 4
    for (n = [[context+0h]+38h]; n != 0; --n) {           // 004037A3, world+38h = 10
        00C42530(ECX = context);                          // 004037B4
        00C42230(ECX = context);                          // 004037B9
    }
    00C37B50(EDX = context);                              // 004037C7
    00C35020(context pushed);                             // 004037CD
}
```

The register conventions are the compiler's own: `00C4F040` takes the context in `ECX` and the
group in `EAX`, `00C37B50` the context in `EDX`, and `00C42230` is called with no register setup
at all because `00C42530` leaves the context in `ECX`. These are link-time-code-generation
custom conventions on file-static functions, not any documented calling convention, so every
callee's register inputs below come from its own entry instructions.

`00C4F040` is only the per-group setup. It stores the group at `context+0EA68h`, sizes the row
block through `00C31C30` to `[group+4h] * 8` rows, calls `00C4DE40` (the row build, `dt` pushed),
grows and zeroes the per-body velocity accumulators to `context+4h` entries, and ends with
`00C42BA0` (`EAX` = context), the warm-start apply.

## The solver context

A `0EAACh`-byte stack object, private to this chain.

| field | meaning |
| --- | --- |
| `+00h` | the world. `+38h` of it is the iteration count, `+2Ch` the substep stamp |
| `+04h` | the solver body count; `00C4DE4C` starts it at 1 |
| `+08h + i*4` | the body with solver index `i` |
| `+0EA68h` | the group being solved, `{manifold** array, int count, int capacity}` |
| `+0EA6Ch`, `+0EA70h` | the velocity accumulator array (`30h` per body) and its capacity |
| `+0EA74h`, `+0EA78h` | the row block and its capacity |
| `+0EA7Ch` .. `+0EA9Ch` | the nine row arrays, below |
| `+0EAA0h` | the final solver body count |
| `+0EAA4h` | the normal row count (= the contact points in the group) |
| `+0EAA8h` | the first friction row, `[group+4h] * 4` |

`00C31C30` makes one allocation of `rows * 7Ch` bytes and carves it into nine parallel arrays,
two of `30h` bytes per row and seven of `4`:

| array | bytes/row | content |
| --- | --- | --- |
| `+0EA7Ch` | `30h` | J: `(J_linA, J_angA, J_linB, J_angB)`, twelve floats |
| `+0EA80h` | `30h` | M⁻¹Jᵀ: the same twelve premultiplied by each body's inverse mass and world inverse inertia |
| `+0EA84h` | 4 | two int16 solver body indices, A then B |
| `+0EA88h` | 4 | the velocity row's right-hand side |
| `+0EA8Ch` | 4 | the accumulated impulse |
| `+0EA90h` | 4 | the bias row's right-hand side |
| `+0EA94h` | 4 | the accumulated bias impulse |
| `+0EA98h` | 4 | the effective mass, `1 / (J · M⁻¹Jᵀ)` |
| `+0EA9Ch` | 4 | the friction coefficient, normal rows only |

Rows `[0, +0EAA4h)` are the normal rows, one per contact point in manifold-then-point order.
Rows `[+0EAA8h, +0EAA8h + +0EAA4h)` are the friction rows, one per normal row, at the same
offset within their block. The block is sized to eight rows per manifold because a manifold holds
at most four points (`00C3F943`) and each point makes one normal and one friction row.

**There is one friction row per contact point, not two.** The direction is rebuilt from the
sliding direction every pre-step (below), so a single row tracks it instead of a fixed basis pair.

## The per-body velocity accumulator

`30h` bytes, all twelve floats zeroed every pre-step at `00C4F0BA..00C4F131`:
`+00h` linear, `+0Ch` angular, `+18h` linear bias, `+24h` angular bias.

The two pairs are a split impulse. `00C37B50` adds the first pair to the motion state's
`M+00h`/`M+0Ch` and the second to `M+20h`/`M+2Ch`, which `docs/RIGID_BODY_INTEGRATION.md`
already documents as "a pseudo-velocity, added to the position but not to the velocity, cleared
every substep". So the bias rows correct penetration positionally and add no energy.

They hold velocity **changes**, not velocities: the right-hand sides already carry `-(J · u)`
from the pre-step, so the accumulators start at zero.

Solver index 0 is a shared slot for every static body (`00C4DEDB` writes `B+5Ch = 0` for any body
with `B+50h` bit 0, and `context+8h` is overwritten with whichever static body was seen last).
`00C37B50` starts its write-back at index 1, so a static body never receives an impulse even
though the apply steps write into its accumulator.

## `00C4DE40`, the row build

For each manifold in the group with a non-zero point count, and each of its points. The body
indexing comes first: a static body takes index 0; a dynamic body whose stamp `B+58h` is not the
world's `world+2Ch` takes the next index from `context+4h`, records it at `B+5Ch` and is appended
to `context+8h`.

Per point, with `A = manifold+0CCh`, `B = manifold+0D0h`, the motion states at `*(A+4h)` and
`*(B+4h)`, `n` the point's normal and `dt` the substep:

```
worldA = A.transform * point.localA          rA = worldA - A.position
worldB = B.transform * point.localB          rB = worldB - B.position

impulse[k]      = world+20h * point[+24h]          // 00C4E0B2, the warm start
impulseBias[k]  = point[+28h]                      // 00C4E0C6
bodyPair[k]     = (A.solverIndex, B.solverIndex)   // 00C4E11F, 00C4E138

J[k]  = (-n, -(rA x n), n, rB x n)                                    // 00C4E1F7..00C4E31E
MJ[k] = (invMassA*J0, invIA*J1, invMassB*J2, invIB*J3)                // 00C4E356..00C4E448

u        = (vB + wB x rB) - (vA + wA x rA)                            // 00C4E4A6..00C4E5CD
vn       = u . n
target   = min(0, vn * manifold[+04h] + 0.05)                         // 00C4E675..00C4E6A3
rhsVel[k]  = -target - (J[k] . bodyVelocities)                        // 00C4E6B2..00C4E763
rhsBias[k] = world+18h * clamp(point[+2Ch], 0, world+28h) / dt        // 00C4E0DC, 00C4E63F
effMass[k] = 1 / (J[k] . MJ[k])                                       // 00C4E7E8, FLD1/FDIVRP
friction[k] = manifold[+00h]                                          // 00C4EFA8, 00C4EFB9

impulse[f]  = 0                                                       // 00C4E807
t           = friction direction (below)                              // 00C4E828..00C4EB0C
J[f], MJ[f] = the same construction with t in place of n              // 00C4EB74..00C4EF7C
rhsVel[f]   = -(J[f] . bodyVelocities)                                // 00C4EEBE
effMass[f]  = 1 / (J[f] . MJ[f])                                      // 00C4EF85, 00C4EF95
```

where `k` is the normal row and `f = k + [group+4h]*4` the friction row.

Three literals are folded in. `00D7A270` is the double whose value is exactly `(double)0.05f`; it
is added to the restitution term, so an approach slower than `0.05 / restitution` produces no
bounce at all. `00D7A288` is `1.0e-6f`, the squared tangential speed below which the friction
direction falls back. `00D7A208` is `-0.0f`, the value every negation subtracts from.

The friction direction, `00C4E828..00C4EB0C`: take the dominant axis of `|n|`, build a
perpendicular by swapping that component with the one at `(1 << axis) & 3`, negating it and
zeroing the third, and normalise it to `p`; form `q = p x n`; project the relative velocity onto
`p` and `q` and normalise the result when its squared length exceeds `1.0e-6f`, otherwise keep
`p`. So a sliding contact gets a row exactly opposing the slide, and a contact at rest gets an
arbitrary but stable tangent.

The effective-mass divide at `00C4E7FA` is unguarded. A row whose two bodies are both static
would divide by zero; `00C4B610` never seeds a group from such a pair, which is what makes it
safe.

## `00C42BA0`, the warm-start apply

Per normal row: the first accumulator pair takes `MJ * impulse[k]`, the second `MJ * impulseBias[k]`.
Friction rows are not warm started. Without this the warm-started `impulse[k]` would only bias the
clamp, because the accumulators would still start at zero.

## `00C42530`, the normal rows

Per row `k` in `[0, +0EAA4h)`, two Gauss-Seidel sweeps over the same `J` and the same effective
mass:

```
r  = rhsVel[k] - J[k] . (velocity accumulators of A and B)
l  = r * effMass[k] + impulse[k];   if (l < 0) l = 0        // 00C42708
d  = l - impulse[k];  impulse[k] = l
apply MJ[k] * d to the velocity pair of A and B

r  = rhsBias[k] - J[k] . (bias accumulators of A and B)
l  = r * effMass[k] + impulseBias[k];  if (l < 0) l = 0
d  = l - impulseBias[k];  impulseBias[k] = l
apply MJ[k] * d to the bias pair of A and B
```

The clamp is one-sided in both sweeps: a contact may push, never pull.

## `00C42230`, the friction rows

Per row `f` in `[+0EAA8h, +0EAA8h + +0EAA4h)`, one sweep into the velocity pair only, with

```
limit = friction[f - base] * impulse[f - base]              // 00C42373, 00C42379
if (l < -limit) l = -limit; else if (limit < l) l = limit   // 00C423AB, 00C423BB
```

so the Coulomb cone uses the normal impulse **of the same iteration**, not the previous
substep's, and friction never touches the bias pair. Because friction runs after the normal rows
inside each of the ten iterations, the two converge together.

## `00C37B50` and `00C35020`, the write-back

`00C37B50` walks solver indices `[1, context+0EAA0h)` and adds each accumulator pair to the
matching motion state, then sets `B+58h` back to `-1`. `00C35020` walks the group's manifolds and
their points in the same order the builder produced the rows in, and copies `impulse[k]` into
`point+24h` and `impulseBias[k]` into `point+28h` (`00C3506B`, `00C35076`). Neither is scaled on
the way out; the attenuation is on the way in, at `world+20h`.

## The float model

The math is x87 (`FLD`/`FMUL`/`FADDP`) with the control word at 24-bit precision
(`docs/X87_CONTROL_WORD.md`), and the compiler spills each partial sum to a `float` temporary
(`00C4264F` `FSTP float ptr`, `00C42653` `FLD float ptr`), so a `float`-typed C++ expression
rounds at the same points. SSE appears only as `MOVSS` loads and stores. The one exception is the
`0.05` at `00D7A270` and the `0.5` at `00D7A280`, which are `FADD`/`FMUL double ptr`; both values
are exact as floats, so the reconstruction uses float literals.

The group orderings are the listing's and are preserved: a three-term group is
`(j1*v1 + j0*v0) + j2*v2`, and the four groups of a row residual are subtracted in the order
A-linear, A-angular, B-linear, B-angular in the iteration routines but A-angular, A-linear,
B-linear, B-angular in the row builder.

## The solver settings this settles

`docs/DYN_WORLD_SETTINGS.md` lists five carried world fields as unread. Three of them are read
here, each with register provenance for the world pointer:

| world | descriptor | shipped value | meaning | reader |
| --- | --- | --- | --- | --- |
| `+18h` | `+2Ch` | `0.1f` | the fraction of the overlap the bias rows remove per substep | `00C4E5D1` loads `EAX = [context+0h]`, `00C4E63F` reads `EAX+18h` |
| `+20h` | `+34h` | `1.0f` | the warm-start attenuation, so the shipped game warm starts fully | `00C4E094` loads `EAX`, `00C4E0AF` reads `EAX+20h` |
| `+28h` | `+3Ch` | `0.5f` | the deepest penetration the bias rows will act on | `00C4E0DA` loads `EDX`, `00C4E0DC` reads `EDX+28h` |

`world+2Ch`, which is not a descriptor field, is the per-substep body-indexing stamp
(`00C4DEB6` loads `EBX = [context+0h]`, `00C4DEBE` reads `EBX+2Ch`).

`world+14h` (descriptor `+28h`, `0.0f`) and `world+1Ch` (descriptor `+30h`, `0.85f`) have **no
reader** in anything this packet read: not `00C4F040`, `00C31C30`, `00C4DE40`, `00C42BA0`,
`00C42530`, `00C42230`, `00C37B50`, `00C35020`, and not the collision pass of
`docs/DYN_COLLISION_PASS.md`, which takes the scene rather than the world. Finding their readers
is the remaining part of `dyn_world_solver_settings`.

## Corrections

**To `docs/DYN_CONTACT_SOLVER.md`, "The solver split".** "Each task runs, per group in its
range, `00C5C7A0` (`SolverPreStep`) then `00C5C710` (`SolveConstraints`)" is true of the
`LCPSolver2Task` only. `00C5C7A0` and `00C5C710` each have one caller and it is `00403850`.
The shipped `LCPSolverTask` body `00403784..004037DB` calls `00C4F040`, then `00C42530` and
`00C42230` `world+38h` times, then `00C37B50` and `00C35020`. Evidence: `python tools/bsp.py
ghidra disasm 00403720`, and `callers` for `00C5C7A0` and `00C5C710`.

**To `docs/DYN_CONTACT_SOLVER.md`, the `dyn_lcp_impulse_math` follow-up row.** Its address list
(`00C431D0`, `00C42ED0`, `00C4F140`, `00C437D0`, `00C37C40`, `00C350C0`, `00C35020`) is the
`LCPSolver2Task` chain plus one routine of the shipped one. The shipped chain is `00C4F040`,
`00C31C30`, `00C4DE40`, `00C42BA0`, `00C42530`, `00C42230`, `00C37B50`, `00C35020`. This packet
read the shipped chain in full and left the `LCPSolver2Task` chain as structure only.

**To `docs/DYN_CONTACT_SOLVER.md`, `DynContactPoint::normal_scale` and the "provisional"
paragraph.** "Penetration depth and accumulated normal impulse both fit; this packet does not
choose." It is the accumulated normal impulse. `00C4E0B2` seeds the row's impulse accumulator
from it, `00C35020` writes the solved impulse back to it, and `00C3F9BD` zeroes it only when the
narrow phase appends a brand-new point. The penetration depth is a different field, `point+2Ch`.
The record is `30h` bytes with six fields, not four: `+24h` accumulated normal impulse, `+28h`
accumulated bias impulse, `+2Ch` penetration depth. `include/bsp/dyn_lcp_impulse_math.hpp`
declares the whole record as `DynSolverContactPoint`; the four-field `DynContactPoint` in
`include/bsp/dyn_contact_solver.hpp` is left alone because this packet does not hold that header.

**Corroborating, not correcting, `reports/dyn_step.json`.** That report already records that
`docs/DYN_WORLD_SETTINGS.md`'s descriptor row `desc+38h -> world+24h` does not exist. Reading
`00C41AE9..00C41B4A` independently agrees: the copies are `desc+28h -> world+14h` (`00C41B0C`),
`+2Ch -> +18h` (`00C41B13`), `+30h -> +1Ch` (`00C41B19`), `+34h -> +20h` (`00C41B1F`),
`+3Ch -> +28h` (`00C41B25`), and `desc+38h` is never read.

## Coverage

| routine | state | coverage |
| --- | --- | --- |
| `00403720` | the per-group loop reconstructed as `dyn_solve_group_00403720`, build-tested | complete for the chain; the exception frame and the two frees at `004037EA`/`004037FE` are not projected |
| `00C4F040` | read in full | complete; the allocation and growth are host-side, so the reconstruction takes the arrays already sized |
| `00C31C30` | read in full, the nine array bases are the header's constants | complete |
| `00C4DE40` | reconstructed per point as `dyn_build_contact_rows_00c4de40`, build-tested, one test case | partial: the per-manifold and per-body loop around the point body, including the solver-index assignment at `00C4DEDB..00C4DF43`, is expressed as inputs rather than reconstructed |
| `00C42BA0` | reconstructed as `dyn_apply_warm_start_00c42ba0`, build-tested | complete |
| `00C42530` | reconstructed as `dyn_solve_normal_rows_00c42530`, build-tested, one test case | complete |
| `00C42230` | reconstructed as `dyn_solve_friction_rows_00c42230`, build-tested, one test case | complete |
| `00C37B50` | reconstructed as `dyn_write_back_velocities_00c37b50`, build-tested | complete for the arithmetic; the `B+58h = -1` reset is host-side |
| `00C35020` | reconstructed as `dyn_store_impulses_00c35020`, build-tested | complete for the arithmetic; the manifold walk that produces the row order is host-side |
| `00403850`, `00C5C7A0`, `00C5C710` | read for the chain they run | none of `00C431D0`, `00C42ED0`, `00C4F140`, `00C437D0`, `00C35160`, `00C37C40`, `00C350C0` is reconstructed |
| `00C431D0` | read for its shape only | none. It is the same formulation over a manifold-major layout: `context+0EAC4h` manifolds, `context+0EA88h` per-manifold point counts, impulse arrays strided `10h` per manifold. Unreachable in the shipped world |

## Run-time evidence

The reconstruction is not on any path `bsp_game.exe` or the probes reach. `src/ship_motion_probe.cpp`
drives one hull in open water through `00C41550` and `00C5B1B0` only; its own banner says the hull
collision AABB producer `00C5C940` is unread and nothing supplies contacts. With no manifolds,
`00C4B610` forms no groups, `00C5BC06` finds the group count zero and the solver tasks never run,
so this chain is unreachable there. The probe is unchanged by this packet. The one added test case
in `tests/math_tests.cpp` builds a contact by hand instead: a 1000 kg hull one substep into
gravity resting on a fixed box, ten iterations, asserting the reconstructed math returns the
substep's weight `m * g * dt` as the normal impulse and leaves friction at zero.

## Follow-up packets

| packet | addresses | what it answers |
| --- | --- | --- |
| `dyn_lcp_solver2` | `00C5C7A0`, `00C5C710`, `00C431D0`, `00C42ED0`, `00C4F140`, `00C437D0`, `00C35160`, `00C37C40`, `00C350C0` | the manifold-major second solver, and whether anything but `004DDB90` ever authors `world+10h == 1` |
| `dyn_world_solver_settings` | `world+14h`, `world+1Ch` | the last two carried fields. `+18h`, `+20h` and `+28h` are settled here, so the follow-up is three fields smaller |
| `dyn_manifold_point_reduction` | `00C3FA46..00C3FFD5` | which of five candidate points a full manifold keeps |
| `dyn_broad_phase_sap_radix` | `[scene+0ACh]` vslots 3 to 6 | the SAP radix manager itself, still a contract |

## no_ghidra_function

| start | inclusive end | evidence |
| --- | --- | --- |
| none | | Every address named or reconstructed in this document lies inside an existing Ghidra function body, checked with `python tools/bsp.py ghidra proto <addr> --brief` for `00403720`, `00C4F040`, `00C31C30`, `00C4DE40`, `00C42BA0`, `00C42530`, `00C42230`, `00C37B50`, `00C35020` and `00C41AD0`. |
