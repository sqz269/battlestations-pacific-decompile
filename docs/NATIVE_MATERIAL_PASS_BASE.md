# Actual material pass base lifetime

Addresses: 00B17940, 00B17970, 00B40C80, 00B40E00, 00B5F3F0, 00B41BD0,
00B41CA0, 00B41D90, 00B5F720, 00B5F510, 00B5F990.

`native_material_pass_base.hpp/.cpp` reconstruct ten complete functions over
actual18h root and5Ch base storage. B17970 remains the existing correctly named
BD30F0 cleanup thunk; its instructions and unwind use are verified. C++ names
are descriptive hypotheses, not recovered symbols.

This composes the actual state owners from `NATIVE_MATERIAL_PASS_STATES.md`.
It completes the B5F720 parent of the earlier owner-initialization fragments.
The derived88h pass, its8Ch pool slot, shader ownership and effect loader remain
separate work.

## Layout and behavior

| Offset | Actual field |
| --- | --- |
| 00/04 | Native vtable / atomic count |
| 08/0C/10/14 | Three DWORDs / borrowed effect pointer |
| 18/1C/20 | Render / third / sampler state owners |
| 24 | 12h header for8-byte pairs; row interpretation unresolved |
| 30/3C/48 | 12h headers for8/12/12-byte rows |
| 54/58 | Two retained owner pointers |

B17940 (ECX fresh18h root, EAX same address, RET) publishes CEB130/count1/D5E508
and zeros08 through14. B5F720 has the same receiver/return ABI for5Ch storage.
It invokes the root, publishes D62A80, clears the four headers and54/58,
allocates three14h owners through BF681B and publishes18,20,1C in that order.
The actual setters install nineteen render defaults, six defaults for each of
sixteen sampler slots, then render state34=0. Final counts/capacities are
render20/32 and sampler96/128; the third owner stays empty.

Render defaults in order are C3=0, AF=0, 7=1, E=1, 17=2, F=0, 1B=0, AB=1,
CE=0, D1=1, 16=3, 8=3, A8=F, BE=F, BF=F, C0=F, B5=0, 9A=304D3241, A1=1,
then34=0 after all samplers. Each sampler gets6=2, 5=2, 7=1, 1=1, 2=1, 8=0.
These are raw native DWORD values; no inferred D3D enum conversion is needed.

B40C80 (ECX actual12h header, signed stack capacity, RET4) is instruction-
equivalent to B40AC0 after normalizing the two external relative call operands.
Its C++ implementation shares that already reconstructed8-byte reserve body.
B40E00 (same ABI, requested count) reserves if capacity is smaller, zeroes
newly exposed pairs, decrements an excessive current count and stores the
request. Shrinking leaves row bytes intact.

B5F3F0 (ECX pair header, RET) calls resize(0), then frees data. B41BD0/B41CA0/
B41D90 clean the30/3C/48 headers respectively: negative capacity first calls
the matching state reserve(0), then count reduces/stores0 and data is freed.
All four leave data/capacity stale after final free. Safe negative-capacity
fixtures use one live row, fitting the reserve(0) clamp to1.

B5F510 (ECX actual5Ch base, RET) publishes D62A80 and releases18,20,1C,54,58 in
order. Each slot clears after its terminal callback. Later slots are loaded
when reached, so a callback can publish a later owner. It then cleans embedded
48,3C,30,24 headers and invokes BD30F0 to publish CEB130. B5F990 takes stack
flags, RET4, returns the original pointer and frees it through BF65AC iff bit0.

The canonical base companion borrows actual+04 and checks current
D62A80/BD30E0/B5F990. Final zero dispatches actual destruction and heap free,
then retires the companion. It uses the caller's shared actual-owner domain.
After base construction, the caller binds the three actual state children in
that domain before their terminal releases; no private registry or copied
reference count is introduced here.

## Unwind and incomplete analysis ranges

Constructor handler CC1214 selects FuncInfo DF9F24 and the five-entry map at
DF9F48. State4 through0 call CC1209/CC11FE/CC11F3/CC11E8/CC11E0: clean48/3C/
30/24, then root B17970. The destructor handler CC11D4 selects DF9ED8 and map
DF9EFC, with corresponding CC11C9/CC11BE/CC11B3/CC11A8/CC11A0 cleanups.
The C++ guards follow this reverse resource ordering.

The constructor's unwind map has no state-owner release. If allocation or
default population throws after an owner was published, the native failure
path cleans embedded arrays/root but leaves those state pointers allocated.
The reconstruction preserves that behavior. Original exception delivery and
the constructor allocation-failure path are byte/map reviewed, not executed
by the fixture.

Free-call continuations were decoded under the Ghidra write lock. Internal
gaps in B40C80/B5F990 are repaired. Five stored bodies remain truncated:

| Entry | Stored inclusive end | Verified full exclusive end |
| --- | --- | --- |
| B5F510 | B5F602 | B5F69E |
| B5F3F0 | B5F401 | B5F407 |
| B41BD0 | B41C07 | B41C0D |
| B41CA0 | B41CD7 | B41CDD |
| B41D90 | B41DC7 | B41DCD |

Complete assembly and fixture spans through RET are checked against the
installed PE. Ghidra exports are refreshed after annotation, but these five
stored ranges are still incomplete. The server's disabled Java scripting
capability was established in the preceding packet; it was not retried or
bypassed here.

## Validation and remaining work

The strict MSVC Win32 build and both existing CTests pass. One ignored native
fixture executes all ten recovered routines plus the root cleanup thunk.
It compares24 root bytes,1,464 constructor bytes (92-byte base, three20-byte
owners,160 render-row bytes and1,152 sampler-row bytes), and92 destructor
bytes, with pointer identities normalized. All20/96 defaults match.
It also compares the four direct array cleanup helpers and pair resize
growth/shrink, including negative-capacity allocation/copy/free paths.

Native parent destruction uses original decrement and calls a relocated
terminal adapter which restores each child's numeric profile and dispatches
its canonical reconstructed owner. Callback order is render, sampler, third,
retained54, retained58; the first callback supplies the previously-null54 slot.
Across the paired cases and final-zero composition, nine actual state owners,
six actual contexts and one actual base owner are retired. A count2 release
first leaves the same base and child owners alive.

The initial fixture build printed successful comparisons but returned1:
MSVC inlined its assembly adapters into main and emitted no implicit EAX=0
assignment. Compiler listings preserve that evidence. Explicit `return 0`
fixes the fixture epilogue, and the final run exits0.

Original CRT allocation/free and the decrement import are rebound to current
host boundaries. Original scalar flags2 keep storage; rebuilt flags1 frees
actual storage. Native SEH exception delivery is untested. Public C++ APIs
require valid readable extents and are not drop-in ABI replacements. Full
derived pass/pool lifetime, real texture/shader loading, draw and gameplay
validation remain open. `reports/native_material_pass_base.json` pins source,
dependencies, original spans, annotations, fixture and frozen build artifacts.
