# Native VFS pending request routes

Addresses: `00BDDA10`, `00BDB0B0`, `00BDC100`, `00BDB740`, `00BDC1D0`,
`00BDC1E0`, `00BE7CB0`, `00BE7CC0`.

This packet implements the actual manager/visitor/provider boundary in
`native_vfs_pending_routes.hpp/.cpp`. It composes the existing raw-header string
pool, mount iterator and lookup traversal. It does not use `src/vfs_pending.cpp`.
The latter retains its separate projected mount array, validation, pump report
and reentrancy policy; those are absent from these original bodies.

| Routine | Inclusive end | Bytes | Coverage | Original ABI |
|---|---|---:|---|---|
| BDDA10 manager submission | BDDA96 | 135 | complete, existing traversal composition | ECX manager; first/second/callback/flags stack; RET10h; raw AL |
| BDB0B0 provider pump | BDB11B | 108 | complete | ECX manager; RET; no specified result |
| BDC100 visitor construction | BDC1C2 | 195 | complete within existing string bridge domain | ECX visitor; first/second/callback/flags stack; RET10h; EAX visitor |
| BDB740 visitor destruction | BDB7B9 | 122 | complete within existing string bridge domain | ECX visitor; RET |
| BDC1D0 acceptance | BDC1D3 | 4 | complete | ECX visitor; RET; raw AL |
| BDC1E0 provider dispatch | BDC207 | 40 | complete | ECX visitor; payload/unused suffix stack; RET8 |
| BE7CB0 FileStore decline | BE7CB4 | 5 | complete | ECX/4 stack arguments unread; XOR AL,AL; RET10h |
| BE7CC0 FileStore tick | BE7CC0 | 1 | complete | ECX unread; RET |

## Storage and execution

BDC100 produces the 20h-byte visitor: original identity D68478 at +0; native
length/data headers at +4/+8 and +0C/+10; raw acceptance byte +14; unchanged
padding +15..+17; flags +18; opaque callback DWORD +1C. Each header is cleared
before its identity comparison. First-copy completion arms only its destructor
before the second copy begins. Allocation can change an input header; the
copy rereads input length and both current data pointers after resize. Original
BF7680 handles backward overlap, so this source uses `memmove` for that copy.

BDDA10 calls the one-byte RET at 4254B0, constructs its stack visitor, then
passes the *original second header pointer* to BDD0A0. Neither copied name is
normalized. Existing BDD0A0 copies that header and matches existing mounts in
tree order. Its temporary provider-relative suffix is still constructed and
destroyed by that dependency; BDC1E0 does not read the suffix argument.
The provider instead receives both complete visitor-owned headers, callback
and flags. Returned AL overwrites visitor +14 after the provider call. The
manager saves the raw byte after traversal and before visitor destruction.
There is no Boolean normalization, callback-null rejection or fallback error.

BDB740 releases the current second header, then rereads and releases the
current first header, then stores D68380. It does not clear header fields,
cancel accepted work, release provider resources or own the callback. A provider
which accepts work must preserve whatever names and resources its own body
requires after this stack visitor is destroyed; no additional lifetime is
invented here. Exceptions escaping traversal destroy the completed visitor.

BDB0B0 begins at manager+40 head.left with the actual descriptor manager+3C.
Each iteration captures the current end before its returning CRT owner check,
reads current node+18, captures its current table+28, calls that provider, then
advances the same iterator through BD97E0. It reloads iterator owner/node and
the next end. There is no provider reference, snapshot, deduplication, lock,
reentrancy guard or result aggregation. Insertions and current table changes
can affect later calls. Native invalidation hazards remain caller obligations.

## Dependencies and integration

`NativeVfsProviderPendingDispatch::invoke_submit` receives the captured entry,
actual provider, two actual 8-byte headers, opaque callback and flags. It returns
raw uint8 acceptance. `invoke_tick` receives the captured entry and provider.
These are explicit source call boundaries, with no default implementation or
implicit fallback. The primary binds concrete physical BF43B0/BF46B0 and
FileStore BE7CB0/BE7CC0 profiles and owns the runtime/CMake changes.

The primary also owns the small extension to `NativeVfsLookupRouteContext`:
optional `pending` points at this route context, D68478 selects the bound
actual pending profile storage, BDC1E0 selects this visitor dispatch, and
BDC1D0 selects its raw acceptance getter. The existing traversal must reread
current profile +4/+8 separately, as it does for other visitor identities.
This worker does not duplicate or edit that shared traversal.

