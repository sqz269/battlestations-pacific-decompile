# Actual resource-loading frontier BN

Read-only packet `orch4_native_resource_loading_frontier_bn`, worker baseline
`eaec86e6356471a43f2cc9bb84c976b84bb26d60`. The
[report](../reports/native_resource_loading_frontier_bn.json) records fresh
installed-image/live-Ghidra spans, native call instructions, source hashes,
current contracts and proposed disjoint packets. Nothing here implements,
dispatches, tests, installs or binds a resource loader.

The game factory getter and creator are already actual-storage source. The
small ready work is the resource-cache lookup, tree links/iterator, and node
construction. The complete `MarkerClassHost::load_and_cache_resource_00b80720`
boundary remains blocked by actual manager/cache lifetime, structured
reader/root dispatch, the current metric service and successful-resource
destruction. Existing typed reader/model/manager interfaces do not supply those
native layouts or their reentrant current-field schedule.

## Worker and current-publication contract

The primary's [loading-thread audit](NATIVE_LOADING_THREAD_LIFETIME_BL.md)
establishes that `501510` constructs an owning name from the initially read
work C string, then recaptures the **current job/work** before `7188A0` and
stores EAX into that captured record's `+0C`. Preserve this ordering when this
frontier is eventually bound. Completion/reset/wait is not stop/wake/join;
original `ExitProcess` after event/context destruction does not prove a safely
joined worker. This packet does not repeat or extend that lifetime audit.

`7188A0..7188BB` saves its ECX name, obtains factory `7175D0`, then manager
`4C1400`, pushes the captured factory and name, calls `B80720`, and passes EAX
through. Incoming EAX is unused. Existing `marker_classes.cpp` matches that
outer schedule but delegates all three operations to its host. Actual factory
getter source borrows `E19B90` and lifetime publication `01090AA0`; it is an
8h object with primary `CFD850`, whose create slot+4 is `71B870`. The creator
and `71B810/B88260` construction already produce an actual 74h resource with
refcount1, owning name and profile `CFD8CC`.

`4C1400` still lacks an actual source owner despite its reconstruction-ledger
entry. Current `GameVfsHost` returns a `GameResourceManager` containing only a
typed parser registry. Native `4C1400` instead locks the captured actual
lifetime-manager section, rechecks `010901C4`, allocates 28h, calls `B81040`,
publishes, reacquires the lifetime manager, reloads publication, and registers
it. The fast return uses its first publication read; the slow return reloads.

| Native manager storage | Contract |
|---|---|
| +0 | D63128 profile |
| +4 | Separate 4h default factory, D63060; not the game factory |
| +8/+0C/+10 | Actual parser tree allocator word/head/count |
| +14/+18/+1C | Actual resource-name cache allocator word/head/count |
| +20/+24 | Mutable current factory/resource, untouched by constructor |

`B81040` leaves allocator words untouched, allocates both sentinels, writes nil
byte+19 and self links, clears counts, then registers the six built-in parsers
in the existing audited order. Their actual getters, `B80A50` registration,
parser-tree storage and owner destruction/EH remain dependencies; constructing
an empty substitute owner would not close this contract.

## Complete B80720 body schedule

The 811-byte body is ECX manager, stack mutable name/factory, RET8, EAX resource.
Its decompiler output contains invalid stack expressions and an unreachable
warning at `B80986`; the full listing, including that reachable cleanup branch,
is the evidence. Current flow inspection reports no gaps in this body.

1. Store incoming factory to manager+20 **before** cache lookup. `B7E7B0`
   probes the original header using tree+14. Validate the returned owner with
   the existing returning CRT boundary. On hit, publish node+14 to manager+24,
   increment that resource+4, reload manager+24 and return. The hit leaves +20.
2. On miss, call current `0109CEFC` virtual+4 with no arguments and save EAX.
   Then read **current manager+20**, invoke its current virtual+4 with the
   original name header, and publish the result to manager+24. A callback can
   replace the field before it is read; do not substitute the wrapper's earlier
   captured factory at this call site.
