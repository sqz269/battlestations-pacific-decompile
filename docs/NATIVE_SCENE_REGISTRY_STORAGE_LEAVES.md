# Raw scene registry storage leaves

Three complete normal bodies are reconstructed: B82390..B823A9 (26 bytes),
B823B0..B823E2 (51 bytes), and B82570..B8259B (44 bytes). The source keeps the
original x86 instruction order in MSVC Win32 naked functions. Names are
descriptive hypotheses. This packet supplies storage primitives needed by the
actual registry constructors and insertion path; it does not reconstruct the
whole inner 3Ch lighting resource, registry lifetime, or its enclosing callers.

| Entry | Coverage | Original ABI | New interface |
| --- | --- | --- | --- |
| B82390 | complete, 26 bytes | ECX ignored; no stack inputs; EAX allocation; RET | Fastcall ECX is the address of a borrowed current BF681B entry slot. |
| B823B0 | complete, 51 bytes | ECX ignored; next, previous, key-source in three stack DWORDs; EAX allocation; RET Ch | Fastcall ECX borrows the allocator slot; explicit unused EDX leaves all three original inputs stacked. |
| B82570 | complete, 44 bytes | ECX destination, EDX unsigned count; four stack DWORDs; RET 10h | Same argument placement; only the first stacked pair-source is read. EAX is incidental, not a return contract. |

The two allocator leaves push Ch and clean the allocator's one cdecl argument
with ADD ESP,4. Their source substitutes a current indirect allocator-entry read
at that call, without adding a stack frame, cleanup handler, null default, or
exception suppression. This context change means the allocation APIs are new
C++ interfaces, not drop-in entry-point replacements.

## Ordered reads and branches

B82390 conditionally stores the returned address at result+0, then separately
tests wrapped result+4 before its self-link store. Result+8 retains the allocator
preimage. In particular, a hypothetical null result does not mean an early return:
the independent second test admits address 4. The corresponding unusual wrap
cases remain in source and were inspected in emitted COFF instructions.

B823B0 separately tests result+0, wrapped result+4 and wrapped result+8. It reads
each current stacked input at that store stage. The key-source dereference occurs
after the next/previous writes; no early key snapshot or light retain is added.
The source preserves alias behavior even though the fixture does not arrange a
key-source alias into the fresh allocation or into the native private stack.

B82570 tests unsigned count before loading the pair-source pointer. Each iteration
skips both source reads when the current destination is null; otherwise it reads
source+0, stores destination+0, then reads the current source+4 and stores
destination+4. It decrements the count and adds 8 with 32-bit wrapping, then
continues while the unsigned count is positive. Overlap may therefore change both
the second read in the same iteration and later iterations. There is no pair
snapshot, count clamp, allocation, or empty-list inference. Only ESI is saved.

## Allocator contract and producer evidence

Full BF681B..BF6883 was inspected. BF6833 calls BF9F1A (`_malloc`); failure reaches
BF6826 / C055B1 (`__callnewh`) and retries when the current handler returns nonzero.
The no-retry route uses the current 109DD74/109DD68 exception state, BF6802,
BF6FF5, BF63A6 and BF6885. The borrowed entry must retain that entire allocation,
retry and throwing contract. This packet does not provide a replacement allocator
or freeze a handler/global. The full malloc, handler and exception helper listings
are retained as supporting evidence; their CRT identity is not reconstructed here.

The actual node producer is these two leaves: allocation size Ch, next at +0,
previous at +4, and copied raw key at +8. B83600 places its list owner at registry+4,
stores the sentinel at registry+8 and count zero at registry+C, then constructs
nine iterator pairs at registry+10. Its temporary pair is `{registry+4,sentinel}`.
B83220 allocates the backing and passes ECX destination / EDX count to B82570;
the caller's own bound checks are not moved into the leaf.

`SceneNodeRegistry::Entry` and `Iterator` in `scene_attachment.hpp` already express
the same three/two logical words, but their private C++ registry owner is explicitly
a different layout. The new leaves do not cast that owner, introduce a competing
owner, resolve a light, or initialize an absent inner resource.

All six direct caller preparations were read, with live containing-body checks:

| Call site | Containing body | Preparation and cleanup |
| --- | --- | --- |
| B8362D -> B82390 | B83600..B83677 | ECX registry+4; no stack arguments to leaf; returned sentinel stored at owner+4. Enclosing constructor RET 8. |
| B82B53 -> B82390 | B82B50..B82B67 | ECX list owner; leaf consumes no arguments; wrapper stores +4 sentinel and +8 zero; wrapper RET 4. |
| B83B21 -> B823B0 | B83700..B83BDE | Push current key-source, current ESI+4 previous, then ESI next; leaf RET Ch. Caller later grows count and links the returned node. |
| B83366 -> B823B0 | B832F0..B83403 | Push EDI+8 key-source, current EBX+4 previous, then EBX next; leaf RET Ch. EBX comes from EBP+C and EDI from EBP+14. |
| B83298 -> B82570 | B83220..B832B5 | ECX freshly allocated backing, EDX EBX count; push two scratch words, ESI owner, and current pair-source; leaf RET 10h. |
| B82BA4 -> B82570 | B82B80..B82BB1 | ECX original destination, EDX original count; first stacked word original source, three ignored words; leaf RET 10h, wrapper RET Ch. |

## Validation and limits

Strict `/W4 /WX /O2 /MD /fp:strict` source compilation passed. The emitted COFF
listing preserves all reads, writes, tests and RET cleanup instructions; its
alignment no-op has a shorter encoding. The existing baseline Win32 build passed
both CTests and all eight native seed checks. That baseline excludes this still
unregistered module; the focused probe explicitly compiled its source. Integration
must register the module and replay against the newly built current library.

One external fixture at `C:/Users/sqz269/bsp-ba-scene-registry-leaves` passed nine
original/source pairs, comparing 932 bytes. It executes all 121 original owned
bytes, permitting only the two allocation CALL displacement operands to point to
the same rebuilt allocator boundary. Sentinel return/self-links are normalized;
the untouched payload, node words/current external key and full 128-byte fill
arenas are compared. Fill cases cover disjoint, exact alias, forward/backward word
overlap, later-iteration source mutation, zero count with a poison source, and one
null destination iteration with a poison source.

Both allocation sides call the existing `singleton_lifetime_allocate`, whose full
current implementation uses malloc, `_callnewh` retry, and `std::bad_alloc`.
A fixture decorator initializes each genuine allocation's Ch-byte preimage and
updates the external node key before returning. It never invents an allocation
result or bypasses failure. This demonstrates leaf behavior with a shared rebuilt
provider, not original allocator bytes, CRT exception identity, or failure parity.
No artificial OOM, null/wrapped allocator result, inaccessible address wrap, huge
count, private-stack alias, or exception unwinding scenario was executed.

Source, headers, provider, probe inputs and all three selected libraries were
hashed before/after the fixture and remained unchanged. The immutable capture
archive includes those inputs, original/live bytes, complete caller listings,
compile/build/test logs, probe executable and recipe. Default `run.ps1` compiles
only the external probe and links the selected root's three current libraries;
`-WorkerSource` is solely for this initial unregistered-module run. No new repository
test, ledger, CMake registration, or Ghidra mutation was made by this worker.

The numeric eight-row call report verifies both owned allocation calls and all six
incoming calls. No frame-path/gameplay claim is made: the fixture establishes the
bounded leaf behavior, not executable reachability, registry construction/growth/
erasure/teardown, an ABI-compatible game replacement, or a runnable game rebuild.
