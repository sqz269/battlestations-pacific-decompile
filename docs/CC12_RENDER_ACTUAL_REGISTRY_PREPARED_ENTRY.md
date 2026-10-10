# CC12 canonical render registry prepared entry

This packet adds a move-only preparation capability to the existing
`NativeRenderActualOwnerRegistry`. A caller can allocate its unavailable map
entry before Native construction and later publish a valid companion into that
same entry without insertion or container growth. The implementation changes
only `include/bsp/native_render_context.hpp` and `src/native_render_context.cpp`.
It does not install a particle profile hook, construct a persistent particle
record, create another registry/domain, or activate the new API in the application.

The accepted design is worker `474fe23c40eeaa26a8f4536e089f38d42a31cf2c`
and Root `454db4227`. Their whole worker document/report are retained as finite
dependencies, without recursively traversing older proof directories. Worker
baseline is `f2bbf5f745dcc33842719739bded3d737d7f3263`, after merging Main
`c151842673f2fadd4819b86a1a282b60dff153c8`.

## Entry, lifetime and failure contract

`prepare_binding(identity)` first rejects null or misaligned pointer values,
then locks the registry and inserts `identity -> nullptr`. It never reads
Native storage or the `+04` atomic. Any existing key, whether live or prepared,
is rejected. Preparation may allocate or throw and belongs before Native
construction. The returned token holds this registry, the opaque identity and
the mapped-value address; it holds no iterator or Native ownership.

References and pointers to an unordered-map element survive rehash. The token
therefore denotes the exact same entry after unrelated insertions and bucket
growth. The map remains private: ordinary bind rejects duplicates, and ordinary
unbind cannot match a prepared null value. No API erases, extracts or replaces
another token's pending entry. The token and Native lifetime operations require
caller serialization; the registry's short metadata lock does not establish
Native storage lifetime or concurrent count ownership.

`publish(token, reference)` requires this exact registry before taking its lock.
Under the lock it verifies the key exists, the mapped-value address matches the
token, and the entry is still null. Only then does it form `raw + 04`, require
the companion's atomic reference to alias that location, and require a positive
count. It writes the existing mapped value and clears all three token pointers.
There is no emplace, indexing insertion, reserve, rehash, retain, release,
Native callback, Native construction or ownership transfer in this operation.

Invalid capabilities, stale/replaced entries, wrong count aliases and nonpositive
counts terminate as contract violations. `publish` itself is not `noexcept`:
mutex failure can throw. Invoking it from an already-`noexcept` profile hook does
not make failure recoverable. These conditions are not permission to swallow an
error, claim usable publication or return a partially published Native slot.
The valid serialized path has no C++ allocation edge in its retained Source
lookup implementation; this is not a universal heap or nonthrowing guarantee.

Cancellation checks the same exact still-null entry under the metadata lock,
erases only that entry, and clears the token. It neither reads nor destroys
Native storage. Default, moved-from and consumed tokens cancel inertly. Move
assignment first cancels its old pending entry; self-move is inert. Destruction
calls cancellation. Move/cancel/destruction are `noexcept`, so mutex or invariant
failure there is fatal. The registry must outlive all tokens; its existing
destructor still requires an empty registry. All live companions must unbind
and all pending tokens must cancel before destruction.

Existing ordinary bind is textually unchanged: alignment, `+04` alias and
positive-count checks still occur before its metadata lock and duplicate
insertion check. Existing unbind, find, resolve, size, empty and callbacks are
also textually unchanged. Prepared entries count toward size/nonempty but find
returns null and resolve reports unavailable. Adding the nested token and
methods adds no enclosing registry instance fields. This is a new reconstructed
C++ API, not a recovered Original symbol or a drop-in game ABI claim.

## Build and focused Source fixture

Both fresh `scripts/build.ps1` MSVC Win32 Release builds passed the three
existing checks: `reconstructed_math`, `native_math_differential`, and
`tool_tests`. Before: 2026-10-10 19:58:26.772930–19:58:46.562827 UTC.
After: 20:01:21.178289–20:06:36.916603 UTC. The after build recompiles 389
actual header-consuming translation units. Both complete logs retain the
pre-existing LNK4006 duplicate `spawn_request_id_matches` warning.

One ignored local Win32 scenario, `registry_capability_probe.exe`, was built
with `/MD /O2 /link /MANIFEST:EMBED` and executed once successfully. The PE
resource contains an `asInvoker` manifest. The initial compiler invocation
failed at shell quoting before compilation; its log is retained separately.
The corrected compiler invocation and single successful execution are pinned.

The scenario verifies unavailable preparation/resolve, unchanged raw bytes,
duplicate prepared and ordinary rejection, move construction, move assignment
cancelling an opaque address `4`, self-move, 4096 additional opaque prepared
identities, publication after bucket growth, ordinary unbind and cancellation.
The extra-token vector is reserved before the observed growth interval.
Registry growth produced 4102 observed C++ allocation calls, the largest
65571 bytes. During valid publication the overridden C++ allocation functions
reject allocations; observed and rejected call counts both remain zero.
The synthetic atomic count remains 7, no terminal callback runs, and final
registry size is zero. These counters observe C++ allocation functions rather
than every possible heap operation. This is a Source registry fixture, not an
Original Native particle constructor, model, application or game execution.

## Complete compiled comparison and optimizer deltas

