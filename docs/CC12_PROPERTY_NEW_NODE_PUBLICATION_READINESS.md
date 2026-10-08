# CC12 property new-node publication readiness

The genuine lookup-miss tail is independently Source-ready on the existing successful raw allocator/string-provider domain. Coverage is **partial**: `[008F2930,008F299C)`, 108 bytes / 38 instructions, SHA256 `5b19bd880704f050dcd2db183fa9cbd68baf265eab0e4e573e45ea728bf3a2e9`. Whole `008F28F0` and `008F33F0` remain **NOT_READY**. This audit adds no Source or runtime fixture.

The boundary is actual control flow: `008F28FF` calls complete lookup; `008F2906` tests its returned node and `008F2908 JZ 008F2930` selects a true miss. EBX preserves the actual query header, EDI the actual 108h map. The lookup writes its bucket into the reused first argument stack cell, not into the query header; incoming mapped word stays in the second cell. This is a real branch cut, not an arbitrary fresh-key projection labelled as whole insertion. Source predecessor input must be the same synchronized actual map/query and freshly produced NULL lookup result/bucket.

Both complete saved bodies were reused and rehashed against current installed PE and fresh read-only verified Ghidra bytes. Lower provider is 172 bytes / 65 instructions, SHA256 `352616a3d3506a800c42a875c8ea4d86ccdeb788250d77a8dcec8abee5520878` (PE offset 5187824); wrapper is 155 bytes / 46 instructions, SHA256 `546af6bc24638da2025274f378467e5e8ef4be515a2e80b0892b4ddc653cdffc` (offset 5190640). The fragment starts at lower-body offset40h. Original lower ABI is ECX actual map, stack actual8h query/incoming mapped DWORD, EAX currentcount, RET8; EBX/ESI/EDI preserved. Its interior branch is not a new Original-callable function/ABI. No x87 or listing repair arose.

| Native sites | Exact miss behavior |
| --- | --- |
| 2930/2935 | Same actual E175B0 pool; call whole004E7C00 for genuine14h slot, allocator+10 preserved. |
| 293A..294F | Nonnull slot zeros only key+0/+4; null selects ESI=NULL. No guard or rollback is added. |
| 2951/2953 | Actual self-header equality skips key assignment; preserve this branch even though valid fresh-slot/query admission is disjoint. |
| 2955..295C | Reload query length; raw resize(actualnode, genuinecontext, length, preserve=true); NativeRET8. |
| 2961..2976 | Reload query length; if nonzero load CURRENT node length/querydata/nodedata; real overlap-capable copy of exactly node length bytes, ADDESP0C. Resize supplies the terminator. |
| 2979/297D | Store caller's opaque mapped record DWORD at node+8, with no record/pointee/profile read or allocation. |
| 2980..298B | Reload actual bucket and CURRENT oldhead; node+C=oldhead before head=newnode. |
| 298F/2993 | Increment map+4 using DWORD arithmetic only after head publication; return CURRENT count. |

Current whole property hash/lookup Source exists: emitted92B41 and195B87 bodies match the qualified hashes. Concrete raw-context resize emits212B85 and calls real Source00419CC0/BD1120/BD1510 with genuine `_memmove`; raw query constructor emits201B83. Current exact E175B0 binding/init/atexit bodies and whole14h allocator are present on the same actual38h pool and canonical E188B4 domain, with genuine CE37A4/410CD0 binding. Actual61B15 bag storage producer establishes the empty map at bag+4; canonical `GameNativeStringProcess::raw_context()` retains the real manager/pool/publication cells and owning query buffer. Sixteen complete selected production COMDATs and actual I386 CRT exports/import library were read and physically retained. No semantic BE0A30 copy helper, NativeString cast, sibling pool, copied header/profile, fake owner or fallback cell is needed or admitted.

The independent caller contract requires genuine live map/query/key-buffer/pool/context provenance, fresh NULL-lookup state and bucket0..63, stable count-matching closed ASCII/C-locale inputs, valid distinct predecessor fallback bindings, unique aligned actual14h slots and successful nonoverflowing allocations/getters. External synchronization prevents mutation/reentry/trim throughout lookup and publication. Source follows the actual self-copy/null-slot branches and rereads fields after resize. If node allocation returns NULL, Original attempts assignment/publication through NULL; key-allocation faults or thrown getters occur before mapped/head/count publication, after the node allocation. Existing provider admission excludes those failures and Native SEH/rollback parity. A proposed Source fragment must add no recovery or early-success/default behavior.

The copied node key owns a separate len+1 buffer; it never borrows the query's data pointer. Caller must retain map, node, key, same pool/domain/bindings and raw context through every consumer, then release the owning key through genuine raw0041DD20 before slot return or pool page destruction. This contract supplies no whole map cleanup owner. The incoming mapped word is only stored: any pointee's independent owner retains it for consumers. The new type1 float storage is therefore unnecessary to this branch and was not expanded. Publishing a word does not prove record ownership, scalar dispatcher, profile/class/factory or lifetime. The outer wrapper still has its own temporary-key FH3 cleanup, record+30/34 stamps and bag+10C ordinal; none are included here. Predecessor E177E4/E17654 CString fallback roles stay separate from node-owned key storage and mapped record.

After Root accepts this audit, the smallest implementation packet can append a distinctly labelled partial service in existing `include/bsp/native_enum_dictionary_lookup.hpp` / `src/native_enum_dictionary_lookup.cpp`, with its own Source doc/report and no CMake change:

```cpp
std::uint32_t publish_native_property_map_new_node_008f2930_fragment(
    void* actual_map, const void* actual_query_header,
    std::uint32_t lookup_bucket, std::uint32_t incoming_mapped_record_word,
    void* actual_initialized_property_pool_00e175b0,
    NativeStringRawPoolContext& actual_key_context);
```

It consumes genuinely fresh miss state and returns map count, calls the genuine same-pool allocator and concrete raw resize, performs exact byte copy and raw mapped/link/head/count stores, and leaves enum/hash/lookup implementations intact. No missing direct child remains inside this admitted fragment. Whole insertion first still needs the real old-record profile-slot0 call at `008F2917` and complete scalar/release chain `004E6730 ->008F0DE0 ->008F0640`; capped current ledger/status checks confirm no whole Source closure. Current scalar audit remains blocked, and raw float storage does not resolve it. Map cleanup, replacement, outer-record publication, app startup, class/world/game and Native ABI remain separate. Any future qualification requires separate Source authorization; this audit ran no build/test/runtime/SDK, replay or fabricated map fixture.

Evidence: `local/cc12_property_new_node_publication_readiness`. Forty-two selected Source/header/report/whole-object/CRT inputs have actual retained copy paths and hashes; current hash/lookup Source matches its retained compiled inputs. Main/worker Git identities are recorded separately from physical retention. Complete native/COFF bytes/listings, direct-call rows and the corrected38-instruction count are pinned; initial count and wrong-target-object-path collector failures are preserved. No full archive/exhaustive compiler-input retention or new compilation claim is made by this read-only audit.
