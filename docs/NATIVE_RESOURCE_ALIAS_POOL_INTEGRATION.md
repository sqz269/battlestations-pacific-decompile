# Resource aliases using the actual owning string pool

Seven existing full algorithms now also accept `ActualNativeStringPoolStorage&`.
This closes the allocation and cleanup composition needed by a future native
resource-record copy. The existing `SizedStoragePool&` interfaces remain available;
they do not become bindings to the native owning pool. Private compile-time
helpers share the algorithms, and overload resolution selects the pool at every
normal and catch edge. No allocator callbacks or additional pool are introduced.

The accompanying audit freezes seven main spans (845 bytes), two complete owned
catch regions (38 bytes), and all 28 freshly guarded native evidence spans
(3,250 bytes). The C++ interfaces are build- and fixture-checked, not binary ABI
replacements or game-validated code. Names remain descriptive hypotheses.

| Original entry | New overload's existing symbol | Original ABI |
| --- | --- | --- |
| 004D48A0 | `copy_construct_native_render_alias_list_004d48a0` | ECX destination; stack source; EAX destination; RET4; main108 + catch17 |
| 004D26A0 | `insert_native_render_alias_range_004d26a0` | ECX destination; seven stack DWORDs, including three iterator pairs and one unread word; RET1C; full276 including catch |
| 004CE6F0 | `allocate_native_render_alias_node_004ce6f0` | Stack next/previous/source header; ECX/EDX unused; EAX node; RET0C; main108 + catch21 |
| 0044BCB0 | `construct_native_render_alias_string_0044bcb0` | ECX destination header; EDX source header; RET; no specified result; full118 |
| 004D0990 | `erase_native_render_alias_node_004d0990` | ECX destination; stack output pointer and by-value owner/node; EAX output; RET0C; full123 |
| 004D05E0 | `clear_native_render_resource_aliases_004d05e0` | ECX owner; RET; no specified result; full83 |
| 004D0A10 | `destroy_native_render_alias_list_004d0a10` | ECX owner; RET; no specified result; full29 |

An actual list owner has an untouched DWORD at +0, its sentinel pointer at +4,
and current DWORD count at +8. Each 10h node has next/previous at +0/+4 and its
actual eight-byte string header at +8 (length) and +C (data). An iterator is the
actual owner/node DWORD pair. The 2Ch resource record's list begins at +8, so
its sentinel/count are +C/+10; its five date words occupy +14..+24 and its
unretained resource pointer is +28. Callers provide valid current native storage.
There is no list projection, tree snapshot, reference-retention policy, or new
null/owner validation.

The selected dependency graph is:

```text
4D48A0 -> 4C3020 sentinel -> 4D26A0 range
  catch 4D490C -> 4D0A10 -> 4D05E0 -> actual pool return, CRT node free
4D26A0 -> 4CE6F0 -> 44BCB0 -> existing actual-header 41DD40
        -> 4CE780 count growth -> current links
  catch 4D2746 -> 4BE820, 4BECC0, 4D0990, 4B9FF0 -> rethrow
4D0990 -> actual pool return -> CRT node free -> current count decrement
4CE6F0 catch 4CE75C -> captured CRT node free -> rethrow
41DD40 / release -> ActualNativeStringPoolStorage -> current 419CC0
                -> actual BD1120 / BD1510
```

`ActualNativeStringPoolStorage` borrows the application's real publication slot
01090AA8, gate01090AA4, and canonical singleton lifetime domain. That domain must
already compose `NativeStringPoolLifetimeBinding` with any other registered
owners. Every allocation/return repeats the current 419CC0 getter, including
large malloc/free requests. Small allocations and returns use the existing
actual native arena, rings, and embedded critical section. The new string-copy
overload passes its actual destination header directly to the already complete
41DD40 resize with preservation enabled. The existing `NativeStringStorage`
virtual interface is bound to the final concrete actual-pool adapter; it is not
an injected allocator or another storage implementation.

The current archive's decorated COFF references confirm the actual-pool overload
at every alias edge, including both catch chains. Clear and erase reference
`ActualNativeStringPoolStorage::release` directly; no selected actual-pool alias
section references `SizedStoragePool`. Inlined string-copy/sentinel operations
are included in full object and code/relocation hashes. The legacy overloads and
unmigrated callers still exist in those same object files.

Copy publishes the new sentinel/count before reading current source links, then
captures the destination first node from its allocated sentinel. Its catch is
armed only after those captures. Self construction abandons the old graph and
sees the new empty list. Node construction captures its raw CRT allocation for
the catch; the catch frees that node and rethrows without releasing a possibly
partially constructed embedded string. 44BCB0 skips source reads for null
destination, compares header identity before zeroing, and has only the original
RET-only placement-delete cleanup. The actual resize rereads current fields
after allocation before copying/releasing/publishing.

Range insertion retains the native by-value argument storage and current
iterator rereads. Allocation precedes count growth and link publication. It
captures the source owner before publishing links. Returning CRT validation can
change storage; no returning site becomes an unconditional throw or early exit.
Rollback uses source progress, the complete checked previous/compare/next
helpers, and the actual-pool 4D0990 overload. Erase preserves captured output
state, current link rereads, string return before node free, and the current
count decrement after returning free. Clear captures data and next before
returning/freeing, reloads the owner's current sentinel after each free, and
does not strengthen validation. Destroy reloads the sentinel after clear, frees
it, and only then clears owner+4.

