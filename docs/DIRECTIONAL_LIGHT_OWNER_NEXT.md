# Directional-light owner dependencies

Read-only discovery from `79e5cd8`. No source, Ghidra, ledger or installed-game
files changed. The existing project was verified as `C:/Users/sqz269/bsp.gpr`,
program `/battlestationspacific.exe`, for every live query. The bounded function
and data spans, native ABIs, actual extents and live/disk hashes are recorded in
`reports/directional_light_owner_next.json`. Ghidra's saved-image data is not
running-game memory; zeroed static type/pool slots are pre-initialization bytes.

The directional owner is not a plain heap-allocated generic scene node. It uses
the static pool at `01090154`, retains multiple scene resources in its own
`+178/+17C/+180` array, and changes its effective native vtable during destruction.
The existing node lifetime implementation covers only a narrower post-virtual18
generated-model path. Its name in the ledger does not establish complete node
destruction coverage.

## Actual allocation and pool ownership

`00B7BD40` ignores incoming context, sets ECX to `01090154`, and jumps to
`00B7BAC0`. Its current `CG_static_dtor_stub` name is incorrect. Allocation
returns one actual raw slot; `00B7C820` returns it through `00B7B2F0` when flags
bit zero is set. It does not CRT-free the light object. `00B7B610` is a separate
ECX-slot adapter that pushes the slot and calls the same pool return routine.

`00CD8080` constructs this static pool with `00B7B940`, then registers
`00CE0EB0` with `atexit`. The latter invokes `00B7B230`; its current static-init
name is also misleading. Pool construction inserts the actual allocator element
into the shared `00E188B4` doubly linked list. Fields are native vtable `+0`,
previous `+4`, next `+8`, Win32 critical section `+C`, explicit recursion counter
`+24`, slab-pointer table `+28`, unsigned count `+2C`, capacity `+30`, and first
available slab index `+34`. The initial pointer table has 32 entries/128 bytes;
unused entries are not initialized. The first-free index begins at `FFFFFFFF`.
This domain is distinct from the singleton lifetime-manager registry.
It is also distinct from the string `SizedStoragePool`: its inspected native
`00BD1480/00BD0F50` constructors initialize their own rings/critical section and
do not register an allocator-list element. Share the existing string pool for
names and the shared raw allocation boundary for slabs; do not invent a string
pool entry in `00E188B4`.

Each actual slab allocation is `0x3E44` bytes: 32 slots of `0x1F0` bytes, 32
16-bit free indices at `+3E00`, a 16-bit free count at `+3E40`, and two untouched
tail bytes. `00B7AC90` writes free indices `31..0`, count 32, and each slot's
slab-table index at slot `+1EC`. It does not initialize the light's other bytes.
Allocation pops the index stack, initially producing slots `0,1,...,31`. Return
reads the slot's live `+1EC`, computes its index from the slab-relative address,
pushes that index and updates the minimum free-slab index. It has no double-free
or range-repair policy and does not free empty slabs immediately.

`00B7BAC0` enters the actual critical section and increments explicit recursion.
If no slab is available it publishes the new slab index before allocation. Table
growth uses `2*capacity+2` and publishes capacity before allocating. Normal return
decrements recursion and leaves the critical section. There is no local exception
handler to restore those published values or unlock after allocation failure;
adding an unconditional RAII unlock/rollback would change that native behavior.

Pool virtual zero is **`00B7BA20`**, a required part of the ownership contract.
It frees fully empty slabs, replaces each removed table entry with the last
entry, decrements count, and rewrites all 32 moved slots' `+1EC` IDs. It examines
the replacement at the same index and finally recomputes the first available
slab. It does not lock internally. `004B46B0` traverses the real allocator list,
calls each actual virtual zero and reloads next `+8` after the call. A separate
private list or no-op trim callback would not reproduce this contract.

Pool destruction frees every slab, then its pointer table, drains positive
explicit recursion by calling `LeaveCriticalSection`, deletes the critical
section, installs `00D7A0C0` and unlinks the same shared allocator-list element.
Several free continuations are absent from old pseudocode: notably `00B7B24D`,
`00B7B9F9`, `00B7BA46` and `00B7BB56`. The constructor's three unwind actions are
`00403970` (allocator-list unlink), `00402F70` (critical-section cleanup) and
`00B7ADB0` (free table storage without clearing its fields).

Use real raw slabs as canonical storage. Keep enlarged C++ companions and binding
maps outside the native slots; do not replace a reused slot with a fresh
zero-initialized host object. The actual slot bytes and `+1EC` identity must
survive construction and later pool compaction.

## Construction writes and untouched bytes

`00B6F5A0` initializes the node reference count to one, clears the parent/children/
sibling fields, attachment `+A0`, root registration `+A4`, `+A8`, retained `+130`,
point-light pointer/count/capacity `+164/+168/+16C`, and generic scene `+170`.
It copies the pooled name at `+54/+58`; matrices at `+B0`, `+F0`, then `+60` are
initialized to identity through the existing `004134F0` x87 matrix copy.
It initializes `+4C` and `+AC` to one, flags `+5C` to zero, auxiliary `+138` to
`40`, byte `+44` to zero, byte `+134` to one, and final mask `+48` to `FFFFF`.
Bounds words use exact `501502F9` and `D01502F9` (positive/negative `1e10`), not
infinity. The complete write map is in the report.

