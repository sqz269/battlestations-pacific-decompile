# Concrete profiles for actual renderer bindings

The immediately useful implementation packets are eight borrowed getters,
actual physical-buffer lifetime, and actual CPU declaration lifetime. The two
surface binders can already use `NativeSurfaceOwnerStorage` and actual renderer
synchronization; their addresses were transferred to the primary integrator
during this packet. Texture, logical stream and hardware-layout binding are
not closed by their existing semantic implementations or by the getter packet.
Their old-owner zero-count paths need the concrete lifetimes below.

This is a read-only dependency packet based on main commit `f16af17`. It changes
only this document and its JSON report. Current Ghidra project/program checks
preceded every live query. The report pins 76 complete code spans and 14 exact
table spans against the installed executable, keeps prior names/comments, and
records source hashes. No C++, shared metadata, Ghidra state, tests or game
installation were changed. Function names are descriptive hypotheses.

## Actual profiles and pointer domains

All listed engine owners use actual intrusive count DWORD +04. Their zero-count
slot +00 points to `00BD30E0`: ECX actual owner, push flags 1, then call the
owner's *current* vtable +04. That deleting slot, rather than a matching getter,
determines destruction and allocation ownership. `00BD30F0` only installs base
table `00CEB130`; it does not itself free storage.

| Owner profile | Complete borrowed getter path | Zero-count path and storage |
| --- | --- | --- |
| Texture 2D `00D61948` | +1C -> `00B3CEA0`: actual +10 is `IDirect3DTexture9*`, compatible with the device's base-texture parameter | +04 -> `00B3F590` -> complete `00B3F2E0`, then raw slot return `00B3D8D0` using pool `0108DB38`; object 50h, pool slot 54h, slab index +50 |
| Logical vertex stream `00D61D6C` | +24 -> `00B48CE0` returns actual declaration +68; +28 -> `00B48D10` returns raw offset +5C; +2C -> `00B48CF0` reloads physical owner +58 and tailcalls its current +1C | +04 -> `00B4BF10` -> `00B4B5D0`, then `00B49570` using pool `0108FE18`; object 74h, slot 78h, slab index +74 |
| Logical index stream `00D61DE0` | +28 -> `00B48DC0` reloads physical owner +08 and tailcalls its current +1C | +04 -> `00B4C1F0` -> `00B4B6F0`, then `00B495E0` using pool `0108FE50`; object 24h, slot 28h, slab index +24 |
| Private physical index `00D61E10` | +1C -> `00B4B840`: actual +28 is `IDirect3DIndexBuffer9*` | +04 -> `00B4BB20` -> `00B4B900` -> `00B4B490`; flags 1 then scalar operator delete `00BF65AC`; object 2Ch |
| Private physical vertex `00D61E34` | +1C -> `00B4B9F0`: actual +28 is `IDirect3DVertexBuffer9*` | +04 -> `00B4BB40` -> `00B4BAB0` -> `00B4B530`; flags 1 then scalar operator delete `00BF65AC`; object 2Ch |
| Pooled physical index `00D61E58` | The same `00B4B840` and actual +28 | +04 -> `00B4C210` -> the same index destructor, then `00B49500` with pool `0108FDA8`; object 2Ch, slot 30h, slab index +2C |
| Pooled physical vertex `00D61E7C` | The same `00B4B9F0` and actual +28 | +04 -> `00B4C230` -> the same vertex destructor, then `00B49500` with a different pool, `0108FDE0`; object 2Ch, slot 30h, slab index +2C |
| CPU vertex declaration `00D61D1C` | Stream getter returns this engine object, not a COM declaration; binding reads its raw stride +CC | +04 -> `00B48CA0` -> complete `00B48B70`, then `00B47950` with pool `0108FD38`; object D0h, slot D4h, slab index +D0 |
| Hardware vertex layout `00D62AF4` | +08 -> `00B5FF00`: actual +40 is borrowed `IDirect3DVertexDeclaration9*`; no creation | +04 -> `00B60770` -> `00B60700` -> base `00B48960`, then `00B60110` with pool `0108FE9C`; object 44h, slot 48h, slab index +44 |
| Surface `00D619A0` | Color/depth binders directly read actual +2C `IDirect3DSurface9*`; no virtual getter | +04 -> already reconstructed `00B3F5B0`/`00B3F4E0`; actual 34h owner and existing surface pool. The two binders do not take or release this intrusive reference. |

The paired physical-buffer getters do **not** identify an allocator: the
private and pooled tables share identical getter targets and destructor bodies
but have different deleting slots. Constructors `00B4BB60` and `00B4BBB0`
install the pooled tables. The private branches inside logical constructors
`00B4BF30` and `00B4BC00` install the heap tables. Do not return a private wrapper
to a shared pool or scalar-delete a pooled slot.

## Transfer: complete color and depth binders

