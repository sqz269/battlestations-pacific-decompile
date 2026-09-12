# Actual particle emitter and container update

Addresses: `00AFF640`, `00B05110`, `00B04990`, `00AF1EB0`, `00B72650`,
`00B04FC0`, `00B05730`, `00B057A0`; analyzed literal leaf `00B05400`.
Names are hypotheses. These are new MSVC Win32 C++ interfaces over actual
storage, not original vtable/FH3/SEH ABI or gameplay replacements.

| Routine | Inclusive native end | Final instruction | Original ABI / coverage |
| --- | --- | --- | --- |
| AFF640 emitter | AFF689 | AFF687 RET4,3 bytes | ECX28h emitter, float delta stack, EAX count; complete |
| B05110 container | B05206 | B05204 RET4,3 bytes | ECX30h container, float delta stack, EAX count; complete |
| B04990 row rotation | B04A6B | B04A69 RET8,3 bytes | ECX row header, stack(first,last); complete |
| AF1EB0 submodel | AF22AC | AF22AA RET4,3 bytes | ECX actual submodel, float delta stack; complete |
| B72650 geometry bounds | B72680 | B7267E RET4,3 bytes | ECX actual geometry, four-float pointer stack; complete |
| B04FC0 base destructor | B0506E | B0506E RET,1 byte | ECX container; complete, no flags/physical owner free |
| B05730 derived destructor | B0579F | B0579F RET,1 byte | ECX container; complete |
| B057A0 scalar deletion | B057BD | B057BB RET4,3 bytes | ECX container, flags stack, EAX original address; complete |
| B05400 current virtual0 | B05402 | B05400 RET4,3 bytes | analyzed literal empty leaf; no new C++ entry needed |

The assembly kernels add a borrowed access pointer in EDX and one stack DWORD
where needed. Existing local offsets and all native x87 spills remain in place;
only the incoming argument offset moves. No state/model/count copy or new
ownership domain is introduced. Direct bridges call the existing actual
geometry/model/element storage APIs and shared CRT square-root kernel.

## Producers and current ownership

The existing B04E30/B053D0 constructor owns the physical30h container;
AFF690 publishes it into actual emitter10. The producer's canonical
`NativeParticleEmitterContainer`,8-byte rows and pointer/capacity members are
reused unchanged. It has no reference count:04/08 borrow model/emitter,0C/10
describe rows,14/18 describe6Ch states,1C is the live count, and2C holds the
derived retained owner. D5DF24 has virtual00=B05400 and virtual04=B057A0.
Emitter20 is the observed count publication; it is not a second container count.

The current deleting-table binding requires actual D5DF24 and its current
slot04=B057A0. AFF640 captures the current container's native profile before
dispatch. The source adds that captured profile as one bridge argument instead
of dereferencing a native identity DWORD as a host C++ vtable. A different
profile or target raises an explicit unsupported-binding error; it does not
silently omit destruction. The table view must remain current and valid.

`NativeParticleEmitterActualCleanupBindings` implements B7C160 through the SAME
`NativePointLightLinksRuntime` and actual backlink descriptor, then B6DFA0
through the SAME `GeneratedModelLifetimeRuntime` and real current virtual18.
It copies no link, transform or count. Missing actual node/light associations
are errors. The inherited current-definition virtual1C capture and actual
72B740 singleton getter remain required application operations. Existing
B04F00 performs their actual lock, retained-light and callback ordering; no
modeled light, substitute lock or successful cleanup fallback is supplied.

## Emitter and state simulation

AFF640 checks current10 before loading delta. A null container publishes20=0
and returns it. Otherwise the native x87 float load/store forwards delta to
B05110; EAX is published to20. On zero, AFF640 reloads current10, captures its
current virtual04, calls with flags1, clears10 AFTER the callback, then reloads
20 for its return. Nonzero count returns the original EAX without a later reload.

B05110 walks signed live count1C, reading each row's WORD ID and using ID*6C
into the current state buffer. It advances state40 through x87 and calls full
AF1EB0 on state30 for definition64+10 type3. It reloads definition64 and the
clock after that callback. Ordered clock>definition20 OR clock>state44 invokes
concrete B04F00(state,0); its actual AL governs removal.

Successful removal rotates the row through signed
`min(current_count, current_capacity-1)` when the current index is less than
count-1, retries that index, and decrements current count. The statistics pass
still uses the ORIGINAL captured state byte offset with the then-current state
buffer and definition. Type0 increments current model1F0 or1F4 according to
definition65; type1 increments1F8. It then advances/rechecks the signed live
count. These post-callback reads and the removed-state statistics are retained.

B04990 shifts WORD ID and float value forward to the inclusive last row,
leaving every row's padding WORD2 in place. It retains the native four-row
unroll and live backing reloads. Intermediate float copies are x87; the saved
first value's final store uses MOVSS. A memcpy/memmove of whole rows would
change padding and signaling-NaN behavior.

