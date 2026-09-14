# Actual sampler owner and cache lifetime

Addresses: 004dc410, 004dda40, 004ddb40, 004ddaa0, 004de290, 004b4f10, 00b1b680, 004de340, 004de360

`native_sampler_owner_lifetime` reconstructs these complete bodies over the
actual1Ch owner produced by4DE4B0. It reuses the existing actual2Ch records,
record-vector header, owning string pool, allocator and canonical resource
owner domain. The earlier `particle_clock_lifetime` remains a separate typed
projection. Names are hypotheses, and these explicit C++ interfaces are not
binary ABI replacements.

| Routine | Complete body | Original ABI |
| --- | --- | --- |
| 4DC410 record resize | 4DC410..4DC4DB,204 bytes | ECX actual0Ch header; stack signed count; RET4 |
| 4DDA40 release all | 4DDA40..4DDA93,84 bytes | ECX secondary cache; RET |
| 4DDB40 release resource | 4DDB40..4DDB5E,31 bytes | Incoming ECX unused; stacked resource; RET4 |
| 4DDAA0 array destructor | 4DDAA0..4DDAB6,23 bytes | ECX actual vector header; RET |
| 4DE290 cache destructor | 4DE290..4DE2EE,95 bytes | ECX secondary cache; RET |
| 4B4F10 base cleanup | 4B4F10..4B4F20,17 bytes | ECX primary owner; RET |
| B1B680 owner destructor | B1B680..B1B6D8,89 bytes | ECX primary owner; RET |
| 4DE340 scalar deletion | 4DE340..4DE35D,30 bytes | ECX primary owner; stacked flags; EAX captured primary; RET4 |
| 4DE360 secondary thunk | 4DE360..4DE367,8 bytes | ECX secondary; subtract4, tail-forward same flags; RET4 through target |

The shared `NativeSamplerLoaderSingletonStorage` has primary CE7D38 and
secondary CE7D24 at04, and the existing `NativeResourceRecordVectorStorage`
at08. The getter initializes08/0C/10/14 and leaves time18 untouched; B19A10
is the float18 producer. The record's existing alias-list header starts at08,
with sentinel0C/count10. Its resource28 is unretained by record destruction.
No projected clock, copied list header or shadow resource count is introduced.

4DC410 compares requested/current capacity as signed DWORDs before actual
4DA180 reserve. It captures current count only after reserve, creates records
at current data plus wrapped index*2Ch, and leaves record08/resource28 unchanged.
Names become zero before sentinel allocation; sentinel/count and five descending
payload zeros follow. Growth publishes no intermediate count. Shrink decrements
current count before destroying the current record, rereads count after each
destruction and finally writes the requested count. No reverse resource release
or whole-vector rollback is invented.

4DDA40 captures current count, current data, then the last record's resource.
It reads current cache profile and slot10; the qualified CE7D08/CE7D24 views
both select4DDB40. It releases that resource first, then rereads count and data
to destroy the current last record. Count is decremented after that destruction.
Thus a resource terminal can change which record is destroyed. A final
4DC410(0) executes even for an already-empty cache.

4DDB40 uses the existing `release_native_render_actual_owner`: real atomic+04
decrement, then zero-only lookup of the same canonical companion and its actual
terminal. Lookup must have no ownership effects, and the companion must borrow
that exact atomic and implement its current native profile. Missing registration
is an error. No new map, substitute terminal, independent reference count or
registration side effect is added. The original does not guard a null resource;
valid nonnull aligned raw storage is required. Null cache records are not skipped.

4DE290 first stamps secondary CE7D08. Its armed region calls4DDA40; normal
completion disarms the array cleanup, calls4DC410(0) again, and frees the current
array through the existing CRT boundary. 4DDAA0 is the concrete resize0/current
array-free helper used by the cache unwind. Data/capacity remain stale after
free; count follows the original resize effect.

B1B680 stamps primary CE7D38 and secondary CE7D24, then destroys secondary+4.
Normal and unwind completion both clear the CURRENT F8D420 publication
unconditionally and stamp primary CE3818. The unwind does not retain a
publication merely because the host frame is diagnostic. 4DE340 captures the
primary, performs destruction, tests only flags bit0 afterward, and frees the
captured owner only on success. 4DE360 subtracts4 before the same operation.
These bodies do not unregister the owner from the singleton manager: manager
drain must have popped it or the caller must resolve that registration separately.

The saved EH evidence is concrete. B1B680's one-state info DF4B04/map DF4AFC
selects CBC790, which tail-jumps4B4F10. 4DE290's D8FCB4/map D8FCAC selects
C66F80, which adds4 to the captured secondary and tail-jumps4DDAA0. 4DC410's
D8F898/map D8F888 has state1 C669B9, releasing the current failed record name
through41DD20, followed by state0 C669A0, which computes current-base/captured
index and placement-pointer arguments for the RET-only401130. This adds no
allocation free or completed-prefix rollback. Source C++ catches express these
effects; original FH3 identities, arbitrary hardware faults and throwing native
unwind delivery are outside the new ABI. Existing pool noexcept/child exception
limits remain in force.

