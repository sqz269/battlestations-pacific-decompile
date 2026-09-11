# Actual resource-cache leaves

Eight complete original bodies (503 bytes) are reconstructed in
`src/native_resource_cache_leaves.cpp`. They use actual caller storage and the
existing owning string pool, comparator, CRT and Win32 services. They do not
construct a registry or implement cache/factory dispatch. The source was
reviewed by the primary before evidence freeze.

| Address | Bytes | Native interface / effect |
| --- | ---: | --- |
| B19980 | 22 | ECX raw storage, EAX same pointer, RET; profile/count/profile stores |
| C30470 | 35 | ECX raw storage, EAX same pointer, RET; base construction then specific zero stores |
| BBC6F0 | 80 | ECX unused, EAX nullable raw resource, RET; allocate34h, construct, final D64478 |
| BBC810 | 80 | ECX unused, EAX nullable raw resource, RET; allocate34h, construct, final D644B4 |
| B19E40 | 80 | ECX unused; stack output/requested/options, EAX output, RET0C |
| 4DDB20 | 21 | ECX unused; stack resource, EAX captured resource, RET4; atomic increment+4 |
| B19B90 | 83 | ECX raw tree; stack key, EAX lower-bound node, RET4 |
| B19D60 | 102 | ECX raw tree; stack iterator-output/key, EAX output, RET8 |

Names are hypotheses. The interfaces are C++ source APIs, not original binary
ABI replacements. Address/ABI/evidence and all dependency limits are recorded
in `reports/native_resource_cache_leaves_audit.json`.

The resource base writes original identity CEB130, reference count+4=1, then
D5E554. C30470 writes D79B54, zeros DWORDs +10/+14/+18/+8 in that order, then
only byte+1C. Every other byte is preserved. The creators allocate exactly34h
through `singleton_lifetime_allocate`, call those constructors, and write their
final original profile word. These words retain native identity; they are not
synthetic C++ vtables. No input factory or profile is dereferenced.

Native creator FH3 state0 actions CC4B70/CC4BB0 free the captured allocation
through BF65AC. The complete 11-byte actions, handlers and maps are evidence
dependencies, not additional reconstructed-entry claims. The source records
that captured-allocation cleanup, but its valid-storage constructors contain
no throwing C++ operation. MSVC eliminated the unreachable catch from the
compiled creator helper; no catch/free/EH relocation remains there. Native
SEH faults are outside this interface and were not injected into the fixture.
Allocation itself occurs before the source/native constructor cleanup region.

B19E40 captures output/source identity before unconditionally zeroing both
output words. A self-copy clears the header without freeing its old block.
Otherwise it uses full actual-header 41DD40 through
`ActualNativeStringPoolStorage`, rereads current source length, then captures
current output length, source data and destination data in native order. It
arms no new cleanup for resize/copy failure. After those pointer captures, a
zero-byte memcpy is omitted, matching the established raw-string source
boundary; nonzero copies require valid backing ranges. Options and incoming
ECX are unused. There is no fallback to SizedStoragePool or a second allocator.

4DDB20 calls real InterlockedIncrement at captured resource+4 and returns the
captured pointer. It adds no null/type guard and does not read a resource
profile or clone a resource. The Win32 compiler emits its atomic intrinsic.

Tree lookup consumes caller-owned actual storage: tree+4=head, head+4=root;
1Ch-byte nodes contain left/parent/right at +0/+4/+8, key length/data+C/+10,
value+14, color+18 and nil+19. Lower-bound captures the initial head, then uses
the complete existing actual-header 443D00 comparator/current `_stricmp` and
current node links. Find calls lower-bound before null-owner CRT validation,
compares against the current head, and rereads the current head on fallback.
Both output words are captured before either store, including when output
aliases the tree. Returning validation does not become an exception/early
return. The supplied callback is the existing CRT boundary, not a lookup or
resource-provider callback.

The strict MSVC Win32 build passed `/fp:strict /W4 /WX`; both existing CTests
passed and all eight native seed spans matched the installed PE. The ignored
project hook adds only this cpp to the actual `bsp_core.lib`. No shared CMake,
saved Ghidra, ledgers or permanent tests changed.

One focused ignored original/current-library composition passed **232 matching
trace words**. All eight original bodies execute: lower-bound through original
find and both constructors through creators as well as direct calls. The
actual source lower-bound is inlined in linked find; its standalone COMDAT is
frozen in the object but is not separately linked/executed. The comparison
covers all constructor output bytes, atomic retain, actual owning-pool header
mutation/self-copy/allocation failure, current-head mutation after CRT compare,
and iterator output aliasing tree+4. Two actual `_stricmp` invalid-parameter
callbacks return and execution continues. Those are comparator CRT checks,
not execution of find's null-owner BF6713 site. Original CRT/resize/comparator
helpers are explicitly bridged to current providers; no original-helper-runtime
or native-SEH equivalence is claimed.

The fixture separately returns the buffer intentionally abandoned by native
self-copy and frees the created raw allocations after observing them. Those
cleanup actions are fixture ownership, not reconstructed resource destruction.
Creator null-allocation branches and creator unwind actions are static-only;
the source allocator's established host domain returns a block or throws.
The copy-failure injection happens at the actual pool's CRT allocation boundary.

Six exact objects match archive members byte-for-byte: this packet, actual
443D00, actual string headers, actual pool storage, actual pool owner and
singleton lifetime. Their complete frozen COFF code sections total **163
sections / 10,212 bytes / 324 relocations**. The linker map proves these are
the only six bsp_core objects linked into the probe and identifies its seven
linked owned exports plus 41DD40 and 443D00. Source, archive, objects, probe,
original trace and finite inputs are pinned; the immutable handoff manifest
is under ignored `local/resource_cache_leaves_handoff/`.

The cache discovery's blockers remain: B19E90 current factory dispatch/domain,
B1B810/B1B730 registry getter/lifetime, actual tree population, resource terminal
ownership and raw 109CF04 platform binding. None is replaced here. Native
BF681B/BF65AC/BF6713/BF7FBF/BF7680 and their internal CRT children remain host
service boundaries, not new full original library reconstructions.

Primary metadata handoff: B19E40 currently has no saved function. Define its
exact `[00B19E40,00B19E90)` extent (last RET0C at B19E8D), preserving adjacent
B19DF0/B19E90 bodies and old comments. The worker made no metadata edits.


## Primary integration

Main now registers all eight unchanged owned bodies. The combined strict Win32 build, two existing CTests and eight seeds passed. Primary verified77 sealed worker files and56 report pins,13 source/header files plus6 required declaration headers, and25 fresh spans1213 bytes. The unchanged fixture linked only the same frozen main library; all232 trace words match the worker exactly. Six exact archive objects now contain166 complete code sections10193 bytes326 relocations because the shared BE0A30 adapter changed separately. Those changed methods are not linked into this fixture; no runtime code-postimage or original-helper proof is added. Lower-bound still executes inlined in source find. Creator EH remains static-only. Primary defined the complete80-byte B19E40 and restored both11-byte returning-free EH bodies, preserving neighbors, old names and comments.

Immutable current proof: `local/resource_cache_leaves_primary/`. Primary saved reviewed names/comments, retained prior values, registered the raw source entries and refreshed affected exports.
