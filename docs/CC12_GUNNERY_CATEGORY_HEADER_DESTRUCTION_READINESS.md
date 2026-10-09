# Gunnery category header destruction readiness

Primary acceptance: Accepted as physical-byte evidence with the complete live listing gate still failed. The primary independently decoded all 75 bytes / 30 physical instructions; the worker capture contains only 27 defined starts and explicitly omits three loop-tail instructions. Current-memory link/count rereads and release order are retained requirements. Exact flow flags and actual gunnery allocation provenance remain open; no destructor Source admission is made.

Worker-capture pins below remain immutable capture context. Current primary document identity is recorded separately in the report. This packet adds no Source, ABI, startup or gameplay credit.

Packet: `cc12_gunnery_category_header_destruction_readiness`.

The 5-byte destructor thunk at `00957080` jumps directly to the 70-byte clear routine at `00955EB0`. The original bytes describe a count-controlled unlink/free loop. The current Ghidra listing omits three instructions after the free call, so the complete saved/live instruction gate is **not passed**. Both routines remain Source-held in this packet; no new Source-ready destructor or prerequisite is admitted.

## Byte evidence and listing defect

Verified CLI queries used project `C:/Users/sqz269/bsp.gpr` and program `/battlestationspacific.exe`. The observed snapshot has 64,730 functions, 64,288 internal. The original image SHA-256 is `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

| Entry | Original range | Bytes / physical instructions | Current defined live starts |
| --- | --- | --- | --- |
| `00957080` | `00957080..00957084` | 5 / 1 | 1 |
| `00955EB0` | `00955EB0..00955EF5` | 70 / 29 | 26 |

All 75 PE bytes agree with live bytes and fresh saved byte captures. All 27 defined live starts agree with the PE decoding and fresh saved defined-instruction listings. There was no prior standalone function export for these entries. The remaining three physical starts are explicitly absent from the live instruction listing:

| Address | Original bytes | Physical decoding |
| --- | --- | --- |
| `00955EEC` | `83 C4 04` | `ADD ESP,4` |
| `00955EEF` | `83 3E 00` | `CMP DWORD PTR [ESI],0` |
| `00955EF2` | `75 C4` | `JNE 00955EB8` |

The eight-byte gap is immediately after `CALL 00BF65AC` at `00955EE7`. The decompiler displays one unlink/free followed by a return. The raw bytes instead perform caller stack cleanup, reread the current count and branch backward before the real POP ESI/RET at `00955EF4/F5`.

The read-only flow-property CLI retained this server response: `Script execution disabled. Set GHIDRA_MCP_ALLOW_SCRIPTS=1 ... to enable.` It returned no property rows. No setting was enabled, access bypass attempted or write performed. The exact callee no-return / call-flow override / fallthrough attributes therefore remain unverified. The existing descriptive comment attributes the defect to decompilation; that comment is not proof of the underlying flow flag. Root must handle any listing/flow repair in a separate packet.

## Complete physical entry and release schedule

The thunk is exactly `E9 2B EE FF FF`: an unconditional tail jump to `00955EB0`. It adds no frame, argument or register change. The clear routine receives actual header H in ECX, saves ESI and keeps H in ESI. There are no incoming stack arguments and the final RET pops only the original return address.

Let H+0 be the raw count word, H+4 the current node word and H+8 the terminal node word. Existing gunnery documentation names them count/head/tail; this body establishes the following accesses. All pointer words are 32-bit actual addresses.

1. `00955EB3` reads H+0. If zero, return without reading or writing H+4/H+8 or calling free. A zero count does not sanitize stale link words.
2. `00955EB8` captures N from the current H+4. `00955EBB` reads P from N+0.
3. If P is nonzero, `00955EC1` reads N+4 and `00955EC4` stores it at P+4. If P is zero, `00955EC9` reads N+4 and `00955ECC` stores it at H+4.
4. `00955ECF` rereads N+4 for the second branch. If nonzero, `00955ED5` reads N+4 again, `00955ED8` rereads N+0 and `00955EDA` writes that previous word at the selected next node's +0. If zero, `00955EDE` rereads N+0 and `00955EE0` writes it at H+8.
5. `00955EE3` performs a current-memory DWORD decrement at H+0. This is 32-bit arithmetic, not an independently cached count.
6. `00955EE6/E7` pushes the captured N and calls `00BF65AC`, currently named `_free`. This is the only physical call site. The visible continuation expects it to return without popping that argument; `00955EEC` removes four bytes.
7. `00955EEF` rereads H+0 after the call. If nonzero, the backward branch reloads H+4 for the next iteration. There is no cached successor carried across the release call.

The release argument is N, not N+8 or a payload loaded from N+8. This body never reads N+8, invokes a payload destructor, or separately releases the header. It does not establish the node allocation extent or allocator family by itself. For a consistent ordinary doubly linked list, the branches remove nodes and eventually leave count/head/tail zero. Arbitrary malformed or aliased storage is not repaired by this function.

The separate next/previous rereads occur after writes that could alias the node fields. A future reconstruction must preserve those reads or state a separately justified alias boundary; it cannot simply cache one next/previous pair. Nor can it replace the count-controlled loop with a null-head or sentinel walk, a precomputed iteration count, or an empty-header no-op.

## ABI and failure boundary

ESI is restored. EBX, EDI and EBP are not touched by this body; a returning callee must honor its own preserved-register contract. On the entry-zero path, ECX remains H and EAX/EDX are unchanged. After a release, residual EAX/ECX/EDX depend on the unopened free callee. There is no established return value. The final count comparison controls the normal exit flags.

No pointer check, allocation, exception-chain record, rollback, loop bound or error recovery is present. A nonzero count with invalid H+4 faults before free. Link writes and the count decrement precede the free call; failure there leaves that partial state. The callee's invalid-pointer handling, allocator internals, exception behavior and actual ABI were not opened or proved. Neither routine is classified as no-return from the misleading pseudocode.

## Existing provider comparison

The newly admitted constructor supplies genuine three-DWORD header storage and zero initialization. It supplies no populated node owner or destruction contract.

| Existing Source | Actual behavior / incompatibility |
| --- | --- |
| `RebuildBinding::clear_list_00955eb0` in `game_hosts_gunnery.cpp` | Clears vectors of gun indices; its append path stores indices derived from tokens. It is not this actual header/node loop. |
| Destructor-level host calls | Emit clear-array/list operations for +53Ch, +424h and +394h. These are abstract schedule calls, not node release providers. |
| `destroy_native_game_list_004c1990` and its private list helper | Sentinel list with opaque+0/head+4/count+8; zeroes count first, walks until the current sentinel and frees the sentinel. It cannot replace this count/head/tail algorithm. Its default free calls `::operator delete`. |
| `destroy_native_scene_registry_list_00b829d0` | Another sentinel algorithm. It requires a genuine supplied `actual_00bf65ac` allocator domain; that requirement is not an implementation of the Native callee. |
| `SizedStoragePool` | Semantic sized-pool projection with arena/free-ring ownership. Small-block release needs the original size. It is not this unsized release call. |
| `singleton_lifetime_free` | A real Source `std::free` operation paired with the Source `std::malloc` allocation path. The path-canonicalizer runtime delegates its scratch allocation/free to that pair. This is a usable existing Source allocation domain, but actual gunnery-node provenance and the Native `00BF65AC` mapping are not bound by this packet. |
| `free_native_sbh_block_00c11d68` | Requires actual descriptor/payload, canonical SBH state, external lock and feature data. It is not a one-argument replacement for the unopened front-end free call. |

No dummy release callback, semantic list adapter, source implementation or empty-list destructor was added.

## Decision and current evidence boundary

There is no newly ready Source packet in the opened pair. The concrete next prerequisite is Root's exact listing/flow repair and refreshed full 29-start gate. A later destructor implementation also needs a genuine node release domain and the complete ordered current-memory behavior above. The constructor's readiness/admission does not close either requirement. The full shared receiver, array unwind and production ownership remain separate.

The pinned Root authority is `reports/cc12_self_refresh_renderer_source_primary_review.json`, with its retained complete evidence under `local/cc12_self_refresh_renderer_Source_primary/complete_artifacts.json`. That receipt records 509 source inputs, four artifacts, 43 selected Core objects plus one application object, 51 positive Core roots and three checks. It also records the admitted constructor's exact 13 emitted bytes. These are receipt facts; this worker did not replay the inputs/artifacts or repeat the build/object review.

Root's real-Lua refresh case and three-tick/two-Present exit-zero smoke are separate bounded fixture/startup evidence. They provide no destructor test or gameplay validation. Source121 and Source507 remain frozen historical contexts; no current artifact was compared with either.

Only this document and the companion readiness report are committed. No C++/CMake, tests, probes, build, ledger, Native child/data extension or GPR mutation occurred. Source, ABI, fixture, startup and gameplay credits from this packet are zero.