The existing complete 4CE780 predicate is unsigned
`uint32_t(0x1fffffff - current_count) < increment`. Its owning length exception
uses the existing actual 28h exception/SBO representation. This packet retains
that provider and the established returning CRT boundary. Host C++ exception
types, vtables and EH machinery are not the original MSVC exception ABI.
The existing actual string adapter's `release` is `noexcept`: failure while a
getter lazily recreates a pool terminates at that source boundary. This packet
does not claim original throwing-getter/SEH parity or make that limitation a
native failure rule.

One ignored focused probe links the current `bsp_core.lib`. Its raw small-list
copy/destruction/reuse sequence composes all seven actual-pool algorithms across
the combined check, including the inlined string-copy body, and confirms returned
small buffers are reused from the
actual owning pool. The differential part executes the original complete
4D26A0 normal/catch body with its FH3 metadata, bridges its dependencies to the
same complete linked actual-pool providers, and compares the rebuilt operation.
A real allocation boundary changes the source sentinel; a post-link invalid
handler then throws. Both executions yield the same 301-word allocation,
ownership, rollback and cleanup trace. The newest copied string/node is erased,
while the earlier copy and preexisting alias remain. All observed CRT
allocations are freed. This is an original-parent/concrete-library composition,
not original machine code for every helper. The historical probe's installed
4D2660 bytes are unused; 4D490C and 4CE75C are static catch proofs, not injected
exceptions in this check. The standard strict Win32 build and both existing
CTest tests pass; all eight existing native seed comparisons match disk.

The full 4CE75C catch is **21 bytes**, ending at exclusive004CE771. A scratch
capture in this task initially requested 20 and failed full-instruction
coverage. The prior committed alias-nodes audit already records the correct
21-byte span; it has no corresponding omission to fix. Fresh guarded Ghidra
inspection still shows the saved `Catch_All@004ce75c` body ending at004CE764,
after the call to returning free. At004CE765 the bytes are `83c404` (ADD ESP,4),
then `6a00 6a00`, then at004CE76C `e814817200` (CALL BF6885 rethrow). The primary
may repair that saved flow/extent after review. The source already preserves
captured-node free followed by rethrow; no source workaround is required.

This packet is a prerequisite, not the cache consumer. Current full native
4D6F70 (162 bytes) establishes the actual 2Ch record copy and calls 4D48A0 at
record+8, but its complete source remains outstanding. B1A4F0 (1,338 bytes),
B1A3C0 append, 4DA180 vector growth, 4D45A0 destruction/cleanup, BECCD0 policy,
and the consumer's current resource virtual+4/+8/+C paths remain separate
named-but-incomplete or unqualified dependencies. Existing B2FC60, B30510 and
4D2660 consumer interfaces remain bound to their existing SizedStoragePool
contracts. A future migration must explicitly select these actual overloads;
this packet does not silently change consumer ownership.

The primary should add the actual-pool overload/domain and new audit reference
to the existing seven reconstruction records without discarding their original
source/ABI evidence. Prior report fields needing a 20-to-21-byte correction:
none (`native_render_resource_alias_nodes_audit.json` already has
`function_evidence[1].owned_catch_region.length=21`,
`end_exclusive=004ce771`, and the free/rethrow instructions). Historical source
and artifact hashes stay historical. Shared CMake, source ledgers, other reports,
and Ghidra metadata are unchanged by this worker.

A separate historical dependency capture in
`native_render_resource_record_construction_audit.json` ends 4D05E0 after
82 bytes, immediately before its terminal RET at004D0632. The exact fields are
`current_native_evidence.spans[16]` and
`primary_integration.verified_evidence.spans[16]`: length82,
end_exclusive004D0632, SHA256 `0f345fdb6274d2d0e8980bee78befb3eece1f9d37637dad0b7aa7b93153e17b6`.
These remain historical partial dependency evidence. The dedicated clear audit
already pins the full83 bytes, SHA256
`5ca7928c4a04fbc82dc7af332a3da235b13ce64f685dd7f5b97ad64fff1ed89a`,
as does this packet. Any primary clarification should identify those old fields
as partial rather than silently changing their historical byte/hash identity.
The current saved clear function already includes the RET; no flow or source
repair is needed for 4D05E0.


## Primary integration

The main strict Win32 build, both existing CTests and eight fresh native seeds
passed. Primary review checked all 63 worker pins, 25 current files and
28 guarded native spans totaling 3,250 bytes. Twelve exact objects from the
frozen actual main library retain the reviewed full code, data and relocation
contents, apart from private scope and debug identities. The unchanged probe
linked that library and reproduced all 301 trace words, including the original
range catch and real owning-pool rollback/reuse behavior. Original node/list
copy catches remain static proofs; seven separate export entries and runtime
code postimages are not claimed. The read-only evidence bundle is
`local/resource_alias_pool_primary/`.

Primary repaired only the returning-free flow at4CE760 and restored the
complete21-byte saved catch through4CE770. Existing comments and names were
preserved, eight annotations saved and refreshed, and seven existing function
records extended with the actual-pool overload/domain. These are overloads of
existing algorithms, so this integration adds zero newly recovered bodies.
The historical82-byte clear captures remain explicitly partial evidence.
Record/cache/vector consumers and gameplay validation remain separate.