The one-shot operation keeps reached owner/resource/array/record identities and
completed cleanup flags through failure. These are diagnostic addresses and can
already refer to freed storage; callers must not dereference freed preimages.
Failed frames reject replay and terminate on unacknowledged destruction.
Acknowledgement performs no cleanup. Keep actual contexts and canonical owners
alive, exclude external retirement, and do not mutate diagnostic fields while
an operation runs or retains failure state.

Three Ghidra repairs remain for the integrator after lease release. Original
bytes and inclusive ends are pinned; no worker mutation occurred:

| Stored flow issue | Required returning tail |
| --- | --- |
| 4DE350 CALL BF65AC, interior gap | 4DE355..4DE357: ADD ESP,4; complete body ends4DE35D |
| 4DE2D7 CALL BF6989, stored body ends4DE2DB | 4DE2DC..4DE2EE: restore saved registration/registers and RET |
| 4DDAAD CALL BF6989, stored body ends4DDAB1 | 4DDAB2..4DDAB6: ADD ESP,4; POP ESI; RET |

The EH dispatch handlers CBC798..CBC7A1, C66F8B..C66F94 and C669C1..C669CA
have no Ghidra function and are raw-pinned through their complete final JMPs.
The four referenced unwind actions already have saved function bodies.

Strict default MSVC Win32 and the two existing CTests pass. The single local
fixture executes copied original bodies for record growth and three owner
destruction schedules: primary flags0, secondary flags1, and a real resource
terminal that changes current cache count. It uses actual pooled records and
real `NativeRenderContextReference` terminal bodies through the canonical domain;
the original release body executes its actual decrement before the same terminal.
Growth compares the preserved A5 record08/28 preimage. The original and source
destruction traces agree, including current-array free, final profiles, captured
return address and unconditional publication clear after terminal mutation.
The count-mutation leg has the same orphaned record/resource effects; fixture
diagnostic cleanup resolves those explicitly after comparison.

A source-only canonical lookup failure after the real decrement proves array
unwind/free, publication clear, base stamp, absence of scalar free, retained
zero-count resource, replay rejection and failed-frame destructor exit77.
Original FH3 failure paths are not executed; growth-unwind effects are inspected
statically. Singleton-manager/string-pool arenas remain alive until process exit,
so this is not full application shutdown validation.

Fresh BBC6F0/BBC810 procedural creators still require canonical companions and
their concrete D64478/D644B4 terminal route (BD30E0, BBC6D0/BBC7F0, C304A0 and
its resource-array dependencies). This packet admits already-qualified canonical
resources; it does not close that remaining procedural-resource lifetime or the
full cache-loading entry. Existing NativeRenderContext terminals provide the
fixture's real admitted resource, not a claim that it is a procedural texture.
The manifest separates581 reconstructed normal bytes,60 profile bytes and222
unwind code/metadata bytes and pins all local fixture inputs and selected linked
project sources. The runner supports `-LibraryRoot` for integration.

## Saved Ghidra correction and integrated validation

The integrator repaired the free-call fall-through gap in 4DE340 and recreated
the complete stored bodies of 4DE290 and 4DDAA0. Their inclusive ends are
4DE35D, 4DE2EE and 4DDAB6, with 11, 26 and 10 listed instructions and zero
gaps. Earlier incomplete repair records remain historical evidence. Existing
comments were preserved. Handlers CBC798..CBC7A1, C66F8B..C66F94 and
C669C1..C669CA are now defined; none counts as an additional reconstruction.
All these saved changes and the reviewed names survived the Ghidra restart.

At f2cf87ab the default Win32 build, both CTests and the integrated lifetime
fixture pass. Independent review confirmed the nine normal source bodies and
shared canonical resource domain. The fixture does not execute the original
4DDAA0 or 4B4F10 entries or original FH3 failure delivery; their source effects
are supported separately. The report links the immutable integrated checkpoint.

## Correction from NATIVE_PROCEDURAL_RESOURCE_LIFETIME

The fresh BBC6F0/BBC810 companion frontier above is now implemented by
`docs/NATIVE_PROCEDURAL_RESOURCE_LIFETIME.md`: both factory profiles use the
same raw+04 reference count and `NativeRenderActualOwners` identity domain,
with concrete BD30E0 and current virtual4 destruction. The integrated fixture
exercises admission through actual 4DDB40 sampler release, including current
profile changes. This closes that qualified lifetime dependency; it does not
make native unconditional null-record release safe or complete the whole
cache/event/compiler path. The follow-up report pins the d260af28 evidence.
