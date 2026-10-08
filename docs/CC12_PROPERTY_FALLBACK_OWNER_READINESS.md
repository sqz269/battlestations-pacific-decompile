# Property fallback and separate pool-owner readiness

Whole Source readiness remains **NOT_READY**. Original `00E177E4` is an initial NUL byte used as a CString fallback. The current hash service accepts its distinct borrowed role explicitly; current Source has no publication/lifetime binding for this cell. It is not an actual eight-byte string header, property record, or profile. Separately, `00E175B0` is the 56-byte raw owner of 20-byte map nodes. Its genuine startup/callback pair is now pinned, and its raw allocator helpers already exist in current Source. The distinct Source bookend/publication for this exact owner is missing; the genuine current shared domain is available for a borrowed interface with explicit lifetime requirements.

This read-only packet owns the two audit files and addresses `00E177E4`, `00E177D4`, `0043E4D0`, `0043B450`, `00E175B0`, `00CC8A30`, `00CD9260`. It makes no Source, Ghidra, ledger, export, config or CMake change. No build, Source/native execution, game invocation or new test was performed. Live queries verified `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`; live bytes mean saved analysis bytes, not a process-memory capture.

## Fallback and neighboring storage

Installed `.data` raw-file byte `00E177E4` is `00` (file offset 10581988). Live saved Ghidra bytes match. Its eight complete indexed direct xrefs are hash `0043B779/0043B784`, sibling hash `0043B717/0043B722`, lookup `0043B8DB/0043B8F3`, and diagnostic consumer `0043B4AA/0043B4CF`. None is an indexed direct writer/startup producer. This does not exclude indirect writes or establish game-time immutability. No real header or default property record follows from neighboring zero bytes.

`00E177D4..00E177E3` is separate diagnostic vector storage. The diagnostic `0043B450` traverses 16-byte float/string entries between `00E177D8` and `00E177DC`, using an entry's string data or the fallback in logging. Its saved listing covers 58 instructions in `0043B450..0043B4F5`; bypassed six-byte alignment at `0043B45A..0043B45F` is `LEA EBX,[EBX+0]`. `0043E4D0` is a timer/formatting consumer, not a fallback initializer: exact instruction `0043E60C` supplies receiver `00E177D4` before the unexpanded `0043E440` call. Its complete saved pseudocode was paged, but x87/overlapping-global warnings prevent any stronger ABI/Source conclusion. No repair or further callee expansion was performed.

Current Source's address search finds only the hash argument/comments for `00E177E4`; no fallback cell owner or publisher is present. Valid NULL-data/zero-length hashing does not dereference the fallback, so that subset cannot establish lookup's genuine fallback lifetime/binding. A new static empty byte, fake string header or profile record would not supply this missing provenance.

## Whole startup and teardown wrappers

| Body | Installed/live extent | Complete instructions | SHA-256 |
|---|---|---|---|
| `00CC8A30` | `00CC8A30..00CC8A45`, 22 bytes | 6; two direct calls, no indirect calls or gaps | `56d28734e8f445ca80ac66f19b480a6feda418430bd63c97f82f2166bbed94a8` |
| `00CD9260` | `00CD9260..00CD9269`, 10 bytes | 2; one direct tail jump, no indirect calls or gaps | `7bae871d18a24016fe4a25599cc71372c5ed50f287fd8b9653bc9b98c3b3599e` |

The complete initializer is:

```asm
00CC8A30 MOV  ECX,00E175B0
00CC8A35 CALL 00411050
00CC8A3A PUSH 00CD9260
00CC8A3F CALL 00BF6FF5 ; Original _atexit
00CC8A44 POP  ECX
00CC8A45 RET
```

It reads no caller arguments, constructs the exact pool owner first, then registers the exact callback. The POP consumes the four-byte cdecl registration argument; EAX retains the real registration result at RET. There is no once guard, failure rollback or alternate owner in this body. Callee exception and lifetime behavior remains the actual helper contract.

The complete callback is:

```asm
00CD9260 MOV ECX,00E175B0
00CD9265 JMP 00410A60
```

It is the zero-argument registered callback, sets the same owner receiver and tail-delegates its original destructor; it adds no stack arguments, guard or result conversion. All eight saved instruction starts were inspected against the whole installed PE bytes.

The genuine initializer pointer lives at `00CE2748`, bytes `30 8a cc 00`, index **5** in the actual C++ initializer array `[00CE2734,00CE36E4)` (1004 pointers, 4016 bytes). Existing verified CRT evidence establishes `__cinit00BFBC47` loading those bounds at `00BFBC90/00BFBC97`, skipping NULL, calling at `00BFBCA7` and advancing four bytes at `00BFBCA9`. The exact slot is pinned in the installed PE and live saved bytes. Callback-pointer xref is the PUSH at `00CC8A3A`. No CRT body or deletion chain was expanded for this audit.

