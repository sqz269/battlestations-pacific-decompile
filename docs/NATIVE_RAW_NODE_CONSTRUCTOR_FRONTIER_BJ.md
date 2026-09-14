# Raw child constructor and composition frontier BJ

Addresses: `00BEA250`, `00BEA680`; allocation providers `00BF681B`, `00BF65AC`, `00BF9DC8`, `00BF9F1A`, `00C055B1`, `00C04FDE`, `00BF6802`, `00BF63A6`, `00BF6885`, `00BF9E1E`. Release bodies `00BE9ED0`/`00BE9FC0` were captured before handing their lease to the independent release reviewer.

This is discovery only at base `fe18a7d83cf6dd44849b890097fee03fff55460e`. No source, build, probe, Ghidra mutation, or runtime validation was performed. The report retains 26 complete PE/live spans and every call in the examined bodies: 47 direct/tail rows pass the native checker; 14 indirect rows remain separately qualified. Native ABI, FH3 exception identity, asynchronous faults, original CRT state and gameplay parity are unclaimed.

## Smallest qualified source packet

The two-body `BEA250`/`BEA680` packet has a recovered source schedule. It needs explicit context-bearing Win32 interfaces, raw writable 24h node storage, fresh 4-byte output wrapper storage, valid parent/reader/path storage and the actual raw string pool context. No typed `StructuredNode`, `std::string`, `unique_ptr`, full-read guard, invented virtual reader or fabricated allocator can replace these contracts.

Baseline includes raw scalar `BF0280`, raw string getter/resize/return/destructor and base `BD30F0`. Raw name `BF0510` is absent from this baseline; its qualified five-body source is pinned at external commit `e956fc7ac39f1b79084e998da51146e5ac8422f8`, with source/header SHA256/SHA512 and Git blobs retained. Integrate/review that dependency before claiming constructor source closure. Its stream-profile and actual-count qualifications carry through here.

For a qualified source projection, existing `singleton_lifetime_allocate`/`singleton_lifetime_free` can supply matched host CRT allocation/free with `host_bytes=native_bytes=24h`. Their outer malloc/new-handler/throw shape is useful; it does not reproduce native heap mode, inner malloc newmode retry, original encoded handler, static bad_alloc object or original FH3 machinery. An exact native allocation claim remains blocked by those providers. Source implementation still requires primary approval.

## BEA250 field and call order

Native ECX is allocated node storage; stack contains parent-node pointer; `RET4`, EAX returns node. The constructor writes, in order: profile `CEB130`; refcount `+4=1`; profile `D68BB4`; reader `+8=parent[8]`; parent `+C`; zero name length/data `+10/+14`; depth `+18=parent[18]+1` with 32-bit wrap. Reader/parent pointers are not retained or checked.

`BEA2BC` calls `BF0510(reader, &local8h, &parent[20h])`. Inside the name provider, BE4620 publishes the actual byte count before constructing its output string. BF0510 debits the parent budget only after BE4620 returns. BEA250 then copies that returned temporary into the node name; no full-read rejection is added. The returned header pointer is captured separately from the local temporary header. If it differs from node name, `BEA2D4` resizes node name using returned length and flag1. The subsequent zero guard reloads returned length; `BEA2EA` copies current node-name length from current returned data into current node-name data. Preserve these reloads and alias-address comparison.

The actual local temporary data/length are then loaded. State2 is disarmed to1 before temporary cleanup: if data is nonnull, pending arguments are `[data,length+1,1]`; `BEA30A` calls real pool getter with no arguments, then `BEA311` returns the block through `BD1510`. Getter failure must not retry this temporary cleanup.

The constructor reloads parent and reader; `BEA320` calls `BF0280(reader,&parent[20h])`. EAX is published first to declared payload `+1C`, then remaining payload `+20`. It reloads reader, captures index `[reader+60h]`, computes path-header address `reader+10h+8*index`, increments and publishes index **before** path-name allocation. Distinct path/name headers cause resize at `BEA348` and copy at `BEA35E`, with current node-name length guard and current path length/data reloads. The path must already designate valid initialized raw8h string storage; this body proves neither its capacity nor the root reader's allocation.

There is no range/remaining check, whole-object zeroing, refcount increment, payload seek, or rollback of parent budget/path index. Copy helper `BF7680` uses existing overlap-compatible raw string source semantics; its already retained body is an external contract, not newly reconstructed here.

## Construction unwind and allocating wrapper

