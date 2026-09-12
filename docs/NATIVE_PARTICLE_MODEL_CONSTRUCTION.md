# Native particle-model construction

Addresses: `00AF74A0`, `00AFF5F0`, `00AFD2E0`, `00AF6120`.

`construct_native_particle_model_00af74a0` reconstructs the complete constructor
sequence over the actual `2DCh` particle-model payload, including its two meshes,
materials, draw sections, matrix parameters, emitters, native array owner, mask,
and final manager registration. Real application callees remain required. This
does not establish successful application construction, rendering, destruction,
native ABI compatibility, or gameplay validation.

| Routine | Inclusive span | Original ABI | Coverage |
| --- | --- | --- | --- |
| AF74A0 | AF74A0..AF7F31,2706 bytes | ECX actual model, stack variant, EAX same, RET4 | complete ordinary body and14-state member/allocation unwind through required callees |
| AFF5F0 | AFF5F0..AFF634,69 bytes | ECX actual emitter, stack model/definition, EAX same, RET8 | complete |
| AFD2E0 | AFD2E0..AFD365,134 bytes | ECX actual18h storage, stack model/signed count, EAX same, RET8 | complete through required actual array/element construction and destruction |
| AF6120 | AF6120..AF617E,95 bytes | ECX pointer header, stack signed capacity, RET4 | complete in the valid native array domain, including verified free continuation |

Descriptive names are hypotheses. The new C++ interfaces preserve actual object
identities, offsets, mutation order and explicit dependencies; they do not expose
the original vtables, register calling convention or native SEH dispatcher.

## Actual storage and ownership

AF45D0 is the variant producer: it writes the native string header at+08,
reference04=1, pointer counts30/54=0, defaults58=1 and5C=30, and clears the
64/65/78/79 flags. AF4280 releases its counted definition rows and string. The
constructor consumes that same actual variant without copying its name, flags,
material source or emitter definitions into an independent definition object.

The model uses the existing `NativeModelOwner` for its actual174h node and
174..183 generated-model tail, its sole reference count, native matrices,
hierarchy and stable `NativeNodeBinding`. Its derived tail starts at184, where
the ordinary188h model pool would store a slab ID. This model belongs instead to
the existing F8D2D0 particle-model pool: payload2DC, stride2E0, slab ID2DC.
The base **scalar** destructor must never return it to the188h model pool.

`NativeParticleModelTailStorage` spans184..2DB with verified field offsets and
preserved gaps. The initialized emitter header is194/198/19C. AFF5F0 produces
actual28h emitters: model08 is borrowed, definition0C is retained through its
actual atomic04, container10 and words14..20 become zero, and24 is untouched.
An emitter's own04 is its only count. This packet creates no emitter registry.

AFD2E0 produces a separate18h object at model190. Its+04 is an8h byte-array
header; its+0C is an8h array header for108h records. AFD130/AFD220 allocate a
4h count cookie followed by the respective elements. The first byte array is
filled with the low byte of each index. The second array invokes real AFDAC0
element construction and AFD9F0 destruction, which remain dependencies. It is
neither AFF690's30h lazy container nor B04F00's6Ch particle state.

All mesh/material/section ownership uses the existing actual pool, storage,
reference and parameter implementations. Host bind hooks register the existing
`NativeMeshReference`, `NativeMaterialReference` and `NativeMeshSectionReference`
in the same `NativeRenderActualOwners` domain without retaining again. The
derived node bind hook must replace callbacks on the existing binding for the
actual D5DA50 profile and provide its real lifetime. A `NativeModelReference`
would dispatch the ordinary model destructor and return the wrong pool slot;
it is explicitly forbidden here. The successful derived terminal lifetime is
still required, not silently mapped onto the base model.

## Constructor sequence

1. B75030 constructs the complete node/model base from variant+08. Publish
   D5DA50, clear emitter header194, set1DC=one, active1A4=1,
   initialized1A5=0,1E4=zero and18C=the captured variant. AF40E0 prepares that
   captured variant before its actual04 retain. Initialize point matrix298.
