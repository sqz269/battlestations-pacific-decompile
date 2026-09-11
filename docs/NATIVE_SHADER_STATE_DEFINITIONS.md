# Native shader state definition storage and lifetime

Addresses: 00b56610, 00b566b0, 00b56750, 00b567b0, 00b57630, 00b57800, 00b57930, 00b58050, 00b58200, 00b58300, 00b58320, 00b585a0, 00b59e50

The actual 40-byte manager and its three owning definition arrays are now reconstructed in
`native_shader_state_definitions.hpp/.cpp`. These are explicit C++ interfaces over actual Win32
storage, not binary replacements. Descriptive names remain hypotheses rather than recovered
symbols. `reports/native_shader_state_definitions.json` pins the installed-PE captures, tested
source, native fixture, unwind evidence and Ghidra annotation readback.

## Storage and production ownership

| Offset | Actual field |
| --- | --- |
| 00 | Numeric vtable: base D621EC, derived D62260 |
| 04 | Render definitions: data, signed count, signed capacity |
| 10 | Sampler definitions: same 12-byte header |
| 1C | Texture-stage definitions: same 12-byte header |

Each definition is 16 bytes: actual 8-byte NativeString at00, DWORD state ID at08 and conversion
tag at0C. Full constructor B585A0 registers 31 render,13 sampler and18 texture-stage definitions
in that order. The existing three `shader_*_state_registry.inc` tables are reused unchanged;
the original full constructor independently verifies every string, ID, tag and ordering.
Final capacities are32/16/32. Tags0/1 designate the subsequent reader's integer/float-bit paths;
this packet does not implement that reader.

The base constructor B56610 publishes the SAME owner in the application's0108FE90 slot before
any derived array is initialized. It writes D621EC, captures the section from the first shared
01090AA0 manager getter, enters/increments it, publishes the owner, then gets the manager again
and registers the CURRENT published pointer. It unlocks the captured section and returns the
original owner. Base destructor B566B0 likewise unregisters the CURRENT global after its second
getter, clears publication unconditionally, releases the captured section, then writesCE3818.
It does not substitute `this` for the publication. No reference counter is present.

`NativeShaderStateDefinitionsLifetimeBinding` composes numeric D62260/B59E50 and D621EC/B56750
dispatch with REQUIRED other-owner callbacks in the existing SingletonLifetimeDomain. Construct
the binding before that domain and explicitly bind the domain/string storage before constructing
definitions. Keep the binding, domain and actual string pool alive through owner cleanup. There
is no private publication slot, owner registry or lifetime domain. Fixture composition starts the
actual string pool before definitions so its lifetime outlasts their strings.

## Full routines and observable ordering

| Routine | Coverage | Original ABI and recovered behavior |
| --- | --- | --- |
| B57630 | complete | ECX fresh record, stack source, EAX same, RET4. Initialize only name, resize/copy with current source/destination reads after callbacks, then copy state/tag. |
| B57800 | complete | ECX header, stack signed request, RET4. Minimum1, signed capacity check, shared-heap request*16 allocation; copy forward with live source count/data, destroy old names forward, free current old array, publish pointer then capacity. |
| B57930 | complete | ECX header, stack signed size, RET4. Reserve if needed; growing initializes ONLY names, leaving state/tag preimage. Shrinking decrements live count before releasing each last name. Final count=request. |
| B58050 | complete | ECX header, stack source, RET4. Grow full capacity by2 with minimum1; copy into current end, then increment current count. No deduplication. |
| B58200 | complete | ECX header, stack name/state/tag, RET0C. Copy name into a temporary record, append, release temporary's captured pointer using its current length. |
| B58300 | complete | ECX header, RET. Resize(0), free current data; pointer/capacity remain stale. A negative capacity can cause minimum1 reserve before cleanup. |
| B567B0 | complete | ECX 8-byte-row header, stack state/payload, RET8. Search captured unsigned count; first existing ID wins without replacement. Grow by5 with minimum10 through B40CF0, then append to current end. |
| B56610/B566B0 | complete | ECX owner, RET. Base publication/registration and unregistration described above; constructor EAX is original owner. |
| B56750 | complete | ECX base owner, stack flags, EAX old pointer, RET4. Base destruction then shared heap free only on flags bit0. |
| B585A0 | complete | ECX fresh40-byte allocation, EAX same, RET. Base, derived profile, zero three headers, then62 registrations. Each normal temporary cleanup uses pointer AND length captured before registration. |
| B58320 | complete | ECX owner, RET. Derived profile; destroy arrays1C,10,04, then base. Each array releases names backward and frees its current allocation. |
| B59E50 | complete | ECX owner, stack flags, EAX old pointer, RET4. Full derived destruction; shared heap free on flags bit0 only. |