## Submodel calculations and bounds

AF1EB0 consumes the existing derived-model fields184..1B4 and actual14h point
records; this packet does not construct that submodel or infer a new owner
layout. It implements the animation-enabled clamp of delta, signed frame-range
arithmetic, loop/clamp branches, CVTTSS2SI frame selection, fractional frame,
age clock, live point expiry with signed IDIV/remainder, and the complete x87
bounding-volume calculation. Native unchecked/nonfinite behavior is retained.

The code uses verified original-width constants at D7A270/D7A280/D7A318,
CE7638/CE4970/CE4ADC, and the existing actual ST0 CRT sqrt entry BF7030 with
borrowed CRT state/exception handling. It calls concrete B74640/B732C0 actual
geometry accessors and B74390/B855B0 bounds setters. New B72650 first sets
geometry80 dirty, then performs four forward x87 writes84..90 with alias order.

At AF21ED the bytes are **DC C9**, multiplying ST1 by ST0. Ghidra prints
`FMUL ST1`, the same shortened operand text it prints for an instruction with
ST0 as destination at AF21F9. The original-byte comparison exposed the wrong
destination; the final source uses `fmul st(1),st(0)`. This distinction controls
the center coordinates and preservation of the0.5 factor for later components.

## Concrete destruction and exception boundaries

B057A0 performs full B05730, then actual CRT free iff flags&1, and returns the
original address. B05730 publishes D5DF24, releases captured current2C via its
sole actual+04/current virtual0, clears2C after the callback, then calls B04FC0.
The existing `NativeRenderActualOwners` resolver validates actual atomic identity;
unknown owners/terminals do not silently succeed.

B04FC0 is incorrectly named a vector deleting destructor in saved analysis.
It has no flags argument and does not free its own physical30h owner. It
publishes D5DF20, walks the signed live count, calls B04F00(state,1) on each
current row, frees/clears the state member and destroys/frees/clears the cookie
row member through their existing concrete implementations. Count1C remains.

DF38D8/mapDF38C8 has state1->0 via CBB7AB/B04A80(+14), then state0->-1 via
CBB7A0/B04C40(+0C). DF3904/mapDF38FC has state0->-1 via CBB7C0/B04FC0. The
source retains C++ exception cleanup order and avoids retrying a failed base;
a second exception during cleanup terminates. Native FH3/SEH dispatch and
arbitrary asynchronous faults are not reproduced or dynamically validated.

## Verification and required saved-analysis repairs

All22 selected live Ghidra code/data spans match the installed PE. Each read
used `bsp.py ghidra`, verifying project `bsp`, program
`/battlestationspacific.exe` and configured image identity. The report records
full spans, hashes, every numeric call and relevant unwind transfer. No Ghidra
mutation occurred in this worker.

Strict Win32 `/W4 /WX /EHsc /fp:strict /O2 /Gy` compilation passed. One fixture
maps only verified spans, preserves relative transfers, applies23 checked
absolute DWORD relocations, and shares the actual reconstructed CRT sqrt/free
boundaries. Original/source comparisons passed for active emitters with four
state types across two updates, full type3 frame/point-expiry/bounds work,
row rotation with signaling-NaN value and preserved padding, real empty
container B057A0/B05730/B04FC0 teardown/free, and the null-container path.
The fixture's initialized backing is not a successful native submodel-constructor
claim. Definition cleanup, retained-owner and light callbacks deliberately fail
if reached and were not exercised. Container state-expiry removal, nonempty
destructor cleanup, exception trajectories, other FP modes and gameplay remain
source/evidence coverage only.

Artifacts and logs are under `C:/Users/sqz269/bsp-am-emitter-update`. No fresh
worktree-wide build was run because of the initial J: space constraint; parent
integration runs the combined standard build and existing checks.

Required instruction gaps are B05022..B05024 and B05054..B05056 within B04FC0,
and B057B5..B057B7 within B057A0: each is a3-byte ADD ESP,4 after returning free.
Missing functions are B05400..B05402 (literal RET4), CBB7B6..CBB7BF
(MOV EAX,DF38D8 then JMP BF6B43), and CBB7C8..CBB7D1
(MOV EAX,DF3904 then JMP BF6B43). The three existing unwind functions already
have correct bodies. Parent integration owns definition/flow repairs,
annotations, ledger metadata, save/export and CMake wiring.

## AM combined validation and saved analysis

The combined strict MSVC Win32 build and both existing seeded CTests passed.
Eight call reports check200 direct CALL rows without failures. All seven focused
replays pass within their documented boundaries. Saved names, native signatures,
full body ranges and old-comment preservation were read back; affected exports
were refreshed. The report embeds the earliest annotation preimages and repair
records. Earlier worker pending notes describe isolated snapshots. No original
exception ABI, complete application composition or gameplay claim is added.