2. Copy the current variant64/65 bytes into1B0/1B1. Allocate/construct the first
   mesh through canonical0108FFF8, publish it at1B4 and retain it through base180
   using B75170 with the current D7A260 sentinel twice. Install the shared index
   stream, create a section, and set primitive4 and all four range words0.
3. Clone the material chosen from the captured variant78/79 low bytes and current
   F8C280 resource owner. Bind B0D140 texture into slot1 and B0D130 texture into
   slot2, then retain the material in section20. If1B0 is zero, write identity
   to218 and258; otherwise leave both matrix blocks' preimages untouched.
   Register borrowed parameters `cCustWorldMat`(16 words,matrix1),
   `cCustInvWorldMat`(16,1), `cColorBurn`(current variant74,1,0), and
   `cDepthSampleOffset`(current shadow owner+04,2,0).
4. Release the temporary material reference. Capture the actual renderer and its
   virtual5C before AF10B0 fetches the descriptor. Invoke that captured entry
   with `(0,1000h,descriptor)`, bind the real stream at mesh stream0, release its
   temporary reference, rebuild the section layout through the existing actual
   layout services, append the section, and release its temporary reference.
5. Repeat mesh/section construction for model1C8. This second mesh is not assigned
   to base180. Clone AF1120's material, bind B0D130 into slot1, and register
   the same two borrowed matrices. The conditional matrix initialization runs
   again. Create and bind a second actual vertex stream and section layout.
6. Zero184/188/1E0. Use x87 `FILD signed variant5C; FLD1; FDIVRP; FSTP float`
   for reciprocal1A0. Allocate/construct each28h emitter from current variant
   rows34/count54 and append its actual pointer to194 with AF6120 capacity
   growth. Reload variant/count after each iteration. No extra emitter retain.
7. Allocate/construct the18h array owner from current variant58 and publish190.
   Clear1A8/1AC, increment the live F8D2C8 model count, store1D8/1E8=one,
   byte1D4=0, and words1EC..1F8=0. Draw BD2F10 from the canonical primary random
   state with `[0,current CE3D64]` (observed upper constant10000). Temporarily
   select x87 truncation, `FISTP qword`, and keep the low DWORD at1FC.
8. Capture the first child, write model mask48=1B, recurse through7099C0 for each
   current child using that exact mask, then AF0950 registers this actual model
   in current F8C274. Return the same actual model address.

Parameter names use the existing native string pool and8h header. Native resize
lengths are13,16,10,18; every memcpy copies the current header length+1. The
variant74 and shadow+04 source addresses are read **after** name allocation and
copy. Each registration borrows those source addresses; it does not snapshot
the values. Local strings are destroyed after registration or on its exception.

## Required callee contracts and evidence

All immediate call sites in the four reconstructed bodies are recorded with
numeric native targets in the report; the verifier checks exact instructions.
Reconstructed-function callers were checked: AF74A0 only87433E; AFF5F0 only
AF7DB8; AFD2E0 onlyAF7E4C; AF6120 atAF618E,AF6838,AF7DEF. The two other AF6120
callers pass a signed resize request or `max(2*capacity,1)`, respectively.

