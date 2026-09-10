# Render command queue ownership

This packet reconstructs queue execution, command/context ownership and ordered
storage destruction. A required executor performs the actual command virtual
operation. It consumes the real `InstanceRenderQueue` objects owned by the two
batch references. This code does not replace command execution with a frame
counter or a no-op callback.

The interface uses host intrusive references and the existing
`InstanceGroupingState`/`OwnedInstanceGroup` objects. It is not a native object
layout or vtable overlay. Camera, scene and target references may be subclasses
of `RenderCommandReference` that own their actual typed objects. A reference's
required terminal operation must release that object. The command executor and
model lifetime operations are abstract requirements with no default behavior.

## Queue iteration and callback order

Native `00B1EBE0` takes ECX queue and returns with plain RET. Its complete range
is `[00B1EBE0,00B1EC9B)`, 187 bytes. The queue stores command pointer data/count/
capacity at `+14/+18/+1C`, mode control at `+20`, and an owned current-context
reference at `+30`.

For each indexed command, execution follows this order:

1. Read the command context at `+28` and the queue's current context. If their
   identities differ, publish the new current pointer first, retain it, then
   release the old pointer. The terminal old-reference callback can therefore
   observe the new pointer and its retained count.
2. Reread the command list and indexed command, then call its virtual slot `0`.
   A terminal callback during the previous release may have replaced the list
   or its current command. The implementation uses an index, not an iterator
   or a captured command pointer across that callback.
3. Reread the **current** context after execution, release it if nonnull, and
   clear the queue field after its terminal callback. A callback that writes a
   replacement into that field is followed by the same unconditional clear as
   native; it does not silently gain a permanent owned reference.
4. Reread control. Zero calls `00B1DDD0` and frees the currently indexed command;
   nonzero retains it. Increment the index and compare against the freshly read
   count. After iteration, reread control again: zero resizes the pointer array
   to zero, retaining its capacity.

The native queue does not clear a command slot immediately after freeing its
command. A mode switch during callbacks can leave stale pointers in a retained
list. The caller must maintain valid indexed slots and a valid list before the
next execution; this implementation does not manufacture ownership for an
otherwise invalid native callback sequence. Callback reentry must not recursively
destroy the active command or invalidate a slot that will still be read.

Execution is `noexcept`, matching the normal native callback path. An adapter
can record an actual draw error for its caller, but it still performs the
queue's required release/clear sequence. A missing command operation is not
treated as successful execution.

## Context and batch ownership

`RenderCommandReference` starts with count one and uses sequentially consistent
atomic increment/decrement. A decrement from one calls the required terminal
virtual operation. The generic assignment helper publishes before retaining the
incoming object and releasing the outgoing one; equal identity does nothing.

Initializer `00B1EDC0` allocates an 18-byte native context, stores it at command
`+28`, retains command scene `+4`, then context camera `+8`, scene `+0C`, and
target `+14`. Context `+10` is a borrowed command backpointer. It subsequently
acquires two batch references at command `+0C/+10` and initializes its diagnostic
string. The typed fragment reconstructs the context and retained assignments
while consuming two explicitly acquired, actual batch objects. Native batch
pool acquisition and the diagnostic initializer remain separate operations.

`RenderCommandBatch` owns a real `InstanceRenderQueue` and exposes the native
preparation-mode slot `+8`. Final release frees its host pointer storage and
does not delete the borrowed render entries. It does not claim native pooled
batch allocation identity.

The context table `00D5E5C4` routes deleting destructor slot `+4` to `00B1D570`,
which calls body `00B1D120`. The latter releases and then clears camera, scene
and target in `+8,+0C,+14` order. It rereads each later field after previous
callbacks; the backpointer is left alone. The typed terminal release executes
this body and then deletes the context. The backpointer cannot outlive its
command unless the owner separately guarantees that lifetime.

## Concrete command and group destruction

Native command destructor `00B1DDD0` spans `[00B1DDD0,00B1DFED)`, 541 bytes.
It installs its command table, releases required batch references `+0C` then
`+10` without null guards, and walks the owned group cells at `+2C/count+30`.
For each nonnull cell it destroys the group, frees it, and **then zeros the
original cell**. The native endpoint is recomputed from data/count after each
iteration. The typed owner uses the actual `by_binding` groups.

After groups, it releases and clears command scene `+4`, then context `+28`.
It shrinks/frees the borrowed ordered-group array `+38/count+3C/capacity+40`,
then the indexed group array `+2C/count+30/capacity+34`, and finally returns the
diagnostic storage at `+18` with `length(+14)+1` bytes. The two batch fields are
not cleared by the native destructor. This is a one-time destruction operation:
call it once before deleting a command outside the mode-zero queue path.

