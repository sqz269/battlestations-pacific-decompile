# Raw locale map-clear Source readiness

Current integration update: the historical absence census below predates the
accepted raw736090 slot-return service. A separate normalized raw73C240
composition is now Source-reviewed and Win32 build-tested, existing checks2/2
19.29s; see CODEX_RAW_LOCALE_CONSTRUCTION_INTEGRATION.md. The original frozen
record is retained below. Canonical initialized pool ownership, lifetime and
application activation remain unbound; no old GO is retroactively changed.

`0073C240` has a bounded composition design, but **C++ implementation GO remains
false**. The accepted dormant `00737CE0` payload helper is ready. The distinct
node-slot return service and its actual initialized owner are still missing from
the reviewed Source. Application and locale lifetime activation remain held.

This packet changes documentation only. It reuses the accepted complete228-byte
map body,119-byte node body and62-byte node EH dependencies. It performs no new
Native/Ghidra queries, original-file reads, C++, build, test, probe or runtime
execution. The original map ABI is ECX=raw bucket base with plain RET; the proposed
interface is normalized C++, with no binary ABI or native fault-timing claim.

The worker freeze is commit `937c26f50f590fe8df9b151fefddb269451e68e0`.
All3354 selected inputs match the earlier worker node-helper build exactly.
The full tracked `include/` and `src/` census adds861 inputs outside that selected
epoch:4208 tracked Source files,4215 unique retained inputs including build files.
All4215 current files match their actual committed Git blobs byte for byte.
These census counts are distinct from compiler project membership.

At that freeze the node-helper worker result `0457dc59c` was pending Root review.
Later Root acceptance `3caaeec9a171fb79ec0acde08cfa2fa97fa53ff5` is retained as a
separate whole committed doc/report dependency, with actual Root current bytes
and its frozen Source3354 manifest. All3354 Root inputs are retained;3348 match
worker bytes exactly and all3354 match after CRLF normalization. The original
pending capture is not rewritten. Root artifact pins in that manifest remain
metadata here; this packet does not rerun Root's build or compare Root objects.

The exact raw layout and service separation are:

| Storage | Required fields and identity |
| --- | --- |
| Raw map |4096 DWORD heads at `map+4*i`; count DWORD at `map+4000h`. Pass the actual bucket base, established elsewhere as locale receiver+14h. |
| Node |1Ch slot; narrow length+0, pointer+4; wide length+8, pointer+Ch; prev+10h, next+14h, block index+18h. All offsets are bytes from the node. The map does not touch prev. |
| String payload context |Existing `NativeStringRawPoolContext` borrows canonical01090AA8 publication,01090AA4 live gate and01090AA0 manager publication. |
| Node-slot authority |Distinct critical section00E17698, depth DWORD00E176B0, current table pointer cell00E176B4, unsigned earliest-block DWORD00E176C0. No enclosing owner base is inferred. |
| Selected node block |1Ch slot geometry, WORD free-index stack+7000h, WORD current free count+7800h. Geometry permits1024 stack entries before the count, but does not establish allocation extent or initialization. |

`GameNativeStringProcess` owns the existing process-lifetime string cells and
`GameVfsHost::raw_strings()` exposes its raw context. This is an available payload
authority; it does not initialize or publish a node-slot owner. Semantic
`LocaleTables::clear_0073c240()` instead clears a vector of unique_ptr nodes,
semantic buckets and a count under noexcept ownership. It is not the raw layout
or the required service.

The minimum future algorithm retains the original map address and visits bucket
indices0..FFFh in order. The Native termination branch is signed JL against1000h;
a uint32_t induction is equivalent over this fixed nonoverflowing range. Each
bucket head is loaded when that bucket is reached. For each nonnull node:

1. Save its current DWORD next at+14h before payload cleanup.
2. Call the accepted payload helper on the original node and same raw context.
   It handles wide header+8 first, guarded by current narrow cleanup on escape;
   it disarms before normal narrow cleanup. Each stage resolves its own current
   payload pool after capturing that header's pointer/size. Headers stay intact.
3. Only after normal payload return, enter the actual node section and increment
   its current DWORD depth. Then read current node+18h, current table cell and
   current selected table entry. These reads must not move before cleanup/Enter.
4. Form the wrapped32-bit node-minus-block address difference, interpret it as
   signed32 and divide by28 with truncation toward zero; retain low16. This is the
   retained signed magic-IMUL/add/SAR/sign correction. Use unsigned address
   arithmetic rather than subtracting unrelated C++ pointers.
5. Capture current WORD free count and store the slot index at
   `block+7000h+2*captured_count`. Then increment the **current** count WORD at7800h,
   not a cached value. Compare block index against the current minimum unsigned,
   lower it when smaller, decrement current depth and leave the section.
6. Continue from saved-next. Do not reload this node's next after callbacks.

Only after every chain returns normally, zero4000h bucket bytes and then clear
the count DWORD. There are no progressive head clears, node unlinks or per-node
map-count decrements. Provider mutations of the current block index/table,
current metadata and future bucket heads are observed at their specified reads.
A callback change to the current next field does not redirect traversal. Final
zeroing also wipes callback changes to heads; insertion into already visited
buckets is not a generally ownership-safe operation.

