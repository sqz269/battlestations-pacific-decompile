# Type-55 concrete container bindings and receive values (R193)

## Result

`NativeMessage55StandardLibrary` supplies all seven library operations required
by the type-55 source context. It uses the installed MSVC `std::list`,
`std::vector` and `std::destroy`, with the existing singleton allocation/free
and string-pool providers. The production source does not execute original
helper bytes or reconstruct the STL implementation.

Two game-specific value methods are reconstructed:

| Address | Bytes | Operation | Native entry/return |
| --- | --- | --- | --- |
| 008E0950 | 104 | Copy construct a 1Ch receive record | ECX destination, stack source, EAX destination, RET 4 |
| 008DDFE0 | 35 | Destroy the receive record's owned string | Stack record pointer, RET 4; ECX is not the receiver |

Names are descriptive hypotheses. Evidence uses the existing
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, and original PE.
The source interfaces take an explicit pool context and change the binary ABI.

## Value semantics

The copy constructor first zeros the destination string header. When source
and destination differ, it resizes with preserve=true, reloads the source
length, and copies the destination length bytes through the overlap-capable
memory-copy contract. It copies byte +8, WORD +Ah and pointer +Ch, while leaving
byte +9 untouched. Each float at +10h, +14h and +18h is transferred by its own
x87 FLD/FSTP pair. These operations preserve the original sequential behavior
and masked floating-point exception flags, including signaling NaNs.

Self-copy zeros the source header too and does not free its former allocation.
This surprising behavior is preserved. The fixture captures and separately
releases that allocation. The destructor returns only the owned string and
leaves the header and payload bits unchanged.

## Installed library contract

The verified configuration is MSVC 14.51.36231, Win32 Release, `/MD`,
`/std:c++17`, `/fp:strict`, `_ITERATOR_DEBUG_LEVEL=0`. Compile-time checks cover
the 8-byte list, 12-byte vector, 12-byte node with next/previous/value at 0/4/8,
and 28-byte receive value. Other iterator-debug layouts fail explicitly when
the provider is constructed. This is a compiler-specific raw-storage binding,
not a portable standard-library ABI.

The native retained DWORD precedes each container. The list is constructed at
the sentinel field; the vector is constructed at the begin field when empty.
The stateless allocator uses the actual singleton raw allocation backend and
does not add the modern STL allocator's large-allocation alignment header.
Thus the reconstructed game destructor can free the vector's raw base and
entry list nodes through that same backend. List and vector maximum counts
are 3FFFFFFFh and 09249249h. The receive value has no move constructor, so
vector relocation copies its owning string rather than stealing its header.

The by-value resize argument adopts the already-constructed native value's
representation; normal type-55 callers provide a zero string header. A scoped
thread-local binding supplies the pool to STL element lifetime operations.
It is a source-only adapter, not a recovered native global. `__try/__finally`
restores the previous binding, including when an SEH exception escapes.
The borrowed pool context must outlive every operation.

| Native helper | Binding |
| --- | --- |
| 008DB560 / 008DB4F0 | Construct list and return its sentinel |
| 008E0EC0 | Resize pointer list, using the provided payload value |
| 008E17C0, with insertion helper 008E1060 | Resize receive vector with a copied value |
| 008DE660 | Destroy receive values only; caller owns allocation release and header clears |
| 008DB580 / 008DBAB0 | Destroy list nodes and sentinel, then clear head/count |

These eight addresses remain library dependency contracts, not reconstructed
game functions. The two 26-byte sentinel helpers allocate a node and return
it with both links pointing to itself; they do not consume ECX as a receiver.
The 56-byte range destructor uses ECX begin, EDX end and RET 8, with two unused
stack owner arguments. It releases values in ascending address order.

Three library listings were repaired under the shared Ghidra write lock:
`008DB580` now includes the full 72-byte body, and returning-free gaps in
`008E0EC0` and `008E1060` were restored. No CALL gaps remain. The list-resize
body retains a three-byte alignment gap after an unconditional jump.

## Validation and limits

The strict Win32 build and three existing CTests pass. The original/source
fixture compares **1,183 pairs and 2,134,893 bytes**. It retains R192's 1,055
constructor, predicate, nested record, handle and creator comparisons, and
adds 128 focused value-copy/destructor comparisons: 16 float-pattern groups,
four x87 rounding modes and separate/self-copy. String lengths cover
0, 1, 149, 150 and 511; retained byte +9, guards, return identity, string-pool
state, unchanged destructor headers and x87/MXCSR exception flags are checked.
One inherited source-only cleanup fault still passes.

The source side uses the concrete MSVC provider throughout. The comparison
side executes the original helpers, including these two game value methods.
The byte corpus is 19,462 live/PE bytes with 472 fixture relocations. Four new
value-method edges and 73 inherited game-body edges are ownership checked;
the raw factory call at `00768C5E` remains separately recorded because it has
no stored Ghidra function membership.

Modern STL reallocation copies newly appended values before the old prefix;
the original insertion helper copies the prefix first. The tested type-55
domain uses append-by-one with a zero-header template and matches observable
results, pool state and floating-point flags. This does not prove arbitrary
insertions, aliased nonempty template values, allocation failure, overflow,
invalid iterators, intermediate callback publication, concurrency, or native
FH3 exception behavior. Error-only original fixture paths fail immediately if
entered. Modern exception types and cleanup schedules are not assumed equal.

The provider is available for explicit context composition. Full stream
factory binding, packet recorder, network/startup composition, whole binary
ABI and gameplay remain open. The report pins the tested artifacts and the
subsequent merged-build validation separately.

Evidence: `reports/native_message55_library_r193.json`.
