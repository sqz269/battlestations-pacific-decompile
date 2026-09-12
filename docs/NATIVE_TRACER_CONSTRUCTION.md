# Native SkinedWaterTracer construction

Addresses: `00BAD6F0`, `00BAC070`, `00BAC310`.

This reconstructs the real constructor over a7ACh payload from the existing
0109049C pool, with the same canonical `NativeModelOwner`, native node, atomic
count, material, mesh, section, stream and string owners. The returned
`RegisteredType4TracerView` points directly at that payload, so the existing
BAABB0/BAA670 update operates on the ring this constructor actually allocates.
Descriptive names are hypotheses. These are new MSVC Win32 C++ interfaces.

| Inclusive native range | Bytes | Original ABI | Coverage |
| --- | ---: | --- | --- |
| BAD6F0..BADC6D | 1406 | ECX actual7ACh; five stack words; EAX same; BADC6B RET14h | complete through required real renderer and canonical owner bindings |
| BAC070..BAC12A | 187 | ECX actual0Ch header; signed capacity; BAC128 RET4 | complete native-valid header/backing domain |
| BAC310..BAC363 | 84 | ECX actual0Ch header; signed count; BAC361 RET4 | complete native-valid header/backing domain |

The fifth constructor argument is the actual template; the first is an actual
source whose+14 supplies the scene root. The second argument is unread, even
though the caller marshals it. The third and fourth arguments are textures.
8728FA..87290D proves this ordering. Ghidra reuses a stack location that held a
texture for the later raw mesh allocation and incorrectly gives the final
template geometry/range/bounds reads other parameter names. ESI remains the
tracer and EBX remains zero throughout BAD6F0.

## Construction and canonical ownership

Prepare one `NativeModelOwner` on the actual7B0h pool slot and one stable
`NativeTracerProfileBindings` from the lifetime packet. The existing B75030
constructs its base with the native17-character `SkinedWaterTracer` name. The
root integration adds `NativeModelEnvironment::actual_names`; this constructor
requires it to equal both material and parameter name services. B75030 and its
eventual B750C0/node cleanup therefore use the same explicit actual string
service as temporary names. The older optional semantic base binding is
rejected by this new tracer path.

BAD780 publishes D63FA0 on that same owner. The lifetime packet's nonthrowing
binding helper installs the actual derived scene callbacks without another
node, count, lookup registry or retain. The native source244 remains borrowed;
template190 receives a retained reference. Header194/198/19C is the actual30h
point ring and header1A0/1A4/1A8 begins empty. Widths184/188, matrices, parameter
source arrays and other constructor-unwritten bytes retain the slot preimage.
The trailing pool ID7AC is untouched.

The constructor creates a real material from template14, uses this same tracer
as its parameter owner with retain byte0, and assigns the two textures through
the existing retained-owner setter. Its current effect name is the actual8h
header at effect+B8. Existing CRT `strstr` selects these registrations:

| Effect name contains `static` | Native name | Same tracer source | Float4 count | Result |
| --- | --- | --- | ---: | --- |
| yes | cParams0 | +258 | 1 | discarded |
| no | BoneData0 | +288 | 40 | actual788 |
| no | BoneData1 | +508 | 40 | actual78C |
| no | cZScale0 | +268 | 1 | discarded |
| no | cZScale1 | +278 | 1 | discarded |

B18AC0 forwards four times that count as DWORDs and matrix byte0 to the existing
B17E10 registration. It does not copy or initialize source values. Parameter
results788/78C are published before destroying the temporary name.

The newly constructed canonical mesh obtains a real stream using the source
mesh at template0C and its actual stream0 descriptor. The constructor captures
the actual renderer and its table before the descriptor virtual24 call, then
reads that captured table's5C entry after the descriptor returns. This preserves
both a changed singleton and a changed table slot. Its three stack words are
exactly `(template30, 1, descriptor)`. The required renderer binding must return
the actual owned, registered stream; it has no successful default.

The stream is published at248, and the mesh's stream0 setter takes a separate
reference. The mesh uses the source mesh's current index owner. A real draw
section receives primitive4 and ranges `(0, template30, 0, template34)`, rebuilds
its actual vertex layout, retains material1BC and is appended to the mesh.
The local section reference is released. Existing B75170 attaches the mesh to
the same generated-model tail180 using two x87 stores of the current D7A260
sentinel. Existing B6D890 propagates the real source14 root; B74390 copies the
template38 bounds. The local mesh reference is released, then250=0 and24C=1.
Material1BC and stream248 retain their original owned references for the real
derived destructor. No188h model scalar destructor or synthetic pool is used.

The bind methods only register the already constructed concrete
`NativeMeshReference`, `NativeMaterialReference` and `NativeMeshSectionReference`
in the same canonical `NativeRenderActualOwners`, without another retain.
After success the caller establishes the lifetime packet's `NativeTracerReference`
before owning-reference dispatch. The source uses real reconstructed calls
for base construction, parameter registration, geometry assignment, section
construction, string operations and root propagation; these are not host no-ops.

## Producer and ring evidence