Fresh full spans are `00B23D80..00B23E4E` (207 bytes, ECX renderer,
stack slot/wrapper, RET8) and `00B21690..00B21731` (162 bytes, ECX renderer,
stack wrapper, RET4). Both use the actual 8-byte guard contract: conditional
renderer store before entry, returned AL store after entry, EH active state
after successful entry, and current-mode cleanup. The skipped record remains
uninitialized. Each API call and counter update lies inside the guarded EH
state. Neither native entry promises an HRESULT return value.

- Color, nonnull wrapper: read wrapper+2C **before** current renderer+1A10 and
  its device table; call COM +94 (`SetRenderTarget`); increment raw +1BA0 even
  on failed HRESULT or null wrapper COM field.
- Color, null wrapper and slot zero: reload renderer+197C, dereference its +2C
  without a null fallback, then load the device and call +94 with slot zero.
  Other null-wrapper slots pass null. Neither null-wrapper branch increments.
- Depth, nonnull wrapper: load current renderer+1A10 **before** wrapper+2C,
  then the device table; call COM +9C (`SetDepthStencilSurface`); increment raw
  +1BCC even on failed HRESULT or null COM field. Null wrapper calls the API
  with null and does not increment.

There is no wrapper identity cache, retain/release, device substitution,
ownership callback or hidden lazy surface creation in these two complete
bodies. `local/actual_surface_binding_profile_verified.json` supplied the
369-byte proof and surface table to the primary integrator before transfer.
The discovery lease now excludes `00B23D80`; `00B21690` was never claimed here.
The primary subsequently reported that implementation and verification were
in progress; this report does not count that separate work as completed here.

## Ready bounded packets

The address sets below are disjoint. Each worker should recheck current leases
and source state before implementation. They are readiness proposals, not new
ledger entries or assertions that code already exists.

1. **Borrowed binding getters, 8 complete leaves / 44 bytes.** Own
   `00B3CEA0`, `00B48CE0`, `00B48D10`, `00B48CF0`, `00B48DC0`,
   `00B4B9F0`, `00B4B840`, `00B5FF00`. Suggested files:
   `include/bsp/native_renderer_binding_getters.hpp`, matching source, document
   and audit. Use actual borrowed storage and the concrete profiles above.
   Preserve the two logical-to-physical current-pointer/current-table
   tailcalls; dispatch their concrete getter implementations, not callbacks or
   copied semantic COM values. No reference updates, lazy creation or owner
   destruction belongs in this packet. Pure field leaves have no outgoing
   call; the original virtual tailcalls do not justify blanket `noexcept`
   across arbitrary profiles. This packet was offered to the primary for an
   independent worker while this report was being completed.

2. **Actual physical-buffer lifetime, 15 complete leaves.** Own
   `00B4BB60`, `00B4BBB0`, `00B4C250`, `00B4C370`, `00B4B900`,
   `00B4BAB0`, `00B4B490`, `00B4B530`, `00B4BB20`, `00B4BB40`,
   `00B4C210`, `00B4C230`, `00B49500`, `00B496A0`, `00B49700`.
   Suggested four files use stem `native_physical_buffer_owner`. Reuse the
   actual resource-support singleton and actual pooled-string/storage-pool
   implementations. Reconstruct the raw +08 pointer-array header and complete
   resize/grow/free behavior; the semantic `VertexBufferBinding` and
   `IndexBufferBinding` are not owner storage. Attach stores flags +14 and
   capacity +18, publishes new COM +28 before AddRef new/Release captured old,
   and includes the original diagnostic strings and support calls. Preserve
   their cleanup paths. Destruction installs the concrete table, calls the
   actual support singleton, then reloads current COM +28 before Release and
   clearing; base cleanup clears/frees the actual array and installs the base
   table. The four deleting leaves select scalar delete versus the two actual
   pool identities. Implement the complete raw pool-return leaf against
   borrowed initialized pool storage and real critical sections. This closes
   those owner lifetimes; it does not claim complete shared-pool initialization
   or the full renderer constructor. Do not invent a second pool.

3. **Actual CPU declaration lifetime, 9 complete leaves.** Own
   `00B48AF0`, `00B47910`, `00B488E0`, `00B48AD0`, `00B480F0`,
   `00B47A30`, `00B48B70`, `00B48CA0`, `00B47950`. Suggested four files
   use stem `native_vertex_declaration_owner`. The D0h object has actual main
   element array +0C and fifteen 0Ch array headers at +18, with stride +CC.
   Constructor initializes the real headers; clear and destruction preserve
   native element defaults, signed count/capacity operations, reverse array
   cleanup, allocator domain and base-table installation. Include the complete
   scalar deleting path and raw D4h-slot return to the actual `0108FD38` pool.
   Borrowed initialized pool storage is an explicit boundary, not a callback
   substitute for deletion. No COM declaration is stored in this owner.
   Existing `VertexDeclaration` vectors remain semantic behavior references;
   they cannot supply the actual intrusive lifetime.

Complete native EH maps and the real source implementation must be checked
when executing the lifetime packets. This read-only packet verifies complete
instruction bodies and explicit outgoing dependencies; it does not claim an
EH differential or assign nonthrowing contracts to those future interfaces.

