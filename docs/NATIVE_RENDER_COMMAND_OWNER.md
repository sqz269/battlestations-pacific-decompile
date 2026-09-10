# Native render command ownership

The reconstruction now operates on the actual 68-byte (`44h`) render command,
using the existing native context, batch pool, group, model, pointer-array and
string-pool implementations. The command itself has no reference count. Its
`+04` field owns a scene reference; its only verified table entry is execution
at `00B1D950`, which remains outside this packet.

| Native range, end exclusive | Reconstructed behavior | Original ABI |
| --- | --- | --- |
| `00B1F170..00B1F1F0` | Three-argument placement constructor; uses the camera as both context owners | ECX command; three pointer arguments; EAX original command; RET `0Ch` |
| `00B1F1F0..00B1F274` | Four-argument placement constructor | ECX command; four pointer arguments; EAX original command; RET `10h` |
| `00B1EDC0..00B1EF57` | Context allocation, retained assignments, two batch acquisitions and diagnostic initialization | ECX command; four pointer arguments; RET `10h` |
| `00B1DDD0..00B1DFED` | Complete populated command teardown without freeing the command | ECX command; RET |
| `00B1E6B0..00B1E6CE` | Teardown, ordinary free when flags bit 0 is set, original-address return | ECX command; flags argument; EAX original command; RET 4 |

Descriptive names are hypotheses. The C++ interfaces are new and are not binary
replacements for these calling conventions.

## Actual storage and dependencies

`NativeRenderCommandStorage` preserves field `+08` and leaves the two batch
slots untouched until each acquisition publishes its result. Constructors set
the native `00D5E5E0` table, clear the scene, diagnostic, three metadata words,
context and both array headers, then execute the full initializer.

The initializer allocates a real 24-byte context and publishes it at `+28`.
It publishes and retains each incoming reference before releasing the old one,
and reloads the command's current context before each later assignment. It then
sets the borrowed command backpointer, obtains the current batch pool for each
acquisition, and writes the native one-byte `X` diagnostic through the shared
string pool. Diagnostic allocation precedes reloading and returning old storage.

The required `NativeRenderCommandAssociations` only installs canonical host
companions. Their storage and capacity must be prepared before entry; association
does not allocate, retain, change native fields, or run ownership behavior.
Each companion borrows the actual owner's `+04` atomic. Context association must
survive command destruction when another reference still holds that context.

Batches use the separate throwing `NativeRenderBatchReferences` domain. The
native `00B55680` terminal can throw while obtaining or growing the pool after
destroying its entry array. Its companion validates current virtual slots 0 and
4, delegates that concrete path and retires the host association after either
successful return or an unreturned dead slot. It never substitutes the
nonthrowing `RenderCommandReference` terminal interface.

## Teardown and failure ordering

Teardown drops the two required nonnull batch references in order, preserving
their slots. It walks the actual indexed array, recomputing its current end
after every group. Each captured group runs the complete binding/model/array/name
destructor and ordinary free before the captured array cell is cleared. The
ordered array contains borrowed pointers and does not destroy its pointed objects.

Next, teardown releases and clears the scene and context, destroys the ordered
then indexed arrays, and returns the current diagnostic. Freed array headers
retain their stale data and capacity with count zero. The diagnostic header also
remains unchanged after its storage is returned.

Constructor and destructor unwind scopes follow the native ordered-array,
indexed-array and diagnostic cleanup states. They do not release unvisited
scene/context/batch/group owners as rollback. Scalar deletion frees the command
only after successful teardown. Native command EH tables were inspected; this
packet does not claim original command exception execution or native C++ EH ABI
compatibility.

## Evidence and verification

`reports/native_render_command_owner_audit.json` records complete extents,
installed-image hashes, source hashes and the existing independent discovery
review. The focused comparison maps checked original bytes from the installed
executable and uses the same concrete dependencies at its declared CRT, Win32,
singleton-manager and string-pool boundaries.

Both native constructors and populated teardown agree with the reconstruction
over 1,451 normalized words. The sequence includes two allocated source arrays
and a pooled name per group, a cleanup callback that changes the indexed end,
retained model/context/batch references surviving command destruction, final
model-pool and batch-pool return, batch reuse, scalar flags 2 and singleton
shutdown. The scalar free-bit branch is grounded in assembly; this command
fixture does not execute that branch. The repository's strict MSVC Win32 build
and existing tests are recorded with the integration report.

The remaining command path includes `00B1D950` execution, the actual queue and
its creators, gathering and preparation jobs, and unresolved scene/target
profiles. This packet establishes ownership behavior; it does not establish
a complete renderer or a runnable, gameplay-validated rebuild.
