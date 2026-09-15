# Plain-node and group resource factories

Addresses: `00B866C0`, `00B86780`, `00B8F5E0`, `00B6F5A0`, `00CC2510`, `00CC2550`.

The graph builder `00B891A0` selects `00D63210` for a plain node and
`00D63220` when record flag `+58h` bit 0 forces a group. Their slot `+04h`
factories now have complete source implementations. The existing default
model-base factory `00B86720` remains in `native_model_base_lifetime.cpp`.
No workers were dispatched for this packet.

## Native contract

Both factories are 95 bytes. Incoming ECX is unused; the single stack argument
is an actual eight-byte string header, with length at `+00h` and data at `+04h`.
Both return the constructed owner in EAX and use `RET 4`.

| Factory | Allocation | Constructor | Failure action |
| --- | --- | --- | --- |
| `00B866C0` | ECX size `174h`, `00B6ED70`, actual pool `0108FF58`, slot `178h` | complete `00B6F5A0` | `00CC2510` -> `00B6E670` |
| `00B86780` | ECX size `188h`, `00B8F450`, actual pool `010902F4`, slot `18Ch` | complete `00B8F5E0` | `00CC2550` -> `00B8EEB0` |

The captured allocation is returned only when its constructor throws. The
allocation itself precedes the protected construction state. A null allocation
returns null without calling a constructor. Neither factory retains an owner,
publishes a root, starts a type registry, or allocates a substitute pool.

`00CC2518` selects FH3 FuncInfo `00DFB944`, whose one-state unwind map at
`00DFB93C` contains predecessor `-1`, action `00CC2510`. `00CC2558` selects
FuncInfo `00DFB99C`, with map `00DFB994`, predecessor `-1`, action `00CC2550`.
Each eight-byte action loads `[EBP-10h]` into ECX and tail-jumps to the correct
pool return routine. The source propagates C++ constructor failures after that
return; it does not implement the native FH3 runtime.

## Group constructor composition

The existing complete 106-byte `00B8F5E0` constructor now also accepts the
actual string header and `NativeStringRawPoolContext`. It calls the established
814-byte raw `00B6F5A0` implementation. Both the existing semantic interface and
the new raw interface share the same group-tail implementation.

The tail reads the current `00D7A24C` word after base construction, stamps
`00D634F8`, clears the actual `178h/17Ch/180h` attached-node header, ORs `138h`
with 2, writes that captured word at `184h`, sets byte `175h`, zeros `08h..10h`,
then reads current `00CE4970` into `14h`. These are SSE bit transfers in the
original. They preserve signaling-NaN bits and introduce no extra x87 conversion.
Bytes `174h`, `176h..177h`, and the pool index at `188h` remain untouched.

## Verification and limits

Strict MSVC Win32 build and both existing CTests pass. One retained probe runs
two paired factory sequences, each constructing three owners with empty, short,
and 300-byte names. The original side executes both complete factory bodies,
the complete group constructor, and the complete node constructor. The explicit
external calls use the same complete canonical pool allocation, raw string resize,
CRT copy, and matrix-copy implementations as the source side.

The probe compares every owner/slot word, copied names, pool free-stack state,
string-pool counters, allocations, and x87/SSE status. The large-name allocation
changes the borrowed constant cells to signaling NaNs before returning; this
checks the constructor's subsequent current reads and bit-versus-x87 transfers.
Two source-only injected allocation exceptions verify base cleanup and recovery
of the exact captured slot. Original FH3 entry points are fail-fast tripwires,
and the null-return branch is inspected statically rather than fabricated by an
allocator stub. Twelve direct-call/tail-jump rows are checked against Ghidra.

Initial diagnostic runs used an invalid reversed name-header layout and exited
with access violation. The fixture was corrected from `NativeString` and the
original `00B6F64E..00B6F66B` listing; production code was unchanged. The empty
name capture also avoids dereferencing a valid null data pointer. The passing
fixture uses the real length/data layout and complete providers.

These are new C++ interfaces over actual storage. The established raw-node
materialization boundary, native exception identity, private stack aliases, and
asynchronous observations remain outside this proof. Diagnostic teardown releases
names and raw slots; it is not proof of live scene/group destruction. Canonical
group companions, type publications, item providers, and executable model/resource
admission still have to be composed by the application. This packet does not
establish a runnable populated graph or gameplay validation.