3. Construct a separate owning name from the current requested header. Resolve
   it through current `0109CEEC -> BDF4C0`; ignore AL. Reacquire current
   `0109CEEC` and open via virtual+4 with flags2 and that resolved name. No new
   null/failure guard appears before subsequent stream use.
4. Construct actual 70h reader `BEA150` at steady ESP+4C, attach the captured
   stream through `BF0430`, copy resolved name into reader+68/+6C, then release
   the opening stream reference. Reader attachment publishes the new pointer,
   increments new+4 before releasing old+4, and leaves same-pointer assignment
   alone. The read-only audit does not replace this with `shared_ptr` ownership.
5. Create the actual root handle through `BEA700`, then dispatch `B7F430`
   against the actual captured manager. Root traversal uses current renderer
   publication `F8D394` virtual+50/+54, parser tree+8, and current resource+24;
   typed `StructuredModelDispatchHooks` explicitly exclude those reentrant
   manager/publication semantics. Root creation depends on the actual 24h node
   constructor, byte-count debit and stream virtual+48/+34 contracts.
6. Capture **current manager+24** after dispatch. Copy the current original
   requested name into a by-value header; `B7F290` consumes it into an owning
   0Ch name/raw-pointer pair. Copy that pair to another actual local and call
   `B803B0`. Ignore its iterator/inserted byte, then release both local names.
7. Clear manager+20. Reacquire current `0109CEFC`, call virtual+4, subtract its
   result from the earlier metric with DWORD arithmetic, and write current
   resource+40. Capture current manager+24 in ESI **before** releasing root,
   reader and resolved name; return that captured pointer after normal cleanup.

There is no manager lock, TLS selection, current-field save/restore or task
queue in this body. This is a source-schedule observation, not a proof that
simultaneous or recursive loads are safe. `0109CEFC`'s currently reached metric
implementation remains unknown. `BE2750` stamps transient base D685E0 and
publishes it, but that base's slot+4 is deleting destructor `BE28E0`; its table
does not establish the later derived object's metric ABI. Do not supply zero
or reinterpret that transient method as a callable metric implementation.

## Cache and release ownership

Cache nodes are 1Ch: links+0/+4/+8, owned name length/data+C/+10, borrowed raw
resource+14, color+18 and nil+19. Ordering treats stored length0 as empty,
otherwise uses the actual `443D00`/CRT case-insensitive comparison without a
length tie-break. Existing B19B90/B19D60 helpers concern other addresses;
their similar storage does not make B7DFA0/B7E7B0 source-complete.

Unique insertion neither retains nor releases the resource. An equivalent key
returns the existing iterator and byte0, without replacing its pointer. Since
`B80720` ignores that result, a resource created after an earlier miss is still
returned even if insertion now encounters an equivalent name. The old mapping
remains. The new factory refcount1 belongs to the load caller; a cache hit adds
another caller reference. Cache teardown owns keys/nodes, not mapped resources.

`483850..48387A` is a ready 43-byte reference-slot leaf: capture the slot and
its pointee; if nonnull, interlocked-decrement captured resource+4, invoke its
**current virtual0** only at zero, then clear the captured slot only after the
call returns normally. Return the slot in EAX, RET. A terminal callback that
changes the slot is followed by the same native clear; a throwing callback
skips it. No seek, retain, null-slot guard or implicit RAII teardown is added.

For CFD8CC the terminal is `BD30E0`, which dispatches current virtual+4 with
flags1, reaching missing `718C20 -> 718810 -> B88430` source. `B88430` releases
primary items, then reacquires `4C1400` and erases cache by its own name through
`B801C0`; that operation checks no expected mapped-resource pointer. It then
destroys hierarchy records, returns actual pool slots and destroys arrays/name.
Actual hierarchy field/pool-return helpers now exist, but cache erase, complete
base-resource EH and concrete item deletion still gate the full resource owner.
The slot leaf can be reconstructed against an explicit actual terminal-dispatch
contract; doing so alone does not make game-resource deletion production-ready.

