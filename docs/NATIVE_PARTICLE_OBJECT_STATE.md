# Native Object particle state

Addresses: `00AF90A0`, `00AF8B00`, `00AF8440`, `00AF8270`, `00BD2F10`, `00BD2E60`.

| Entry and proposed descriptive name | Inclusive end | Original ABI | Coverage |
| --- | --- | --- | --- |
| AF90A0 ParticleObject InitializeState | AF9650 | ECX definition; stack(state6Ch,record108h); RET8 | complete normal paths |
| AF8B00 ParticleObject ClearState | AF8BA9 | ECX unused definition; stack(state6Ch,unused argument DWORD); RET8; AL true | complete |
| AF8440 ParticleObject ConstructRandomMatrix | AF84FF | ECX actual40h matrix; RET; EAX same matrix | complete |
| AF8270 ParticleRecord ComposeObjectMatrix | AF82AD | ECX actual108h record; stack(output40h); RET4; EAX output | complete |
| BD2F10 Random UniformFloatRange, existing name retained | BD2F30 | ECX stream; stack(minimum,maximum); RET8; ST0 float | complete |
| BD2E60 RandomState UniformFloatRange | BD2ECD | ECX actual RandomState; stack(minimum,maximum); RET8; ST0 float | complete |

These are descriptive hypotheses and new C++ access ABIs. The initializer's
original incidental EAX is not a specified result. Its FH3 stack metadata is
replaced by borrowed access storage; native C++ exception unwind equivalence
is unclaimed. C++ guards retain physical lock/string/raw-slot cleanup on the
implemented light branch and physical lock cleanup around root propagation.
Allocation failure and hardware-fault unwinding through naked numeric frames
remain outside the validated contract.

## Evidence, callers and actual storage

Each live query/export batch used the repository client, which verifies project
`C:/Users/sqz269/bsp.gpr` and program `/battlestationspacific.exe` on bridge8089.
All six full instruction listings were reviewed; numeric code uses the listing
because the pseudocode changes x87 precision, misses register inputs and hides
the cleanup's returning-free continuation. No Ghidra mutation was made.

The only incoming initializer/cleanup references are D5DB18/D5DB1C, actual
Object profile D5DB00 slots18/1C. AF89E0 installs that profile, initializes
Object +8C/+90/+94 as pointer/count/capacity and +80/+84 as runtime parameter
pointers. AF8BD0 produces RotationSpeed +80; AF80F0 produces Size +84 and cache
+88. B0CA40 produces the actual6Ch state and calls captured current virtual18
with two stack words at B0CADF. This packet changes no shape/profile identities.
The cleanup caller B04F00 loads its own argument DWORD at B04F06, captures
current state+64 definition virtual1C at B04F25, then pushes that word and state
before B04F2A. The cleanup's second stack word is an opaque argument, not a
record pointer; AF8B00 never reads it.

AF9660 fills +8C with actual resources from B80720; F8D31C can override the
factory, so its outputs do not establish one universal virtual08 target.
Initializer AF94F3 captures the current resource table+08 and passes (0,1.0).
The result is the actual instance at state+34; its physical +4 is the reference
count and +C is the node consumed by B6D890. State+30 points to an actual40h
matrix allocation. No resource companion, shadow collection or alternate
reference count replaces these words. Resource factory/instance/definition
virtuals remain explicit captured-target application boundaries.

AF8440 has one caller, AF9517; AF8270 has one caller, AF95A1. The latter reads
record+A4, snapshots its +298 matrix with REP MOVSD16, then obtains the same
node's current +B0 local matrix via B6DB60 and multiplies local*snapshot.
BD2E60's only direct caller is BD2F29. BD2F10 is shared by many systems; its
range contract is established by its complete body/RET8 and BD2E60, with this
packet's five callsites retaining their individual stack spills and reloads.

## Initializer and ownership ordering

When definition+64 is set, AF90A0 captures the shared population section from
72B740, enters it and increments the physical +18 depth. It allocates from the
same PointLight pool, constructs `dynamic_light_` and concatenates the current
definition+14 parent's actual8h name at +8. The existing B7C710 constructor
builds the real light; the integrator's `adopt_constructed_native_point_light`
binds its canonical owner/reference without extra native allocation or retain.
The actual slot is published to state+60 before concatenated-name and prefix
cleanup, then the captured lock depth is decremented before leaving. Null
allocation publishes null. The native 1FCh request reaches B7BD30's wrapper,
which ignores ECX and delegates to the same actual200h pool allocator.

The +1C parameter's amplitude compares against CURRENT D7A218; ordered equality
uses CURRENT D7A24C. Otherwise the stream-zero draw undergoes signed FILD,
FMUL amplitude, FMUL CURRENT double D5DA30, FADD CURRENT double D7A210 and a
float32 spill. Const/Linear/Cubic evaluation at time zero then scales state+44.
The complete BD2F10 draws the initial angle into state+50 from (0,CURRENT
CE3D9C). Subsequent amplitude comparisons for +2C/+30/+34/+80 use literal SSE
zero; they must not be changed to the mutable D7A218 operand. They update
velocity +18/+1C/+20 and state +48/+4C respectively. RotationSpeed's factor
multiplies either CURRENT one or CURRENT negative-one; definition+29 controls
the extra BD2F40 unsigned fraction draw and its FCOMIP against CURRENT CE3800.
Size +84 evaluates at zero into state+58 without a random multiplier.

