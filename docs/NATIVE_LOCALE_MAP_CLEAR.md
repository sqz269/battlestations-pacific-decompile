# Raw locale map clear

`clear_native_locale_map_0073c240` composes the complete 228-byte Original
`0073C240..0073C323` schedule over actual bucket/count bytes. The descriptive
Source name is a hypothesis; the existing semantic `LocaleTables` stays intact.

Capture the actual map address. For each of 4096 indices, load the CURRENT head;
for each nonnull node capture CURRENT `next+14` before calling genuine
`destroy_native_locale_map_node_payload_00737ce0(node, strings)`, then genuine
`return_native_locale_node_slot_00736090(pool, node)`. Continue the saved next.
No head is cleared/unlinked progressively, and no map count changes per node.
Only after all normal returns, perform the real `memset(4000h)` followed by a
separate zero DWORD store to count+4000h. The empty path makes no pool call.

Factoring was compared with the saved inline pool return. Pool+C matches
E17698; depth+24/E176B0 is incremented before node+18/table+28/E176B4 reads.
The signed slot offset/28, WORD free-stack/count at block+7000/+7800, current
minimum-index pool+34/E176C0, depth decrement and Leave order match the existing
736090 helper. Original clear captures Enter from CE2218 once; the Source
helper calls its imported Enter each time. Admit this factoring only with
stable Enter/Leave bindings, no import replacement or hooks during the call.
No new callback, provider, critical section or pooled owner is introduced.

The caller supplies real writable/live map4004h storage, genuine allocated
1Ch nodes and SAME initialized E1768C pool authority/table/slabs/critical
section, plus the SAME actual raw string publication cells and providers.
Addressability or zeroed fields do not prove owner authority. Native-width
raw reads/stores and wrapping address arithmetic are explicit; private
parameter/frame aliases, native hardware/SEH timing and concurrent destruction
are outside the supported Source domain. No validation or guarded no-op is
inserted. Caller custody must cover every reached current/captured object.

Existing 737CE0 owns its qualified wide/narrow payload cleanup and ordinary
C++ exception limits; 736090 retains its current depth/table/count/import
behavior. Any escape propagates without new map cleanup, rollback or retry.
Earlier nodes may already have payloads destroyed/slots returned while heads
and count still contain the old history. Retain the full partial graph,
contexts and providers; no later drain or repeated clear is assumed safe.

Validation covers one isolated MSVC Win32 translation unit. Its initial
compiler attempt caught a MASM `offset` keyword collision; after renaming that
local identifier, the strict object compile passed. Both receipts are retained
and recorded in `reports/native_locale_map_clear.json`.
No new test/probe/executable, runtime,
Ghidra mutation or vendor-library body credit is added. `source_ready=false`:
the actual initialized pool producer/lifetime, canonical locale owner,
loading/startup and consumer admission remain unbound. Empty clear is not
loaded-manager permission. Root owns CMake/ledger and historical readiness
updates; this packet adds no caller and grants no Native ABI/runtime credit.