| Reached sites | Callee | Contract established by body |
| --- | --- | --- |
| AF7521 | AF40E0 | rows variant10/count30; AF9F50 recursively visits definition children and current member virtual14/0C; required |
| AF7610,AF7A4F | AF10A0 | current resources04; borrowed shared index stream |
| AF7670 | AF10F0 | exact two low bytes; returns resources08/0C/10/14; RET8 |
| AF7A99 | AF1120 | current resources18; borrowed second source material |
| AF7690 | B0D140 | owner60 -> B4D170(+0C) -> B4CB10(+08); borrowed texture |
| AF76A5,AF7AB7 | B0D130 | owner3C -> B4CB10(+08); borrowed texture |
| AF79AE,AF7CDD | AF10B0 | resources1C; borrowed vertex descriptor |
| AF79C0,AF7CF0 | captured renderer virtual5C | three stack words0,1000h,descriptor; real stream/ownership required |
| AF7804,AF7873,AF7C16,AF7C85 | B18B40 | B17E10(name,source,16,1), RET8; reuse canonical registration |
| AF78E5,AF7957 | B18B20/B18B00 | count1/count2,matrix0; reuse canonical registration |
| AFD329,AFD331 | AFD130/AFD220 | actual cookie arrays, sizes1/108h; real element lifetimes required |
| AF7EBD | BD2F10 | primary state getter BD2ED0 then BD2E60 x87 range; RET8 |
| AF7F04 | 7099C0 | actual mask48 plus recursion through34/3C; required |
| AF7F17 | AF0950 | append raw model to manager04/08/0C, AF0630 reserve; no retain; required |

No host binding may turn one of these calls into successful no-op work. The
fixed numeric callee names preserve native boundaries without claiming the
remaining resource-owner, stream, array-element or manager implementation.

The BF681B emitter/array allocations have `ADD ESP,4` atAF7DA4/AF7E2E.
Memcpy calls have `ADD ESP,C`, includingAF7C00/AF7C6F in the second material
block. The pseudocode incorrectly omits four reachable second-material string
blocks and mislabels their stack locals; the full listing resolves them. EBX
remains zero throughout: it is clearedAF74CE and never rewritten. ESI is the
captured model throughout; the first material source uses original EDI variant
before EDI becomes the material. Matrix and reciprocal operations are checked
against x87 assembly rather than inferred from pseudocode.

## Failure cleanup

AF74A0 handlerCBAE1C points to descriptorDF2C34 and14-entry mapDF2C58.
State0 runs CBADA0 -> B750C0. State1 runs CBADA8 -> AF6B70 on model194,
then state0. AF6B70 calls AF6180(0) and frees the raw pointer backing; it never
releases its emitter cells and preserves the dangling data/capacity words.

States2/8 return the raw unconstructed mesh via B72F70; states3/9 return the raw
unconstructed material via B17D70. States4..7/10..11 destroy the local name
header through41DD20. States12/13 free the raw emitter/array allocation through
BF65AC after that constructor has unwound its own members. Each returns to1.
AFD2E0 descriptorDF31B8/mapDF31A8 cleans second header0C through AFD1E0,
then first header04 through AFD0F0. Cleanup throwing while unwinding terminates.

There is no native constructor-unwind release of the retained variant, already
constructed emitter objects, extra raw mesh references, or live model count.
The implementation does not add those rollbacks. The parent8742A0 separately
returns the failed raw2DCh model slot through AF62F0. Successful destruction and
manager unregistering belong to the real derived lifetime still to be supplied.

## Live analysis repairs and verification

