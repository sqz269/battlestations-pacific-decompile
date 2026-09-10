# Native renderer resource alias-node construction

This packet reconstructs complete string copy construction `0044BCB0` and
alias-node allocation/copy `004CE6F0`, including its catch at `004CE75C`. It
uses the existing `NativeRenderResourceAliasNode`, `SizedStoragePool` and
native CRT allocation domain. It does not insert nodes, change a list count,
assign a resource record, or implement a length-error/iterator substitute.

The [audit](../reports/native_render_resource_alias_nodes_audit.json) records
original ABI, exclusive extents, byte preimages, EH maps, proposed descriptive
names, previous annotations, source hashes and verification limits. The target
is `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`; all selected spans
were checked against the installed PE and live saved analysis. Names are
reconstruction hypotheses. No Ghidra or shared-ledger mutation was made here.

## Actual storage and interfaces

The existing alias node is exactly `0x10` bytes on MSVC Win32:

| Offset | Field |
| --- | --- |
| `00` | Next node pointer |
| `04` | Previous node pointer |
| `08` | Owned string length |
| `0C` | Actual pooled string buffer |

`construct_native_render_alias_string_0044bcb0` takes actual destination/source
eight-byte header addresses and the caller's shared string pool. Representation
reads/writes operate on the existing `{length, data}` fields. They neither
overlay a `NativeString` object onto the alias node's typed members nor copy
state to a temporary header. A live `NativeString` representation is also a
compatible header source. Valid readable/writable native spans are required.

`allocate_native_render_alias_node_004ce6f0` takes next, previous, source header
and that same pool. It returns the actual allocation after establishing the
existing node type with placement construction. It adds no ownership wrapper,
reference count, allocator field, list header or second string representation.

## String copy construction: 0044BCB0

The complete 118-byte body is `[0044BCB0,0044BD26)`. Original ABI is ECX =
destination header, EDX = source header, `RET 0`, with no semantic return value.

A null destination takes the native early exit and never reads the source.
Otherwise the native compares identities, clears destination length and data,
then acts on that comparison. Self-construction therefore clears the header
without freeing its former buffer. The helper preserves this behavior; it must
not be used as an ownership-preserving assignment operation.

For distinct headers, the constructor reads source length and calls the existing
`0041DD40` resize operation with preservation enabled. This call site starts
from a zeroed actual header: requested zero takes resize's equal-length early
return; a nonzero request allocates `requested + 1` bytes from the string pool.

The implementation adapts this established resize behavior to the node's
existing fields because `NativeString` owns private fields and cannot borrow
them. After allocation it reads the current destination length/data, preserves
the minimum of current and requested length, reloads the old data and current
length for sized release, publishes the replacement data before requested
length, and writes the terminator. It does not assume the header remained zero
across allocation. The shared pool's native geometry and established host
projection limits remain unchanged.

The outer constructor then rereads source length. Only if it is currently
nonzero does it reload destination length, source data and destination data,
then copy **destination length** bytes. An allocation callback can therefore
change the final copy source or suppress that copy. The original terminator is
not part of this final byte count. The source uses the existing convention of
skipping a zero-byte host `memcpy` instead of passing a null source to it.

## Node allocation and copy: 004CE6F0

The normal body is `[004CE6F0,004CE75C)` (108 bytes); the parent's catch body
is `[004CE75C,004CE771)` (21 bytes). Original ABI has three stack DWORDs:
next pointer, previous pointer, source string header. ECX and EDX are not
inputs; EAX returns the actual node and the normal body ends with `RET 0Ch`.

Ordinary allocation `00BF681B` obtains `0x10` bytes before the copy catch becomes
active. The body stores next and previous, then calls `0044BCB0` on actual
node `+08`. Success returns that same raw address. Node allocation never
modifies the neighboring nodes or a list count; the later caller owns insertion.

The native allocation contract supplies nonnull storage or throws. Its emitted
placement-store null checks do not create a useful null-return path: a null
allocation would next try to write address `4`. The host uses the existing
nonnull/throwing allocation contract and adds no fallback node or null result.

## Exception responsibilities

String-copy handler `00C5FFC1` selects FuncInfo `00D86D9C`, with state 0 mapped
by `00D86D94` to leaf `00C5FFB0`. That leaf forwards its saved placement
arguments to `00401130`, whose entire body is the verified byte `C3` (`RET`).
It does not release a string buffer or restore the header. The implementation
adds no cleanup at this layer; the no-op is an original byte-verified callee.

Node handler `00C65A70` selects FuncInfo `00D8E3A4`, unwind map `00D8E380`,
try map `00D8E390`, and catch-all descriptor `00D8E370`. Catch `004CE75C`
loads the captured allocation from `[EBP-14h]`, calls ordinary free `00BF65AC`,
then calls `00BF6885` with two null arguments to rethrow. Source `catch (...)`
frees the captured raw allocation and uses bare `throw`.

No embedded string release belongs in this catch. Allocation failure before
the raw node exists is outside it. A subsequent count-growth error in
`004CE780` is also outside this helper: it cannot clean up a node that has
already been returned. Full checked range insertion `004D26A0` and resource
record assignment `00B30510` remain separate prerequisites.

## Verification and limits

One focused native/host fixture passed 426 matching normalized words. It
executes the original copy constructor, node constructor and complete
`0041DD40` resize instructions. The original pool getter/allocation/release
boundaries route to the same concrete `SizedStoragePool` implementation used
by the host path. Its large-block allocator performs real malloc/free, and
raw nodes use the shared native CRT allocation boundary with observation hooks.

The case checks null destination, empty source and destructive self-copy;
constructs two actual alias nodes; compares all normalized node fields and
356 string-buffer bytes including terminators; then destroys the populated
list through the existing actual resource-record destructor. Three raw nodes
(including the sentinel) and four system-backed string buffers are freed.
Small strings use the real sized-pool arena/ring; a callback-published four-byte
old buffer is returned with the correct size and retrieved from that same ring.

Two controlled allocation-boundary callbacks verify the late reads. The first
changes the actual destination header and switches source data/length from
177 to 172; the constructor copies 177 bytes from the new source after sized
release of the callback-published old buffer. The second changes source length
to zero; the new allocation retains its known payload preimage while receiving
the requested length and terminator. Neither construction changes list count.

A host-only tail injects a real exception from `SizedStoragePool`'s system
allocation path while copying. The raw node is freed exactly once, the source
remains owned, and the exception type/payload propagate. Native EH map and
catch bytes are verified statically; native exception dispatch and injected
native failure are **not** claimed. Successful raw allocation followed by copy
failure is distinct from raw node allocation failure, which was not injected.

All 14 code/hook spans and two data spans freshly matched live Ghidra and the
installed executable. The sparse fixture uses ten preimage-checked address
relocations, with trap bytes/inaccessible pages outside its verified spans.
The source passed strict MSVC Win32 `/W4 /WX /fp:strict` compilation. The full
baseline repository build and existing `reconstructed_math` test passed; the
primary owns CMake registration and the combined build. No tracked tests were
added. These are new C++ interfaces, not original binary ABI replacements,
complete container/texture lifetime reconstruction or gameplay validation.