Ghidra currently has **no function object/body** at either wrapper entry, although their instructions and bytes are present. Normal function containment/call-row verification therefore cannot fully qualify these rows. Root must claim the two wrappers after lease release, create the exact function objects under the write lock, annotate/save/refresh and rerun verification. The worker made no analysis mutation; the audit records this as a repair requirement.

## Genuine current Source dependencies

`008F28F0` selects pool `00E175B0` at `008F2930`, as the prior whole insertion audit establishes. Nodes are actual `14h`: owning key at `+0/+4`, mapped property-record pointer at `+8`, bucket link at `+C`, allocator-page identity at `+10`. Pool-owner size `38h` must not be confused with the separate `38h` property-record size.

Current `native_enum_node_pool` provides whole raw `initialize_native_enum_node_pool_00411050(void*, AllocatorListDomain&)` and `destroy_native_enum_node_pool_00410a60(void*, AllocatorListDomain&) noexcept`. They are available dependencies, not historical missing bodies. The constructor requires genuine writable `38h` storage and the same initialized borrowed `00E188B4` shared-head domain; it prepends the base, stamps `00CE37A4`, initializes the real OS recursive critical section, zeroes depth/table/count/capacity, sets cursor `FFFFFFFF`, and obtains the real allocator's initial 80-byte pointer table (capacity 32). Pages are genuine `584h` allocations with 64 actual `14h` slots.

`bind_native_enum_node_pool_virtual0_00ce37a4` supplies the genuine complete `00410CD0` trim binding for that owner in that same domain before construction/publication. `AllocatorListDomain(AllocatorListElement*& shared_head_00e188b4)` borrows the shared head explicitly. Current `singleton_lifetime` allocation/free identities are pinned, with no new allocator-body expansion or execution.

The destructor releases pages/table, unlocks captured depth, deletes the critical section and unlinks from the same live domain. It is not idempotent and does not recursively destroy the owning keys or mapped property records. Their teardown must precede releasing the pool's pages, and the list/domain binding must outlive the callback. Those exact-owner ordering and publication requirements remain unresolved.

Real `std::atexit` is already used by Source, including `gameplay_effect_name_index`, but that semantic map/context does not publish this raw `00E175B0` owner. A sibling pool or semantic container cannot establish the missing instance/domain/lifetime contract. The current separate `native_property_key_bucket_0043b760` whole ordinary Source service is available for its valid ASCII/C-locale borrowed-input contract; it makes no original stdcall, populated-map or game ABI claim.

## Next bounded work and evidence

The genuine current canonical domain is available through `GameNativePhysicalPoolProcess::allocator_list_domain_00e188b4()`: its constructor binds the list to its zero-initialized process-owned shared head, and the accessor is available before physical-pool startup. Current `native_hardware_layout_pool_static` demonstrates the whole borrowed-pool/list binding, construct-then-real-`std::atexit` status/no-rollback and exact matching callback pattern. These are Source identities, not a substitution of that sibling pool for `00E175B0`.

An independent ordinary Source borrowed-bookend packet is therefore ready for Root review once the wrapper function objects are repaired. Use the existing registered `include/bsp/native_enum_node_pool.hpp` and `src/native_enum_node_pool.cpp`, without CMake changes. Provisional interfaces are `bind_static_native_property_node_pool_00e175b0(void*, AllocatorListDomain&)`, `initialize_static_native_property_node_pool_00cc8a30()` returning real registration status, and `destroy_static_native_property_node_pool_00cd9260() noexcept`.

Its whole contract is one explicit binding of genuine aligned, initially unconstructed `38h` owner storage and the same canonical list domain, establishing the actual `00CE37A4` trim binding before publishing borrowed pointers. Always construct, then call real `std::atexit` for the same owner's callback; preserve status, failure/no-rollback and pre-registration exception behavior. No once guard or fabricated global is required. Keep the same owner/list/bindings alive through the matching callback and meet the raw destructor's payload-before-pages preconditions. This is a complete borrowed bookend; actual Source global/startup-dispatcher publication, populated maps and full record lifetime remain outside its claim. The fallback's image/process-lifetime CString binding remains separate.

Actual populated-map production and lookup linkage, mapped-record profile/release, recursive property/key lifetime, constructor/class/EH/factory/world/game behavior and original ABI compatibility stay explicit. No deletion chain or unrelated body was expanded.

The JSON report contains all native byte hashes, exact direct/tail-transfer rows, 19 frozen Source/input hashes and ignored raw-artifact hashes. Selected current Source blobs match worker input `9c61316c1b3a3979ce01ec2d3ce0a601005199a2` and main input `b7d4663c272bcddae6b9fe52e9d9fcc54ee96098`. Installed PE SHA-256 is `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

Raw evidence is under `local/cc12_property_fallback_owner_readiness/`: `byte_and_startup_receipt.json`, six `.bin`/saved-byte pairs, both whole disk listings, `whole_wrapper_instructions.json`, xref receipts, missing-function receipt, diagnostic receiver context, current Source search/input receipt, collector/sealer scripts and validation receipts. Evidence is static/saved-analysis only; Source/native/game execution was not performed.
