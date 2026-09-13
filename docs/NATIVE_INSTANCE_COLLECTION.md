# Actual instance collection and batch routing

Addresses: 00b1dff0, 00b172d0, 00b17300, 00b73770, 00b51cb0, 0043c130

The actual Traceline/mesh rendering service can now route its original 28h
entry directly through B1DFF0. The full normal source body borrows the actual
44h command, its existing batches and arrays, actual binding/group identities,
and the application's canonical model, material, stream and string domains.
It does not copy the entry into the older semantic grouping implementation.
Names below are descriptive hypotheses, not recovered symbols.

| Routine | Coverage | Original ABI | Source behavior |
| --- | --- | --- | --- |
| B1DFF0..B1E6A7 | complete normal body | ECX command, stack entry; RET4 | Fade/cull, direct batch routing, existing group accumulation, cold two-model construction and source-pointer append. |
| B172D0..B172D6 | complete | ECX effect; EAX effect+B8; RET | Borrow actual effect name header. |
| B17300..B17306 | complete | ECX effect; EAX DWORD+AC; RET | Read current batch index. |
| B73770..B737FA | complete | ECX threshold owner; two floats; RET8, ST0 | Current threshold selection, x87 fade and unordered-preserving clamp. |
| B51CB0..B51CEB | complete | ECX batch; stack raw entry; RET4 | Grow actual borrowed-pointer storage with minimum256, then append. |
| 43C130..43C1B5 | complete normal body and nonthrowing cleanup | ECX output8h, EDX prefix C-string, stack right8h; RET4, EAX output | Construct temporary prefix, concatenate, release current temporary. |

The new interfaces add explicit access bindings. They are not binary detours
or proof of original whole-object ABI compatibility. The six complete byte
spans and current constants were matched against the installed PE and saved
Ghidra image; the report carries hashes and all44 direct call sites.

## Visibility and ordinary batches

The material effect and descriptor come from section+20 -> material+7C ->
effect+C4. Descriptor byte16 gates the visibility work. Current camera/model
world transforms refresh through actual B6DB70 when their +5C bit2 is clear.
The numerical prefix retains the original SSE position captures, x87 copies,
subtraction spills, actual419440 length and current camera-mode mask test.

Camera modes whose masked shift intersects17h apply the distance factor:
camera+178 minus measured distance, divided by current doubleCE4D70, spilled
to float and clamped by the original x87/SSE comparisons. B73770 selects
defaultCE77DC when mesh+50 is zero; otherwise it reads mesh+4+16*index. Its
width and final result have the original float stores. The caller multiplies
that returned ST0 by original entry+18 and the distance factor without an
intermediate store, writes entry+18, then compares with doubleCEB690.
Unordered comparisons follow the assembly rather than a host min/max policy.

Without a section+5C binding, COMISS(current one, entry+18) selects the faded
batch at command+10 only on the ordered greater-than branch. Otherwise the
captured effect's current +AC selects command+C+4*index. B51CB0 stores the
same raw pointer and increments the current native count; it neither retains
nor interprets the row. Its existing actual B51B50 allocator specialization is
reused, including the native minimum256 capacity.

## Existing and cold groups

Category selection preserves FSTP32, saved x87 CW|0C00, FISTP signed64 and the
**unsigned low-DWORD** comparison against1. Values outside the usual0..1 fade
domain are not replaced with a convenient C++ bool or saturating conversion.
Category0 uses group+10 count and +30 source array; category1 uses +C and +24.

Binding+8 indexes the actual command+2C array. The code repeats the native
ID/count reads around resizes, publication and later accumulation. A new4Ch
group uses full B1D6F0 and B1CA50: publish incoming binding, retain its same
actual+04, then release the captured prior actual owner at current zero.
The group name is current effect name + ` - ` + current model name, through
the existing raw-header string operations and same actual pooled storage.

