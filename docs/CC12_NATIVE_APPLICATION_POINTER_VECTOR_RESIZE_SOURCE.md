# Application pointer-vector resize Source

This packet implements the complete ordinary `00735F30..00735F7F` schedule
using the existing `NativeApplicationPointerVectorStorage`,
`NativeApplicationPointerVectorAllocation`, and concrete
`reserve_native_application_pointer_vector_00735ec0` service. It adds no
Application owner, table, callback domain, dispatcher or production wiring.

The public function is
`resize_native_application_pointer_vector_00735f30(actual_rows, requested, allocation)`.
It is an ordinary C++ interface with explicit existing bindings, distinct from
the original native `ECX` receiver, signed stack request and `RET4`. The
descriptive name is provisional. The original has no established semantic
EAX result.

Source is authored and statically reviewed. Registration, the normal Win32
build, complete emitted core/adapters/actual reserve binding review, naming and
admission belong to Root and were not performed by this worker. There is no
new Original credit or binary-compatibility/runtime claim.

## Complete schedule and preserved edge cases

The [accepted native audit](CC12_APPLICATION_VECTOR_RETIREMENT_READINESS.md)
established all **80 bytes and 31 instructions**. Every native site maps to an
explicit statement in the private naked function; the
[Source report](../reports/cc12_native_application_pointer_vector_resize_source.json)
records all mappings and current Source pins.

| Native sites | Source behavior |
| --- | --- |
| `735F30..735F3E` | Preserve ESI/EDI; capture actual header and signed request; compare current signed capacity; call the actual reserve service only when request is greater. |
| `735F43..735F64` | Load post-reserve current count into EAX; compare signed; reload data every iteration; calculate low32 `data + EAX*4`; test that calculated address; conditionally zero its DWORD; increment EAX and compare signed again. |
| `735F66..735F76` | Compare request to current actual count; if smaller, set EAX to all ones and repeatedly ADD it to the count field, with a fresh signed recompare each iteration. |
| `735F78..735F7F` | Unconditionally write requested count, restore EDI/ESI, and return with four-byte argument cleanup. |

Growth does not directly publish the header count. Its slot calculation uses
native 32-bit wrapping arithmetic, and its null test is on the resulting slot,
not on the data base. Caller-specific aliases are not blocked or replaced.
Shrink changes the actual count one step at a time; it reads and destroys no
retired element. There is no request-zero fast path or early final store.

Negative count, capacity and request values retain signed branch behavior.
Index, calculated-slot and decrement arithmetic retain native wrapping, with
no C++ signed-overflow substitution. Invalid reached storage can fault after
preceding stores or child effects, and cyclic behavior is not guarded away.
No validation, `noexcept`, catch, rollback or new cleanup is added.

The exact six-byte `8D 9B 00 00 00 00` at `735F4A` and two-byte `8B FF` at
`735F6E` are written with `_emit`, preserving the physical no-op encodings.
All native branch labels and the stack request displacement remain in the
original schedule.

## Concrete call graph and context adaptation

1. The public noinline C++ function borrows the existing header and allocation
   binding, then calls private `resize_body(&rows, &allocation, requested)`.
2. That naked private `__fastcall` body receives the header in ECX, the extra
   Source allocation pointer in EDX, and the signed request on the stack. It
   preserves the native register/stack schedule and ends with `RET4`.
3. The only call at native `735F3E` targets the concrete noinline
   `reserve_for_resize` bridge. It receives the same ECX header, EDX binding
   and pushed request, then directly calls the existing reserve function with
   references to those same objects.

EDX is untouched by every instruction preceding that call. No later core
instruction requires the binding, so the extra Source context needs no spill,
stack-offset change, new global, copied callback table or default provider.
The path that skips reserve never consumes the allocation binding.

The intended private core layout remains 80 bytes: all 76 non-relocation
bytes should match the original, including the E8 opcode, with only its four
relative call-target bytes redirected to the bridge. This is a Source review
expectation; no emitted code was produced or claimed here. Root must review
the entire core, public adapter, reserve bridge and resolved real service edge.

## Existing dependency and lifetime boundary

The borrowed storage is the existing actual 12-byte header: data at `+0`,
signed count at `+4`, signed capacity at `+8`. The implementation asserts
Win32 pointer/integer sizes and those offsets. It does not add an Application
overlay or treat projected frame bookkeeping as the header.

The unchanged reserve service owns its established minimum-one capacity
policy, signed capacity comparison, wrapping allocation size, current
source/count reads, current-old-data free, and subsequent new data/capacity
publication. The resize bridge calls it directly with the original captured
request and actual allocation binding. It does not duplicate those effects or
replace the existing allocation/free domain.

The actual reached header, data, binding and services must satisfy their
existing lifetime and allocator contracts. This helper transfers no ownership
of the borrowed header, binding or element references. It does not release
elements, free the data, destroy the outer Application, change publication,
lock or invoke a scalar destructor itself.

Native child/emitted proof, original CRT/new-handler/FH3 behavior, hardware
fault cleanup and exact service register/exception identities remain external
qualifications. The public interface and private EDX adaptation do not admit
original binary callers. Common Application construction, callable profiles,
full exceptional retirement and production migration remain separate work.

## Evidence and checks

The starting main was `98fec2587fdac0a3c4d9fe90e5a30bc05c03a6f3`. The accepted
Root capture's complete 80 bytes and 31 decoded instructions were independently
replayed against the current installed PE. No fresh Ghidra or child query was
made. Its recorded target remains `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, with function count 64729 at Root's capture.

All 31 native sites map to the authored assembly. Static checks confirm the
two exact `_emit` groups, single concrete reserve bridge, signed comparisons,
current reloads, calculated-slot test, wrapping updates, unconditional final
store, and absence of added implementation policy. Current Source/dependency
hashes and all nine input pins from the accepted audit were replayed.

Only the new header, implementation, this document and its report changed.
No CMake, ledger or GPR edit, build, test, probe or native execution was performed.
