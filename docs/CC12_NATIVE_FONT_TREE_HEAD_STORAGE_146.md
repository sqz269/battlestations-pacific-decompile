# Native Font tree-head storage, packet 146

`allocate_native_font_tree_head_00ad42c0()` now implements the complete AD42C0 allocator leaf over the existing owning Source CRT allocation domain. It returns raw 18h-byte storage, performs the three conditional DWORD zero stores and two BYTE flag stores, and leaves every other byte untouched. No Font object, tree owner, registry or Text rebuild is created.

The clean worker fast-forwarded from captured 141 commit `87b7f7cec3c3993ebd237db02eb1acfd41b5f7c9` to published Main `9f81f799522c920d35123b7b0ecc3890f75f995b`, exit 0. Only the two new Source files, this receipt pair and the owned AD42C0 name/reconstruction rows change. Root registers the new translation unit; no CMake or host activation changed here.

## Original body and storage

Fresh `bsp.py ghidra` batches verified the existing `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, x86 language and base before reading. Complete live `[AD42C0,AD42F7)` equals the unchanged installed Original: **55 bytes / 17 instruction starts**, SHA256 `941effb874f57a5fe53446c2d9672ccc73ddd763021ba52d0879fa4aeb80a9ef`. The installed executable SHA256 remains `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`. Saved and fresh listing starts agree exactly.

Assembly establishes no incoming arguments, allocation result left in EAX and plain RET. Ghidra's inferred `undefined(void)` signature does not override that observed result. The whole schedule is:

| Original site | Operation |
| --- | --- |
| AD42C0 / AD42C2 / AD42C7 | Push 18h; call actual BF681B `operator_new`; remove the stack argument. |
| AD42CA..AD42CE | Test the returned base separately; conditionally store DWORD zero at +00. |
| AD42D4..AD42DB | Compute wrapped base+4; test that address separately; conditionally store DWORD zero there. |
| AD42E1..AD42E8 | Compute wrapped base+8; test that address separately; conditionally store DWORD zero there. |
| AD42EE / AD42F2 / AD42F6 | Store BYTE +14=1, then BYTE +15=0; RET. |

The three tests are not one blanket nonnull guard. Bytes +0C..+13 and +16..+17 remain untouched. There is no local EH state, cleanup action, name, count, self-link, publication or singleton registration in this leaf. AD5590/AD55C0 subsequently change/link the returned head; those bodies are not duplicated or newly composed here. In particular, head+08 is a pointer slot, and raw Font+04 is a head pointer rather than a Font reference count.

## Actual Source allocator boundary

The implementation directly calls `singleton_lifetime_allocate({SingletonAllocationKind::object,0x18,0x18})`. It accepts no replacement allocator, callback or default provider. Fresh current `singleton_lifetime.hpp/.cpp` copies and baseline Git blobs are pinned; both match the captured 141 inputs exactly. The actual lower code loops on owning `std::malloc`, invokes owning `_callnewh` on failure, and throws owning `std::bad_alloc` when that handler declines. `singleton_lifetime_free` uses the matching owning `std::free`.

Successful allocation must be genuine aligned, writable 18h storage. That owning SDK returns nonnull or throws; it cannot fabricate a null-success path. The implementation nevertheless preserves all three wrapped-address tests. Native null or invalid addresses, private CRT heap/new-handler state, cross-CRT exceptions, FH3/SEH and fault timing are outside this Source allocation domain. Unsigned pointer arithmetic and volatile DWORD/BYTE accesses assume the explicit MSVC Win32 raw-storage implementation domain; static assertions enforce 32-bit pointers and DWORDs. This is not a portable C++ object-layout or Native binary replacement claim.

The caller owns the returned allocation and must retain it through the actual tree lifetime, then use the SAME `singleton_lifetime_free` domain. This leaf adds no automatic destructor, rollback, retry metadata or new owner/count. The existing allocator's genuine allocation retry remains its own lower behavior. No Source descriptor, `FontData`, texture wrapper or raw Font cast is involved.

## Complete optimized inspection

Strict compilation used MSVC Win32 C++17, `/MD /EHsc /O2 /Gy /W4 /WX /fp:strict /permissive-`. It completed without diagnostics. `local/cc12_native_font_tree_head_storage_146/strict.asm` lines 27–111, `strict_dumpbin.txt` and `physical_strict_function.txt` preserve the complete emitted function.

The function is **108 bytes / 29 starts**, SHA256 `1a6b55f0dcfec6a8c2b89f0138c68acd6b235258dc8f5adcd8d90017632fffdd`. All three COFF relocations are resolved symbolically:

| Offset / type | Actual Source symbol |
| --- | --- |
| +04 / DIR32 | `___security_cookie` |
| +2A / REL32 | `bsp::singleton_lifetime_allocate(const SingletonAllocationRequest&)` |
| +64 / REL32 | `@__security_check_cookie@4` |

The prefix builds the real 12-byte `{object=3,native=18h,host=18h}` request and calls the real allocator at +29. Source `[+31,+5D)` is byte-identical to Original `[+0A,+36)`: **all 44 bytes** of separate tests, branches, DWORD stores and BYTE stores. The Source tail checks its compiler cookie and returns. The request, Source stack frame and GS support are explicit Source differences; equality of that effect block does not prove whole Native ABI, heap, exception or instruction/fault timing equivalence. No relocation calls a fixed Original address or a stub.

## Validation, annotations and limits

One required `scripts/build.ps1` run completed with exit 0 and all three existing checks passing in **7.66 seconds**. This validates the current registered baseline. The new TU is deliberately unregistered on the worker, so its compile evidence is the strict direct compilation; the canonical build neither includes nor executes this new leaf. No new test, behavioral probe, Native differential or game run was added.

The existing correct library-role name **`STL_Tree_BuyHeadNode_00ad42c0`** is preserved. The locked annotation tool recorded the prior state, added only this function's evidence, saved the project, and the owned export was forced/refreshed. Post-annotation live bytes and all 17 listing starts remain unchanged. A first reviewed-name mapping and a distinct whole-function Source record were added at the owned address; all pre-existing rows, other addresses, tags and old projections remain intact. The index was refreshed after those ledger additions.

The 141 sealed manifest still hashes to `64665f34912b49359fc49f36a4c82692e3b37ef097db7a27ec519d6aa7364c1a`; no captured family was regenerated. The private 146 manifest freezes fresh provider Source/Git pins, Original bytes/listings, the exact new Source/object/relocations, compiler/build logs, annotation history, forced exports and ledger preservation checks before commit. The report separately pins that manifest to avoid a hash cycle.

This closes one genuine raw head-allocation prerequisite. Actual AC-byte Font construction, raw registry publication/lookup, loading, retirement and whole ABA8D0/ABB1D0 composition remain separate work, as established in 141. Source compilation and existing checks establish neither production authority nor normal activation readiness. Root reviews the complete Source, Original, optimized body and this receipt before final sealing; only owned files are committed, the lease is released and artifacts are held unchanged for capture.
