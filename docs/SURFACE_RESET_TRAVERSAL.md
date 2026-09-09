# Surface reset traversal

The native release and restore routines have different owner order, guards,
and callback-loop mechanics. Their surface operations can be composed with the
current typed wrappers only within explicit stable-owner/list boundaries;
copying callbacks to a snapshot or allowing arbitrary vector mutation would
not preserve the observed traversal.

## Entry conditions and ordering

Both routines receive the renderer in ECX and consume no stack arguments.

`00b262c0` enters the existing optional renderer guard when global `0108d6dc`
is nonzero. It checks renderer byte `+1d8bh` (resources ready); zero skips
all resource work. Otherwise it stores zero **before** any virtual callback,
then executes this exact sequence:

1. Renderer virtual `+130h` receives `(slot,0)` for slots 0 through 19.
2. Renderer virtual `+134h` receives `(0,0)` four times. The native countdown
   does not change either argument; replacing this with stream indices 0..3
   would change behavior.
3. Renderer virtual `+138h` receives `(0,0)` once.
4. If nonnull, depth owner `+198ch` receives virtual `+3ch`.
5. Four owner slots are visited in address order: `+197ch`, `+1980h`,
   `+1984h`, `+1988h`. Each nonnull wrapper receives virtual `+3ch`.
6. Listener array `+19a0h` / signed count `+19a4h` receives virtual `+18h`.
7. Resource array `+1b00h` / count `+1b04h` receives virtual `+20h`.
8. Surface array `+1b0ch` / count `+1b10h` receives virtual `+3ch`.
9. An arithmetic-only loop advances across `2ch`-stride records at
   `+1a78h` / `+1a7ch`; there is no callback or resource write in that loop.
10. `00b241c0` clears the binding-cache subobject at renderer `+34h`.

The optional-guard global is checked again at exit. This matches the existing
renderer guard convention; changing guard mode while executing remains outside
the safe typed usage domain. Release does not check the lost byte `+1d8ah`,
thread identity, focus, or cooperative level. Those are surrounding scheduler
responsibilities. HRESULTs from the virtual resource operations are not tested.

`00b23b10` has **no local optional guard or SEH setup**. It proceeds only when
ready `+1d8bh` is zero and lost `+1d8ah` is zero. It stores ready one before its
first COM call, then performs:

1. `GetRenderTarget(0)`; pass the resulting pointer to the existing wrapper
   at `+197ch` via virtual `+14h`; release the getter COM reference.
2. `GetDepthStencilSurface`; pass it to the existing wrapper at `+198ch`
   via virtual `+14h`; release the getter COM reference.
3. Resource array `+1b00h` / count `+1b04h`: virtual `+24h(device)`.
4. Surface array `+1b0ch` / count `+1b10h`: virtual `+40h(device)`.
5. Listener array `+19a0h` / signed count `+19a4h`: virtual `+1ch`.

The three extra render-target owner slots are not independently reacquired by
restore. Existing default wrappers are reused, not reconstructed or replaced.
No final `bind_depth_surface_00b21690` call appears here, unlike initial default
capture. Restore ignores HRESULTs and assumes nonnull default wrapper/getter
pointers. The ready byte remains one even if a native COM operation fails;
a typed error result does not imply the original rolled readiness back.

These routines do not themselves call device `Reset`. Scheduler `00b2abd0`
places the release, Reset, and restore phases around additional buffer, state,
window/focus and failure-handling work. A composition of the surface phases
alone is not a complete scheduler port.

## Exact callback traversal semantics

The four fixed render-target owners use a pointer cursor initialized to
renderer `+197ch`, advancing four bytes after each iteration, with a separate
four-iteration countdown. Each slot value is loaded immediately before use.
A callback can therefore affect a later slot's value; the routine neither
snapshots all owners nor deduplicates identical wrappers. Depth is read and
processed before this fixed loop. No temporary wrapper retain is taken.

Both dynamic resource/surface arrays use a different, unusual pattern. For
surface release, `00b263ff..00b26431` is equivalent at the address level to:

```text
cursor = renderer.surface_array
end = renderer.surface_array + 4 * renderer.surface_count
if cursor != end:
    repeat:
        wrapper = *(pointer*)cursor
        wrapper.release_for_reset()
        count = renderer.surface_count       // fresh load after callback
        base = renderer.surface_array        // fresh load after callback
        cursor += 4                         // retained old address, not base+index
        end = base + 4 * count
    until cursor == end
```

