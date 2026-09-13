# Native renderer listener update

The packet reconstructs both complete normal bodies through borrowed existing
owner fields and the canonical node/root registration implementation.

| Address | Coverage | Native ABI |
| --- | --- | --- |
| `00B0D7B0..00B0D7C8` | Complete, 25 bytes | ECX=renderer, stack handle, RET4 |
| `00B4EC90..00B4ECBA` | Complete, 43 bytes | ECX=invalidation owner, no arguments, RET |

`00B0D7B0` writes the exact handle into renderer `+1C0h`, including null, then
reads current `+30h`. A nonnull result becomes the receiver of `00B4EC90`.
Existing `controlled_unit.hpp` already identifies this listener slot; both null
and nonnull caller arguments are preserved. Renderer cleanup also releases and
clears this same `+30h` owner at `00B0FAB8..00B0FAD1`.

`00B4EC90` first captures its `+3Ch` owner in EAX, then sets `+250h` to one.
The first gate reads that captured owner's `+0Ch`. Each iteration rereads the
current `+3Ch/+0Ch`, passes that node and a null requested root to `00B6D890`,
then rereads current `+3Ch/+0Ch` again for the continuation test. ESI remains the
original receiver throughout and is restored. There is no caller-side head
advance, erase, reference release or iteration limit.

The `+3Ch` producer is concrete: `00B4FD19` calls existing scene constructor
`00B724E0`; `00B4FD2A` publishes its EAX result, or the allocation-failure zero,
into the original ESI owner's field. Existing `NativeGuiSceneStorage` and
`RenderNodeRootList` identify that outer scene's actual `+0Ch` root head and
`+1Ch` lighting slot. The bindings resolve these existing owners without a new
renderer, scene, root list or semantic-to-native cast.

| Call site | Target | Contract |
| --- | --- | --- |
| `00B0D7C1` | `00B4EC90` | Same captured nonnull `+30h` receiver |
| `00B4ECAB` | `00B6D890` | ECX=current head, PUSH0; callee RET4 |

The complete `00B6D890` body and existing source were read. The default binding
delegates directly to `propagate_native_node_root_00b6d890`; it unlinks the
actual root when required, publishes the requested root and recursively updates
children. Required owner resolvers are side-effect-free. Any propagation override
must preserve the complete contract. Missing owner bindings have no fallback.

Three full original-byte/source cases passed: absent `+30h`, empty roots, and
three canonical removals across two root owners with child propagation. A test
wrapper runs the actual canonical operation before changing the published owner,
proving rereads through the `0,2,1` order. It also rebinds the listener to verify
there is no later overwrite. Surrounding renderer, owner, scene and node bytes
are checked. Native and source renderer layouts differ; the native bridge uses
the established host root-head representation, not a binary node ABI claim.

Type-query and scene-attachment fixture contracts throw if reached; the null-root
path does not use them. The original callee bytes are retained but that callee
is executed through existing source. All 264 evidence bytes match live Ghidra
and disk. Build, both existing CTests, eight seeds and call checks are recorded
with exact artifacts and hashes. Native exception, concurrency, renderer runtime
integration and gameplay equivalence remain unproved. Ghidra analysis was read-only.