## Exception and packet boundaries

B80720 FH3 handler `CC21A4`, FuncInfo `DFB3BC`, map `DFB3E0` has states0..4,
no try blocks. At steady ESP S, reconstructed handler-frame F=S+C8:

| State -> next | Action | Local |
|---|---|---|
| 0 -> -1 | CC2170 -> 41DD20 | resolved name S+10 = F-B8 |
| 1 -> 0 | CC217B -> BE9F10 | reader S+4C = F-7C |
| 2 -> 1 | CC2183 -> BE9ED0 | root S+24 = F-A4 |
| 3 -> 2 | CC218E -> B7E8F0 | first pair S+34 = F-94 |
| 4 -> 3 | CC2199 -> B7E910 | second pair S+18 = F-B0 |

There is no resource-release, current-field reset or stream-opening-reference
action in this map. Preserve completed-temporary arming and each dependency's
own EH; a generic scope rollback would invent behavior. B7E8F0/B7E910 destroy
only their current key header through the actual pool, not the resource value.

The report's packet boundaries give disjoint addresses/files; this audit
dispatches none. The primary separately dispatched the lookup packet during
the audit, so its lease and source status must be refreshed before assignment:

| Packet | Addresses | Readiness |
|---|---|---|
| Actual cache lookup | B7DFA0, B7E7B0 | Ready; existing actual443D00 and returning CRT |
| Cache links/iterator | B7CBD0, B7D5F0, B7CDF0 | Ready; raw rotations and predecessor only |
| Cache node construction | B7F220, B7F6A0; catch B7F712..B7F726 | Ready with exact raw-pool/EH contract |
| Resource reference slot | 483850 | Ready leaf; concrete zero-reference resource owner remains gated |
| Reader attachment leaves | BF09A0, BF0430, BE9ED0 | Ready leaves with concrete stream/node terminal dispatch; full reader teardown is separate |
| Cache insertion | B7FF80, B803B0 | After node and link packets; preserve owning native length-error transport |

The node constructor compares key/source identity before writing links/clearing
key, reloads source fields after resize, stores raw value and color only after
copy, and adds no reference. Its allocator catches failures while constructing
the 1Ch node. State1 unwind `CC20D0` calls RET-only `401130`; the separate
catch at `B7F712` frees captured outer allocation `[EBP-14]` through BF65AC,
then rethrows `BF6885(0,0)`. There is no added partial-key cleanup. Decoded
continuation `B7F71B..B7F726` currently has no function membership after the
listed free call. Record/attach that continuation under coordinated repair
later; this packet applies no repair or no-return edit.

Complete reader owners require the optional-buffer `BF0700/BF05D0` dependency,
ten-string array EH and concrete stream deletion. Root/node dispatch additionally
requires actual debit/read, child/skip and parser/resource-method contracts.
Actual manager construction/registration/lifetime and successful-resource
destruction remain distinct deferred packets. The full load composition is
last: borrow the actual publications and stable acquired VFS/reader frames,
preserve each current-field reread, and retain failures without substituting a
host cache, projected vector, typed model or invented completion protocol.

No C++ change, build, fixture, executable invocation, tests, source registration,
ledger update, Ghidra mutation or game-state change was performed in BN.

## Primary call-row correction

The whole-report verifier also examines the separately recorded EH/tail edges. Eight JMP rows now explicitly declare tail transfers, and four unwind rows identify their separate native funclets. The rethrow at B7F722 remains decoded-only because it has no function membership; its bytes/target and unresolved repair are preserved. The original worker report is retained as a hashed local preimage. These are evidence-schema corrections; no source behavior or Ghidra body changed.