BEA250 handler `CC716B` points to FH3 information `E01B94`; unwind map `E01B7C` gives state2 -> local temporary destruction `CC7163`, state1 -> node-name destruction `CC7158`, state0 -> base stamp `CC7150` / `BD30F0`. Base cleanup is not `BE9DF0`: no reader path decrement, parent payload debit or general node destructor occurs during constructor failure. Before name-reader return only node name/base belong to this constructor; the name provider owns its internal failure cleanup. Later path mutation is not rolled back. Exact native FH3 scheduling is documented; a future source cleanup implementation must explicitly qualify compiler exception semantics.

BEA680 takes ECX parent-wrapper and one stack output-wrapper pointer, `RET4`, EAX output address. `BEA6A3` calls `BF681B(24h)` with caller cleanup4. It saves allocation for unwind, then reloads `[parent-wrapper]` **after allocation/new-handler callbacks**, calls BEA250 at `BEA6C0`, and publishes the returned node into output. Its explicit null-allocation branch publishes zero even though inspected BF681B normally returns nonnull or throws. It never releases an existing output pointer.

Handler `CC71FB`, information `E01C64`, map `E01C5C` own the allocation at state0. `CC71F0` passes saved allocation to BF65AC on construction unwind. Constructor member/base cleanup runs first; the wrapper then frees storage. Calling BE9DF0 in place of this schedule would add observable debit/path mutations.

## Allocation and deletion obligations

BF681B repeatedly invokes native BF9F1A; after null it calls C055B1(original size) and retries on nonzero. On refusal it initializes static exception storage `0109DD68` under bit1 of `0109DD74`, calls BF6FF5 with `CE1122`, copies through BF63A6, stamps `D6923C`, and throws via BF6885 / RaiseException. BF6FF5/BF638E/C03DE0/BF93A4 bodies remain address-named unread contracts in this bounded packet. E154B4 is pointer storage, not an inline message literal.

BF9F1A rejects unsigned sizes above FFFFFFE0h, invokes C055B1 once on that branch, writes error12 and returns zero. Normal mode1 uses HeapAlloc with max(size,1); mode3 tries BF9E56 first; fallback rounds max(size,1) to16 bytes. Heap is `0109E1BC`, mode `0109ED7C`; null heap enters unread native failure helpers. Allocation failure can invoke an **inner** C055B1 retry when `0109E314` is nonzero. Error-cell helpers remain unread. C055B1 reads encoded `0109DE44`, decodes through C04FDE, then invokes that runtime handler. C04FDE uses TlsGetValue and a runtime callback/table or GetModuleHandleA/GetProcAddress DecodePointer; current callback/handler addresses are not established by static PE data.

The baseline does contain qualified `native_crt_decode_pointer_00c04fde` source. Its borrowed context requires the current TLS indices, actual TlsGetValue IAT word, real getter/PTD storage and module-gate context; it neither creates these providers nor establishes the owning allocation/new-handler state. Its outgoing argument-slot write and additional context argument are explicitly documented. Source/header hashes are retained. This is a reusable dependency, not a native CRT ownership closure.

BF65AC tail-jumps BF9DC8. Native free enters SEH4, optionally locks mode3 storage through C11C21(4), resolves/frees through C11D3D/C11D68, unlocks through separately bounded BF9E1E -> C11B31(4), otherwise calls HeapFree(original heap,0,pointer). Failure uses GetLastError/BFFB50 and the native error cell. Those backend bodies/state are concrete unresolved native dependencies; host std::free is not proof of original heap identity.

D68BB4 contains slot0=BD30E0 and slot4=BE9FC0. BE9ED0 captures wrapper pointer, InterlockedDecrement(node+4), dispatches current slot0 only on exactly zero, and clears wrapper only after dispatch returns. Negative nonzero counts still clear without deletion; an exception leaves wrapper uncleared. BD30E0 reloads current slot4 and passes deleting flag1. BE9FC0 calls BE9DF0 then BF65AC only when flag&1, returns original owner, RET4. Existing `NativeRefCountedDeleteCalls` is a dispatch interface, not concrete D68BB4 resolver closure. Follow-on release worker reviews these now-unleased bodies independently.

## B7EB90 composition remains open

At this pinned baseline B7EB90, B936E0, B932E0 and B93310 remain typed StructuredNode projections; raw detach BE9C40/BF03E0 and raw release/deleting bodies are absent. Raw B7D220 sphere bounds and raw scalar/destructor/predicate bodies are useful dependencies but do not replace those typed consumers or prove root reader/path provider storage. Later commits may close individual gaps; this report makes no readiness claim for unreviewed later source. A faithful producer packet must explicitly assemble reviewed raw consumers, constructor, detach, release, exact output storage and append/unwind obligations.

Evidence lives in `local/raw_node_constructor_bj`; the report carries a whole-directory size/SHA256/SHA512 inventory outside that directory, audited twice. Failed orientation/read attempts are recorded rather than erased. No original installation or prior worktree evidence was changed.