`00B7C4C0` calls that constructor, sets light vtable `00D62F58`, clears shadow
`+174` and the retained-scene array, sets effective diffuse `+184` and base
diffuse `+1A4` to `(1,1,1,1)`, base specular `+1C4` to `(0,0,0,1)`, scalar
`+1D4` to 64 and `+1D8` to one. `00B7C6B0` then writes only directional vtable
`00D62FB0`.

Across these constructors, the following slot ranges remain untouched:

| Native range | Meaning/boundary |
|---|---|
| `+08..2F` | Unrecovered base payload; no constructor stores |
| `+45..47`, `+135..137` | Padding after separately written bytes |
| `+194..1A3` | Effective specular |
| `+1B4..1C3` | Mode-3 diffuse |
| `+1DC..1DF` | Specular scale |
| `+1E0..1EB` | Direction |
| `+1EC..1EF` | Pool-written slab ID; constructors must preserve it |

The last row is initialized by the pool rather than being arbitrary padding.
Later configuration is a separate packet; do not supply zero/default light
values here to make a new owner render plausibly.

## Actual scene attachment and destruction phases

Both directional and light vtables use `00B7C020` at `+50` and `00B7BD60` at
`+54`. Attach searches the separate retained-scene array for identity. If absent,
it appends the scene, invokes `00B83D50` against the existing shared registry,
then retains the scene. A duplicate still recurses when requested. There is no
null-scene guard on the append/retain path. Detach erases the first match with
swap-last, invokes `00B83EC0`, releases that scene, then optionally recurses even
when the scene was absent. Child calls use each child's actual virtual `+50/+54`
and reload next sibling after the call. Neither override substitutes a generic
node `+170` assignment.

`00B7C5B0` installs the light vtable and drains retained scenes from the back.
It reloads array/count after the registry callback before selecting the scene
to release, then reloads count again before its conditional decrement. It
releases/clears shadow `+174`, resizes the array to zero, frees it and continues
at `00B7C686` into the **actual** `00B6F440` node destructor. True function end
is `00B7C6AA`, beyond the saved Ghidra end `00B7C685`.

Native vtable phase changes affect callbacks: directional predicate `00B7C6D0`
accepts four runtime tokens; light predicate `00B7C580` accepts three; node
predicate `00B6F570` accepts two. When `00B6F440` installs `00D62C88`, its `+50`
and `+54` become generic `00B6ED80/00B6EE10`. Base-destructor registration must
therefore dispatch the node phase, even while the host C++ allocation remains a
directional-owner companion. Type boot `00CD80A0/00CD80F0` is analyzed in the
current ledger but not implemented as production bootstrap. Actual runtime
tokens must be supplied or that bootstrap completed; token addresses and saved
zero words are not substitutes.

## Existing reuse and the remaining base closure

Reuse `NativeString` and the same existing `SizedStoragePool`, raw allocation/
free boundary, `CameraTransform`, `SceneNodeAttachment`, `SceneResource` and its
canonical registry. `GeneratedModelLifetimeRuntime` already supplies actual
child virtual18/54 and attachment-identity associations. Existing code also
covers root unlink, attachment backlink removal, point-light backlink removal,
generic scene removal and transform invalidation/notification.

`destroy_generated_model_after_release_00b750c0` is final-model-specific and
terminates unless byte44 is set and parent/root/children are cleared. It cannot
be used as a complete directional node destructor. The missing bounded direct
base closure is `00B6F440` plus the **new-parent-null branch** of `00B6E680`,
`00B6D940` (unlink child), `00B6D850` (recursive attachment clear), `00B6D890`
(root propagation) and `00B721F0` (prepend root). `00B6D890` can read the actual
outer scene-resource slot and invoke the current node's virtual50. The existing
`RenderNodeRootList` only stores its first node, so root integration must bind
that list to the same actual outer lighting slot. This dependency does not
require reconstructing the whole outer weak-handle constructor.

The arbitrary nonnull-new-parent branch (`00B6E010`, parent type/attachment
selection) can remain separate from direct destructor closure. Nonnull shadow
`+174` still needs its actual reference owner and virtual-zero policy. The
current `SystemShadowMapOwner` is a borrowed rendering view, not proof of shadow
ownership; never add a no-op destruction callback to make the light owner link.

## Three useful worker packets

The report defines disjoint files and function ownership for these candidates:

1. **Directional pool storage:** implement actual slabs, return, compaction,
   allocator-list registration and lifecycle, including the minimal unwind leaves.
   This can run independently of all node/light reconstruction.
2. **Native node construction:** reconstruct the complete `00B6F5A0` write map
   over canonical raw-slot storage, existing transform/scene bindings and pooled
   name. This is independently ready; do not label it full terminal ownership.
3. **Light retained-scene operations:** implement `00B7C020/00B7BD60` over the
   supplied actual light state, the same scene registry and actual child
   dispatch. This is independently ready after the primary fixes the common
   raw-slot/binding contract; it does not need a pool or a full owner factory.

The primary should freeze that shared contract before edits: one actual native
slot identity, one transform/scene binding, the actual retained-scene array,
runtime type-token references, and explicit current native vtable phase. No
worker owns a second representation of a pointer slot or list order. Node
terminal closure, production type bootstrap, concrete shadow ownership and the
final directional constructor/destructor factory remain named followups. A
packet graph edge or an available worker slot does not remove those dependencies.

No tests were added or run for this read-only packet. Hash agreement establishes
byte provenance, not native execution, ABI compatibility or visual parity.
