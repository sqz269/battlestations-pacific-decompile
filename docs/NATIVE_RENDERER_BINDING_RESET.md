# Renderer binding-reset dependency discovery

The complete `B24BF0..B24DB4` body is 453 bytes. It is not reconstructed in this
packet: its full raw B24840 vertex-stream provider and that provider's logical
owner terminal remain incomplete. This document records the verified boundary
so a semantic binding adapter or a null-only teardown is not mistaken for a
complete dependency.

The original ABI is ECX actual renderer, no stack arguments, RET, no semantic
result. A future new fixed-context interface can use ECX renderer and EDX
context, borrowing the existing texture/index/layout/synchronization contexts
and original profile views. Its vertex-binding context cannot be finalized
until the concrete logical-owner lifetime is established. No API or source
stub is added here.

## Full reset schedule and concrete selectors

The constructor instruction at B3243B installs D5F0A8. Fresh profile cells
resolve these current-table virtual calls:

| Selector | Actual provider | Raw source state |
| --- | --- | --- |
| +130 | B24710 texture binding | Complete |
| +134 | B24840 logical vertex-stream binding | Missing; semantic `shared_ptr` projection is not the raw provider |
| +138 | B24B00 index binding | Complete |
| +E0 | B23F20 vertex-layout binding | Complete |

B24BF0 reloads the renderer's current profile before every virtual call. It
first calls +130 with `(sampler, nullptr)` for samplers 0..19. It then calls
+134 **four times with `(0, nullptr)`**, followed by +138 with `(nullptr, 0)`.
The stream countdown is never passed as a stream index.

Next it conditionally enters the actual optional guard, captures old cached
pixel shader +176C, arms state0, and clears +176C. A nonzero old value calls
the current device at renderer+1A10 through current table+1AC with null input.
The wrapping counter +1BBC increments only after that call returns; HRESULT
does not gate it. Current mode is read before disarming and optional leave.
If leave occurs, current mode is read again before the second optional entry.

The same eight-byte guard record is reused. Vertex shader +1770 follows the
same capture/arm/clear pattern under state1, current device table+170, and
post-return counter +1BC0. These cached words are not retained or released.
Current mode is rechecked before disarm/leave. The next +E0 call passes null.
Four direct full B23D80 calls bind null color surfaces at indices 0..3.

Finally another optional entry uses that same record. The current device,
its current table and +9C target are captured before state2 arms around the
unconditional SetDepthStencilSurface(nullptr) call. Current mode is read
after return and before disarm/leave. There is no outer reset guard or rollback.
Guard records skipped on entry are not initialized or repaired; the existing
native synchronization domain must account for current-mode changes.

FH3 handler CBD088 loads DF5728. Its three unwind-map entries at DF5710 each
have previous state -1 and select CBD070, CBD078 or CBD080. All three full
eight-byte helpers use `[EBP-14h]` and tail-jump to complete B21110. Thus the
three states independently destroy the shared optional-guard record.

## Exact missing provider chain

B24840 is the full 504-byte body ending at B24A37, with native ECX renderer,
stack stream index/logical pointer and RET8. It reads current logical owners,
declaration stride and physical COM binding, replaces intrusive ownership,
and calls actual device SetStreamSource through +190 before incrementing
+1BB4. Its equal-device/stride/offset path still performs the full B23710
intrusive-slot assignment. B23710 is 59 bytes, ECX destination slot and EDX
source slot, EAX destination, RET. Both paths can release the old logical
owner at its final reference; a null reset input does not eliminate this.

Fresh D61D6C slots are +0 BD30E0, +4 B4BF10, +24 B48CE0, +28 B48D10 and +2C
B48CF0. Declaration/offset getters are complete. B48CF0 is a complete naked
tailcall through the current physical table+1C; original numeric profiles
need a concrete token adapter or genuinely callable relocated tables. Both
D61E34 and D61E7C select complete B4B9F0 at +1C.

The zero-count release chain is BD30E0 -> current +4 B4BF10[32] ->
B4B5D0[280] -> B62010[135]. The actual pool-return B49570, renderer/physical
unregistration, declaration/physical lifetimes and optional guards already
exist. B4BF10 and B4B5D0 do not have complete raw source. Their base B62010
captures its independent retained +4C pointee, decrements its +4, dispatches
its current slot0 at zero, and only then clears +4C. The concrete nonnull
writer, incoming type/profile and reached final-zero terminal remain unknown.
The known D61D6C profile of the enclosing stream does not type that pointee.

Full B62010 also frees current +50 and clears it only after free returns.
The original ten-byte B6206D..76 continuation omitted by Ghidra's false
CALL_RETURN is included in this fresh capture. Base construction zeroing +4C
does not prove it stays null after producer/alias escape. Prior bounded
exclusions in the logical-owner and retained-terminal discovery are not a
whole-lifetime proof.

No missing function in this chain is independently source-ready under the
concrete-owner contract. The next useful task is a bounded search for a
nonnull +4C producer and its full terminal/allocator chain. A generic terminal
callback, assumed retained-memory profile or constructor-null restriction
would not close B23710, B24840 or B24BF0.

## Evidence

Twenty freshly guarded live/installed-PE spans match, totaling 1,753 bytes,
with 461 completely decoded instructions in the captured code spans. The
ignored immutable bundle is `local/renderer_binding_reset_discovery/` in
`J:/PROG/battlestations-pacific-decompile-native-renderer-binding-reset`.
It retains exact bytes, disassembly, checks and source-provider hashes. The
assigned report records the same boundary. This packet changes no C++ code,
shared CMake, Ghidra annotations or metadata. No build, fixture, original-ABI
compatibility or gameplay claim is made.


Primary review verified all 43 worker pins and reread twenty guarded spans,
1,753 bytes. The complete 461-instruction capture and dependency boundary
were retained. B24BF0 received a descriptive name, with existing comments
preserved and reviewed evidence appended, saved and re-exported. The
read-only primary proof is `local/renderer_binding_reset_discovery_primary/`.
No source body, runtime behavior or readiness was promoted by this review.