| Sites | Native callee | Established contract / source dependency |
|---|---|---|
| BDDA32 | 4254B0 | One-byte RET; diagnostic has no effect |
| BDDA52 / BDDA67 / BDDA7C | BDC100 / BDD0A0 / BDB740 | Construct, traverse second input, destroy |
| BDC144 / BDC17E | 41DD40 | Current actual header resize; preserve=1; RET8 |
| BDC159 / BDC193 | BF7680 | Current data/count copy with overlap; caller ADD ESP,0Ch |
| BDB776 / BDB798 | 419CC0 | No-argument current owning-pool getter; caller-pushed release arguments survive |
| BDB77D / BDB79F | BD1510 | Captured data/current length+1/unused1; RET0Ch |
| BDC1FF | current provider vtable+0C | Both complete headers, callback, flags; result raw AL |
| BDB0DB / BDB0E8 / BDB0F2 | BF6713 | Returning invalid-parameter boundary; continue afterward |
| BDB0FF | current provider vtable+28 | Pump captured actual provider, no arguments |
| BDB105 | BD97E0 | Advance existing {owner,node} iterator |

The callee bodies were inspected before defining these source contracts.
Live function ranges and exact call instructions are recorded and mechanically
checked in `reports/native_vfs_pending_routes_bk.json`. Indirect rows are
manually body-range verified and are reported as indirect by the checker.

## Unwind evidence and scope

Constructor handler CC6173..CC617C loads E0054C. Its E0053C map has
state1 -> CC6168..CC6172 (first header through 41DD20), then state0 ->
CC6160..CC6167 (BD8FE0 base reset). Failed initial construction owns no
incomplete string. Destructor handler CC6083..CC608C loads E003E8;
E003D8 similarly releases first on a second-release failure, then resets base.
Dispatcher handler CC63E8..CC63F1 loads E008CC; E008C4 sends state0 to
CC63E0..CC63E7, which tail-jumps to BDB740. The three handler entries have
no Ghidra function definition; the recorded ends come from their complete
ten-byte disk listings and live-byte comparison. Existing unwind funclets
do have Ghidra bodies. All were read without Ghidra mutations.

The existing `ActualNativeStringPoolStorage::release` is noexcept. The complete
ordinary destructor schedule is implemented in that returning-getter domain;
a throwing pool getter during release is outside this bridge's domain and
does not have native FH3/SEH parity. Source construction/traversal C++ cleanup
is preserved, but native exception execution was not probed. These APIs have
new C++ interfaces and are not drop-in x86 ABI replacements.

## Verification

Read-only wrappers verified the configured C:/Users/sqz269/bsp.gpr project,
/battlestationspacific.exe, x86 language and image base before live batches.
Nineteen fresh native/EH spans, 842 bytes, matched the installed executable.
All eight complete owned bodies were exported. No annotation, saved-project
mutation or reconstruction ledger edit was made; retained descriptive names
remain hypotheses. The report includes source/evidence artifact SHA-256 pins.

`local/compile_pending_routes.cmd` passed MSVC x86 C++17 /W4 /WX /fp:strict
/MD /O2 for this translation unit. One ignored probe,
`local/pending_routes_probe.cpp`, linked this exact object with frozen existing
core/Lua/zlib libraries and embedded its manifest. It executed seven original
bodies (all owned bodies except BDDA10), patching only direct dependency calls
to explicit existing source bridges. It compared the exact 32-byte visitor
plus complete 8AD4A0h actual string-pool storage: 9,098,432 bytes total, and
compared provider traces. Cases cover nonnormalized names, borrowed null
callback, raw AL80 overriding provider mutation, no-read FileStore leaves,
and insertion of a mount during the pump while the same provider changes its
current +28 slot and is called three times. The native/source traces matched.

Original dependency internals are not independent evidence: string pool/resize,
copy, returning CRT and iterator edges are bridged to the existing complete
rebuilt dependencies. BDDA10 has compile/static coverage here; pending visitor
traversal wiring and whole-executable validation belong to the integrator.
No native throwing-unwind, provider I/O, terminal failure/cancellation, original
game runtime, full repository build or gameplay result is claimed by this packet.
