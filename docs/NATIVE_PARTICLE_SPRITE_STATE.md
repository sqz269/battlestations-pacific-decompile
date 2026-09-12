# Actual Sprite particle state initialization and cleanup

Addresses: `00B08F60`, `00B007F0`.

| Routine | Descriptive hypothesis | Original ABI | Coverage |
| --- | --- | --- | --- |
| B08F60..B094DC | BSP_ParticleSprite_InitializeState | ECX actual90h definition; stack(state6Ch,record108h); RET8 | complete |
| B007F0..B007FD | BSP_ParticleSprite_ReleaseState | ECX definition; stack(state,argument DWORD); EAX1; RET8 | complete |

Ends are inclusive: RET8 starts at B094DA and B007FB. The initializer's EAX
is incidental, including LAHF-modified curve-pointer bits and unsigned frame
DIV quotients. The C++ initializer returns void without normalizing EAX.
The cleanup's MOV EAX,1 is explicit and its caller preserves AL. EDX borrows
`NativeParticleSpriteStateAccess`; these are new C++ interfaces, not drop-in
native ABI replacements. The record argument of the initializer, and all
original arguments of the cleanup, are unread.

## Identity, producers and callers

Every live batch used the repository Ghidra client, verifying
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, bridge8089. The worker
made no Ghidra mutations. B08F60 has one complete body through B094DC and
zero flow gaps. B007F0..B007FD is executable disk/live code but has no Ghidra
function; the integrator must create precisely that function before annotation.

The complete incoming reference sets are D5DD30/D5E00C for B08F60 and
D5DD34/D5E010 for B007F0. Actual Sprite table D5DD18 and base table D5DFF4
hold those entries at +18/+1C. There are no recovered direct callers.
B00CE0 allocates90h at B00D1A, calls B08830 at B00D45, then installs D5DD18.
B08830 calls the existing B01150 base constructor, clears +10/+80/+84/+88,
and installs D5DFF4. B01150 produces the common curve-pointer/frame/flag
fields. Its +14 word is supplied by the factory's source record+10; the
Sprite initializer treats the current word as a pointer and reads the actual
eight-byte native name at that target+8. No speculative parent class is added.
Existing Sprite parsing/setters produce +80/+84/+88 curves and +8C bound.

B0CA40 produces actual6Ch states: current definition+64, emitter+68, null
light+60, the initial vectors and record+24/+28/+2C. B0CAD5..B0CADF reloads
the definition, captures its table+18, pushes record then state and calls the
captured target. B04F00 reloads state+64, captures table+1C at B04F25, pushes
the caller's argument DWORD then state, and invokes it at B04F2A. It preserves
the returned AL and separately unlinks/releases a light after this callback.
The Sprite cleanup itself does not release a light or change state+60.

## Complete behavior

When definition+64 is nonzero, capture the shared population owner's actual
section+4 from the existing 72B740 getter. Enter its physical critical section
if present and increment its current +18 depth. Allocate through the SAME
native PointLight pool as B7BD30/B7B810 before creating either name temporary.
A null allocation publishes null and skips name construction. Otherwise
41E870 constructs `dynamic_light_`, then the current definition+14 target+8
is concatenated with 4261A0. The existing B7C710 constructor initializes the
actual200h slot and its physical backlinks. The existing
`adopt_constructed_native_point_light` binds that same freshly constructed
slot to the established scene/backlink/lifetime domains. Publish its actual
address at state+60, release concatenation then prefix using their current
pointer and wrapping length+1, decrement the captured section and leave it.
Disabled light creation never reads `access.lights` or changes state+60.

The original FH3 metadata at DF3BA4/DF3BC8 has six unwind states: the guard
through411EE0, raw slot throughB7B600, then conditional prefix/concatenation
through41DD20. The C++ composition arms each ownership bit only after its
constructor returns. A constructor failure releases completed name temporaries,
returns the raw slot and releases the captured guard in native order. Once
B7C710 returns, adoption consumes the fresh construction; its host binding
failure cleanup belongs to that existing API. Temporary strings borrow the
application's `NativeStringStorage` (actual publications can use
`ActualNativeStringPoolStorage`); retained light names use the existing
PointLight constructor's `environment.nodes.strings` contract. No alternate
light, reference count, pool or owner graph is created.

Each curve amplitude is compared with current D7A218 using UCOMISS/LAHF/TEST
AH,44h. Ordered equality uses current D7A24C. Otherwise the canonical primary
RandomThreads draw is consumed as signed int32 by FILD, multiplied by the
amplitude and current D5DA30 double, added to current D7A210, and spilled at
the original float32 site. Unordered comparisons draw; no clamp is added.

Definition+1C and +80 curves evaluate at time zero: mode+0A zero reads curve+4,
one uses the existing exact AFFA70 evaluator, all other modes use AFFAE0.
The +1C value times its factor stores state+44; the +80 value multiplies
current D5DAF8 double then its factor and stores +50. Definition+2C scales
existing +18/+1C/+20 with the original distinct x87 operand order. Factors
from definition +30/+34/+88/+38/+5C store state +48/+4C/+58/+5C/+3C.

