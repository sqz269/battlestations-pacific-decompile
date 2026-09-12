# Rudder curve and adjacent unit routines

Addresses: 00419010, 00438AA0, 00438B10, 00811890, 00811940, 00811960, 00811AB0, 0082E890,
0082ECB0, and from packet cc_ship_inputs 0083B5E0, 00424C40, 008E6430, 0080FC30.

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

---

# The curve's producer and its shipped values

Packet `cc_ship_inputs`, 2026-09-11, added everything below. Reconstructed in
`include/bsp/unit_rudder_curve.hpp` and `src/unit_rudder_curve.cpp`. The existing
`UnitRudderCurveSettings` in `include/bsp/unit_rudder.hpp` is reused unchanged.

The earlier packet established what `0082E890` reads and deliberately added "no assumptions
about the order or default values of those settings". This part answers where the values come
from and what the installed game puts there.

## The block is on the settings singleton, not on the unit

The packet brief described the block as "the rudder curve settings block at unit
`+438h..+44Ch`". It is not a unit field. `0082E890` calls `00424C40` four times
(`0082E893`, `0082E8B0`, `0082E8B7`, `0082E8BE` on the low branch, `0082E906`, `0082E90D`,
`0082E914` on the high one) and reads `+438h..+44Ch` off each returned pointer. `00424C40`
is the gameplay settings singleton on `[00F8753C]`: `76Ch` bytes, allocated once at
`00424C9A`, constructed by `00424A10`, registered with `BSP_SingletonLifetime_Register`
(`docs/VEHICLE_CLASS_LUA_LOAD.md`). It takes no arguments and no unit, so the curve is one
global table shared by every ship in the mission, not a per-class or per-hull tuning.

## The producer is `0083B5E0`

`00424A10` does not initialise the block; a pass over its body (`00424A10..00424C15`) finds
no store to any offset in `+438h..+44Ch`. A byte-pattern scan of `.text` for a float store to
each of the six offsets finds exactly one writer of all six, `0083B5E0`, the Lua-driven
settings loader `00424A10` tails into:

| store | field | Lua expression |
| --- | --- | --- |
| `0083CEDE` | `+444h` | `TurnMultiplierMinSpeed[1]` |
| `0083CF4D` | `+440h` | `TurnMultiplierMinSpeed[2]` |
| `0083CFBC` | `+44Ch` | `TurnMultiplierMedSpeed[1]` |
| `0083D02B` | `+448h` | `TurnMultiplierMedSpeed[2]` |
| `0083D09A` | `+43Ch` | `TurnMultiplierMaxSpeed[1]` |
| `0083D109` | `+438h` | `TurnMultiplierMaxSpeed[2]` |

Index 1 is the speed coordinate and index 2 the denominator in all three pairs, which agrees
exactly with the first packet's point table read from `0082E890`.

The fragment `0083CE56..0083D10D` is six copies of one block plus a sub-table step:

```
0083CE56: 00B67700(temp)                                   ; BSP_LuaObject_Destruct
0083CE6F: t = 00B67800([ESP+0BCh], &out, "TurnMultipliers"); BSP_LuaObject_GetByName
0083CE84: 00B67690([ESP+0BCh], t)                          ; assign into the current-table slot
          ; then, six times:
0083CE98: 00B67700(temp)
0083CEB1: a = 00B67800([ESP+0BCh], &out, <key>)
0083CECA: e = 00B67720(a, &out, <1 or 2>)                  ; BSP_LuaObject_GetByIndex
0083CED9: f = 00B66270(e)                                  ; BSP_LuaObject_GetNumber, ST0 float
0083CEDE: [ESI + <offset>] = f
```

The assignment at `0083CE84` is what makes the three key lookups run against the
`TurnMultipliers` sub-table rather than its parent: `[ESP+0BCh]` is the fragment's
current-table wrapper, and the surrounding reads (`0083CDD7` and `0083CE19` fetch
`HdgDiffDangerMul` and its neighbour into `+6E8h` and `+6ECh`) are the `AutoThrust` fields,
so the parent is `ShipGlobals["Navigator"]`. The four `00B67xxx` helpers are already
reconstructed in `include/bsp/gui_lua_reader.hpp` and `include/bsp/lua_numeric.hpp`;
`00B67690` is not, and is `contract: unread` beyond "assigns the wrapper".