Ghidra was read-only in this worker. Every live wrapper verified existing
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`; no re-import,
definition, annotation or save was performed. Installed PE bytes equal live
Ghidra bytes across all four entire spans. SHA256 values are in the report.

The root integrator was notified of these exact continuation repairs:

| Routine | Missing continuation | Decode and final instruction |
| --- | --- | --- |
| AF6120 | AF6171..AF6179 | `83 C4 04 89 1E 89 7E 08 5B`: ADD ESP4, publish data, publish capacity, POP EBX; existing RET4 atAF617C length3, inclusive endAF617E |
| AF6B70 | AF6B82..AF6B86 | `83 C4 04 5E C3`: ADD ESP4, POP ESI, RET atAF6B86 length1; inclusive endAF6B86 |
| AFD130 | AFD16A..AFD16C | `83 C4 04`; final RET4 atAFD1CC length3, inclusive endAFD1CE |
| AFD220 | AFD25D..AFD25F | `83 C4 04`; final RET4 atAFD2D2 length3, inclusive endAFD2D4 |
| AFD0F0 | AFD114..AFD117 | `83 C4 04 5F`; final RET atAFD126 length1, inclusive endAFD126 |
| AFD1E0 | AFD207..AFD20A | `83 C4 04 5F`; final RET atAFD219 length1, inclusive endAFD219 |
| CBAE06 unwind | CBAE0F..CBAE10 | POP ECX, RET atCBAE10 length1; inclusive endCBAE10 |
| CBAE11 unwind | CBAE1A..CBAE1B | POP ECX, RET atCBAE1B length1; inclusive endCBAE1B |

AF7EFA..AF7EFF is unreachable alignment after an unconditional jump and is not
a missing constructor branch. AF74A0 already has the correct inclusive body end.
The report retains the AF6120 pre-repair pseudocode correction.

Validation records distinguish strict source compilation, repository build and
existing tests, exact call-row verification, and one focused ignored fixture.
The new source passes MSVC Win32 `/std:c++20 /EHsc /MD /W4 /WX /fp:strict
/permissive-` compilation and links with the existing core library. After
`verify-seeds`, `scripts/build.ps1` passes both `reconstructed_math` and
`native_math_differential`. The report verifier passes98 direct/tail rows with
zero failures;16 explicit indirect rows remain outside that mechanical check.
The fixture executes the original69-byte AFF5F0 body with its real Windows
InterlockedIncrement import relocated, compares all28 output bytes after
normalizing the two definition addresses, verifies one actual definition retain,
same returned address and preserved24..27 preimage; the comparison passes. The
import relocation goes through a WINAPI wrapper over the same atomic intrinsic.
It does not supply a fake
successful AF74A0 dependency or claim model rendering. The source is intentionally
not added to shared CMake/ledgers by this worker; root integration owns that step.

## AK saved-analysis and combined-build integration

The seven-module AK batch is registered in bsp_core. Strict MSVC Win32
compilation and both seeded CTests passed with explicit `--parallel 1`; the
standard parallel script hit environment MSB3491 before compiling C++.
The report records saved Ghidra name/signature preimages, prior-comment
preservation and readback, original-byte fixture coverage and exact call checks.
Reported returning-free continuations and missing definitions are now repaired
and saved; worker-era pending-integration notes above describe the earlier snapshot.
New C++ interfaces and required real runtime bindings remain as documented.
Successful full construction, native EH compatibility and gameplay are not implied.

## AK final merged validation

After merging current main at `aab1373abde21d9a8d03ad113f8a53317cfd8ff5`, the repository standard
`./scripts/build.ps1` completed successfully and both existing seeded CTests
passed. The focused original-byte fixture was rebuilt with `/fp:strict` and
replayed against that combined library; it passed. The report pins its log and
library hash. Earlier parallel MSBuild failures and worker-pending notes above
are historical; the final build required no global configuration change.
All stated constructor, simulation, current-slot, native EH and gameplay limits
remain in force.

The parallel MSBuild invocation remains intermittent: a later documentation-only
rerun again hit MSB3491 before C++ compilation. The final serial full build and
both existing CTests passed again on unchanged source. This environment issue
was recorded rather than changing global permissions or build configuration.

## Correction from AL manager and lifetime reconstruction

AF74A0 now directly calls the concrete AF0950 weak-pointer registration in
native_particle_model_manager.cpp. Constructor unwind directly uses concrete
AF6B70, AFD0F0 and AFD1E0 from native_particle_model_lifetime.cpp. Those three
required host methods were removed; array resize AFD130/AFD220 remains required.
The model environment must bind the same actual_names storage as material
parameter names so its node-name destruction returns storage to the same pool.
See NATIVE_PARTICLE_MODEL_MANAGER.md, NATIVE_PARTICLE_MODEL_LIFETIME.md and
reports/native_model_actual_names.json. Full AF74A0 execution remains unvalidated.