Definition+84 first computes its factor. If byte+29 is set, exact existing
BD2F40 draws an unsigned fraction from the SAME RandomThreads owner, using
current CE3978 correction and D63B80 double scale, including its native
guard and float32 spill. FCOMIP compares that result against current CE3800.
Carry, including unordered comparison, selects current D7A260; otherwise
current D7A24C is selected. Multiply that sign value by the saved factor and
store state+54. EBP's access pointer is saved while the two native frame
branches temporarily use EBP for the inclusive wrapping unsigned range.

Byte+60 selects a primary uint32 draw modulo `(last-first+1)` plus the signed
first frame's bits; otherwise use the signed first frame directly. CVTSI2SS
stores +34 then +30. Byte+61 similarly selects the second range draw for +38;
otherwise load current +30 and add current D7A210 double. Zero-width DIV
faults remain. Finally increment current F8D388 with wrapping DWORD ADD.
B007F0 subtracts one from current F8D388, also with wrapping DWORD semantics.

## Callee evidence and validation

| Native | Existing concrete binding | Native contract |
| --- | --- | --- |
| 72B740 | actual manager/lock publications and getter | no args; actual8h owner in EAX |
| B7BD30 | environment's canonical pool B7B810 | no consumed args; raw slot in EAX |
| 41E870 | construct_native_string_cstring_0041e870 | ECX header; stack nonnull text; EAX header; RET4 |
| 4261A0 | concatenate_native_string_headers_004261a0 | ECX left; stack output/right; EAX output; RET8 |
| B7C710 | construct_native_point_light_00b7c710 | ECX slot; stack name; EAX slot; RET4 |
| 419CC0 / BD1510 | current string storage getter/release | no-arg getter; release ECX pool, stack block/size/1, RET0C |
| BD2FC0 | canonical RandomThreads primary stream | ECX0; EAX uint32; RET |
| BD2F40 | native_particle_unit_random_00bd2f40 | ECX0; ST0 fraction; RET |
| AFFA70 / AFFAE0 | existing exact curve evaluators | ECX curve; stack float zero; ST0; RET4 |
| CE2218 / CE2210 imports | EnterCriticalSection / LeaveCriticalSection | stack captured physical section; RET4 |

Callee bodies, owned routines' full incoming references, current tables,
producer code and full register listings were checked. The JSON carries all
25 direct call sites plus both imported calls with their containing function.
Consumed service routines and unwind thunks are not claimed by this packet.

Scratch evidence is `C:/Users/sqz269/bsp-ar-sprite-state`. Five complete spans
(initializer, cleanup, two curves and unsigned-fraction draw) and fourteen
table/global/literal/EH-metadata spans match live saved Ghidra and installed
PE bytes. Installed SHA256:
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
The PE relocation directory is empty; Capstone decoded all instructions and
recorded76 call/data/immediate operand rebindings before fixture execution.

MSVC Win32 `/MD /W4 /WX /O2 /fp:strict` compiled the actual new source and
the integrator's updated PointLight owner source, linked the current existing
library, and embedded a manifest. **72 initializer comparisons and six
cleanup comparisons passed** across x87 24/53/64-bit precision. Checks compare
all6Ch state/90h definition/108h record bytes, complete canonical RandomState,
counter, incidental EAX, x87 status/control and a preexisting80-bit sentinel.
Forty-eight first draws have their sign bit set. Cases cover zero/linear/cubic,
qNaN/sNaN, signed zero, signed/singleton/wrapping/zero frame ranges, current
state/global aliases, sign threshold zero/one/NaN, modified unit globals and
the unsigned draw's deliberate guard fault. Hardware fault codes agree.

Eighteen light cases use real pool allocation, existing constructor/adoption,
and actual embedded native string storage. They compare102 ordered string
events (sizes, physical lock depth, publication timing), actual name text,
all200h light bytes except the separately checked pooled-name pointer, and
identity in all three physical binding domains. Cases include null/recursive
locks, the small-return shutdown gate, empty parent names and a callback
changing definition+14 during prefix allocation to verify the subsequent
reload. Lights are released through their real canonical reference between
runs so the same physical slot is compared. The captured probe exit is zero;
source, helper, executable and linked-library hashes are in `results.json`.

This is compositional native-byte evidence: the original outer initializer,
cleanup and math dependencies run from copied original bytes; the native
light service call sites bind the same existing real C++ services. It does not
establish native FH3 or service register/throw identity. The fixture rebinds
the original handler to a continue-search boundary and uses `/SAFESEH:NO`
only to observe hardware faults; no original C++ unwind is claimed. MXCSR,
all CPU registers, allocator exhaustion, gameplay and rendering were not
validated. Root integration owns CMake, the combined build, Ghidra repair,
annotations, exports and ledger publication.

## Correction from AR integration

B007F0 was defined as the full14-byte function ending B007FD.
Saved annotation/export and final build evidence are recorded in the report.