## Conditional packets after those prerequisites

Logical index lifetime is the next smaller consumer of physical-buffer
lifetime: `00B4B6F0`, `00B4C1F0`, `00B495E0`, renderer removal wrapper
`00B26900`/array helper `00B25370`, and dynamic physical unregister
`00B4B390`/`00B4B2E0`. The full constructor `00B4BF30` additionally needs its
real COM creation/retry and renderer-device boundary. Destruction uses the
current global renderer for guard/removal, releases actual physical +08
without a null fallback, leaves its optional guard, installs the base, and
only then permits the pool return. The renderer removal is a raw
swap-with-last registry at renderer+1AB8, not an unknown arbitrary callback.

Logical vertex lifetime adds actual declaration ownership, renderer registry
`00B268E0`/`00B25300` at renderer+1AAC, and the separate embedded tracked
critical section at renderer+19F4 for dynamic unregister. The lock is captured
for Enter/depth increment; the global renderer is reloaded for decrement and
Leave. Declaration +68 is released before a **fresh** physical +58 load;
neither normal destructor release checks null. Base `00B62010` has a further
intrusive owner at +4C, whose concrete terminal profile remains unresolved in
this packet, plus owned decode allocation +50. Constructor `00B61E20` initially
sets +4C/+50 null, which does not prove +4C remains null for all live owners.
Do not close the complete logical vertex terminal by assuming it away.

Hardware-layout lifetime requires actual declaration terminals and its base
renderer notification: current renderer table +44 resolves to `00B2F4C0`.
That function walks the actual global checked tree rooted at `0108D530`,
matches node payload +20, and calls erase `00B2EF00`. Its sentinel byte is +25.
The already reconstructed render-resource alias nodes are 10h linked-list
nodes, so their checked-list eraser is not this tree implementation.
`00B60700` releases/clears COM +40, visits resource support, then base
`00B48960` installs `00D61D10`, notifies the renderer, destroys four actual
0Ch declaration records at +08 in reverse order, and installs the base table.
`00B60770` may then return its 48h slot through `00B60110`. The checked-tree
erase and actual declaration release paths must exist before this is a ready
full layout-owner packet.

Texture 2D has the reusable **actual** `D3D9Texture2DPool`, surface owners,
resource support and pooled-string support, but not a complete current owner.
`00B3F2E0` first releases actual retained-stream +4C, notifies current renderer
+6C (table target `00B32250`, no current Ghidra function definition), removes
itself through `00B27D40`/`00B25580` at renderer+1B00, performs the captured COM
AddRef/Release pair, visits support, then reloads current COM +10 for Release
and clear. It releases cached surface owners in actual 8-byte records at +40,
consults current flags +1C for the global live counter, clears/frees the array,
and destroys its base name through `00B33F50`. The retained-stream terminal and
texture registry notification still need concrete implementations. Existing
`D3D9RetainedTexture` preserves source-before-COM behavior semantically, with
shared ownership; it does not close those actual paths.

## Integration constraints established by this trace

`00B23710` is a complete ECX-destination/EDX-source pointer-slot assignment,
RET with original destination in EAX. It publishes the captured new owner,
increments its +04, decrements captured old +04, and invokes old's *current*
zero terminal only at zero. It does not add a reference to COM. This helper
and complete texture/vertex/index binders should follow the real terminal
packets; the existing nonthrowing `RenderCommandReference` interface does not
establish a throwing-terminal contract for these resource owners. The separate
`NativeRenderBatchReference` demonstrates an already implemented explicit
throwing terminal boundary, but is a different profile and allocator.

The important new layout-binding order is at `00B23FA7..00B23FC6`: capture the
current device **vtable before** invoking the layout's current +08 getter;
after that getter, reload renderer+1A10 for the COM self argument and read
slot +15C from the captured table. Replacing this with an unconditional fresh
`device->SetVertexDeclaration` lookup changes the native ordering when a
getter changes fields. The full 226-byte binding is pinned, but unimplemented
actual layout terminals still prevent general lifetime closure.

The full texture destructor is 302 bytes through `00B3F40D`, despite the
current function extent stopping at `00B3F3E8`. Physical base destructors
`00B4B490` and `00B4B530` are each 93 bytes, through `00B4B4EC` and
`00B4B58C`; their current extents stop immediately after `_free`. Declaration
destructor `00B48B70` is 132 bytes through `00B48BF3`, and array destructor
`00B48AD0` is 23 bytes through `00B48AE6`. All missing tails were checked
against live bytes and the installed PE, including base cleanup and returns.
The library names stay intact; no Ghidra extent or no-return flag was changed.

No result here closes `00B241C0` beyond its separately assigned null-reference
construction slice. Twenty texture unbinds, cached owner clearing, terminal
dependencies and the final stop composition remain distinct work. The report
records reconstructed versus semantic source boundaries without promoting
build, fixture or gameplay status from another packet.
