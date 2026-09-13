# Actual sampler loader singleton and factory forwarder

Addresses: 004de4b0, 00b1b810, 00b1b4d0, 00b1a4f0

`native_sampler_loader_context` implements the complete normal bodies of the
actual F8D420 singleton getter and its CE7D24 factory-forwarding slot. It reuses
the actual raw singleton manager, registry lookup and resource creators. Its
explicit C++ interfaces are not native ABI replacements. Descriptive names are
hypotheses; the existing Ghidra name `BSP_ParticleClock_GetSingleton` is retained.

| Routine | Coverage | Original ABI |
| --- | --- | --- |
| 004DE4B0..004DE576, 199 bytes | Complete normal body; explicit source guard cleanup | No consumed inputs; EAX owner; plain RET |
| 00B1B810..00B1B823, 20 bytes | Complete normal body | Incoming ECX and second stacked argument unused; first stacked name; EAX creator result; RET8 |
| 00B1B4D0..00B1B5E7 | Analyzed dependency, unimplemented in this module | ECX primary owner; stacked name; RET4 |
| 00B1A4F0..00B1AA29 | Analyzed dependency, unimplemented in this module | Cache-loading route requires its separate actual operation and service contracts |

The getter itself produces the 1Ch owner: profile00=CE7D38,
profile04=CE7D24 after temporary CE7D08, and zero DWORDs08/0C/10/14.
The existing `NativeResourceRecordVectorStorage` is reused at08; its existing
2Ch record type is not redefined. B19A10 writes float18 and walks that vector.
The getter leaves18 unchanged, including its allocation preimage. The older
`ParticleClock` in `system_time_constants` is a projection with a different
layout and is not admitted as this owner.

The fast path returns its first captured publication without touching the
manager. The slow path captures the first manager's section10, enters it and
increments the section's actual depth18, then rechecks publication. Allocation
and initialization precede publication. The getter obtains the current manager
again and registers the current publication through BD0C30, releases the first
captured section, then rereads publication for its return. Null section and
null allocation branches are retained; the established allocator normally
throws after failed allocation and a declining new handler.

| Call site | Callee | Concrete reused contract |
| --- | --- | --- |
| 004DE4D9, 004DE53F | 00415350 | Actual01090AA0 raw manager getter, no arguments |
| 004DE50A | 00BF681B | Canonical allocator for1Ch; ADD ESP,4 at004DE50F |
| 004DE54D | 00BD0C30 | Current raw manager/current object registration, RET4 |
| 004DE4F2, 004DE55B | IAT CE2218/CE2210 | Real Win32 Enter/LeaveCriticalSection on the captured section |
| 00B1B810 | 00B1B730 | Existing actual F8D41C registry getter |
| 00B1B81C | 00B19E90 | Existing actual registry lookup/creator, RET4 |

All six getter caller sites are checked numerically: 004DEED5/004DF2C4 in
004DE610; 00B46CC2 in00B46A70; 00B3B2BA in00B3B280; 00B33A13 in00B33A10;
004E53A6 in004E4A40. OnMove's float is stacked before the getter for the later
B19A10 call; it is not a getter argument. B1B810's only current xref is the
CE7D2C profile data slot. Its assembly consumes only the first stacked name.

CE7D24 contains secondary deleting thunk4DE360, requested-name operationB19E40,
factory forwarderB1B810, retain4DDB20, and release4DDB40. The thunk adjusts ECX
by-4 before4DE340. These numeric profiles are actual identities, not callable
host vtables; this packet does not add terminal singleton destruction dispatch.
The existing D5F088 texture-cache operation binds different renderer loader
slots and cannot stand in for this procedural resource-factory cache.

The forwarder obtains the existing actual registry and returns B19E90's result
unchanged. Its borrowed `NativeResourceRegistryLookupContext` retains the same
four actual D64470/D644AC/D644E8/D644F0 factory profile views and returning CRT
service. The current qualified creator selectors are BBC6F0 and BBC810. This
adds no fallback loader, cache population, retain, null substitution or new
factory/provider identity. Unknown profiles/selectors remain the existing
lookup's explicit source-contract boundary.

The persistent one-shot frame retains captured managers, section, allocation,
publication cells, registration argument/status, lookup context and result.
Failed frames reject replay and terminate if retired without explicit diagnostic
acknowledgement. Callers must retain all borrowed storage, exclude owner retirement,
and not mutate the diagnostic frame while it runs or holds failure state.
Acknowledgement frees and unregisters nothing. Registration failure leaves the
allocation and publication intact. Guard state0 is established after Enter and
the depth increment; its cleanup reuses actual411EE0. The original handler
C66FA8..C66FB1 selects D8FCE0, whose one-entry unwind map atD8FCD8 names
C66FA0..C66FA7; that existing `Unwind@00c66fa0` thunk takes the saved guard and
jumps to411EE0. The handler C66FA8..C66FB1 has no Ghidra function; both small
code ranges are pinned, including their complete final instructions. Native FH3 metadata, original hardware faults
and provider exception identities are not reproduced by the C++ interface.

The scoped dependency audit leaves B1B4D0's copy/lowercase/suffix and B1A4F0
cache operation separate. In particular the B1A518 BECCD0 service edge through
BECB20 needs an actual platform contract; resource request, VFS/cache ownership,
current profile dispatch, and all failure cleanup must agree before full
B1A4F0, B1B4D0 or descriptor-sampler B3B280 closure is claimed. Those two cache
addresses were handed to an independent packet after this module's interface
stabilized. No analyzed-only function is added as an implementation here.

Validation at base2d5ec452 uses the strict default MSVC Win32 build and its two
existing CTests. The one local fixture executes copied original getter and
forwarder bodies, rebinding six direct edges to the existing actual library
providers and both OS imports to observed real Win32 operations. Four getter
schedules and real factory creation/miss agree for84 trace words. It checks
the untouched18 preimage, actual manager registration, post-Enter publication
recheck, current manager after allocation, and post-Leave return reload. The
registry is producer-layout raw fixture storage with actual read-only profiles;
its constructor is not executed. The manager and tracked section use their
actual construction and cleanup providers. Empty singleton/fresh resource
fixture cleanup is diagnostic, not a completed terminal profile route.

Two source-only failures cover allocation before publication and returning-CRT
registration failure after publication, actual guard cleanup, retained state,
replay rejection and failed-frame destructor exit77. Original child bodies,
original native exception paths, multithreaded races, arbitrary factory slots,
full cache loading, binary ABI and game behavior are not fixture-proved.
The pinned totals are219 implemented code bytes,84 profile bytes and62 guard
code/metadata bytes; these categories are separate. The local runner accepts
`-LibraryRoot` for integration. The report and final local manifest pin source,
original bytes, transitive fixture inputs, executable, library, runner and logs.
