# Locale node-pool lifecycle contract and Source readiness

The admitted bytes establish the ordinary initialization/destruction schedules
at `00436230` and `00435A50`. Earlier entry prefixes select the **same actual
`00E1768C` owner** used by the accepted node-slot return wrapper. Construction
publishes that owner's intrusive element in the shared `00E188B4` list before
initializing its section and table. Destruction releases slabs and table before
section teardown and unlinking that same element.

This closes the bounded normal byte-flow contract, not exceptional lifetime,
profile dispatch or canonical Source activation. `00C5EEBE` unwinding and the
`00CE4120` profile's virtual-zero operation remain unread. **Implementation,
raw-map composition and application GO remain false.** No C++ is added.

The ten fresh read-only `bsp.py ghidra` commands verify the existing
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`. Complete typed captures
are frozen at modification 72. Two pseudocode reads precede stored-range
assembly inspection, two admitted hull reads, and exact metadata inspection.
The two additional callee slots are used only for the five-byte `00BF6989` and
`00BF65AC` free-thunk metadata; no callee bodies are read. No predecessor,
caller, unrelated owner, startup or registration expansion occurs.

| Entry | Stored owned ranges, inclusive | Stored bytes / instructions | Admitted hull / decoded instructions |
| --- | --- | ---: | ---: |
| `00436230` | `436230..4362E8`; `4362EC..436302` | 208 / 67 | 211 / 68 |
| `00435A50` | `435A50..435A6C`; `435A78..435A84`; `435A88..435AD9` | 124 / 46 | 138 / 51 |

All 349 hull bytes match the frozen original PE copy, SHA-256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`,
length 12,223,752. The installed file is not reopened. The 332 stored bytes and
17 unowned continuation bytes stay distinct; the latter are instructions, not
padding:

```text
004362E9  83 C4 04        ADD ESP,4
00435A6D  83 C6 01        ADD ESI,1
00435A70  83 C4 04        ADD ESP,4
00435A73  3B 77 2C        CMP ESI,[EDI+2C]
00435A76  72 E9           JB 00435A61
00435A85  83 C4 04        ADD ESP,4
```

The three preceding calls at `4362E4`, `435A68`, `435A80` retain `CALL_RETURN`
overrides and effective `CALL_TERMINATOR`, with no stored fallthrough. Both
free thunks themselves have current `no_return=false` and stored cdecl
`void(void*)` signatures. Those facts are separate. Stored listing omits the
continuations and pseudocode incorrectly terminates at free; its initializer
`return extraout_EAX` is not accepted as the return ABI. No function, body,
listing, flow, prototype or no-return repair is performed.

Offline analysis admits the exact in-hull continuation flow: after a returning
free, `4362E4 -> 4362E9 -> 4362EC` publishes the replacement; `435A68 -> 435A6D`
advances the slab loop and either branches to `435A61` or reaches `435A78`;
`435A80 -> 435A85 -> 435A88` continues section teardown. All direct jump targets
are in their respective hulls. All 68/51 instruction starts are syntactically
reachable in that graph, conditional on external calls returning normally;
this is neither path execution nor proof of external return/failure policy.

Both routines take the actual receiver in ECX, have no stacked arguments and
end in plain `RET`. Initializer normal return is EAX=the original receiver;
EBX/ESI/EDI are saved/restored and EBP is untouched. Destructor saves/restores
ESI/EDI and conditionally EBX around its depth-drain loop; EBP is untouched.
It establishes no meaningful EAX result. Prior entry-readiness prototype
metadata is `undefined FUN_...(void)` and is not normalized here. These
observations do not create a binary-compatible
C++ replacement or close SEH, unwind or fault behavior.

Initializer field order is exact, with at least `38h` bytes required:

| Order | Actual operation |
| --- | --- |
| 1 | Install FS handler `00C5EEBE`, state -1; retain the receiver for cleanup. |
| 2 | Store base profile `00D7A0C0` at +00, previous=0 at +04, current global head at +08; update current old head's previous if nonnull, then publish this element to `00E188B4`. |
| 3 | Store state0 and concrete profile `00CE4120`; call the current `InitializeCriticalSection` import at `00CE220C` with owner+0C. |
| 4 | After that call returns, write depth +24=0, table +28=0, unsigned count +2C=0, capacity +30=0, minimum +34=FFFFFFFF. Store state2 after the capacity comparison; no state1 store appears in this body. |
| 5 | If current unsigned capacity is below 32, publish +30=32 before pushing 80h and calling `00BF55BE`; capture returned EAX and caller-clean4. |
| 6 | Copy against CURRENT unsigned count, reloading CURRENT old table for each read. Skip a store at a null current destination, then advance the captured destination by4. |
| 7 | Pass nonnull CURRENT old backing to `00BF6989`; only after normal return publish the captured replacement at +28. Restore prior FS head and return the original owner. |

The ordinary fresh, non-reentrant path reserves 32 pointer cells and leaves
count zero and minimum FFFFFFFF. It allocates no slab and constructs no node;
the fresh count-zero path performs no explicit table-cell initialization.
The copy/free branches still exist and retain their reload order; they are not
removed by assuming that the allocator cannot affect observable storage.

The admitted call boundary is one DWORD size80h to `00BF55BE`, with the pointer
returned in EAX. Prior finite CRT evidence identifies its route to `00BF681B`,
and both free spellings converge through `00BF65AC` to `00BF9DC8`. The slab path
calls `00BF65AC`, the pointer-table path `00BF6989`; neither result is consumed.
These routes do not establish `new[]`/`delete[]`, ultimate heap/failure policy,
exception identity or permission to mix native and host allocation domains.