Native caches EnterCriticalSection's CE2218 import once at function entry and
reloads CE2210 LeaveCriticalSection per slot. A normalized helper using ordinary
stable Win32 functions must exclude import rebinding/hooking from its domain.
It must not silently claim that identity-sensitive behavior is reproduced.

The complete Source census contains no references to the four node authority
addresses or to graph candidates0073C220/00736090. The7000h/7800h constant search
has only an unrelated CRT small-block allocation7000h expression. Existing
similar return routines have fixed incompatible geometry:

| Existing service | Node block ID | Stride | Index stack / count |
| --- | --- | --- | --- |
| Physical provider `00BF2FC0` |+38h |3Ch |+1E0h / +1F0h |
| Particle model `00AF60B0` |+2DCh |2E0h |+5C00h / +5C40h |
| Native scene node `00B6E490` |+174h |178h |+2F00h / +2F40h |
| Enum node `0043B0A0` |+10h |14h |+500h / +580h |
| Required locale slot |+18h |1Ch |+7000h / +7800h |

Those comparisons and the retained finite census identify no exact reusable
service. They do not prove a universal absence of every possible equivalent
algorithm. A generic allocation callback or a differently configured foreign
owner would hide the unresolved contract rather than establish it.

`0073C220` has no retained complete body, export or reconstruction in the reviewed
evidence. Its offline graph lists00736090 and00737CE0 as callees; that does not
establish order, flags, ABI or return semantics.00736090 is also graph-only here.
Neither body was opened. Their graph records are retained with this limitation.

An escaping payload failure prevents this node's slot return and final clears,
but earlier returns and provider effects remain. The accepted C++ node helper
supports ordinary propagating getter exceptions; a second exception during
cleanup terminates. After entering the node section the map body has no local
cleanup guard, unlock rollback or catch. A failure can leave the section held,
depth changed or metadata partially changed. No noexcept or strong-recovery
promise is justified. A fault during final bucket zeroing precedes count zero.
Native invalid-read/SEH timing and double-failure ABI remain outside this design.

Callers must supply alive, aligned map/header/node storage, valid saved-next
nodes, canonical string cells and the actual initialized node section, cells,
table and selected blocks. Chains must be finite, acyclic and uniquely owned;
nodes must be coherent unrecycled1Ch slots. Payload aliasing must not cause
duplicate returns. Require valid index/address extents and free-stack capacity;
do not add new overflow checks, saturation or deduplication. Preserve DWORD
and WORD wrapping where established. In the admitted count<1024 domain the
free-index write does not alias count. Count1024 would write at7800h itself,
illustrating why current-count increment differs from cached-count+1.

Use established raw byte access, not an invented native C++ Node object. Exclude
unsafe overlap among map, nodes and pool metadata, arbitrary invalid pointers,
concurrent ownership mutation and reentrant map clear. The node-pool section
does not protect payload callbacks or map traversal. No idempotent retry or
native fault recovery is established.

The smallest later helper is a void operation on actual bucket base, existing
raw string context and a proved borrowed node-slot context/service. This packet
creates no type, global, owner, callback family or private registration domain.
The next bounded packet should establish the exact node-slot return and actual
owner provenance. If Native work is separately authorized, size-check00736090
and compare its complete body with the retained73C27A..73C2DC inline schedule;
0073C220 requires separate body evidence before any wrapper claim. Stop at an
unresolved owner/exception contract. Allocation, startup/teardown, the derived
locale destructor and its EH/ranges, loader consumers and application ownership
remain separate work.

The finite proof retains whole current/Git Source, whole committed dependencies,
reused Native409 bytes, and eight complete worker COFF objects from the existing
build. Whole worker artifact identity is unchanged at capture; the four artifacts
themselves are not copied into this bundle. The complete node-helper normal
section remains103 bytes/37 instructions. No new compilation or execution of
reconstructed code is performed. Strict ZIP-only read-only verification and a
physical-copy replay both pass, including a deliberate denied external read.

Evidence: `reports/cc12_locale_raw_map_clear_source_readiness.json` and
`local/cc12_locale_raw_map_clear_source_readiness/evidence.zip` (14,611,721 bytes,
SHA-256 `b99da35346305292591123560620e6ad6c2c334ed046f880493cb64603dca7e0`).

A separate layout correction supersedes the swapped pointer/length labels in
the original `5cd08cb76` report's `layout.node`. Narrow is length+0/pointer+4;
wide is length+8/pointer+Ch. Retained Native instructions737CFD/737D0C load the
wide pointer/length, and737D23/737D32 load the narrow pointer/length. The complete
retained raw Source helpers use header-relative pointer+4 and length+0; the node
composition supplies node+8 to the wide helper. The original report, design and
ZIP remain historical, including the erroneous labels. Its original passing
replay did not check internal header offsets. A focused read-only two-ZIP replay
now asserts those offsets from the complete119-byte node body and three complete
retained Source files, and proves all other report contracts unchanged.
Its physical-copy replay also passes. Correction proof:
`local/cc12_locale_raw_map_clear_source_readiness/layout_correction/correction.zip`
(62,029 bytes, SHA-256
`353f0570034dd260be300990537eca80b36e5df5a04fdea0e53e8b5628904cfc`).
