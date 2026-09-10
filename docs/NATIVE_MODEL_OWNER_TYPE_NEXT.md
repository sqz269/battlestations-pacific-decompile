# Native model owner and type implementation boundary

The complete owner/type packet is ready independently of generated geometry
construction. It can use one actual `188h` model-pool slot, the existing
`174h` node prefix, the shared type counter and node/root descriptors, and
actual retained owners at `+174` and `+180`. The nine core entries below close
construction, direct destruction including unwind, scalar deletion and live
type dispatch. `00B75170` is a complete adjacent setter worth including as
the tenth entry. `00B752B0` is a clone operation, not a type predicate.

This is read-only discovery. No implementation, Ghidra annotations, shared
metadata, build, native execution or gameplay checks were performed. The
accompanying [report](../reports/native_model_owner_type_next.json) records
28 exact spans, 2,006 bytes, old Ghidra names/comments, original ABIs and
dependency artifact hashes. All spans matched the installed PE and guarded
Ghidra reads of `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`.

## Ready entries

Ends in this table are exclusive. Names remain descriptive hypotheses.

| Entry | End | Bytes | Contract and original ABI |
|---|---|---:|---|
| `00B75030` | `00B750B4` | 132 | Model construct: ECX actual slot, stack `NativeString*`, EAX same slot, RET4 |
| `00B750C0` | `00B75165` | 165 | Full direct model destroy: ECX actual model, RET |
| `00B75290` | `00B752B0` | 32 | Scalar deleting destructor: ECX model, stack flags, EAX original address, RET4 |
| `00B74F20` | `00B74F4B` | 43 | Destroy geometry member: ECX model+178, release member+8, RET |
| `00B74330` | `00B74336` | 6 | Current object type ID: ECX ignored, EAX live DWORD `01090034`, RET |
| `00B74340` | `00B74346` | 6 | Current type-name address: ECX ignored, EAX live DWORD `01090040`, RET |
| `006EF860` | `006EF888` | 40 | Current object type predicate: stack token, ECX ignored, AL bool, RET4 |
| `00CD7E60` | `00CD7EAF` | 79 | Static object descriptor initializer, no arguments, RET |
| `00B74F90` | `00B74FD7` | 71 | Initialize ECX descriptor under the process guard, RET; no defined result |
| `00B75170` | `00B751EB` | 123 | Adjacent geometry setter: ECX model; stack unused word, raw geometry, float, float; RET10h |

`00B74330` and `00B74340` are currently undefined in Ghidra, although their
table references and six-byte bodies are unambiguous. Do not invent existing
function names when annotating them. Other old names and comments are
preserved in the report. The discovered complete ends need no flow repair.

## Actual storage and construction

The object owns `[0,184h)`; the DWORD at `+184` belongs to the model pool.
`NativeModelPool` in the independent pool packet supplies the canonical
`01090054` owner, `188h` slots and the physical return operation. The
`00B4C8F0..00B4C8FD` caller loads the nominal object size `184h` into ECX,
but `00B74EB0` replaces ECX with `01090054` and tail-jumps to the pool.
The size argument therefore does not create a separate heap allocation.

Keep a `NativeNodeStorage` prefix and a four-word tail in that same slot:

| Offset | Meaning |
|---|---|
| `000..173` | Existing `NativeNodeStorage`, including actual atomic `+04`, name, hierarchy, point-light array and scene |
| `174` | Raw retained owner identity; exact semantic role remains opaque |
| `178` | Float initially bit pattern `D01502F9` (`-1e10f`) |
| `17C` | Float initially bit pattern `501502F9` (`+1e10f`) |
| `180` | Raw retained geometry/mesh owner identity |
| `184` | Pool slab index; untouched by all model constructors/destructors/setters |

`00B75030` first passes the stack name pointer to `00B6F5A0`. On successful
base construction it loads `CE4ADC`, installs `D62DE8`, clears `+174`, stores
the captured `CE4ADC` word to `+178`, loads/stores `CE4970` to `+17C`, loads
`D7A208`, then clears `+180`. It writes the captured `80000000` negative zero
to `+18,+1C,+20`. After `XORPS`, it writes positive zero to
`+24,+28,+2C,+08,+0C,+10,+14`, in that order. Preserve the signed zero and
native load/store order; these are not unit scale or identity-pose defaults.
It adds no outer EH frame. A throwing node constructor performs its existing
cleanup; the model constructor does not return the physical slot itself.

The caller must preserve allocation preimages when establishing typed
lifetimes and binding the shared node views. An external owner companion can
follow `NativeCameraOwner` preparation/phase rules without creating another
count, transform, point-light array, scene pointer or name buffer. Store raw
owner identities in the model tail, not host interface addresses. The main
checkout now checks the node constructor's minimum against
`sizeof(NativeNodeStorage)==174h`; this prerequisite was inspected after the
primary changed it. The discovery branch still contains the earlier guard.

