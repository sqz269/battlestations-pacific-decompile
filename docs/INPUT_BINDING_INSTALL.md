# Input action binding installation

`00a93750` installs one slot in an existing action, then calls the existing
`00a91e80` reconstruction to resolve every binding in that action. `00a92260`
checks the action's separate registration byte. These are value projections
over the same `InputActionRecord` and `InputActionBinding` used by runtime
polling; no parallel action store or synthetic device collection is introduced.
Names below are hypotheses, not recovered symbols.

| Address / original ABI | Reconstructed behavior |
| --- | --- |
| `00a92260..00a92284`, ECX=input manager, stack=unsigned action, EAX=0/1, RET4 | Check action against count at manager+08h, then test record+00h. The per-frame enabled byte is +01h and is independent. |
| `00a93750..00a937c1`, ECX=input manager, stack=action, slot, five binding dwords, float scale, RET20h | Grow through slot+1 if necessary, copy source fields, set response flag from class==2, store scale, rebind entire action. |
| `00a93500..00a93614`, ECX=binding vector, stack=signed count, RET4 | Reserve if necessary, construct missing records or discard trailing records, then assign count. |
| `00a93220..00a93341`, ECX=binding vector, stack=signed capacity, RET4 | Clamp capacity to at least one, skip if sufficient, allocate and copy all live bindings, release old modifier arrays and outer allocation, install new base/capacity. |
| `00a93100..00a93194`, ECX=destination binding, stack=source pointer, EAX=destination, RET4 | Copy scalar fields, deep-copy both modifier vectors through `00a92ee0`, copy scale with x87 FLD/FSTP. |

The record's native binding array is at +10h/count+14h/capacity+18h, with
34h-byte elements. The installer source fields map as follows:

| Source argument | Native destination | Projection |
| --- | --- | --- |
| device class | binding+04h | `device_class` |
| device index | binding+08h | unsigned `device_index` |
| cached pointer | binding+0Ch | `cached_device` |
| input code | binding+10h | unsigned `input_code` |
| flag dword | binding+14h | `force_unit_scale` models its low byte's nonzero test |
| scale | binding+30h | binary32 `scale`, no arithmetic |

`00a9379d..00a937aa` assigns byte+01h from `device_class == 2`. Installation
preserves both modifier arrays; it does not clear them. It writes no action
registration/enabled fields and does not poll a device or query its virtual
methods. Rebinding clears each `resolved` flag, retains a copied cached pointer
when the primary class is -1, and otherwise resolves through the actual supplied
backend device groups under the existing `00a91e80` contract.

The typed `InputBindingInstallSource` makes pointer translation explicit.
`KeyboardInputBinding::unknown_08` is the source native pointer word in this
call chain. Zero translates to null; a nonzero word requires a caller-provided
mapping to an actual host `InputDevice`. Truncating or reinterpret-casting that
word into an unrelated host address does not establish a valid object. This
matters even for class -1, because later polling can inspect retained pointers.

Construction at `00a9354a..00a93574` matches the existing binding defaults:
resolved/response false, class -1, index/pointer/code zero, flag low byte zero,
empty required/forbidden modifiers, scale 1. `00d7a24c` was read as bytes
`00 00 80 3f`. Reserve value-copies both modifier vectors. The copy helper
explicitly preserves `00a93179..00a93180` FLD/FSTP, including conversion of a
signaling NaN under the caller's x87 environment.

Saved Ghidra listings initially omitted five continuations after `_free`.
Read-only byte inspection established the missing destruction loops, new
base/capacity assignments and shrinking return. The integrator repaired those
124 bytes under the shared write lock and refreshed the exports. No global
`_free` no-return annotation was changed. Exact gap evidence is retained in
`reports/input_binding_install_flow_repair.json`.

The C++ functions use host vectors and a new ABI. Native vector capacity,
allocator identity, object layout, modifier+10h's unused dword, and the upper
three bytes of flag+14h are not represented. Their allocator/exception ABI is
not reproduced. Invalid action or negative slot/count throws before mutation;
the maximum signed slot throws `length_error` before slot+1 overflows. Native
installation performs unchecked action/slot addressing. The existing rebind
precondition still requires valid class indices for all visited primary and
modifier references, and stable device groups during the call.

Validation: MSVC Win32 Release build and both existing CTest tests pass after
seed verification. One ignored fixture compares relocated, disk-byte-verified
native installer, registration predicate, rebind, and resize growth against the
projection. It covers a sparse slot install, default slots, both modifier lists,
class-2 response, flag low-byte behavior, copied/retained cached pointers,
unsigned code values, signed-zero and NaN scales, whole-action rebinding, and
missing-device resolution. Native resize uses preallocated capacity in this
fixture; allocator growth and shrink are checked only on the host projection.
The same fixture checks invalid indices, reserve deep copy, shrink and x87
signaling-NaN conversion. This is bounded fixture evidence, not game validation
or binary replacement compatibility. The concrete `KeyboardRuntimeHost` and
native-word/device translator are integrated by the primary packet. The primary
integrator applies the reviewed names/comments and saves Ghidra in its batch.
