# Actual resource-record vector growth and append

The new source implements complete4DA180 (221 bytes) andB1A3C0 (111 bytes) using
the already complete actual record copy/destruction and owning-pool providers.
It preserves native allocation, current-header reads, and failure ownership.
The cacheB1A4F0 and resource virtual ownership policy remain outside this packet.

The header is **12 bytes/0Ch**, not12h: pointer+0, DWORD count+4, DWORD
capacity+8. Records occupy2Ch bytes. The C++ storage struct borrows that actual
header; no array or element projection is created. Counts retain DWORD bits and
native comparisons use their signed interpretation. Pointer offsets, allocation
products, doubling, and increments wrap as native DWORD arithmetic. Caller
storage must support the actual reads/writes; no extra overflow, bounds,
allocation-null or ownership guard is added.

| Original span | Original ABI | New source symbol |
| --- | --- | --- |
| 004DA180..004DA25D | ECX header; stack requested capacity; RET4; no specified result | `reserve_native_resource_record_vector_004da180` |
| 00B1A3C0..00B1A42F | ECX header; stack source record pointer; RET4; no specified result | `append_native_resource_record_00b1a3c0` |

The source APIs accept the actual header, `ActualNativeStringPoolStorage&`, and
the existing returning validation boundary. Reserve also takes an int32
request; append takes a source pointer that is dereferenced only if its computed
destination is nonzero. Existing typed particle/render vector APIs are unchanged.
The API and EH machinery are new C++ interfaces, not original ABI replacements.

Reserve clamps a signed request below64 to64, then returns if current signed
capacity is already at least that value. Otherwise BF55BE calls the existing
BF681B CRT allocation boundary with lowDWORD(request*44). It captures the new
base and loops from index0 while signed index is below the current signed
count. Each iteration computes a wrapped destination address. A zero computed
address skips element construction. Otherwise it rereads the current old base,
uses the same index offset, and invokes complete4D6F70. Placement construction
starts the actual raw record lifetime without initializing its fields.

After copying, a second loop destroys old elements in ascending order through
complete4D45A0. Both the old base and count are read again around each destruction.
The native register holding requested capacity is reused as the byte offset
during a nonempty destruction loop, then reloaded from the current argument
word at004DA234. The source retains that distinction. It next frees the
**current** old base through BF6989 -> BF65AC. Only after free returns does it
publish the captured new base and capacity, in that order. It leaves count as
it stands after callbacks.

Reserve has no rollback for the new array or completed copied prefix. Its
registrationC66697 selects FuncInfoD8F59C and UnwindMapD8F594. The one state0
cleanup atC66680 is23 bytes: it computes two arguments from captured new-base,
index and destination locals, calls the one-byte RET-only401130 placement-delete
leaf, then returns. There is no array free or completed-element destruction.
If copying fails, the current4D6F70 and alias chain perform only their own
established cleanup. The already copied prefix and new allocation remain
orphaned. No RAII array owner or synthetic cleanup is introduced in the source.
There is likewise no additional cleanup during a later old-element destruction
failure. Native SEH and invalid-memory behavior remain outside the host interface.

Append snapshots current capacity and grows only if current count equals it;
it does not use count>=capacity. Doubling wraps as a DWORD. The result is clamped
to64 when its signed value is<=64, then passed to reserve. That reserve call
precedes append's armed cleanup region. After reserve returns, append reads
current count then current data and computes the wrapped destination. It arms
state0, copies through4D6F70 only if that destination is nonzero, and increments
the current count after copying. The source record pointer is not snapshotted
into a temporary record before growth; aliases invalidated by growth receive
no new protection.

Append's CBC65C registration selects FuncInfoDF491C/UnwindMapDF4914. Its28-byte
CBC640 action rereads the current count and then current data before calling
RET-only401130. Those current-header reads occur even though placement deletion
does nothing. The source preserves the volatile reads in the armed catch and
rethrows without cleanup. An exception from reserve occurs before this catch
is armed. C66680/CBC640 are owned evidence dependencies, not additional full
reconstructed entry claims.

The concrete dependency chain is reserve/append -> current4D6F70/4D45A0 ->
actual4D48A0/4D0A10 alias operations -> complete insertion, node/string copy,
count/iterator/rollback providers -> existing raw41DD40/41DD20 and actual owning
pool. `ActualNativeStringPoolStorage` uses the application's current01090AA8
publication,01090AA4 gate, canonical lifetime domain and concrete native pool.
No semantic allocator callback, extra pool, resource retain/release, or generic
element provider is added. Canonical `singleton_lifetime_allocate/free` supply
the existing CRT boundaries. Full current source/object pins and COFF references
verify the selected actual-pool composition, including the predecessor record
packet and inlined helpers. Unused legacy overloads remain in frozen objects.

One ignored focused fixture links the unmodified current archive and executes
originalB1A3C0 -> original4DA180 with concrete rebuilt record/pool dependencies.
The native full bodies, FH3 records and actions are byte-verified and relocated;
original record entries are bridged to the exact current library providers.
During empty growth, a real free(nullptr) observer changes count to3. Both
original and rebuilt reserve preserve that current count after publishing the
new64-capacity base; append recomputes index3 and finishes with count4. The
copied record preserves its uninitialized+8 word and copies name/list/payload/
raw-resource fields through the established record provider.

A second-element alias-copy failure is triggered at a real allocation boundary.
The complete alias/record cleanup runs, but both vector implementations leave
the old2/2 header unchanged and the new array plus completed record0 live. The
failed record1 has its own list/name cleanup and no copied payload/resource
tail. The fixture verifies that orphan state before separately reclaiming it;
cleanup performed by the fixture is not attributed to the native routine.
A direct zero-destination check uses source pointer1 and confirms no source
read before current count increments. Original and rebuilt traces match in172
words. The standard strict Win32 build, both existing CTests and all eight
native seed byte checks pass. No permanent tests are added.

Both original vector entries execute. The injected growth failure selects the
native reserve FH3 cleanup; its RET-only action has no independent execution
counter in this fixture. The append-copy CBC640 action is static proof only.
The old-element destruction loop is statically reviewed and compiled but is
not reached by a successful nonempty reserve in this focused check. Not every
installed dependency or frozen object section is dynamically exercised. Native
machine code for every record/string/pool dependency, every overflow case,
lazy singleton startup, native exception ABI/SEH and game behavior are not
claimed. Existing string-release `noexcept` and zero-byte memcpy boundaries
remain as documented in the predecessor packet.

The audit pins14 fresh guarded spans/789 bytes, including332 owned main bytes
and51 bytes in the two evidence-only EH actions.

## Primary integration

Main now registers the unchanged source and both raw functions. The main strict
Win32 build, both existing CTests and eight fresh seeds passed. Primary verified
69 worker pins, 31 current files and all fourteen fresh spans, 789 bytes.
Fourteen exact main archive objects retain all reviewed code/data/relocations.
The unchanged fixture linked only that frozen library and reproduced all172
trace words, including post-free current-count mutation and native orphan
preservation. The successful nonempty old-destructor loop and CBC640 exception
remain static proof. Immutable bundle: `local/resource_record_vector_primary/`.

Only the returning-free callsite4DA23B flow was repaired; the saved full extent
was unchanged. The hidden4DA240..249 stores are decoded again. Original names
and comments are retained in the journal, four annotations are saved and
exports refreshed. No global free annotation was changed. Cache/resource
ownership and gameplay validation remain separate.
