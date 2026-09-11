# Native renderer file reload discovery

Read-only discovery against `bsp.gpr`, `/battlestationspacific.exe`, from
checkout `cd52b91d`. The companion report pins 57 spans / 8,423 bytes and 34
current provider files: complete native routines, selected caller/initialization
fragments, current profile words and exception metadata, checked against fresh
guarded Ghidra reads and the installed PE. No source,
Ghidra or shared-ledger mutation is included. Names below are descriptive
hypotheses, not recovered symbols. Complete bytes are not a build, original
ABI compatibility or game-execution claim.

## Entry and enable byte

The activation branch of native BED3B0 loads current F8D394 at BED4B3 and
calls B24FB0 at BED4B9. This occurs after the activation/input/UI calls and
before restoring window focus. B24FB0 is a complete 49-byte ECX-renderer,
plain-RET routine with no semantic return value:

1. Read byte 108D4BB once. Zero skips all three stages.
2. Call B22030 on the captured renderer's effect registry at +1A98.
3. Reload the captured renderer's current table and call +120.
4. Tail-jump to B21F70 on the same renderer's texture registry at +1A74.

It does not clear the enable byte, reread the global renderer, install a
guard, retain the receiver, or recover from a child exception. If a child
throws, later stages are not reached.

108D4BB is an enable option, not a pending-change marker. The two known
writes are in Initialize73D410: case-sensitive substring `reloadresources`
in its second mode-string argument sets the byte at 73D597; substring
`devrr` forces it to one at 73D5E6 and also sets 108D6F1. These checks use
`strstr`, without token or word-boundary parsing. The initializer captures
that second argument in EBP at 73D4C2.

This is not direct OS-command-line parsing. The sole reported native caller,
WinMain8F81F0, pushes literal CE8168=`cachedload` and zero before 73D410 at
8F8429. That observed entry does not enable either reload option. It does
not justify deleting the nonzero branch or treating mutable runtime globals
as permanently zero.

## Polling contracts and actual storage

B21F70 and B22030 are each complete 158-byte ECX-container, plain-RET bodies,
with no semantic EAX return. They share the same behavior:

- Capture `begin=container+4` and `end=begin+DWORD(count*2Ch)` from +8 once.
  Equal pointers skip the loop. No signed-count or null-pointer guard exists.
- For each captured record address, reload VFS global 109CEEC and call full
  BDD340 with hidden output storage and the record's name header.
- Compare five DWORDs at record+14/+18/+1C/+20/+24 in ascending order,
  lexicographically **unsigned**. Reload only if the new tuple is strictly
  greater. Equal, older and all-zero dates do not reset a previously newer
  tuple.
- Store all five new words in order, then load current record+28, its current
  table, and current slot+8; call it with ECX=resource and no stack arguments.
- Advance the old record cursor by 2Ch and compare against the captured end.
  Neither callback mutation nor a changed base/count refreshes that bound.

The timestamp is committed before the resource call and remains changed if
that call fails or throws. Pollers have no native EH registration or rollback.
They do not retain the callback resource. Existing raw record storage is
already defined by `NativeRenderResourceRecord`:

| Record offset | Established field |
| --- | --- |
| +00/+04 | Native string length/data |
| +08 | Preserved list-owner word |
| +0C/+10 | Alias sentinel/count |
| +14..+24 | Five saved file-date words |
| +28 | Resource pointer; record storage helpers do not own/release it |

The raw container has profile+0, records+4, count+8, capacity+C and accounting
+10. Renderer construction installs D5F074 at +1A98 and D5F088 at +1A74.
Their +8 creation entries are B2EBB0 (effect) and B2C2D0 (texture). Their
+C/+10 entries are B31FF0/B32010 and B31D80/B31DA0 respectively.

The selected renderer +120 is **B24DD0**: D5F0A8+120=D5F1C8 contains it.
B24FF0 appears at renderer+E4, not +120, and its body is outside this packet.

## Current +120 and registered texture refresh

B24DD0 is a complete 65-byte ECX-renderer, plain-RET routine. It traverses
the effect records at +1A9C with count+1AA0 and 2Ch stride, calling full
B19000 on each current record+28. Unlike the pollers, after each call it
reloads count and then base, computes the new end, advances the **old**
cursor and compares equality. This is not a mutation-safe container walk.
It has no guard or exception cleanup. Every reached effect is visited even
when no date changed in the first polling stage.

B19000 is a complete 462-byte ECX-effect-owner, plain-RET body. The owner
prefix has eleven texture pointers at +0C, a short high-water count at +38,
eleven native names at +3C and a short name count at +94. The initial count
gate reads the word against zero; subsequent comparisons sign-extend each
current short and compare unsigned against the index. It reloads +94 after
each iteration and preserves the native high-water growth stores.

For each index, B19000 calls B1BC70 on **current F8D434**, obtaining a native
override string. A nonempty override takes precedence; otherwise it copies
the current registered name. Empty selected names skip loading and retain
existing texture slots. Nonempty names call current F8D394/current +64,
which D5F0A8 selects as B319B0, with the selected native name and flags zero.