Self-aliasing copy first zeros the source's same name header. Append sources that point into an
array can dangle across growth; valid input lifetime/backing extents are caller contracts. No
allocator-null fallback, rollback container or normalized stale pointer was invented. Zero-byte
memcpy omission follows the existing native string host boundary.

## Unwind and Ghidra limits

Seven FuncInfo maps and76 cleanup states are PE-verified. Base ctor/dtor each have two states:
captured-section guard411EE0 then base412430 (CE3818). Registry reserve/append each have only a
placement-delete state invoking401130, whose original body is a single RET. They do not free
the new array or destroy earlier successful copies when a later copy fails. Registration's one
state invokes B56C10, verified to release only the actual temporary name and leave its header
stale. These library/trivial helper names were retained and are analyzed dependencies.

Destructor B58320 has three states covering base,render04,sampler10; the in-progress stage1C
is not retried. Constructor B585A0 has66 states: base,three arrays,then62 temporary-name states,
each returning to state3. C++ uses structured cleanup preserving those remaining-member paths.
Native exception delivery was not executed; the rebuilt throwing-allocation check exercises
failure while copying the first registration name. It releases the completed outer temporary,
cleans arrays, unregisters and restores the base profile, leaving raw object allocation with
the caller. The existing string release interface remains noexcept.

Three internal CALL_RETURN gaps were repaired under the shared Ghidra write lock: after
B56760,B578FF,B59E60. A3-byte alignment gap B578CD..CF is untouched. Stored bodies B58300 and
B58320 remain incomplete (end atB58311 andB5835F), despite their verified complete installed-PE
extents B58300..B58316 andB58320..B583BA. No unsupported Java/body-extension workaround was used.
Their missing tails (5 and91 bytes) are captured, decoded, fixture-executed, and explicitly
separated from live-listing call verification. Prior names/comments are preserved before all
13 annotations; affected exports are refreshed and the Ghidra project saved.

## Validation and follow-up packets

Strict MSVC Win32 build and both existing CTests pass; no permanent tests were added. One ignored
fixture executes all13 original bodies with verified external calls rebound to the shared heap,
actual string pool/resize and existing singleton manager boundaries. Its full-manager comparison
uses all original internal packet callees and actual62 literal captures. Original singleton calls
use an explicit native-layout facade for the rebuilt manager and a real Win32 critical section;
this is a fixture adapter, not evidence that the host manager has the original binary layout.

Native/rebuilt snapshots and string allocation/release sequences agree for definitions, reserve,
growth preimage, shrink,17 unique state pairs, full manager construction/destruction and scalar
flags1/2. The base leaves the remaining36 bytes untouched. Production shutdown uses the actual
40-byte owner and actual string pool in one canonical domain. Native exception ABI, full Lua
parsing, shader creation, rendering and gameplay remain unvalidated.

Follow-up packets: B579B0 conversion into actual eight-byte state rows; full B57B50/B41830 sampler
reader using these actual registries and pooled headers; B573F0/B419B0 owning field-table parsing;
full actual B43B00 descriptor reader and subsequent B45EE0 effect-loader composition. Existing
`shader_lua.cpp` semantic projections remain distinct from these actual storage dependencies.