Each phase retains all 1989 actual Core COFF members in the complete 1992-entry
archive and all 80 actual App objects, reconciled against compiler item logs,
the App project and on-disk object enumeration. All 56 whole compiler logs,
389 actual consumer objects, 1345 tracked dependencies and 361 external compiler
paths are recorded. External compiler paths alone do not retain SDK headers.
The separately captured four link libraries provide actual CRT members/import
records for the relocation checks below, not a whole historical SDK snapshot.

The code census has 174279 sections before and 174290 after: 174277 have exact
bytes and relocation records, two existing sections change, eleven are added,
and none are removed. All changed/new code belongs to
`native_render_context.obj`. Whole bytes change in 366 Core and 21 App objects
after header recompilation. Every App code section remains exact. Complete
headers, symbol/auxiliary records, payloads and relocations are retained for the
changed native-render object and all 21 changed App objects: 22 complete indices
per phase, 32235/32252 physical sections. Zero raw pointers are handled as empty
file payloads even where declared uninitialized storage is nonzero.

There are 31 complete selected code decodes before (2979 bytes, 998 instructions)
and 42 after (4000 bytes, 1362 instructions); 29 pre-existing selected decodes are
exact. Preparation is 265 bytes, publication 167, cancellation 198, move
construction 48, move assignment 63, and token destruction 5. Complete emitted
code and cleanup handlers are retained rather than selected instruction windows.

The two optimizer changes are explicit:

| Function | Before | After | Checked effect |
| --- | ---: | ---: | --- |
| `unbind` | 360 bytes | 242 bytes | Its previously inlined map erasure becomes a call to the existing exact 156-byte erase helper. Lock, exact companion check, erasure, unlock and fatal failure exits remain present. |
| `unbind_callback` | 29 bytes | 293 bytes | Its null-context check and call become the same operation inlined into the callback, with corresponding `noexcept` exception metadata. |

Before has a complete transitive closure of 16 positive code/data/EH/import
nodes and 18 relocation edges; after has 20 nodes and 31 edges. Every target has
a positive object or import definition. Every complete physical section in that
closure matches the actual linked EXE after applying exact symbolic relocations.
Actual CRT archive members cover delete and security-cookie/check paths. Import
library records match actual PE import descriptors and IATs; complete six-byte
linker thunks are checked where present. Imports terminate the proof at the
link-time boundary: no runtime DLL implementation or execution is claimed.

Publication's valid emitted path calls only mutex lock, the complete existing
99-byte fixed-key find wrapper, and mutex unlock. That wrapper calls only the
complete existing 99-byte lookup body, which has no relocation/call target.
The alias and positive-count reads follow the entry/capability checks, then a
single mapped-value store precedes token clearing. The separate move/cancel
and unchanged erase bodies are also retained and checked. Complete Core/App
code-relocation scans find zero direct callers of public prepare or publish;
the standalone probe is outside that application census.

The game EXE, map and `.text` are changed. `.text` grows from 3992064 to
3992576 bytes; no whole-image, `.text`, App-whole-object, startup or game identity
claim is made. The whole images/maps and full changes are retained in the proof.

## Source, Git and portable proof

Both phases retain 4226 tracked Source/build/test-input paths; exactly the two
owned C++ files change. The complete query corpus is 4209 C/C++/inline files.
The original frozen typed-view Source3352 epoch differs from this worker before
in nine paths and the later wide-context Source3352 epoch in seven; those exact
historical differences are retained and replayed. They are not asserted current.
After Root accepted the prior particle admitted constructor on Main `295b982af`,
its complete Source3354 project epoch was separately retained and found LF-equal
to this frozen worker before. Historical manifest build declarations are not
substitutes for this packet's fresh before/after artifacts.

Separate live Git checks verify all 4226 actual staged-after blobs and current
worker files, with retained commit/tree chains and three additional historical
report metadata blobs. The staged Source tree is
`f18b70bca704aebb609df56d5dd24c07c3f98e9b`; final commit verification preserves
its six Source/build/test root entries. Later Main is observed separately,
without treating another worker or Main's artifacts as this build cohort.

| Artifact | Bytes | SHA-256 |
| --- | ---: | --- |
| `evidence.zip` | 142474843 | `0f1fc0c475b418341a874640dcd34bf2c8893183ea9574461e891f7aa7336ba2` |
| `portable_replay.py` | 45728 | `17c3f757f40d7d272804aef516490d987ee271991bf5242f82929749cf6a4681` |

Both files are under `local/cc12_render_actual_registry_prepared_entry/`.
The ZIP contains 5287 members (5286 unique checked payload pins).
Run `python portable_replay.py evidence.zip` after physically copying those two files.


The standalone entrypoint imports Python/Capstone/decoders first, then permits
only reads of the supplied ZIP. It blocks outside-file access, writes,
subprocesses, network access, directory enumeration and dynamic-library loads.
It rechecks every unique payload, Git tree/blob identities, whole Source epochs,
compiler cohorts, all complete code/section indices, semantic checks, optimizer
relocation closures and the retained fixture receipt. It executes no retained
binary. Both in-place replay and replay from a physical copy pass with equal
results. Live Git/current files are checked separately; portable replay does
not claim a live checkout, compilation or a repeated fixture execution.

The next application binding still needs its admitted persistent record,
borrowed owners, profile hook and retirement/quiescence integration. This packet
only makes the accepted same-registry preparation/publication capability concrete.
No new Ghidra query/mutation, Native particle body or Original ABI proof is added.
