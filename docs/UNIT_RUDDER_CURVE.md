# Rudder curve and adjacent unit routines

Packet `orch2_unit_rudder_curve`, 2026-09-10. Reconstructed in
`include/bsp/unit_rudder.hpp` and `src/unit_rudder.cpp`; semantic interfaces for
MSVC Win32, not drop-in native replacements. Descriptive names are hypotheses.
The saved project is `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. Each export/live query used the tools' target
verification. No Ghidra mutation was performed by this worker.

## Curve established

The previously missing `0082ECB0` curve is:

```
r    = float(forwardSpeed / class.MaxSpeed)
d    = curve(abs(r))
base = float(class.MaxRotAngle / d)
rate = float(((x87(base) * r) * rudder) * turnEfficiency)
```

The sign survives in `r`; only the denominator receives its magnitude. Class
`+4F8h` is the existing `ShipClassFields::max_rot_angle` (`MaxRotAngle`, field
loader at `00831882`), and `+500h` is its `max_speed`. These existing projections
are reused. Neither division has a zero check. Rudder and efficiency are not
clamped here. The curve does not impose a cap on the signed speed ratio.

`0082E890` evaluates a two-segment clamped interpolation using these settings:

| point | speed coordinate | denominator value |
| --- | --- | --- |
| first | `+444h` | `+440h` |
| split | `+44Ch` | `+448h` |
| last | `+43Ch` | `+438h` |

The segment through first/split is selected for `magnitude <= split.speed`;
otherwise it uses split/last. Unordered comparison takes the second segment.
No assumptions about the order or default values of those settings are added.
The settings getter `00424C40` runs four times: once to capture `+44Ch`, then
three times to retain the objects from which that segment's remaining fields
are read. The host reproduces these calls and subsequent read order, allowing
the getter's objects to alias. The curve is not a lookup table or polynomial.

`00419010(x0,y0,x1,y1,x)` first returns `y0` for ordered `x1 == x0`.
Otherwise it evaluates `((x-x0)/(x1-x0))*(y1-y0)+y0` in x87 intermediates, stores
to float at `0041905F`, and clamps to the two ordinates. The implementation
keeps that intermediate x87 expression in inline assembly and retains the
comparison directions, including the native unordered fall-through behavior.

`0082ECB0` stores the normalized speed at `0082ECBE`, captures MaxRotAngle as
double before calling the denominator helper, stores the divided base at
`0082ECF1`, and stores the final three-product chain at `0082ED03`. Inline x87
preserves these stores and arithmetic order; ordinary MSVC `double` or
`long double` would not establish equivalent extended intermediates.

## Unit wrappers

`00811890` loads the optional manager `00F88C30` and the class pointer at
unit `+538h`. If the manager exists, byte `00E0C978` and manager `+C4h` jointly
gate `008E6430(5, unit)`. Its float result is the gameplay multiplier; a closed
gate supplies `1.0f`. The turn efficiency `+9DCh` is captured before querying
forward speed through controller `+1018h` and existing routine `0092D730`.
Then it invokes the class curve. A present manager causes a final x87 multiply
and float store, even when the multiplier is 1; an absent manager returns the
curve result directly. `0092D730` already stores its return to float at
`0092D765`, so the semantic float host boundary adds no speed-return rounding.

`00811940` loads unit `+984h`, calls `00811890`, and returns the x87 value.
The saved decompiler's `void` return is wrong; `00811959` returns with ST0 live.
The existing `UnitCommandMotionHost` can delegate its rudder-map operation to
this module; its unrelated call sequence was not changed.

## Heading command and shifted update

`00811960` is a command-object method, not a unit scalar accessor. It obtains
the object `+50h` unit's virtual `+50h` result, stores that to float, and queries
the unit controller's forward speed. For speed `<= -1.0f`, it adds the game's
pi and wraps. It then wraps `(heading - desired)`, clamps the result to
`[-pi/4, pi/4]`, wraps `(heading - clampedDifference)`, stores to object `+44h`,
and sets byte `+4Ch` to 1. Thus a requested heading is limited to a 45-degree
change about the current direction, adjusted for reverse travel. The heading
interpretation of virtual `+50h` remains provisional; the call and arithmetic
are established. Caller `009F4D10` supplies its `+324h` field.

The shared `00438AA0` and `00438B10` helpers add or subtract two float arguments,
store that operation to float, and wrap into `(-pi, pi]`, with a float store on
every wrap iteration. These are not `void` functions despite their saved
signatures. There is no `fmod` substitution. Native constants are:

| native address | type | exact value used |
| --- | --- | --- |
| `00D7A260` | float | -1 |
| `00D7A264` | float | 3.1415927410125732421875 |
| `00CE3D18`, `00CE3D28` | double | negative/positive 3.1415927410125732421875 |
| `00CE3828` | double | 6.283185482025146484375 |
| `00D09448`, `00CEDCD0` | double | negative/positive 0.785398185253143310546875 |
| `00D09440`, `00CEB5A8` | float | the same negative/positive quarter-pi |
| `00D7A308` | double | 2 |

`00811AB0` instead receives ECX = unit `+310h`. In order it:

1. Multiplies the supplied delta by field `+340h` only when that field is >1.
2. Uses existing `004134F0` to copy the matrix from unit `+674h` to `+74h`.
3. Calls `0092F930` on controller `+1018h` with the resulting float delta.
4. Calls unit virtual `+D8h`.
5. Reads timestamp-like field `+308h`; if nonzero and less than
   `double(clock_00F876A4)+2.0`, zeroes float field `+2F8h`.

The C++ sequence uses explicit field references so host-side changes made by
steps 3/4 are visible at step 5. The fields' semantic identities and the bodies
of `0092F930` and virtual `+D8h` remain external. This packet does not claim a
reconstruction of those callees, the gameplay scaling policy, or the physics
body implementation. No default host methods or invented global objects exist.

## Native ABI and boundaries

All listed entries already have Ghidra functions. `no_ghidra_function` is empty;
no free/no-return fall-through defect was found. The expanded routines and
`00811AB0` were also checked with read-only `bsp.py ghidra flow` (zero gaps).

| entry | native input / return | final instruction | length |
| --- | --- | --- | --- |
| `00419010` | 5 stack floats; ST0; RET 14h | `004190CC` | 3 |
| `00438AA0` | 2 stack floats; ST0; RET 8 | `00438B0B` | 3 |
| `00438B10` | 2 stack floats; ST0; RET 8 | `00438B7B` | 3 |
| `00811890` | ECX unit, stack float; ST0; RET 4 | `0081193C` | 3 |
| `00811940` | ECX unit; ST0; RET | `00811959` | 1 |
| `00811960` | ECX command object, stack float; void; RET 4 | `00811A22` | 3 |
| `00811AB0` | ECX unit+310h, stack float; void; RET 4 | `00811B48` | 3 |
| `0082E890` | stack float; ST0; RET 4 | `0082E959` | 3 |
| `0082ECB0` | ECX class, 3 stack floats; ST0; RET Ch | `0082ED0E` | 3 |

The non-member float helpers are callee-clean, stdcall-shaped ABIs. Stack
cleanup does not establish their original source declarations. No prototype
edits were applied; several saved signatures still say undefined/void.

## Verification and limits

`scripts/build.ps1` passed with MSVC Win32 Release and `/fp:strict`. After
`python tools/ghidra_export.py verify-seeds` matched all 8 existing seeds, the
build's existing `reconstructed_math` and `native_math_differential` tests both
passed (2/2). No repository test cases or test framework were added.

One local composed-curve differential probe passed 55 finite fixtures bitwise,
with 220 settings-getter calls on each implementation. It executes the complete
saved instructions of `00419010`, `0082E890`, and `0082ECB0`, relocated only for
their direct calls and the fixture singleton getter. All 492 instruction bytes
matched the installed executable before execution. The fixtures include both
speed signs, signed zero, near-zero speed, both sides of the split, and speeds
beyond the final knot. Exact commands, hashes and local artifact paths are in
`reports/unit_rudder_curve.json`. Those ignored probe artifacts belong to this
worktree; they are not automatically present in another checkout.

This finite-fixture result is not a claim of universal floating-point or ABI
equivalence. NaN payloads, signaling exceptions, alternate x87 control words,
zero divisors, and heading-loop nontermination were not differentially tested.
The angle loops deliberately retain the native lack of bounds: infinite or
very large inputs can fail to make progress. Unit host integration and gameplay
motion have not been executed. The wrapper/heading/shifted routines are
reconstructed and build-tested; only the composed class curve is fixture-tested.