A nonzero current +90 count converts with signed CVTSI2SS, draws range(0,count)
and invokes the required actual BF7420 ST0 conversion, including its current
SSE/legacy dispatch. No index clamp is added. The selected current resource
virtual08 returns state+34. The matching BF681B service allocates40h; AF8440
draws X, Y, Z angles in that order and builds `(RotZ * RotX) * RotY` with the
existing current-global rotations, exact 413920 multiply and 4134F0 x87 copy.
The matrix publishes to state+30. Under a fresh captured population lock,
the routine reloads record+A4's +A4 roots, then current state+34 instance+C,
and calls the complete existing B6D890 binding. Zero count clears +34 then +30.

The current definition+14 parent's bytes+64/+65 choose either AF8270's composed
matrix or an identity matrix with one captured CURRENT D7A24C and literal SSE
zero. Finally it captures the current definition virtual28 and passes
(state,float0,uint0,matrix,float0), retaining FLDZ/FST/FSTP effects and RET14.
The required callback receives that captured target; there is no successful
render/update fallback.

AF8B00 captures and enters the same population section even for empty state.
It reads current state+34, performs InterlockedDecrement on actual instance+4,
and only at zero captures current instance virtual00 and calls it. State+34
is cleared after that call, including callback mutations. It next reloads
state+30, invokes matching BF65AC free if nonnull, then clears +30 even when
the free callback changed it. It decrements/leaves the captured lock and returns
AL=true. State+60 is not a field this cleanup releases.

## Exact shared range provider

BD2F10 spills/reloads its incoming endpoints before the shared RandomThreads
state lookup, then invokes BD2E60. The state routine preserves both native
guard checks, calls the existing BA2C20 on that same RandomState, and interprets
the uint32 draw via signed FILD plus CURRENT CE3978 correction for the sign bit.
It multiplies CURRENT double D63B80 and spills to float32 before evaluating
`minimum + (maximum-minimum)*fraction` with the native x87 stack schedule and
final float32 spill. Reversed/equal ranges, NaNs, infinities and signed zeros
are not normalized. The public API borrows NativeParticleUnitRandomAccess and
does not create another random domain or cache native scalar values.

## Native fixture and integration limits

Scratch: `C:/Users/sqz269/bsp-ar-object-state`. The installed EXE SHA256 is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
`make_fixture.py` confirms all16 complete owned/dependency spans and13
global/table spans equal live Ghidra bytes. The PE has no relocation directory.
Every relocation comes from Capstone operands; the external short conditional
branch in BF7420 uses a nearby rel32 jump stub, preserving its one-byte operand.
Original FH3 handler addresses are inert in these fault-free normal cases.

Strict MSVC Win32 `/MD /W4 /WX /fp:strict /O2` compilation and the executable
pass. **36** initializer-plus-cleanup cases cover twelve scenarios at24/53/64
x87 precision: Const/Linear/Cubic, qNaN/sNaN/equality, later curves and globals
aliasing state writes, model count zero/one/three, both matrix choices,24 actual
light paths,24 model paths, both original CRT conversion modes, current lock
null/non-null, retained and zero resource counts, and callback changes to state
pointers before the native clears. Comparisons include every state/definition/
record/instance byte, actual40h matrices, actual1FCh light-slot bytes with the
pooled-name pointer normalized, full primary RandomState, ordered dependency
events and observed physical lock depths, x87 status/control and a preexisting
80-bit sentinel. **48** further cases compare both real RNG streams for eight
normal/reversed/equal/signed-zero/qNaN/sNaN/infinite ranges at all three precision
modes. This is not a comparison of every MXCSR/x87/environment field.

The string probe uses constructed actual8AD4A0h storage through
ActualNativeStringPoolStorage; light allocation/construction/adoption and node
root propagation use existing canonical domains. Model virtual08/instance00/
definition28 use explicitly controlled native-ABI observation boundaries on
physical fixture storage. They prove selection, arguments, current target,
state/ownership and call ordering; they do not prove the game's model factory
or renderer bodies. No successful VFS/model load is invented. AF85B0's actual
virtual28 body and arbitrary resource factory overrides remain application
integration dependencies. No game was launched or installation changed.

Ghidra's returning BF65AC call AF8B76 hides AF8B7B..AF8B84 (ten bytes):
`ADD ESP,4; MOV DWORD PTR [EDI+30h],0`. The integrator should clear only that
callsite's false-no-return override and refresh the export. There are no
missing Ghidra function starts in this packet. Reports carry every direct call
and indirect import/virtual boundary. Full worktree build validation excludes
this new source until the integrator registers it in CMake. The existing full
worktree build and its single CTest pass; the new source's actual compiled
source and exact current library hashes are captured by `record_results.py`.

## Correction from AR integration

AF8B00 returning-free continuation AF8B7B..AF8B84 is now present in saved Ghidra flow.
Saved annotation/export and final build evidence are recorded in the report.
