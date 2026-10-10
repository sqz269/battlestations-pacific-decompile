# CC12 locale raw map cleanup readiness

`0073C240` is a 228-byte raw bucket-map cleanup body. It captures each node's next link before calling opaque `00737CE0`, then recycles the current node through inline pool metadata. It clears the bucket array and count only after all chains return normally. The existing semantic `LocaleTables::clear_0073c240` owns different storage and cannot provide this raw lifetime contract.

This packet is read-only readiness work. It expands only `0073C240..0073C323`, below the 512-byte admission limit. No node/string/free provider, neighboring map method, or exception action body was opened. No C++ or application activation is admitted.

## Original receiver, layout and body evidence

The Native receiver arrives in `ECX` and is the bucket-array base, not the complete locale image. The accepted constructor/profile dependencies place it at `LocaleTableManagerImage +14h`; the exact image is `4040h` bytes. This routine reads 4,096 four-byte heads at map offsets `0..3FFCh`, and finally clears the count at map `+4000h` (locale image `+4014h`). It reads each node's next pointer at `+14h` and its pool-block index at `+18h`. Node fields `+00h..+13h` remain opaque here.

The original contract consumes no stack arguments and ends with plain `RET`. It preserves `EBX`, `EBP`, `ESI`, and `EDI`; `EDI` initially captures the map base, which is also saved on the stack. The final memset's result happens to remain in `EAX`; this body does not establish a separate semantic return-value contract.

All 228 bytes match the installed PE. Raw decoding accounts for 64 instructions. Current Ghidra lists 63 instructions in two body ranges totaling 225 bytes, omitting `0073C26D..0073C26F`. Those three bytes decode to `LEA ECX,[ECX]`; the preceding `0073C26B` is an unmodified unconditional jump to `0073C270`, with no fallthrough. The gap is undefined DataDB outside the stored body. Exact typed flow and instruction-context captures account for it without repair. Every live batch verified `C:/Users/sqz269/bsp.gpr` and `/battlestationspacific.exe`; modification number remained 55.

## Observable cleanup and reload order

| Address | Observable operation |
| --- | --- |
| `0073C245` | Capture the EnterCriticalSection IAT target from `00CE2218` once for the whole call. |
| `0073C260` | Load the current head of the next bucket. Buckets advance in ascending index order. |
| `0073C270..275` | Capture current node `+14h` next link in `EBX`, pass the current node in `ECX`, call opaque `00737CE0`. |
| `0073C27A..281` | Enter fixed section `00E17698`, then increment its current depth at `00E176B0`. |
| `0073C288..290` | Read the current node's `+18h` block index, reload the current block-table pointer at `00E176B4`, and load that indexed block base. These reads occur after both callbacks above. |
| `0073C293..2AF` | Compute the node/block byte delta, derive the apparent `1Ch` slot index, read the current unsigned 16-bit count at block `+7800h`, and store the low 16 bits of the slot index at block `+7000h + old_count*2`. |
| `0073C2B7..2CB` | Increment the block count as a 16-bit word. If the node's block index is unsigned-less than current `00E176C0`, replace that minimum. |
| `0073C2D1..2DC` | Decrement current depth at `00E176B0`, then leave the fixed section through the current `00CE2210` IAT target. |
| `0073C2E2..2FC` | Continue with the saved pre-callback next link; restore the original map base before advancing to the next bucket. |
| `0073C302..312` | After all 4,096 chains finish, call `00BF79F0(map,0,4000h)` and then write count zero at map `+4000h`. |

The slot computation is retained exactly as signed x86 arithmetic: wrapped 32-bit pointer subtraction, signed multiply by `92492493h`, high-product plus delta, arithmetic shift by four, then sign correction. Under a valid nonnegative `1Ch`-aligned node/block relationship this gives the slot number; the stored result is only its low 16 bits. This packet does not infer bounds checking, valid block capacity, or arbitrary-pointer safety. The 16-bit stack count increment and 32-bit depth updates have their original wrapping widths. The minimum comparison is unsigned.

The inline writes are consistent with returning a node index to a pool free-index stack; that ownership interpretation is a reconstruction hypothesis backed by their order and offsets. There is no direct CRT node-free call in this body. The only opaque node-cleanup call precedes pool entry and recycling. Internal key/text release order inside `00737CE0`, its ownership effects, and its exception contract remain unresolved.