The former `CG_vector_deleting_dtor_00B1D760` classification is incorrect. It is
the group destructor, with ECX group and plain RET. It releases/clears binding
`+0`, releases generated model `+1C` then `+20` through `00B6DFA0`, clears each
model pointer, and destroys two 12-byte borrowed-pointer arrays in reverse
order. The indirectly called array destructor is missing candidate `00B1D1D0`:
it invokes `00B1C770(0)`, frees data, cleans the stack, and returns. Neither array
destruction deletes the borrowed source render entries. Group diagnostic storage
at `+8` is returned with `length(+4)+1` bytes afterward.

The existing groups use shared and unique C++ owners. The port explicitly
releases binding, calls each real model lifetime operation, clears the model
adapter/owners, releases its geometry owner, frees category pointer arrays in
reverse order, and returns the diagnostic block. It does not rely on the
compiler's reverse member order for these native effects. During group teardown,
callbacks must not resize/reorder `by_binding` or inspect a `shared_ptr` while
its terminal deleter is running. Exact native intrusive-field observability
inside the existing shared-pointer representation is not claimed.

## Model hierarchy and resolved release operation

`00B6DFA0` concretely unlinks the existing `CameraTransform`. For a parented node,
it clears parent `+30`, patches previous `+40` and next `+3C` sibling links,
updates parent first-child `+34` when necessary, decrements parent child count
`+38`, and clears parent again. A root node with registration `+A4` instead calls
`00B72220` and clears that registration. `00B72220` patches the same sibling
links and root first pointer `+0C`; it does not release references or clear the
node's fields. There is no duplicate hierarchy in this packet.

Only after that unlink does `00B6DFA0` tail-dispatch model virtual `+18`.
Generated model table `00D62DE8` resolves this slot to `00B6F310`. That body
removes the node from its linked point lights through `00B7C1A0`, clears that
pointer array with `00B6EC70`, detaches and recursively releases children, and
on first disposal clears root/hierarchy links and sets byte `+44`. It unregisters
attachment `+A0` through `00B8F4C0`, releases the model's reference and calls its
terminal virtual operation at zero.

Those per-model ownership services remain a required actual virtual operation
after the reconstructed unlink. A caller supplies a `RenderCommandModelLifetime`
for every live generated model; a no-op implementation would not satisfy the
interface's contract. The model operation and its owner must use the real scene,
geometry and child lifetimes. This packet does not invent a native model layout
or silently substitute scene detachment for all of `00B6F310`.
`GeneratedModelLifetime` now supplies this concrete operation with the same
scene, hierarchy, point-light list and retained geometry; see
`GENERATED_MODEL_LIFETIME.md` and `FRAME_BOUNDS_INTEGRATION.md`.

## Pointer and diagnostic storage

`00B1C6C0` clamps capacity requests to at least one, grows only when needed,
allocates exactly four bytes per requested pointer, copies existing pointers,
frees old storage, and publishes new data then capacity. `00B1CC80` grows through
that helper, initializes newly exposed pointers to null, decrements count while
shrinking, and sets the final count. The typed owner preserves requested capacity
and count behavior using `new[]`; negative counts and byte overflow are rejected.
Native corruption and unchecked allocation-failure behavior are excluded.

Diagnostic strings use an actual supplied `std::pmr::memory_resource`. Their
storage returns to that same allocator with exactly `length+1` bytes and alignment
one. The allocator must outlive the bytes. This preserves concrete allocation
ownership and return order while keeping the native singleton pool layout and
size-class/free-list internals outside the new C++ interface.

## Evidence and checks

`reports/render_command_queue_audit.json` contains thirteen full code ranges,
PE/Ghidra byte identities, original prototypes/comments, proposed names/comments,
and decoded continuations after every false no-return free. The queue's missing
`00B1EC78` continuation is `ADD ESP,4`. The growth tail publishes the new pointer
and capacity. The command destructor continues past all five frees; the first
continuation at `00B1DE45` also clears the freed group's original cell. Full raw
decoding reaches the actual returns; truncated pseudocode is not compiled.

Live queries verify project `bsp`, `/battlestationspacific.exe`, x86 language and
image base through the repository CLI. Only the recorded ranges establish byte
identity. The primary integrator applies missing-function and flow repairs,
shared name/reconstruction ledgers, annotations and refreshed exports.

Strict MSVC Win32 `/O2 /MD /W4 /WX /fp:strict` compilation and a focused local
lifetime check passed. The check covered publication before old-reference
release, callback-driven list replacement/growth, current-context replacement,
clear after its terminal callback, mode rereads, the exact eight-event disposal
order, retained pointer capacity, exact pooled byte counts, and hierarchy unlink
before model virtual release. It is a typed lifetime check, not execution of
the full original command/renderer binary. Existing repository build/CTest
results are recorded in the audit; CMake integration is owned by the primary.
There is no gameplay, visual or binary ABI validation claim here.
