# Gameplay effect name index

Addresses: 00871750, 00cdeb00, 004bf370, 00505ba0, 00505340, 00504bd0,
00502170, 0058d860, 0058b520, 00b65f80.

Packet `orch4_effect_name_index_k` reconstructs `00871750` and its process-exit
cleanup, a dependency of effect acquisition `00871B50`. Descriptive names remain
hypotheses. The implementation uses actual Lua 5.1.1 and existing pooled-string,
numeric-conversion and case-insensitive comparison services.

## Static cache and lookup

The native globals are a map header at `00F87670` (allocator word), head at +4,
count at +8, and guard DWORD `00F8767C`. Static storage starts zero. At first
lookup, bit 0 of the guard is **set before allocation**. `005826B0` allocates a
28h sentinel; the caller sets nil byte +19, links the head to itself, publishes
count zero, and calls real CRT `atexit(00CDEB00)`. The registration result is
discarded. No guard rollback or registration retry is present.

Whenever count is zero, lookup captures the current game's **embedded Lua owner
at `00E188A8 + 1A0C`**, takes globals, retains `Effects`, and releases the globals
temporary. It iterates actual Lua key/value references. Only numeric keys (native
Lua type 3) whose value has a strictly string-typed `Name` field are accepted;
numeric-looking names are not coerced into eligibility. Non-table values still
reach normal Lua indexing behavior; they are not silently filtered.

For each accepted row, native code performs all of these operations:

1. Convert the numeric key through `00B66290`, discarding its result.
2. Read the retained Name string and construct a first pooled temporary.
3. Convert the key again, reading the current CRT conversion mode again.
4. Read the same retained Name reference again and construct a second temporary.
5. Insert the second name and second ID through unique map insertion. A new node
   owns another deep copy of the key; a case-insensitive duplicate retains the
   existing key and ID.
6. Destroy the second name, first name, then the Name Lua reference.

The discarded conversion still affects floating-point state before the first
allocation. Allocation can change the live conversion mode before the second
call. The C++ implementation preserves both calls and both temporaries. Integer
conversion retains the existing float32 spill and late SSE2/x87 selection.

After iteration, cleanup releases value, key and Effects references. Lookup then
uses the caller's current NativeString, lower-bound lookup and a reverse less-than
test to distinguish an exact equivalent from a missing key. The comparison is the
existing length-zero-gated CRT `_stricmp`, including its locale and embedded-NUL
behavior. Missing keys return zero. A stored zero ID also returns zero.

An empty cache is loaded again on later calls. A nonempty cache persists even if
the game's Lua state or Effects table changes. This cache is separate from the
non-owning definition manager recovered in `GAMEPLAY_EFFECT_MANAGER.md`.

## Destruction and storage

`00CDEB00` was an unrecognized atexit entry point. Its verified native range is
`00CDEB00..00CDEB3F` inclusive (end-exclusive `00CDEB40`). It captures begin/end,
erases the entire tree, reloads and frees the current head, then clears head and
count. It preserves the allocator and guard. It is not an idempotent reset API.

Full-range erasure `0058D860` invokes `0058B520`, which recursively visits the
right subtree, captures the left link, releases the current pooled name using
its current length+1, frees the node, and continues left. Keys therefore release
in descending comparison order; count is cleared after node destruction.

Ordinary nodes allocate **1Ch** (`00504BD0`), even though the shared blank-head
allocator reserves **28h**. Node layout is left/parent/right at 0/4/8, NativeString
at C/10, integer at14, color18 and nil19. `00502170` separately initializes and
deep-copies the key before copying the integer. Stock insertion `00505340` checks
count against `15555554h`, then allocates, publishes and balances a node and
returns an iterator. Its old whole-function throw-site name was incorrect;
the length-error path is only one branch.

The host uses one optional standard map, with independently published count and
guard. There is no duplicate native sentinel. Process setup binds the canonical
cache, the current-game Lua accessor, NativeStringStorage and live CRT mode.
Those dependencies must outlive **real CRT atexit**, where the production cleanup
is registered directly. No private shutdown collector substitutes for atexit.

## Original ABI and scope

| Address | ABI | Treatment |
| --- | --- | --- |
| 00871750 | ECX unused; one stack NativeString pointer; EAX signed ID; RET4 | reconstructed normal lookup and population |
| 00CDEB00 | no arguments; native EAX zero; RET | reconstructed actual atexit callback, new void C++ API |
| 004BF370 | ECX map; stack name; EAX lower-bound node; RET4 | analyzed stock lookup contract |
| 00505BA0 | ECX map; output iterator/bool, pair pointer; EAX output; RET8 | analyzed stock unique insertion |
| 00505340 | ECX map; output iterator, left flag, parent, pair; EAX output; RET10h | analyzed stock insertion/balancing, not a whole-body throw |
| 00504BD0 | five stack node-construction arguments; EAX node; RET14h | analyzed allocation contract |
| 00502170 | ECX node; left/parent/right/pair/color stack; EAX this; RET14h | analyzed node/key construction |
| 0058D860 | ECX map; output iterator and two owner/node iterator pairs; EAX output; RET14h | analyzed stock range erase |
| 0058B520 | ECX map; stack subtree root; RET4 | analyzed pooled-key erasure |
| 00B65F80 | ECX LuaObject; EAX Lua type; RET or tailcall | analyzed type accessor; index keys are tracked references |

`00B65F80` returns -1 for kind0, calls Lua type for kind2, and returns 5 for other
kinds. This loader only applies it to tracked iterator keys, so the existing
reference host's type operation is sufficient; no general equivalence for its
borrowed-reference forms is claimed.

STL algorithms are not reimplemented. Native tree topology, exact comparison
counts, allocation failure, invalid-iterator callbacks, topology mutation during
string-allocation callbacks, native SEH and binary ABI remain outside the standard
container projection. Lua reference handles are the existing host representation.
The application must bind its actual game state and string pool. This dependency
does not create definitions or replace still-unreconstructed component loading.

## Validation and Ghidra evidence

Win32 Release and both existing test targets passed. One ignored focused process
uses actual Lua and actual CRT atexit to check empty-cache reload, numeric/name
filtering, first duplicate ID retention, nonempty-cache persistence, independent
pooled key ownership, both conversions, and descending key cleanup. Its string
allocator observes the first SSE invalid flag for numeric key 2^32, switches the
live mode, and verifies the second x87 conversion stores low32 zero. A verifier
registered before the production callback confirms all keys were freed, head/count
cleared, and guard/allocator preserved after real atexit execution.

Native spans are compared against live Ghidra bytes after project/program
verification; hashes and prior annotations are recorded in the JSON reports.
The 11-byte free-call continuation in `0058B520` is repaired. The new `00CDEB00`
function still has a truncated stored body ending `00CDEB2B`: its verified 20-byte
tail is decoded and the free-call override cleared, but the available flow tool
does not extend stored function-body metadata. Full semantics use the verified
native listing, not the shortened pseudocode. The report keeps that distinction.

These results are build/fixture evidence. The existing native differential target
covers its existing math seeds; no native-index ABI or gameplay validation is claimed.

## Follow-up packets

- Connect this concrete lookup to `00871B50` while recovering `008700E0` definition
  acquisition, `0086B870` identity assignment and the manager's weak cache contract.
- Recover actual component loading/lifetime `00870400`, `00871440`, `00870D00`
  and their 13 component families before replacing startup/warning acquisition.
- Extend stored atexit body `00CDEB00` through `00CDEB40` with a supported Ghidra
  body operation; the same metadata limitation remains for `0086FE20`.