Restore at `00b23beb..00b23c24` uses the same cursor/end scheme for virtual
`+40h`. It additionally reloads renderer `+1a10h` before **each** callback and
passes that current device pointer. Neither loop null-checks entries, tests a
signed-positive count, uses `< end`, or retains the wrapper across the call.
The preceding `+1b00h` loops use the same raw-cursor/dynamic-end pattern with
their own virtual slots.

The listener loops instead keep an integer index, reload the base before every
entry, and use signed `index < current_count` after callbacks. Their initial
signed-positive test skips zero or negative counts. They are not equivalent to
the resource/surface raw-cursor loops and should not share an invented generic
iteration abstraction that erases these differences.

Mutation implications follow directly from the instructions:

- Appending without reallocation can extend the end and make appended entries
  participate in the same traversal.
- Swap-with-last removal of a current entry can leave the swapped-in object
  unvisited because the cursor always advances.
- Shrinking while visiting the final entry can move the end behind the next
  cursor; equality-only termination does not protect against overrun.
- Reallocation updates the computed end but leaves the retained cursor pointing
  into old storage. A fresh base load is not a safe reallocation strategy.
- Repeated entries receive repeated callbacks. An owner slot and list entry
  referring to the same wrapper are not deduplicated.

These are not claims that the original game deliberately exercises unsafe
mutation. They show that safe registration/removal must occur outside these
traversals unless a narrower callback contract proves otherwise. A typed port
can explicitly require stable storage, membership, and lifetimes during the
batch. It must not claim native mutation parity for a snapshot, range-for
iterator, or automatically repaired index loop.

Early readiness writes inhibit a same-phase recursive call under unchanged
state: recursive release sees ready zero, recursive restore sees ready one.
They do not prove safety for a callback invoking the opposite phase or changing
ready/lost state. No blanket reentrancy guarantee is established.

## Safe composition with current typed surface helpers

For ordinary stable owners, current `surface_release_for_reset_00b3d510`
releases and nulls the COM pointer while preserving metadata. It does not
unregister or destroy the wrapper. Its second visit is a no-op once the pointer
is null, consistent with repeated release callbacks to the same native wrapper.
The existing default owner can therefore release depth first and color second;
additional native render-target owner slots require an explicit typed mapping.

After a successful device Reset, reacquire color and depth into their **existing**
bindings using `surface_initialize_00b3cc80`, then release temporary COM getter
references. That helper requires an empty binding; the preceding release phase
provides the condition. Do not call the initial
`capture_default_surfaces_00b238d0_fragment` as a substitute: it replaces owner
state and binds depth, operations absent from native restore.

For registered offscreen surfaces with stable membership and a null COM pointer,
`surface_recreate_00b3d550` already supplies the correct creation API selected by
stored recreation kind. Do not use it on default surfaces, whose kind is zero
for both color and depth and which are reacquired from the device instead.
The typed helper's empty-owner precondition and HRESULT reporting are explicit
interface differences from unchecked native overwrite/failure behavior.

The current `D3D9DefaultSurfaces` owner represents color0 and depth only.
Full release/restore also needs the extra fixed owners, the other resource and
listener arrays, binding-cache sequencing, and scheduler ownership. Those
missing paths must remain explicit rather than being replaced with no-op
callbacks. A stable typed surface-list batch is a useful component, not proof
of a complete reset lifecycle.

## Verified byte evidence

The export and memory-read batches each verified project `bsp`, program
`/battlestationspacific.exe`, x86 image base `00400000`. Saved Ghidra bytes
matched installed PE bytes exactly for these complete/bounded ranges:

| Range | Bytes | SHA-256 |
|---|---:|---|
| Full release `00b262c0..00b26493` | 468 | `b267293435d48a5841ad373e91ab258f36260557523c1afd72031fd06673b83a` |
| Full restore `00b23b10..00b23c4e` | 319 | `f04f69fd95e3410bd547a18f49ce949021b9f911cf2388dfd1c4e5cffb81926e` |
| Depth/four-owner release `00b26373..00b263a4` | 50 | `51eb3901a021925bdd74721e07ffc40b1f126c6b73144757f835181a34985185` |
| Surface release loop `00b263ff..00b26432` | 52 | `d7a3c07c6f4576505b35e6c917d47ba378431edd34d5f9c05065fcfb2149cabd` |
| Surface restore loop `00b23beb..00b23c25` | 59 | `7103fbc3a6afc3b2fa98c29ea6072d5e08de283e84fcd2735ae5b6976a2dd768` |

This document is an assembly audit, not a new runtime/reset validation result.
Only this document was edited; Ghidra exports were refreshed read-only.