## Destruction and exact unwind

`00B750C0` installs `D62DE8` and sets EH state 1. It captures current `+174`;
if nonnull, decrements captured-owner `+04` atomically and calls that owner's
current virtual `+00` only on zero. It clears model `+174` only after that
release returns. Then it reloads current `+180`, sets state 0, performs the
same release-then-clear operation, sets state -1 and calls full `00B6F440`.
Neither model reference field is precleared. A callback may replace later
fields or rebuild parent/root/scene links; use the full native node destructor,
not the older typed terminal path's precleared-hierarchy restrictions.

The handler `00CC1C66` selects FuncInfo `00DFAC50`, maxState 2, unwind map
`00DFAC40`:

| Active state | Next | Action |
|---:|---:|---|
| 1 | 0 | `00CC1C58`: ECX = saved model+178, tail-call `00B74F20` |
| 0 | -1 | `00CC1C50`: ECX = saved model, tail-call full `00B6F440` |

`00B74F20` reloads its current `+8` field, which is model `+180`, and performs
the same captured raw-owner decrement/current-zero-dispatch/clear sequence.
It preserves the two scalar words. Therefore an exception during `+174`
release leaves that field uncleared, but cleans the current `+180` and then
the node base. An exception during normal `+180` release does not retry that
release; only the node base runs. An exception from the normal node-base call
does not invoke that base twice. Cleanup that itself throws during native C++
unwinding must not be silently swallowed or turned into successful deletion.

After successful direct destruction, `00B75290` returns the slot through
`00B74750` on canonical pool `01090054` iff `flags & 1`; it returns the original
address in EAX even when that address now designates a free slot. It does not
decrement model `+04`, inspect byte `+44`, or call logical release first.
If destruction throws, no pool-return call occurs. Companion associations and
typed tail lifetimes need the same explicit retirement order used by the
native camera owner, including exceptional direct-destruction cleanup.

## Required actual retained-owner and current virtual dispatch

The raw reference operation is already available through
`release_native_render_actual_owner`: decrement actual raw `+04` first,
resolve the canonical companion only on zero, validate that it borrows this
same atomic, then dispatch its current native terminal profile. Nonzero
release does not require a lookup. `NativeRenderActualOwners` is suitable for
the model tail. Its current terminal interfaces are nonthrowing; lookup errors
can still unwind the direct destructor. Supporting native throwing terminals
would require an explicit corresponding direct-call interface and scope.

Nonnull `+174` and `+180` need real live storage and the correct terminal path
whenever they can reach zero. A native geometry owner may be supplied through
that canonical binding without reconstructing geometry construction first.
The existing `GeneratedModelGeometryReference` owns a host shared geometry;
it does not have a native intrusive layout and cannot be published as raw
geometry. Likewise, do not publish the address of `RenderCommandReference`
instead of the actual owner. Missing current profiles fail explicitly.

The existing `NativeNodeDestructionRuntime` remains responsible for actual
`+130`, attachments, node/root/scene operations, name and array cleanup. Reuse
its bindings and `GeneratedModelLifetimeRuntime`. A native model reference
companion can borrow the actual model `+04`, provide shared logical
`00B6F310` over its existing node/light-array views, and retire after full
scalar deletion. Do not wrap the older copied-field `GeneratedModelLifetime`
as though it were the `184h` native object.

The current model vtable is `00D62DE8`, 23 entries through `+58`:

| Slot | Target | Required interpretation |
|---|---|---|
| `00` | `00BD30E0` | Current deleting-destructor trampoline with flag 1 |
| `04` | `00B75290` | Complete model scalar deleting destructor |
| `08` | `00B74330` | Live object type ID |
| `0C` | `006EF860` | Live object/node/root token predicate |
| `10` | `00B752B0` | Clone, separate dependency closure |
| `14` | `00B74340` | Live native type-name address |
| `18` | `00B6F310` | Shared logical node release |
| `20` | `00B748E0` | Render submission/culling, outside this owner packet |
| `40` | `00B6DBE0` | Shared node world-change notification |
| `48` | `00B6E8C0` | Shared bounds access |
| `50` | `00B6ED80` | Shared live scene assignment |
| `54` | `00B6EE10` | Shared live scene removal |
| `58` | `006EF890` | Native five-byte `AL=1; RET8` visibility leaf |