The one-time color guard is set before the24 ordered DWORD stores. It uses
the current one bits and zero to initialize six float4 rows. Selection is
unsigned binding ID modulo5, so the sixth row is initialized but unselected.
For each of two categories the routine clears its count, increments current
entry-cache+8, reads the captured cache's current+4 base and publishes the
corresponding28h output row. It builds `instanced - ` + current model name,
then calls actual B4C8D0 with binding+C generator, source mesh and selected
section. The returned actual model is stored in group+1C/+20 before temporary
name cleanup. Its first mesh/section material receives the four raw color
words, then the source-entry array reserves8.

After both creations, the selected count is set to1 and the group pointer
is appended to command+38. Existing groups increment the selected count and
reload the binding ID/array before selecting their final source-array owner.
The original borrowed entry is appended last. All arrays reuse established
producer-created storage and native allocation services.

## Lifetime and composition boundaries

The original B1DFF0 FuncInfoDF4F7C has five unwind states: allocation0,
separator1, first concatenation2, second concatenation3, generated name4.
The allocation cleanup00CBCAE0 calls BF65AC; the current
actual B1D6F0 constructor is nonthrowing in its valid-storage domain. The
other four actions tail-call41DD20 at the original temporary headers. The
source arms each temporary only after construction succeeds and disarms it
before normal cleanup. Completed group/model publications are not rolled back.

43C130 FuncInfoD85BDC uses temporary cleanup C5F350 and output ownership-bit
cleanup C5F358. Its bit is set only after4261A0 returns. Current string release
is nonthrowing; exceptions during concatenation release the prefix while the
callee supplies its own output cleanup. Hardware faults, invalid pointers and
original FH3/SEH interoperability remain outside the source contract.
The visibility prefix's CRT/math-error callback must return without throwing:
its naked helper preserves original local offsets with dummy frame words and
does not register a native exception frame. The callback's C++ type alone does
not enforce that application binding requirement.

`NativeInstanceCollectingRenderServices` supplies the concrete B1DFF0 method
for the existing `NativeTracelineRenderServices`. Its remaining current-profile
virtual services still belong to the application. The collection access uses
the same canonical geometry/model/string domains; an external acquired-record
diagnostic preserves any creators left by a failed cold geometry construction.
It is not a second native owner or queue. Current raw globals must remain valid.

## Verification and follow-up

Strict owned-source Win32 compilation and76 original/source comparisons passed:
48 full hot-group/ordinary-batch/fade collector cases,24 threshold cases, actual
minimum256 batch growth, two owning-prefix cases and the two getter leaves
checked together. Numeric cases compare all raw storage, x87 control/status
and SSE status across four x87 control words. The ignored focused fixture is
at `C:/Users/sqz269/bsp-au-collection`. Its initial executable includes the
two new source modules against the existing library; final combined build and
probe-only current-library replay are recorded separately. No gameplay or
complete GPU render proof is claimed. Cold creation requires the actual
`native_instance_geometry` module and its concrete runtime domains.

The existing B1D760/B1D8E0 group destructor/deleting-destructor now also accepts
`ActualNativeStringPoolStorage`, preserving the same release/array/name order.
This closes the group's name allocation/release pairing without routing an
actual name into the older semantic pool. The old pool interface remains.

Next compose the application's canonical command lifetime with the same actual
string pool, close the remaining actual command execution/upload providers,
and validate the composed scene/render path. B29670 device recreation remains
a separate packet; this reconstruction does not supply a fallback device.

## AU cold collection composition evidence

The original B1DFF0 caller now invokes original B4C8D0 twice in a focused
original/source fixture:593 normalized DWORDs passed across the command, group,
cache, colors and two generated model graphs. External call adapters use the
current actual source ownership domains, with a real D3D9 HAL device,16MiB
shared vertex buffer and COM declaration. Full group retirement uses the same
ActualNativeStringPoolStorage as name allocation. Borrowed effect/binding
records, disabled visibility, null source index, effect identity normalization
and two normalized pool-slot preimages bound this evidence. The initial probe
compiled the new source objects; final current-library-only replay and its
exact source commit are recorded separately in the report. No visible scene
or gameplay result is established.
