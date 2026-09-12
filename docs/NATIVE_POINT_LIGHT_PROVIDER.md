# Native PointLight volume and Model population

Addresses: `00B6EF20`, `00B72140`, `00B7B090`, `00B0CA40`, `00B6E8C0`,
`00B8F100`; discovery evidence `008740E0`, `008738B0`, `00872D30`.
Names are descriptive hypotheses. These are new C++ interfaces, not binary ABI replacements.

The native population route now writes actual Model `+164` light arrays from a
real PointLight owner: `B7B090 -> B72140 -> B6EF20 -> B6EED0 -> B7BE40`.
The existing canonical scene/hierarchy bindings resolve raw node identities;
the existing physical-array allocator stores the same raw PointLight and Model
addresses in both directions. No additional scene, bounds cache, light graph,
reference count or replacement light is created.

| Routine | Original ABI | Coverage |
| --- | --- | --- |
| B7B090 | ECX PointLight, root list on stack, RET4 at B7B0A3 | Complete body through B7B0A5 |
| B72140 | ECX root list, raw PointLight on stack, RET4 at B72161 | Complete body through B72163 |
| B6EF20 | ECX node, raw PointLight on stack, RET4 at B6F084 | Complete traversal through B6F086; current type/bounds dispatch required |
| B6E8C0 | ECX node, EAX actual node+13C, RET at B6E922 | Complete storage adapter; existing affine-sphere numerical boundary retained |
| B8F100 | ECX group, EAX actual group+13C, RET at B8F12E/B8F18B | Partial: cached and byte175==0 branches; dynamic cache-miss B8F118..B8F12E excluded |
| B0CA40 | ECX particle, six stack DWORDs, RET18 at B0CC06 | Partial volume fragment B0CAE1..B0CBA6; earlier particle initialization and subsequent lock/publication excluded |

B7B090 tests the signed reverse count for equality with zero. Any nonzero
value skips population, so movement, radius change and a second call do not
automatically refresh existing links. B72140 reads the actual root `+0C` head
and reloads each node's `+3C` after the recursive call. B6EF20 reads the current
type predicate with live tokens from `0109032C`, `01090034`, `01090344`,
`010903C8`, in that order and with the original short circuit behavior. The
last two tokens remain address-named; no class meaning is inferred here.

A group is a pruning volume: its distance strictly greater than radius sum
returns before visiting children. It is never appended. Accepted non-group
types append only when distance is strictly less than radius sum. Exact
tangency therefore descends through a group and excludes a Model; unordered
comparisons also descend through a group and exclude a Model. Unknown types
still descend. Each sphere getter is called twice, and the second current
radius is combined with the PointLight radius captured before the length call.
The code preserves x87 subtraction/spill, shared CRT length, double temporary,
float radius-sum spill and the opposite FCOMIP operand orders. It does not use
a squared-distance approximation or snapshot child list.

The concrete bounds adapter reads local sphere `+08`, world matrix `+F0`,
flags `+5C/+138`, and the same cached sphere `+13C`. Current Model profile
`00D62DE8` resolves directly to B6E8C0. Current Group profile `00D634F8` handles
static or already-cached bounds. A dynamic group cache miss requires B8EBE0
aggregation and its B8E9F0 sphere merge and fails explicitly until supplied.
Other profiles require their actual virtual48 binding. The affine-sphere
kernel still requires its established finite-input/CW007F or027F domain; this
packet does not expand that CRT/exception equivalence claim. The traversal
itself uses the existing explicit CRT state and exception-handler binding.

## The actual volume producer

