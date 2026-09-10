# Ambient and scene-resource ownership

This packet reconstructs the concrete ambient owner, concrete `SceneResource`
ownership and assignment into an already supplied outer owner's scene-resource
slot. `ConcreteSystemSceneResource` extends the same `SceneResource` used by
`SceneAttachmentRuntime`. Its lighting view reads that object's actual ambient
slot and the one canonical `SceneNodeRegistry`; it has no separately ordered
light list or mirrored environment/outer pointer slot.

The C++ interfaces are new interfaces. The ambient fields retain the native
Win32 layout through `+98`, with a host view appended. The scene resource has a
different host layout and explicit native vtable marker. Neither class is a
drop-in native binary replacement. Descriptive names remain reconstruction
hypotheses. Address extents, original ABIs and installed-byte hashes are in
`reports/system_lighting_owners_audit.json`.

## Recovered behavior

`00B7C290` initializes native ambient reference count to one, an empty borrowed
scene array, scalar `+14` to `1.0`, ambient `+18` to `(0,0,0,1)`, and each of six
cube faces to `(0,0,0,1)`. It never writes the mode-3 ambient at `+28..37`.
`construct_system_ambient_00b7c290` therefore reads those actual allocation bytes
before placement construction. It copies their raw words into the owner without
materializing floating-point values, preserving signaling-NaN representations
and signed zero. The factory uses the existing `singleton_lifetime_allocate`
boundary with native size `0x98` and the real larger host size.

The ambient's pointer/count/capacity array owns only its storage. Reserve clamps
the signed minimum to one, allocates exactly the native `capacity*4` byte count,
copies existing pointers, frees the old allocation and publishes the new pointer
and capacity. Append doubles capacity when full, with the native signed clamp.
Erase finds the first matching scene pointer and replaces it with the last entry.
Resize zeroes newly exposed entries but never retains/releases scene objects.

`00B83C50` copies the native pooled name before creating the registry. It then
allocates and constructs its ambient, assigns it through `00B825D0`, and releases
the creator's reference. A completed resource and its ambient each have one
reference; the ambient has one borrowed backlink to that resource. Name storage
uses the caller-supplied existing `SizedStoragePool`, not another pool instance.

`00B825D0` removes the resource's old borrowed backlink before comparing ambient
pointers. A changed pointer is published, then retained, then the old ambient is
released. The current slot is loaded again for append. A same-pointer assignment
still removes and appends, so it can reorder backlinks without changing any
reference count. The ambient's zero-reference path concretely invokes its
scalar destructor with flag one, matching `00BD30E0` dispatch.

`00B723F0` captures the old actual `SceneResource*` slot, compares, publishes the
new pointer, retains new and releases old. On zero it calls the old resource's
existing `destroy_on_zero` policy. There is no second slot update after that
callback. `SystemSceneResourceSlot` borrows this actual slot; its accessor resolves
the same embedded concrete lighting view. A supplied resource without this
concrete owner binding raises an explicit host binding error.

## Registry and prefix integration

The primary integration extends `SceneNodeRegistry` with deferred construction,
raw node access and explicit bucket/list destruction. `00B82390` allocates the
actual 12-byte sentinel and writes only its next and previous pointers. Its
payload at `+8` remains the allocation preimage. `00B83600` initializes count zero,
mask one, active bucket count one, and nine actual 8-byte iterator records. Each
record contains the stable list-owner identity and sentinel pointer; they are
not begin/end pairs. The registry owns these records and list nodes while
borrowing light keys. Host binding associations do not contain ordering links.

`SystemSceneResourceLightList` directly reads the registry's sentinel, next and
key. A required `SystemDirectionalLightResolver` associates each actual key with
its live directional-light view. `SceneAttachmentRuntime` supplies that resolver
from its existing bound nodes and explicit `system_directional_light`
associations. An unbound nonzero key is a binding error, including an untouched
sentinel payload. A zero key is an actual null key, not a substitute for an
unresolved pointer.

The prefix captures sentinel and first-node identity once. When first equals
sentinel, the supplied actual invalid-parameter runtime may return. The prefix
then loads the camera mode, environment and retained sentinel's key at their
native read points. Appending a new registry member inside that handler does not
replace the captured first node. The resolver can resolve a sentinel key for a
bound node that is not a member of this registry.

## Destruction and exception evidence

`00B7C450` resizes the borrowed scene array to zero, frees its storage, installs
the intermediate `00D5C104` vtable and executes `00BD30F0`'s `00CEB130` store.
Its pointer and capacity remain unchanged after free. `00B7C7E0` runs this body
and frees the owner only when flags bit zero is set, returning the original
address in either case.

`00B82ED0` releases and clears its ambient, frees and zeroes bucket backing,
then runs `00B829D0` to reset/drain nodes, free the sentinel and clear its slot.
It returns the name buffer to the existing sized pool and executes the root
base destructor. Its inlined name release does not clear the name's stored
length or pointer. `00B83410` controls final owner free with flags bit zero.
Native body destruction and C++ allocation teardown are kept explicit.

The scene destructor does **not** call the ambient backlink-removal helper.
An ambient retained elsewhere therefore keeps its borrowed pointer to the
destroyed scene. This reconstruction preserves that behavior and does not
silently repair the array. It also never releases borrowed light keys while
destroying registry nodes.

Raw assembly was required because several `_free` calls have misleading
no-return annotations. The continuations at `00B7B3E1`, `00B7C48A` and
`00B82A0C` publish/clear fields or restore vtables after free returns.

The live exception metadata and raw handler bytes establish these cleanup
edges: registry handler `00CC237B` uses unwind action `00CC2370` to invoke
`00B82B40` (a jump to `00B829D0`) on the already constructed list. Iterator-vector
handler `00CC2350` catches at `00B832B6`, calls `00B82C60` to free/zero storage and
rethrows. Scene handler `00CC23B9` has actions for root base, name, registry and
the newly allocated ambient block. The ambient constructor itself cannot throw
through a called operation. Once the environment setter begins, its raw ambient
slot has no member unwind destructor. A backlink-allocation exception is not
converted into a different retain/release transaction by this host constructor.

## Validation and remaining boundary

Strict MSVC Win32 compilation (`/W4 /WX /fp:strict`) and one focused local
fixture passed against the shared accessor/registry integration. That fixture
checks raw ambient preimages, pooled-name copying, constructor counts,
same-pointer backlink reordering, actual outer-slot retain/release, canonical
registry identity, the returning-handler sentinel continuation and flag-zero
destructor state. Final integrated build status is recorded in the audit.

The outer weak-handle constructor/domain, native directional-light pool and full
node lifecycle, actual directional virtual attachment, shadow owners, lighting
configuration apply and complete world construction remain separate packets.
Caller-supplied directional and shadow projections still need those real owners.
These checks establish reconstruction and focused host behavior, not native
ABI compatibility, in-game execution or visual parity. Invalid/corrupt native
array counts are not upgraded into a new recovery policy.