After that call it captures the old texture, publishes the returned pointer
when different, increments nonnull incoming+4, and decrements captured old+4.
Zero invokes old's current slot0. It separately decrements the returned
temporary, including the identity case. A null return therefore clears an
old slot. Names/counts and acquired resources are not rolled back on failure.

Its FH3 map has two cleanup states for the two local strings, no catch map
and no resource-temporary cleanup. State1 unwinds override then selected
string; normal override release first changes to state0, and selected-name
release changes to state-1. Do not add a resource-release retry on exceptions.
The nine bytes B19037..B1903F are alignment NOPs omitted from Ghidra's listing;
the full native body includes them.

B1BC70 is 173 bytes, ECX override manager, output/name stack pointers, EAX
output, RET8. It uses the embedded Lua owner at manager+4, accesses Lua
globals by the supplied name, uses exact-string conversion with an empty
fallback when non-nil, and constructs from the C-string at **F8D438** when
nil. F8D438 is not a native string header. Its saved Ghidra-program byte is
zero in a zero-filled PE region and is queried separately from the 57 original
file-byte spans. That does not establish a permanent empty runtime override.
Its four-state
FH3 map covers both Lua objects and conditional output cleanup.

## Concrete changed-resource operations

| Registry resource origin | Published profile | Changed-resource slot+8 |
| --- | --- | --- |
| B2EBB0 -> B407A0 effect construction | D61A00 | B469A0 |
| B2C2D0 -> B3F930 2D construction | D61948 | B3FA90 |
| B2C2D0 -> B3CED0 cube construction | D61870 | B3DF40 |
| B2C2D0 -> B3CFA0 volume construction | D618B0 | B3E080 |

These are evidenced creation paths, not a promise that arbitrary current
profile words are valid. The shader-owner base D5E534 has a plain-RET slot+8
at BEFAF0; it is distinct from the fully constructed effect profile.

B469A0 lacks a current Ghidra function record. Its complete **207 bytes**
were captured directly without creating one. It checks 108D6F1. Zero returns;
nonzero logs the owner+B8 name through the existing B172D0/4254B0 route,
calls B41B10 then B187A0, reloads owner+B8 and calls B46950. On false AL it
repeats both cleanups and loads `shaderfx/common/error.shfx` into the same
effect. A one-state FH3 map only destroys the fallback name. B41B10 clears
the descriptor, two fourteen-pointer program/pass arrays and +138; B187A0
clears texture and secondary-owner references. Their concrete child lifetimes
and the full descriptor/program loading graph remain source dependencies.
B46950 also reads 108D6F0 and can perform two mode-load passes with intervening
cleanup; its existing filename/load description is not a full implementation.

The three texture reloads are complete B3FA90/747 bytes, B3DF40/258 bytes and
B3E080/272 bytes, all ECX-owner with plain RET. They open the owner's name via
current VFS+4, test current stream+18, and use direct deleting slot+4(flags1)
on the failed-open path. Otherwise they convert to a memory stream, release
the original, notify through captured current renderer+6C=B32250, release
the current COM resource, load through D3DX, update metadata on nonnull
output, and release the memory stream. Cube/volume final-zero dispatch is a
tail jump after stack/register restoration. The volume output dimensions
come from the returned local descriptor, not undeclared ESI/EDI values in
the pseudocode.

B3FA90 additionally unregisters the texture via B27D40, applies its own
dimension/mip policy and constructs/releases a diagnostic name record through
B3F5D0. Its `detail.dds` test is a case-sensitive substring test; it must not
reuse the initial loader's whole-name case-insensitive exception unchanged.
Its sole EH cleanup is the late diagnostic string, not the opened streams
or replaced COM pointer. Cube and volume have no FH3 frame. Complete bytes
are pinned; full file-loader/COM failure behavior is not claimed implemented.

B32250 invokes existing raw container removal. It can change the same texture
record array being polled, without retaining/releasing the removed resource.
The captured end in B21F70 remains captured nevertheless. An implementation
must preserve this reached-memory contract, rather than silently replacing
the iteration with a refreshed vector snapshot.

## Available providers and concrete blockers

Current provider hashes are recorded in the report. Existing source supplies
the actual 2Ch record, record copying/storage lifetime, array operations,
texture-name notification/removal, texture owner destruction/pools, retained
memory-owner release, and native string storage. The existing texture-binding
context already dispatches the concrete 2D/cube/volume deleting profiles.
These providers are reusable parts, not implementations of slot+8 reload.

BDD340 currently has a typed `VfsMountContext`/`std::string` projection with
extra host validation. Its physical/FileStore/MPKG date semantics exist, but
a complete raw-manager/current-provider adapter for this native caller has
not been established here. B319B0/B30B40/B2C2D0 and B2EBB0/B46950 have bounded
typed fragments; they do not close the native cache/reload route. The fresh
checkout also contains platform-message fragments, not a verified complete
BED3B0 source implementation.

The report proposes bounded dependency packets for the raw date adapter,
B19000/B1BC70 and its actual texture-loader dependency, effect reload, and
the three file-texture reloads. The wrapper/pollers become implementation-ready
only when those selected operations and their real owner domains exist.
No callback stand-in, permanently-zero gate substitute, initial-load substitute
or general shader/backend reconstruction is supplied by this discovery.
