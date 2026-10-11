# Canonical unlock Source readiness

The existing Source pair implements the seven-instruction shape of the accepted `00C11B31..00C11B45` body. A directly routed existing Win32 COFF has the same 21 instruction bytes except its four-byte import relocation. This is a positive Source contract and physical artifact review. It adds no new implementation, authenticated current compilation, linked image, Original ABI, provider identity or runtime credit.

The worktree base is `d040b4a6d424765e07ce089ab674c7fc1a359bae`. Actual main during capture was `7b3676abb46b448af7a38cf1912e1de676f4e7a3`. Both commits' actual objects and selected tree paths establish the same eleven positive input blobs. Index blobs and LF-normalized working files agree in both checkouts. Source tree: `dfb2ee135e7b642465d448dcc1616fd92686ad51`; include tree: `7bb7ed2fc76e16eb09ff91be796c5b264aab9ca9`. These are capture identities, not a later-main claim.

## Existing implementation and required ownership

`src/native_crt_canonical_unlock.cpp` and `include/bsp/native_crt_canonical_unlock.hpp` declare `void __cdecl bsp::unlock_native_crt_canonical_00c11b31(std::int32_t)`. The implementation requires MSVC x86, asserts four-byte pointers and index words, and uses a naked inline-assembly body:

```asm
push ebp
mov ebp, esp
mov eax, dword ptr [ebp + 8]
push dword ptr [eax * 8 + 00e16478h]
call LeaveCriticalSection
pop ebp
ret
```

`cmake/startup.cmake` registers this exact implementation in `bsp_core`. Registration establishes a Source input route; this review does not inspect a current library or executable link.

The Source caller must supply the actual fixed descriptor mapping, a readable pointer DWORD at wrapping `00E16478 + 8*index`, and the valid initialized Win32 critical section currently owned by the calling thread. The descriptor and lock must remain valid through `LeaveCriticalSection`. The instruction reads the current descriptor value; the second DWORD is not read. A signed C++ parameter does not introduce signed multiplication, validation or a range check into this assembly. Actual data/stack segment mapping and stack-address conditions must support the accesses.

Canonical page admission alone does not initialize or acquire a lock. This leaf supplies no table activation, storage allocation, initializer, acquisition, lifetime owner, replacement lock, null/range guard, exception conversion or return-value policy. The directly routed initializer-cleanup contract specifically requires the initialized, currently owned lock at descriptor `00E164C8` when index 10 is used; it does not establish that any Native caller meets that contract.

## Exact observed object

Exact filename routing from the Source/CMake entry found `build/win32/bsp_core.dir/Release/native_crt_canonical_unlock.obj` in main. The copied object is 778 bytes, SHA-256 `87ee84bbeb1ca73c4f03ceb336c9eaa77b0288fa3d2e28249bfbf7adaca6fb89`. The filename and placement route an existing candidate; its timestamp and older build claims are not current compilation provenance.

Complete physical COFF parsing retains all four section headers/data, all twelve symbol records including auxiliary records, and the single relocation. Its only executable section is section 3 `.text$mn`, exactly 21 bytes. The other sections are `.drectve` (47 bytes), `.debug$S` (188 bytes) and `.chks64` (32 bytes), with no relocations. The eight primary symbols retain their original record indices.

| Evidence | Complete body / relationship |
| --- | --- |
| Accepted Native body | `558BEC8B4508FF34C57864E100FF151022CE005DC3` |
| Existing COFF body | `558BEC8B4508FF34C57864E100FF15000000005DC3` |
| Sole difference | Offset 15, four zero bytes in the object versus Native `1022CE00` |
| Relocation | `IMAGE_REL_I386_DIR32` (6), offset 15, symbol record 9: `__imp__LeaveCriticalSection@4`, undefined external |
| Definition | Symbol record 8: `?unlock_native_crt_canonical_00c11b31@bsp@@YAXH@Z`, section 3, value 0 |

The indexed memory PUSH, including fixed displacement `00E16478`, is exact and has no relocation. Actual detailed Capstone 5.0.7 decodes all seven COFF instructions and agrees with the physical byte comparison and Source sequence. Default DS/SS selection and the accepted Native contract remain separate from an observed live segment configuration.

The Source names genuine Win32 `LeaveCriticalSection`, and the object references its genuine import symbol. No current link resolution, import cell, provider body or library member is examined here. In particular, the new Source symbol does **not** authenticate Native cell `00CE2210` or saved `EXTERNAL:00000159`. Older canonical-unlock documentation/report contain broader PE/import, caller and build claims; their blobs are pinned as historical contract material, without reopening or replaying those older evidence archives.

## Native correspondence stops at the opaque call

The accepted body/primary reports establish the physical seven-instruction sequence. The memory PUSH reads its current DS source before its stack write. CALL reads current DS at `00CE2210` after that preceding PUSH write and before its own return-address write. Possible aliases between DS and SS memory remain admitted; these are architectural value dependencies, not measured bus or fault order.

Native provider identity, callee stack cleanup and preserved state remain unproved. After an opaque return, registers including EBP/ESP, segments, flags and memory are unknown. Let returned ESP be Q. Only renewed actual 32-bit SS stack-address size and normal completion permit POP from current SS:[Q], then RET from current SS:[Q+4], ending at Q+8. No relation between Q and entry ESP is established by this review. The Source's naked cdecl declaration and genuine Win32 API contract cannot be substituted for this unknown Native boundary. Original caller ownership, stack balance, frame restoration, exception behavior, fault PCs and reachability remain held.

## Copied proof and validation boundary

The finite set is the Source pair, accepted body/primary document-report pairs, `config/target.json`, `cmake/startup.cmake`, the directly routed canonical-unlock document/report and initializer-cleanup document. Actual commit/tree/blob records authenticate these eleven files through both captured commits. No Source absence census, directory-wide Source read or prior archive recursion is used.

Private evidence is under `local/cc12_locale_onexit_canonical_unlock_source_readiness/`. `proof.zip` and a distinct physical `proof_copy.zip` contain 248 members, including the 24 distinct actual Git objects, eleven inputs, object bytes, original Git command streams, inspection result and pinned producers. Archive SHA-256: `d286d4e516dea094efab85127dcead7c88284161daa62e6f7790006f9b7fc206`. The 9,253-byte pure replay helper is SHA-256 `deb70ed931e2b6946e4e121b9d50299249acf4713f2b02b6510889a41d662295`.

Both strict processes passed on their first execution. Each permits exactly one supplied ZIP read, one exact pinned helper execution and one exclusive receipt write; every other audit event is denied, with zero recorded denials. Helpers were physically read and pinned before execution. Original helper stdout/stderr and results are retained. The package producer's postexecution streams, strict receipts/streams and later publication/commit records necessarily live outside the already-created proof ZIP and are separately pinned in the report.

Routing incidents are disclosed separately: the new worktree lacked an index, causing two initial offline lookup commands to fail; main's existing index supplied the positive routes. One guessed private helper filename was absent before exact private helper discovery. One read-only PowerShell JSON inspection encountered an unsupported empty root key in the selected-tree map. Two initial contract-read outputs were truncated; their missing text was not used as evidence. These caused no Native query, Native code execution or mutation. The actual capture, COFF inspection, packaging and both strict replays succeeded without retry. Historical failures reported inside pinned older documents remain historical.

No Ghidra query/mutation, original executable access, build, test, probe, game invocation, OS/audio check or runtime call was performed. The next readiness boundary is independently authenticated Native provider/caller and fixed-domain ownership evidence; this packet does not authorize activation or another implementation.