Only `0083CDC0..0083D10D` of `0083B5E0`'s `0083B5E0..00842951` body was read. The ledger name
says so.

## The values the installed game ships

`I:/SteamLibrary/steamapps/common/Battlestations Pacific/scripts/datatables/shipglobals.lua`,
inside `ShipGlobals["Navigator"]["TurnMultipliers"]`:

| key | `[1]` | `[2]` | settings fields |
| --- | --- | --- | --- |
| `TurnMultiplierMinSpeed` | `0.0` | `0.4` | `+444h`, `+440h` |
| `TurnMultiplierMedSpeed` | `0.5` | `1.5` | `+44Ch`, `+448h` |
| `TurnMultiplierMaxSpeed` | `1.0` | `2.0` | `+43Ch`, `+438h` |

All three rows carry the same Hungarian comment: *"mekkora gazkar allasnal mennyivel
szorzodjon a vehicleclass-ban megadott fordulokor sugara. Elso ertek a gazkar allasa,
masodik a fordulokor szorzoja"* - at what throttle setting the turning-circle radius given in
the vehicleclass is multiplied by how much; first value the throttle setting, second the
turn-circle factor. That is the meaning of the denominator: `0082ECB0` divides `MaxRotAngle`
by it, so a larger turning circle is a smaller yaw rate, and the three points are authored
against the throttle fraction, which is what the normalised speed `speed / MaxSpeed` is.

The curve is therefore, for `r = speed / MaxSpeed`:

```
|r| <= 0.5 : d = clamp(0.4 + (|r| / 0.5) * 1.1, 0.4, 1.5)
|r| >  0.5 : d = clamp(1.5 + ((|r| - 0.5) / 0.5) * 0.5, 1.5, 2.0)
```

At full throttle `d = 2.0`, so a hull turns at half its `MaxRotAngle`; at rest `d = 0.4`,
though the rate is multiplied by `r` afterwards and so still goes to zero.

The same file has a second `ShipGlobals["Fordulas"]` table at line 396 with the same three
keys and `TurnMultiplierMinSpeed = { 0.0, 0.5 }`. No push of a `"Fordulas"` key appears in
the fragment read here, and nothing in this packet shows it being loaded; it reads as a
legacy duplicate, but that is an absence of evidence, not a proof.

## `008E6430`, the gameplay-modifier product

`UnitRudderHost::gameplay_scale_008e6430` had no body behind it. `008E6430` is
`float __thiscall(manager, int category, unit)`, `RET 8`, body `008E6430..008E649D`. The
list object is at `manager + 80h + category*0Ch`, its sentinel node pointer at `[list+4]`.
The accumulator starts at the `1.0f` at `00D7A24C` and each node whose filter `008E4680`
accepts the unit multiplies in that node's `+1Ch`. An empty or fully filtered list returns
exactly `1.0f`.

`0080FC30` (`BSP_UnitInstance_GetReferenceSpeed`) uses category `4` and gates the call twice:
the byte at `00E0C978` must be set and the list size at `manager+B8h` must be non-zero,
otherwise it substitutes the same `1.0f` literal. So the `1.0` a host returns for a mission
with no modifiers is the routine's own value, not a placeholder. `008E4680`'s body, a reject
chain over the unit's kind, owner and index fields, is **not** reconstructed; the host method
`entry_matches_008e4680` is `contract: unread`.

## What this changed in the probe

`src/ship_motion_probe.cpp` carried an identity curve (all three denominators forced to
`1.0`), a hand-written `0.0f` ocean height, a hand-written `1.0f` gameplay scale and an
explicit-Euler integrator. All four are replaced. Twenty seconds at full throttle and hard
over, `--steps 400 --dt 0.05 --throttle 1.0 --rudder 1.0`:

| | class 11 before | class 11 after | class 23 before | class 23 after |
| --- | --- | --- | --- | --- |
| peak forward speed / class reference | `0.9972` | `0.9993` | `0.9977` | `0.9994` |
| peak yaw rate (rad/s) | `0.15708` | `0.07854` | `0.13963` | `0.06981` |
| peak yaw / `MaxRotAngle` | `1.0000` | `0.5000` | `1.0000` | `0.5000` |
| forward speed at t = 20 s | `10.8477` | `17.4030` | `13.5392` | `18.7036` |