The current bucket head is not progressively unlinked or nulled, and the map count is not decremented per node by this body. Callback changes to the current node's next link do not alter the saved next pointer. Callback changes to its block index or the block-table publication can affect the later pool lookup. Future bucket heads are loaded when reached; they are not snapshotted at entry. The terminal memset clears even heads changed during callbacks if normal traversal completes.

## Failure and lifetime boundary

There is no locally installed exception frame, armed cleanup state, guard destructor, or catch in this body. If `00737CE0` fails before returning, this body has not yet entered the pool section for that node; earlier nodes may already be recycled while bucket heads and count remain uncleared. Any node/string side effects already performed by the opaque provider remain. A later failure during Enter, metadata writes, or Leave has no visible local lock/depth rollback. An unsuccessful terminal memset prevents the explicit count-zero store. Arbitrary SEH translation, double faults, callback reentrancy, and invalid memory are not qualified.

A later implementation therefore cannot promise transactional cleanup, automatic retry, or a strong exception guarantee. It must not silently unlink nodes early, add publication repair, replace the pool with `delete`, or introduce a guard whose cleanup changes failure effects. A safe executable contract still needs the node-cleanup provider's actual behavior and the pool context's ownership/lifetimes. The original receiver, saved next node, post-callback node metadata, block-table cell, selected block storage, section, counters, and minimum-index cell must remain valid for their observed accesses. Acyclic uniquely owned chains, correct pool membership, and valid free-index capacity must be explicit domain assumptions unless additional evidence establishes validation elsewhere.

## Current Source comparison and qualification

The complete 57-byte, 25-instruction Source COFF section for `LocaleTables::clear_0073c240` is retained with its relocation. It iterates the semantic storage-vector range and calls the `unique_ptr<Node>` destructor, resets the vector end, clears its own buckets at receiver `+0Ch`, and zeroes its count at `+400Ch`. The Source node contains `std::string`, `std::u16string`, and a semantic next pointer; storage is `vector<unique_ptr<Node>>`. Its declaration is `noexcept`. Neither these offsets, storage order, deleter, nor exception contract is interchangeable with Native raw-map traversal and inline pool return. The unique_ptr/string destructor bodies were not expanded.

All 3,352 project inputs and four artifacts from `Source3352-particle-model-typed-views` matched current Root bytes at capture. All 3,352 actual Git blobs match the worker inputs; 3,345 also match the frozen compiler-input bytes exactly and seven differ only in line endings. All cohorts are retained separately. The whole Core archive was reparsed (1,991 members, 1,988 ordinary COFF objects), as were 70 frozen `bsp_game` target objects. Root's separate inventory of 80 current application objects is not borrowed here. The full indices contain the semantic `0073C240` name and no address-named `00737CE0` provider; this limited symbol observation does not establish general implementation absence by itself.

Whole constructor, profile-lifetime, and base-lifetime worker reports are retained as hash-pinned dependencies, without recursively copying their proof bundles or Root review trees. Constructor and profile packets are accepted; base worker `90c7ba1a2` is pending Root acceptance at capture. The eight constructor-source dependency pins remain unchanged. Portable replay rechecks retained bytes and saved capture receipts, separately from actual Git/current-source qualification at capture.

## Smallest next frontier

The next missing-provider read is **`00737CE0` only**, with body size checked before admitting any expansion. It must establish exact raw node fields, owned string/payload release order, preservation or mutation of `+14h/+18h`, and callback/exception behavior. The pool's observed global cells also need a concrete borrowed context with compatible lifetimes before any dormant C++ raw-map helper can execute.

A later bounded helper can accept the raw bucket image plus that actual pool context and a fully qualified node-cleanup contract, while reproducing saved-next traversal, post-callback reloads, exact metadata widths/order, and final-only clears. Current semantic `LocaleTables::clear` is not that implementation. **Raw map implementation GO: false. Full locale lifetime and application activation GO: false.** Derived destructor exception cleanup and the remaining locale pool/range contexts stay separate dependencies.

The adjacent JSON report pins the evidence ZIP and successful original-location and physically relocated ZIP-only replays, including a deliberate external-read denial. No C++ changes, build, CTest, Original/fixture execution, ABI replacement, startup, gameplay, SDK/OS interaction, or Ghidra mutation is claimed.