The separate template producer BADC70 writes actual04=1, D63F94, scalar08,
effect14 and ring count10 (BADD81). Its static BACBD0 and skinned BAD150 paths
produce the actual mesh0C and its real vertex/index streams. BAD172 updates30;
BACCD8/BACFD8 produce34 in the static path, and BACD29/4C/53/5E write38..44.
The producer listings were inspected to establish the consumed layout, not
ported or renamed by this packet. Supplying the full actual template production
is still required for a running application.

BAC070 clamps a growing capacity to at least1 and uses the existing ordinary
CRT allocation/new-handler boundary with wrapped32-bit `capacity*30h` bytes.
It reloads the old backing and count while copying. Each record copies ten
ordered FLD32/FSTP32 words and two raw MOV link words. Link addresses are not
rebased. A memcpy would change signaling-NaN and denormal behavior. After
returning free it publishes the captured new backing and capacity while leaving
the count alone. There is no allocator substitute, rollback or EH guard.

BAC310 grows through that reserve only when requested count exceeds capacity.
For newly exposed records it reloads current backing, checks the computed
address for zero and initializes links28/2C only. It decrements count during
shrink without invoking a point destructor, then publishes the requested count.
Unused float words and stale backing/capacity remain. Lifetime BAC860 consumes
this same full resize entry before freeing the current194 backing.

## Failure cleanup and boundaries

BAD6F0 handlerCC3E94 selects FH3 descriptorDFD694, twelve-entry mapDFD6B8.
State0 destroys the initial name. State1 destroys base then that name, while
state2 destroys only base. State3 destroys ring194 through BAC860; state4 first
destroys header1A0 through BAC880. The body advances directly from2 to4 after
initializing the headers. State5 returns a failed raw material through B17D70;
states6..10 destroy the current parameter name; state11 returns a failed raw
mesh through B72F70. These latter states return to4.

The lifetime binding is restored before B750C0 during construction unwind,
because canonical model callbacks require `context=&NativeModelOwner`. There
is no derived reference companion to retire before full construction succeeds.
Cleanup throwing while a C++ exception is active terminates. The parent caller
returns the failed raw tracer allocation through BAC2B0 after the member unwind.

The native constructor does not unwind retained template190, a constructed
material1BC, stream248, or local references to constructed mesh/section owners.
Those omissions are preserved. C++ RAII resource rollback would change native
behavior. Invalid native pointers, corrupt extents, original EH/SEH dispatch
and unchecked allocation-failure faults are outside the supported domain.

## Verification and integration

Every live Ghidra wrapper verified the existing `C:/Users/sqz269/bsp.gpr` and
`/battlestationspacific.exe`. Ghidra remained read-only. All1677 bytes across
the three complete spans match the installed PE; individual SHA256 values and
all46 numeric CALL rows plus9 indirect calls are in the report. The live call
checker passed all46 numeric rows.
All eight existing installed/live seed-byte checks also passed.

Strict standalone MSVC Win32 compilation passed with
`/std:c++17 /EHsc /fp:strict /O2 /Oy- /MD /Gy /Gw /W4 /WX`, using the root's
actual-name header extension and sibling lifetime header. An ignored focused
fixture executes all271 original BAC070/BAC310 bytes, fills unrelated code with
INT3, checks every byte unchanged except two four-byte CRT CALL relocations,
and uses the same existing real CRT allocation/free services. It passes102
original/source pairs:96 reserve pairs across12 x87 control modes and eight
numeric seeds, plus six full384-byte shrink/regrow pairs. It compares signaling
NaNs, denormals, signed zero, infinity, raw link words, x87 status and control
word. Additional checks cover the minimum1 and no-grow cases. No permanent
test cases were added.

The repository standard build was attempted and failed because J: had no free
space: C1085 `No space left on device`, followed by MSB6003. Only this worker's
freshly generated ignored `build/win32` was removed after checking the exact
absolute path; this freed about772MB. The root owns the final combined build
and seeded CTests, CMake registration and ledger annotations.

BAC070 needs saved flow restored at BAC11D..BAC125: `ADD ESP,4`, data/capacity
publication, then one-byte `POP EBP` at BAC125. Its complete inclusive end is
BAC12A, with three-byte `RET4` at BAC128. The worker made no analysis repair.
The missing FH3 dispatcher function CC3E94..CC3E9D loads descriptorDFD694 and
ends with a five-byte JMP to BF6B43 startingCC3E99; define that start at integration.
Constructor endBADC6D contains three-byte `RET14h` startingBADC6B; resize endBAC363
contains three-byte `RET4` startingBAC361. Preserve prior comments and refresh
the affected exports after the root's annotation/repair batch.

The fixture does not execute full BAD6F0 or native EH dispatch. Complete template
production, application renderer/current-slot binding, construction/update/
destruction composition, native ABI compatibility, rendering and gameplay remain
unvalidated. The returned view is the actual constructed owner when all required
real callees and canonical bindings are provided.

## AL saved-analysis and combined-build integration

All four AL modules are registered in bsp_core. The standard Win32 build and
both seeded CTests passed. The report records saved name/signature preimages,
old-comment preservation, full body-range readback and the current-library
replay of bounded fixtures. Returning-free gaps and missing function definitions
are repaired and saved. Earlier worker pending notes describe their original
snapshot. Full constructor/teardown coverage, native EH compatibility and
gameplay remain bounded as documented above.