Check the actual owner's current vtable word and required current table slot
before dispatch. During base-node destruction the actual phase becomes
`D62C88`, whose `+0C` is `B6F570`, with the same `+40/+50/+54` entries.
Unknown derived or base profiles must not inherit a constructed-type fallback.
The real constant-true `+58` leaf is evidence, not permission to substitute
success for unimplemented rendering or other slots.

## Live object type bootstrap

Use a descriptor of four DWORDs bound to actual `01090034..01090043`, and
the process-wide byte guard `01090030`. The words are object, node and root
IDs, then native name address `00D62DD4` (`c3dObject`). Reuse the SAME
`TypeIdCounterLifetime`/`0109DB7C` and `LightTypeBootstrap` node/root storage.
No second counter or invented startup token values are needed.

Both initialization entries test only the guard byte. If zero, they set it
to 1 before publishing the name or invoking dependencies; they do not roll
it back on failure. `00CD7E60` targets the canonical descriptor, calls node
initializer `B6F110`, loads BOTH node ID `0108FF90` and root ID `0108FF94`,
then stores them to `01090038/3C`. `00B74F90` can target another descriptor
through ECX under the same global guard; it interleaves each source load and
destination store. Both obtain the shared counter, capture its current
`+04`, store captured+1 to that counter, and only then publish the captured
value as the object's own ID. Arithmetic wraps at 32 bits.

`00B74330`, `00B74340` and `006EF860` never initialize a type. The predicate
loads live words `01090034`, then `38`, then `3C` until a match, returning only
AL as a boolean. Its upper EAX bytes are incidental pointer-derived values.
Before initialization it still follows these reads, including any zero IDs;
do not insert an initialization gate or synthetic default rejection. A
properly initialized object rejects unrelated light IDs, but this is a result
of the shared descriptor/counter state, not a hardcoded class relationship.
For a native owner, read the descriptor directly. The current
`SceneAttachmentRuntime::object_type_tokens` array is a copied projection and
is not authoritative for this live predicate.

## Geometry setter and separate construction/clone work

`00B75170` captures old `+180`; unequal raw identities publish the incoming
pointer, retain incoming actual `+04`, then release captured old actual `+04`
and dispatch current virtual `+00` on zero. Equal identity skips both counts.
After callbacks it loads the two scalar arguments and the `D7A260` sentinel
(`BF800000`, -1.0). It writes each scalar unless equal to the sentinel; the
SSE comparison also stores unordered/NaN inputs. Both comparisons share the
captured sentinel. There is no rollback/EH frame: an exception during old
release leaves the published incoming geometry in place and scalar stores
unvisited. The first stack DWORD is not read by this complete function.

Full `00B752B0..00B753F1` is a separate clone closure. ECX is source; stack
inputs are flags and requested parent; EAX is new model, RET8. It calls the
canonical pool, constructs using source `+54` name, then calls `B6F150` with
source ECX and `(destination, flags, parent)`. That dependency copies actual
node state, light/backlink and hierarchy relationships and invokes current
child/scene/transform virtuals. It is not a plain prefix memcpy.

If current source `+180` is nonnull, the clone calls that geometry's current
virtual `+10` with `(flags, 0)`, passes the result to `B75170(0, geometry,
source+178, source+17C)`, then releases its temporary geometry reference.
It subsequently retained-assigns source `+174` into the destination and copies
all ten pose floats at `+08..+2C` through x87 loads/stores. This must not be
replaced by a shallow geometry assignment or a bitwise copy for all FP modes.

Clone EH handler `CC1C78` uses FuncInfo `DFAC7C`, map `DFAC74`: state 0 calls
`CC1C70`, which passes the saved raw slot to `B748C0` for pool return only if
construction unwinds. The state is -1 before `B6F150`; later failures have no
full model or temporary-geometry rollback in this function. The native
allocation-null branch still enters downstream clone work with null, so a
usable success contract requires successful model allocation and construction.

The generated geometry composition at `B4C8D0` additionally needs actual mesh,
section, material clone, stream/layout and renderer registrations. The existing
typed geometry fragment documents these boundaries. Neither full composition
nor `B752B0`, `B748E0` render submission, new pose/bounds algorithms or arbitrary
derived owner profiles is required to implement the nine-entry owner/type
packet and its complete geometry setter.

## Proportionate implementation verification

Use the existing MSVC Win32 build/tests, then one focused native trajectory
through real model-pool allocation, exact constructor bytes and type reads,
retained assignment, logical/queue release and physical return. Compare signed
zero, unwritten preimages, the actual `+04`, `+184` survival, current-field
reload after a terminal callback and type descriptor mutation. Direct-dtor
exception checks should observe state 1 geometry cleanup and state 0 base
cleanup without retrying a failing release; keep native throwing-terminal
support separate from the existing nonthrowing reference interfaces. This
discovery packet adds no tests and makes no execution claim.
