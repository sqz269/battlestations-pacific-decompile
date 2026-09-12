# Actual particle Axial/Floating state initialization

Addresses: `00B059C0`, `00B077D0`.

| Routine | Proposed descriptive name | Original ABI | Coverage |
| --- | --- | --- | --- |
| `00B059C0..00B05CDA` | `BSP_ParticleAxial_InitializeState` | ECX actual definition; stack(actual6Ch state, actual108h record); RET8 | complete |
| `00B077D0..00B07BE4` | `BSP_ParticleFloating_InitializeState` | same | complete |

The last instructions begin at B05CD8 and B07BE2 and occupy three bytes;
the table uses inclusive function ends. These are hypothesis names. Original
EAX is incidental, including LAHF-modified pointer bits and frame DIV quotient;
the original return type is unestablished. The C++ interfaces return void and
do not normalize it. EDX supplies required
borrowed `NativeParticleTypeStateAccess`, so this is a new C++ ABI.

## Identity, callers and producers

Live Ghidra queries verified `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, through the repository client before each batch.
The full incoming reference sets are D5DCD8/D5DF48 for B059C0 and
D5DD04/D5DFC8 for B077D0; there are no recovered direct callers. Table bytes
establish actual Axial D5DCC0+18 and Floating D5DCEC+18. The base constructor
tables D5DF30 and D5DFB0 have the same respective slots. B058E0 installs
D5DF30 at B05919, and B076F0 installs D5DFB0 at B0771D. Their destruction
paths also restore these base tables. B00CE0 allocates A4h Axial storage,
calls B058E0 and replaces its table at B00D9F; the Floating path allocates
8Ch and calls B00770, which calls B076F0 then installs D5DCEC at B00789.
These constructor/parser routines are evidence dependencies, not reconstructed
by this packet.

The recovered common caller B0CA40 produces the 6Ch state: +64 definition,
+68 emitter, +60 null light, initial position and direction vectors, and
+24/+28/+2C copied from the actual 108h record. At B0CAD5 it reloads current
definition, captures its table+18 target at B0CADB, pushes record then state,
and calls EAX at B0CADF. Both initializers accept but never read the second
record argument. This call boundary must use the actual captured target.
Shape/profile tables are different types and are not substitute particle
definitions. Other particle targets require their own implementation.

B01150 initializes the common definition's curve-pointer slots +1C, +2C,
+30, +34, +38 and +5C, signed frame words +50/+54 and flags +60/+61.
B058E0 adds Axial +8C; B076F0 adds Floating +80/+84/+88. No packed duplicate
definition/state class or speculative ownership representation is introduced.
Initialized parameter curves remain required real input storage.

## Complete behavior

Each random factor first loads the current curve+00 amplitude, compares it
with current D7A218 using UCOMISS/LAHF/TEST AH,44h, and uses current D7A24C
only on ordered equality. Otherwise it draws stream zero through the existing
application `RandomThreads::next_00bd2fc0`. Although that result is uint32,
native FILD consumes its bits as **signed int32**. FMUL amplitude, FMUL current
double D5DA30, FADD current double D7A210 and the original float spill follow.
No unsigned conversion correction, clamping or finite-value check is added.
Unordered comparison takes the random path.

Both routines evaluate the current definition+1C curve at time zero: mode
word +0A zero copies curve+04, one calls the existing exact AFFA70 linear
evaluator, all other modes call AFFAE0 cubic. Multiplication by the prior
factor stores state+44. They then multiply existing state+18/+1C/+20 by the
definition+2C factor, preserving their different native x87 operand order.
Factors from definition +30/+34/+38/+5C store state +48/+4C/+5C/+3C.
Axial additionally maps +8C to state+58. Floating evaluates definition+80 at
time zero, multiplies by current double D5DAF8 then its random factor, and
stores state+50; factors +84/+88 store state+54/+58.

If flag +60 is set, the first frame uses a stream-zero uint32 draw modulo
the wrapping uint32 width `(last - first + 1)` plus the signed first-frame
bits; otherwise it uses the signed first frame. Native CVTSI2SS stores +34
then +30. Flag +61 selects a second draw/range operation for +38; otherwise
the current +30 float is added to the current D7A210 double. A zero width
retains native DIV-by-zero behavior. Floating increments current F8D384
after storing +38 on either successful branch, with wrapping ADD semantics.

All definition, state and global loads remain at their original sites,
including globals aliased to output and curve amplitudes produced by earlier
state writes. The added EBP save holds borrowed access; the native temporary
EBP frame minimum saves/reloads that access around its random draw. Float32
spills, x87 stack lifetimes, MXCSR-controlled signed frame conversions and
the caller's x87 control/status effects remain. There are no math callbacks,
invented globals, substitute particle owners or success fallbacks.

## Callees and validation

| Native | Read body / concrete binding | Original contract |
| --- | --- | --- |
| `00BD2FC0` | Calls BD2ED0 then tail-jumps BA2C20; existing `RandomThreads` | ECX stream zero; EAX raw uint32; RET |
| `00AFFA70` | Existing `evaluate_native_particle_linear_curve_00affa70` | ECX actual curve; stack float zero; ST0; RET4 |
| `00AFFAE0` | Existing `evaluate_native_particle_cubic_curve_00affae0` | same; original unbounded segment walk and cubic x87 sequence |

Every direct call is listed with exact containing function and instruction
address in `reports/native_particle_type_state.json`: 11 Axial and 15 Floating
calls; no tail transfers occur in these two routines. Callee bodies and all
target/table references were read. Library/service callees are reused, not
claimed as newly reconstructed.

Scratch evidence: `C:/Users/sqz269/bsp-ap-state`. `make_fixture.py` compares
the four complete installed-byte function spans (both initializers and both
curves) and ten table/global spans with live saved Ghidra bytes. All matched.
The installed EXE SHA256 is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Its PE relocation directory is empty; Capstone decoded every instruction,
relative call and absolute data operand before fixture rebinding. Both runs
use one authentic primary `RandomThreads` domain restored from the identical
complete native state. The original branch executes original curve bytes;
the rebuilt branch calls the existing C++ curve implementations.

`build.ps1` compiled the actual new source with MSVC Win32, `/O2 /MD /W4 /WX
/fp:strict`, linked the existing `bsp_core.lib`, and embedded the manifest.
`record_results.py` captures the actual executable subprocess exit, which is
zero. All **72** cases passed: two routines, twelve risk scenarios, and x87
24/53/64-bit precision. Checks compare every 6Ch state, 108h record and
definition byte, complete primary RandomState, counter, incidental EAX,
x87 status/control, and a preexisting x87 sentinel. Cases cover Const/Linear/
Cubic at zero, sign-bit draws (48 first draws), qNaN/sNaN/ordered equality,
signed frames, singleton/wrapping/zero ranges, current state/global aliases,
and the counter aliasing final frame storage. Native DIV faults also match.
Default scalar bits match the installed image, including D5DAF8's widened
float value `3f91df4660000000`, not an invented higher-precision pi/180.

Commands: `python C:/Users/sqz269/bsp-ap-state/make_fixture.py`,
`powershell -NoProfile -ExecutionPolicy Bypass -File C:/Users/sqz269/bsp-ap-state/build.ps1`,
`python C:/Users/sqz269/bsp-ap-state/record_results.py`, and
`python tools/verify_report_calls.py reports/native_particle_type_state.json`.
The report records source/executable/library hashes and captured results.
This is complete reconstruction plus isolated compilation and native fixture
validation. Whole-project build and composed dispatcher replay belong to
integration; binary replacement compatibility and game validation are unproven.
