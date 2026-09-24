# Service-facing raw terminal dispatch (CC10)

This packet extends the existing explicit source lifetime for B0F6E0/B14F60.
It admits service-facing surface D619A0, holder D61EB8 and runtime2D D61948
owners through their genuine existing destruction contexts. It adds no native
body, reference credit, producer, registry, profile, application invocation or
initialization. Existing callers retain their prior aggregate initialization.

## Exact native schedule

| Existing body | Exclusive interval | Bytes | Last instruction |
|---|---|---:|---|
| B0F6E0 member release | [B0F6E0,B0FBF9) | 1305 | B0FBF8 RET, 1 byte |
| B14F60 destruction | [B14F60,B15086) | 294 | B15085 RET, 1 byte |
| BD30E0 virtual0 | [BD30E0,BD30EE) | 14 | BD30ED RET, 1 byte |
| B3F5B0 surface delete | [B3F5B0,B3F5D0) | 32 | B3F5CD RET4, 3 bytes |
| B4E410 holder delete | [B4E410,B4E42E) | 30 | B4E42B RET4, 3 bytes |
| B3F590 texture delete | [B3F590,B3F5B0) | 32 | B3F5AD RET4, 3 bytes |

The live Ghidra bytes match the installed PE for all six bodies and the three
original two-word tables. B0F6E9 first calls B52270 on current service+34.
B0F6EE captures service+50, then B0F6F1 captures CE2220 once for all 42 visits.
Every reached nonnull owner decrements its own actual+4. Only a zero result
loads its current profile/slot0 (first B0F707/B0F709), calls that captured
invoker (B0F70D), and then clears the current parent slot (B0F70F). BD30E0
reloads the owner profile at BD30E4 and deleting+4 at BD30E6, pushes1 and calls
EDX at BD30EB. Numeric original words select established source providers.

B14F93 captures current service+34 only after B0F6E0 returns; B14F96 captures
a fresh CE2220 epoch for +34/+668/+0C. This packet changes only the shared
zero-terminal dispatcher. Both decrement epochs, the duplicate +1D4 visits,
current field loads/clears, final +1C4 byte and exception projection remain.
The caller supplies native lifetime synchronization: registry lookup adds no
callback and holds no lock across terminal dispatch. Returning callbacks may
change later parent fields and import cells; they cannot race admission or
retirement of the captured zero-count identity. Parent clears still overwrite
a returning callback's write. No owner read follows terminal return/free.

## Backing and lifetime admission

| Service-facing family | Concrete producer/backing | Existing terminal/context |
|---|---|---|
| Surface D619A0 | B2A7C0 creates +50; +58 retains that current surface. B3FD80 creates/retains +5C. Full B3F630 initializes the 34h payload in its genuine 38h canonical surface pool slot. | B3F5B0 -> B3F4E0 -> B3D860 pool return; existing frame_targets.actual_surface_context. |
| Holder D61EB8 | B107F0's five 18h allocations call full B4E020 for +4C/+38/+3C/+44/+48; +40 retains current +4C. Constructor produces actual runtime texture and retained/cache or external surface fields. | B4E410 -> B4E140 -> BF65AC free; supplied existing holder context, no new holder companion. |
| Runtime2D D61948 | B2A070/B3F7B0 produces +54 in the real 54h pool slot with 50h payload and preserved pool ID at +50. Loaded D61948, including constructor +668, may already have a canonical companion. | B3F590 -> B3F2E0 -> B3D8D0 pool return; holders.textures.construction.owners, or the existing companion when bound. |

The optional `NativeRenderResourcesDirectTerminalDomain` is an explicit
borrowed admission contract, not evidence inferred from a profile token.
Every reached unbound owner must be a completed genuine producer result with
valid cleanup fields, actual count and allocator/pool metadata. Its producer
and every callback replacement must use these same providers. The service,
raw owners, context, publication cells, original tables and companions stay
valid until their native terminal operation and subsequent host quiescence.

The implementation checks that the concrete registry is exactly
textures.actual_owners, and that holder texture, getter, render-target and
frame contexts share the same surface context. It also checks actual string
storage, renderer publication cell, decrement import cell, surface table and
2D table identities. The retained context then carries the actual pools,
renderer, counters and support lifetime; it creates none of them.

With the domain present, a pure canonical lookup precedes direct dispatch.
Any existing companion wins, must borrow exactly captured+4 and receives the
already-zero terminal call. In particular, a loaded D61948 companion retires
and unbinds normally. An absent binding permits only the three admitted raw
profiles, with current virtual0 and separately refreshed deleting-slot reads.
The single refreshed profile read also selects its expected scalar deleter;
the captured +4 target must match that family's provider before dispatch. A
mismatched table cannot route surface storage to a holder or texture deleter.
Other identities retain the existing frame/cockpit/canonical behavior. An
absent domain retains ordinary canonical lookup for nonframe/noncockpit owners;
an unbound raw identity fails explicitly. No failure is interpreted as absence.

The raw nested surface/texture paths inside the existing holder/texture/frame
providers remain direct. Their existing subtree contract requires those nested
identities to remain unbound whenever these raw deleters can reach zero. This
packet does not authorize raw deletion of a subtree containing an unretired
bound nested companion, and does not blanket-register nested resources.

## Validation and limits

Strict MSVC Win32 `scripts/build.ps1` and the three existing CTests pass.
The report records exact call rows, byte hashes, table words and complete
instruction boundaries. Ghidra is read-only; no native name/body/ledger changed.

One ignored focused source lifecycle uses real D3D9, original table data,
canonical pools and full surface/holder/runtime producers. It places a genuine
surface, holder, two credits to one raw runtime2D and a separately registered
D61948 in sparse service/helper fixtures. The bound texture uses the existing
NativeTextureLoadOwners companion, not a test replacement. This admission of
a completed runtime factory result does not claim execution of file loading.
Five outer decrements retain the entry import despite callback replacement;
the holder captures the replacement import for its two nested decrements.
A callback's service+50 replacement is cleared after the captured surface is
destroyed. Registry, renderer lists and tracking counters return empty/baseline;
final actual device/API references are zero. No tracked test is added.

The sparse fixture does not execute B14A10, B107F0 or B14F60, nor prove general
reentrancy/concurrency or every exceptional path. Existing cold constructor
callers leave the optional domain absent. Application composition remains
separate. Distortion cleanup preimages, bloom +430..43F backing and the complete
initializer's lifetime/exception/ABI/game validation gaps are unchanged. The
normal B0F6E0 schedule still does not release service+67C noise.