B0CA40's post-initializer block establishes the four constructor-unwritten
words: `+1EC/+1F0/+1F4` are spatial-test center XYZ, and `+1F8` is its radius.
The particle `+60` stores the actual raw light. If emitter byte `+1B0` is zero,
three forward FLD/FSTP pairs copy the supplied position. Otherwise the emitter's
same world transform refreshes if needed, the current raw particle light is
reloaded, and the position is added to world translation with the original
float spills and Z operand order. All three relative sums precede any light
write. The current minimum at D7A238 loads before the final Z store; template
`+8C` radius is COMISS/JA-clamped to that value and written to the current raw
light slot. The installed minimum is DWORD `3C23D70A` (0.01f), but the API
borrows the actual live word. Unordered radius selects the minimum.

The fragment consumes existing source fields and a previously constructed
canonical PointLight; it does not allocate a particle, invent an emitter, or
substitute a host light. B0CAE1..B0CBA6 is explicitly bounded. The earlier
particle virtual18 dispatch and the B0CBA7..B0CBF5 shared lock/attachment route
remain external. The caller must invoke the completed population body at that
real synchronization point. Current bindings must stay alive across callbacks;
foreign identities and missing dispatch are diagnostics, not native malformed
memory behavior. A failure after forward append has the existing nontransactional
array contract and must not be retried as a completed operation.

## Discovery, ownership and remaining provider work

008740E0 creates a dynamic-effect PointLight, initializes base diffuse/specular
and clamps its radius. Its 008738B0 update writes Node world position through
virtual30, calls B7B090, then updates radius and lifetime. It does **not** write
PointLight XYZ at1EC. Treating those transform writes as a producer for the
separate test-center words would be unsupported. The AF90A0/B08F60/B0B6A0
constructor candidates are particle initializers; BC0510 is lightning. Those
names/strings do not identify Text's source Model provider.

Raw array publication performs no retain and preserves duplicates. The source
Text clone's existing B6F150 positive-light loop therefore copies the same raw
light and appends a reciprocal raw clone identity. Node-first logical release
removes the corresponding reverse entries; light-first destruction removes
forward entries before ending light storage. Existing Model/PointLight owners
perform terminal deletion and pool return. The new functions never create or
own another owner-reference path or retained scene list.

The route that creates and supplies Text's actual source Model selected at
AB9D75 is still unresolved, including its resource lookup/geometry and the
actual caller that chooses this light population route. Effective specular,
diffuse scales, renderer-light projection and illuminated output are not
established by these spatial words. The rebuilt game does not yet reach this
provider; there is no illuminated Text or gameplay claim.

Read-only Ghidra verification used project bsp /battlestationspacific.exe.
No Ghidra mutations occurred. Two unrelated discovery leaves are undefined in
Ghidra despite matching installed bytes: `008721A0..008721B1` is the dynamic
effect expiry predicate and `00B7AB50..00B7AB55` returns current010901C0.
They are requests for integrator definition only, not completed implementations.
Every implemented body has its complete live containing range; no raw-only
call has been attributed to another body. Exact calls are in the JSON report.

## Verification

MSVC Win32 `/std:c++17 /W4 /WX /O2 /MD /EHsc /fp:strict` compiled the new
source. One local fixture linked the parent combined library and actual CRT/
Win32 services with `/MANIFEST:EMBED`. It passed actual PointLight and Model
pool ownership, absolute/relative volume writes, unordered radius clamp, real
Model bounds refresh, strict tangent exclusion, physical root population,
nonempty guard, duplicate raw clone links, unchanged owner references, and
node-first then light-first teardown. The fixture initially omitted the root
registration+A4 backlink; supplying the consistent root binding corrected its
final-head assertion. No implementation behavior was changed to accommodate it.
The fixture does not execute original bytes or cover dynamic Group aggregation.
The integrator registers the new source and runs the combined build.

## Correction from docs/ORCH5_MENU_LISTBOX_GROUP_BATCH.md

Group B8F100 dynamic cache misses are now composed with the actual Group owner and B8EBE0 physical attachment aggregation. Either cache bit is a hit; callbacks do not cause mode175 redispatch. Static numeric limits remain. Final-library native/owner evidence is in reports/orch5_menu_listbox_group_batch.json. Complete particle/Text and other child48 providers remain open.