Destructor captures the initial unsigned count comparison before stamping
CE4120. For each ascending index, it reloads CURRENT table, passes the current
entry (including null) to BF65AC, and increments the index only after normal
return. It caller-cleans4 and reloads CURRENT count for the unsigned loop test.
It then captures CURRENT backing and frees it through BF6989 when nonnull.

Only afterward does it drain depth while its signed value is positive. Each
iteration decrements CURRENT depth before calling `LeaveCriticalSection`, then
reloads depth. The import address from `00CE2210` is captured once in EBX for
that loop. Zero/negative depth skips the drain; deletion still follows through
the current `DeleteCriticalSection` import cell `00CE2214`. There is no Enter.

Finally it captures previous +04 before writing base D7A0C0, updates
previous.next or the actual E188B4 head, and updates next.previous when needed.
It does not clear its own links, table/count/capacity/minimum, invoke node
payload destructors, or free physical owner storage. It installs no local
FS/EH frame or rollback. Payloads and borrowers must finish before slab release;
repeated destruction is not admitted by stale metadata.

Failure timing remains material. Before section initialization, this element
is already globally visible with state0. At the allocation call, its section
has initialized, count/table are zero, minimum is FFFFFFFF, capacity is already
32 and state2 is active. A failure after obtaining the replacement but before
publication can leave that allocation only in a register/local execution state.
The handler's unwind map is not inferred from the similar sibling Source code.
An escaping destructor call prevents later normal cleanup; a depth-release
failure occurs after decrementing the depth. No retry, rollback, leak outcome
or exceptional post-state is claimed without the out-of-hull handler contracts.

The prior 22-byte initialization prefix and 10-byte destruction tail select
E1768C for these exact routines. The earlier 193-byte node-return proof uses
that same base, section +0C, depth +24, table +28 and minimum +34. Its slot stride
1Ch, identity at slot+18h, free-index stack at block+7000h and WORD count at
block+7800h remain separate evidence. This constructor does not establish slab
allocation size, slot-construction identity, capacity or trimming semantics.
The old entry windows and provider/map bytes remain separate epochs: 730 reused
bytes in this package, not fresh Native reads. No CRT registration execution or
complete startup routine is inferred from the earlier prefixes.

Current Source offers useful services, but no locale owner substitution:

| Source service | Readiness and limit |
| --- | --- |
| `AllocatorListDomain` | Implements the evidenced base-link schedules. Borrow the genuine existing E188B4 domain exposed by `game_native_physical_pool_process().allocator_list_domain_00e188b4()`; do not create another head. |
| `singleton_lifetime_allocate/free` | Existing matched host malloc/new-handler/free domain. Preserve requested80h and the two native free boundary identities; foreign providers require matching frees. Native failure parity remains separate. |
| `native_node_pool_owner` | Similar schedule, but actual 0108FF58, profile D62C78, 174h payload/178h stride and 32 slots. Its trim and FH3 maps are not CE4120/C5EEBE evidence. |
| `native_enum_node_pool` / `NativeWeakHandlePool` | Other concrete profiles and slot formats, not a generic locale lifecycle binding. Reusing either owner would change identity and behavior. |

The smallest justified dormant design is a borrowed locale-pool context carrying
one actual aligned owner buffer of at least 38h bytes, that SAME list domain,
and a matched allocation/free service set. Ordinary raw initialize/destroy and
slot-return helpers must share that exact buffer and current fields. They own
no second table, global head or payload, and perform no startup or registration.
Successful ordinary paths can be specified now; activating an owner remains
blocked by CE4120's genuine virtual-zero binding, C5EEBE unwinding, canonical
E1768C storage lifetime, slab-allocation/identity production and payload-before-
pool teardown ordering. The host domain's virtual-zero map is bookkeeping;
a placeholder trim callback would not satisfy the Native profile. Startup,
list traversal and destruction require appropriate external serialization.

The frozen Source commit is `6285c5af606dde35623499f8ff81f84c0643ffd7`.
All 4,215 inputs equal the preceding entry packet; all 4,215 actual Git blobs
also equal the accepted `Source3354-particle-admitted-constructor` Git cohort.
Of its 3,354 retained selected files, 3,348 are exact and six differ only by line
endings. The 4,208 tracked include/src files contain no literal entry, E1768C
or CE4120 spellings; this finite search does not exclude semantic equivalents.
Accepted particle build/artifact metadata stays historical and separate. No
current artifact read, build, fixture, test, Native execution or gameplay occurs.

The evidence ZIP is `local/cc12_locale_node_pool_lifecycle_contract/evidence.zip`,
21,454,497 bytes, SHA-256
`2ac9681ad36c883276831391458a30e72bddbd85777966f4fbbfddd0ee607def`.
Its 4,273 payload files plus manifest retain current/accepted Source, 12 actual
dependency blobs, typed captures, raw hulls, full decodes, gap ownership and
the explicit normal-flow graph. Guarded replay passes on the original and a
physically copied ZIP; after verifier loading, only read-only ZIP access is
allowed, with outside files/directories, process, network and dynamic loads
denied. It verifies all byte/hash/Source/flow assertions without running Native
code. Earlier closed proof packages remain unchanged. The next bounded frontier
is CE4120's first cell/target metadata and C5EEBE handler/unwind readiness;
neither is silently admitted here. Ghidra annotations and ledger decisions stay
with the primary integrator.