The yaw halving is the shipped curve: the denominator at full throttle is `2.0`, so the
steady rate is `MaxRotAngle / 2` and not `MaxRotAngle`. The speed differences follow from it
rather than from the integrator: a slower turn keeps more of the velocity on the hull's
forward axis, and `0092D300` only rewrites the axial component, so less speed is lost to the
lateral one. Class 11 is `Allen M. Sumner class 1945` (`MaxSpeed 17.4911`,
`MaxRotAngle 0.15708`); class 23 has `MaxSpeed 18.7772`, `MaxRotAngle 0.13963`.

## Routines and coverage, packet `cc_ship_inputs`

| routine | state | coverage |
| --- | --- | --- |
| `0083B5E0` | named and documented for the curve block | partial: only `0083CDC0..0083D10D` of `0083B5E0..00842951` read |
| `00424C40` | named; the lazy-singleton head read | partial: the registration tail `00424CA0..00424CFF` not read |
| `008E6430` | reconstructed, build-tested, probe-exercised | complete |
| `008E4680` | not read | none; the filter is a host method |
| `0080FC30` | re-read for the `008E6430` gate | complete for that gate, reused otherwise |
| `00B67700`, `00B67800`, `00B67720`, `00B66270` | reused as reconstructed in `docs/GUI_LUA_READER.md` | as reconstructed there |
| `00B67690` | not read | none; `contract: unread` |

## Corrections

**To the packet brief, "the rudder curve settings block at unit `+438h..+44Ch`".** The block
is on the gameplay settings singleton `00424C40` returns, not on the unit instance. The
follow-up name `ship_rudder_curve_settings` and the suggestion that the settings object's
constructor fills it are both answered by `0083B5E0`, not by `00424A10`.

**To `docs/SHIP_MOTION.md`, the stand-in list.** It records the curve, the ocean sampler, the
gameplay scale and the integrator as four stand-ins that "keep the result from being the
game's numbers". All four now have producers; what remains between the probe and the game's
numbers is the hull body's mass, inertia and damping, the force channel at `unit+10D4h` and
the library's substep size. See `docs/RIGID_BODY_INTEGRATION.md`.

**To `docs/UNIT_CONTROLLER_UPDATE.md`, the settings row at `+4E0h + variant*38h`.** Not
edited here, but recorded because the same loader produces it: the row is
`ShipGlobals["Physics"]["Ship" | "TBoat" | "Submarine"]`, `38h` = fourteen floats, in order
`KozegellenallasiEgyutthatoL`, `...N`, `...LFel`, `...NFel`, `...LOldalra`, `...NOldalra`,
`...LElore`, `...NElore`, `NyomatekSzorzo[3]`, `Kitevo`, `Gravitacio`, `Friction`. That makes
`+50Ch` (`+4E0h + 2Ch`) the `Kitevo` exponent, whose shipped values `1.0` (Ship) and `0.5`
(TBoat) are exactly the constants the routine branches on, and `Gravitacio = 10.0` the source
of the `10.0` double at `00CE3DC0`. The doc's note that variant 1 forces `+50Ch` to `2.0f`
conflicts with the shipped `TBoat` value of `0.5`; a re-read of the branch around `00932B00`
should settle which. This mapping is inferred from the key names and the row stride, not from
a read of the loader's stores into that range. Follow-up `ship_hydro_settings_row`.

## Follow-up packets

| packet | addresses | what it answers |
| --- | --- | --- |
| `ship_hydro_settings_row` | `009329C0`'s `+4E0h`..`+50Ch` reads, `0083B5E0`'s stores to the same range | bind each drag coefficient to its axis using the named Lua keys, and settle the variant-1 `+50Ch` question |
| `gameplay_modifier_filter` | `008E4680`, `009FFD20`, the producers of `manager+80h..` | what a modifier record filters on and who registers one |
| `settings_lua_block_map` | `0083B5E0` in full | the whole `76Ch` settings layout as a key-to-offset table, which would retire several "not recovered" notes at once |

## no_ghidra_function

none. `0083B5E0`, `00424C40`, `008E6430` and `0080FC30` each lie inside an existing Ghidra
function body, checked with `python tools/bsp.py ghidra proto <addr> --brief`.
