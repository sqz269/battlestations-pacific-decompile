# Raw property library lookup

This packet supplies complete ordinary Source readers for the library's actual
eight-byte key headers and 108h map. It also adds a distinct retained initial-NUL
authority for Original `00E18560` to `GameNativeStringProcess`. The readers borrow
storage; they neither create nodes nor acquire mapped-bag ownership.

| Original body | Bytes / instructions | New Source entry |
| --- | --- | --- |
| `004689D0..00468A29` (exclusive) | 89 / 39 | `native_property_library_key_bucket_004689d0` |
| `00468ED0..00468F2D` (exclusive) | 93 / 41 | `find_native_property_library_node_00468ed0` |

The names are descriptive hypotheses. Original hash takes the header at entry
`ESP+4`, returns the bucket in EAX, and executes `RET4`; ECX is not an input.
Original lookup takes the map in ECX and query/output pointers on the stack,
returns a node or zero in EAX, and executes `RET8`. The context-bearing ordinary
C++ declarations do not reproduce those stack/register interfaces.

## Hash and lookup order

The hash physically reads header data at `+4` before length at `+0`. Null data
selects the distinct `E18560` role. It uses `step=(length|20h)>>5`, a separate
unsigned remaining counter, and consecutive prefix bytes: each iteration
subtracts `step` from remaining but advances the data pointer by **one**. Signed
byte promotion feeds the real current CRT `toupper`; only the returned low byte
is retained. The update is `h ^= byte + (h<<5) + (h>>2)` modulo DWORD, followed by
`h & 3Fh`. Empty input selects its fallback without dereferencing it. There is no
replacement locale table, altered stride, signed loop test, or overflow repair.

Lookup publishes the bucket before physically reading `map+8+4*bucket`. A null
first head returns before loading query length. Otherwise that length is captured
once; every candidate tests node length before loading node data. Null node data
uses `E18560`. Each matching-length candidate rereads actual query data and uses
the existing `E17654` fallback, retaining the whole `00419CA0` rule without placing
a `NativeString` C++ object over raw storage. It then calls the accepted
`compare_insensitive_00438e10(query, node)` service from `entity_identity.hpp`.
That service preserves pointer equality, left-null/right-null ordering and the
current CRT `_stricmp` call. The first match returns; failures follow node `+C`.
The reader never reads map count, mapped `+8`, or allocator-owned node `+10`.

Inline physical DWORD helpers preserve these loads/stores in the selected Win32
compiler output. The hash is inlined into the lookup as well as emitted as its
own entry; both complete emissions were reviewed.

## Canonical fallback and ownership

`GameNativeStringProcess::library_empty_00e18560()` returns a separate const-zero
member appended after every established field. Its ordinary process factory
retains the object through termination. It does not reuse property `E177E4`,
query `E17654`, or clone `F89450` storage and creates no extra manager, pool,
singleton dispatcher, or application startup. The initial-zero role is supported
by the saved program byte; this is not a claim that Original writable data is
universally immutable.

Callers borrow genuine initialized storage and stable canonical fallback cells.
Keys are closed NUL-free ASCII, lengths match their buffers, null data occurs only
for empty keys, and the current CRT locale is C. Chains are finite, consistent,
correctly bucketed 14h nodes. The bucket output is writable and nonaliasing;
storage and bindings remain live without concurrent mutation or reentry.

The real outer library has a 10Ch root constructed by existing `008F5670`, with
its 108h map at `+4`. A mapped value can be a separately owned 114h raw61-produced
property bag; neither phase words nor this reader establish that provenance.
The outer E17540 node pool, whole insertion/replacement `008F29A0`, outer cleanup,
and loader/global publication remain separate work. The existing E175B0 pool
belongs to inner property nodes and is not an outer-pool substitute.

## Validation and limits

The first strict MSVC 19.51.36244 Win32 compilation used `/O2 /Ob2 /Oy- /MD /EHsc
/Gy /GL- /fp:strict /W4 /WX /std:c++17` for the new lookup and changed string-process
translation units. The original tool handle and natural successful terminal are
preserved; no replacement compile was used. Source copies and first objects are
frozen in `local/WholePropertyLibraryLookupSource01`.

The complete lookup object contains five code sections, 498 bytes and 193
reachable instructions. The changed string-process object contains thirteen code
sections and fourteen function entries: 419 reachable code bytes / 125
instructions plus five unreachable INT3 alignment bytes. All 922 emitted section
bytes, 24 code relocations and 13 non-code relocations were accounted for and
compared byte-for-byte with complete dumpbin transcripts. The process includes
its ordinary current-compiler unwind handler and associated metadata; reviewing
those does not establish Original exceptional-unwind equivalence.

The emitted new accessor returns `this+3E`; property/query/clone accessors retain
distinct offsets `2C/2D/3D`. Both the standalone constructor and its inlined factory
emission zero byte `+3E`. Neither selected translation unit emits x87, SIMD or
floating-point-environment instructions. The comparison implementation and all
existing providers remain accepted dependencies, without a repeated ancestry or
runtime claim. The required repository build result is recorded in the report.

No new test, probe, or Target process was added. This packet does not claim
Original class/vtable/register ABI, historical CRT, arbitrary locale/non-ASCII,
fault or exception transport, final linked retention, application wiring, or
game validation. Root owns CMake registration and shared ledgers; the new lookup
TU's registered Main compilation remains a separate integration gate.
