# Live-effect owning arrays and insertion

Packet `orch3_live_effect_insertion_s` reconstructs four complete functions in
`src/live_effect_insertion.cpp`. Names describe inferred behavior, not recovered
symbols. The source targets MSVC Win32 and exposes new C++ interfaces.

| Native span (exclusive end) | Original ABI | Reconstructed behavior |
| --- | --- | --- |
| `004C5CE0..004C5DD4` | ECX header, stack signed capacity, RET 4 | Reserve owning pointer storage |
| `0074D780..0074D7F3` | ECX header, stack source-cell address, RET 4 | Append one owning reference |
| `004C9550..004C95CE` | ECX header, stack signed count, RET 4 | Resize owning pointer storage |
| `00867500..008675AB` | ECX actual 28h manager, stack raw effect, RET 4 | Insert into manager array at +10 |

Analysis used `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`.
All four full spans matched the installed executable byte for byte. Span hashes,
annotation history and validation are in `reports/live_effect_insertion.json`.

## Physical storage and ownership

These operations reuse `NativeRenderPointerArrayStorage`: data at +00, signed
count at +04, signed capacity at +08, total 0Ch. Cells hold actual raw owner
addresses. Retain changes that owner's actual atomic at +04; release uses
`release_native_render_actual_owner`, which decrements first and resolves the
required current terminal binding only at zero. No alternate owner, reference
count, container or registry is introduced. The same layout also has existing
borrowed-pointer operations; ownership belongs to the particular operation.

Reserve clamps the request to at least one and grows only when signed capacity
is smaller. Allocation uses the wrapped DWORD product `capacity * 4`. Each
ascending copy captures the current source-cell address before zeroing its
destination, then reads that source and retains a nonnull value. The advancing
destination is captured; source backing and live count are reloaded. Old slots
are released in ascending order. A nonnull slot is cleared at its captured
address after terminal reentry; a null slot is not rewritten. The next iteration
reloads current backing and count. It frees the current backing before publishing
the replacement pointer and captured capacity; it does not publish a new count.

Append grows only at count == capacity, doubles with DWORD wrap, and signed-clamps
the result to one. After reserve it reloads the header. A nonnull destination is
zeroed before the source cell is read, including when those cells alias. A null
computed destination skips the source read but still increments current count.
The caller must keep the source cell alive across a possible reserve.

Resize reserves if necessary, zeroes growth slots using current backing without
publishing the growing count, then shrinks against current count. Each shrink
decrements count before capturing and releasing the slot. It clears that captured
slot after a nonnull release and reloads current count/backing on the next loop.
Finally it publishes the captured requested count. Valid native spans, owners
and count domains remain caller preconditions; no overflow recovery is added.

## Insertion and unwind

The native live-instance manager is the 28h owner published at `00F8765C`.
This packet borrows its actual +10 array explicitly; it does not construct that
whole owner. The separate 8h lock owner at `00F87650` comes from existing
`effect_manager_singleton_00866440`, even for a null effect argument. Insertion
captures its +04 critical-section binding, enters it and increments tracked depth.
A null section is permitted by the native branch; the manager itself must exist.

For a nonnull effect it creates a stack temporary without an initial retain and
appends through the address of that cell. On success it disables temporary
unwind, releases the captured incoming effect, then retains that same raw +04.
The release and following retain remain separate operations. With an ordinary
creator reference the sequence is 1 -> append 2 -> release 1 -> retain 2.

Handler `00C94E90..00C94E9A` selects `00DC6CFC`; unwind map `00DC6CEC`
has state 1 -> 0 through `00C94E88` / `00440A30`, then state 0 -> -1 through
`00C94E80` / `00411EE0`. The first releases the current temporary and clears
it after a nonnull terminal callback; the second unlocks the captured section.
The C++ implementation expresses this order with nested local guards.

Reserve handler `00C65157..00C65161` selects `00D8D8A4`, map `00D8D89C`;
append handler `00C878AC..00C878B6` selects `00DB71D8`, map `00DB71D0`.
Both maps contain a state-zero range cleanup, but their main bodies keep state
-1. Consequently no replacement-prefix rollback is inferred from those unused
entries. All three handler functions were defined and saved under the Ghidra
write lock; `reports/live_effect_insertion_definitions_s.json` records changes.

`reports/live_effect_insertion_flow_s.json` records removal of the incorrect
CALL_RETURN override on the reserve free call at `004C5DAA`. Restored fall-through
bytes `004C5DAF..004C5DC2` include replacement pointer/capacity publication.
The repaired body has no remaining call gap and includes the final RET 4.

## Validation and remaining work

`scripts/build.ps1` passed, including both existing CTests. No permanent tests
were added. Ignored `local/live_effect_probe_s.cpp` executes all four copied
original bodies alongside the reconstructed operations. It covers growth,
reserve clamping, duplicate references, balanced shrink, source/destination
aliasing, the explicit null-destination branch, and terminal callbacks changing
backing/count while overwriting the old captured slot. Terminal fixtures free
their actual owned payload; missing raw-owner identities are rejected.

Insertion comparisons cover null-input lazy lock creation, the untouched manager
prefix/tail, actual Win32 nested critical-section depth, and final owner counts.
Native allocator/free calls are rebound to the canonical implementations; array
calls use the copied originals. Interlocked imports use calling-convention
bridges to real Interlocked operations; lock imports use Win32. The native getter
bridge invokes reconstructed 866440 and presents an 8h read view with its actual
native section address. That view creates no independent lock or depth counter.

One C++ failure case recompiles only this production module with its allocation
symbol rebound locally: successful calls delegate to the canonical allocator,
and one call throws `bad_alloc`. This verifies incoming temporary terminal cleanup
at depth one, followed by depth zero and an unchanged array. Production code has
no added allocation seam. Native handler immediates remain unchanged in copied
code and native exception dispatch was not executed.

The actual 28h manager's construction, lazy lifetime, deferred-deletion list and
destruction remain separate reconstruction work, as do whole point-effect
construction and current effect terminal bindings. This is source, build and
focused fixture evidence; it is not original exception ABI or gameplay proof.
