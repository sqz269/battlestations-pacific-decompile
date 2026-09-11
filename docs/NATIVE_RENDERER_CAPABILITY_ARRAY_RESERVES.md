# Native capability array reserves

Full `B22B30[130]` and `B236B0[95]` now have raw MSVC Win32 source interfaces.
They reserve 12-byte records and DWORD elements respectively, using the actual
three-word `{data, signed count, signed capacity}` header. This closes two
primitive dependencies of the capability gather; it does not implement the
remaining nested arrays or `B2C8E0` itself.

The original ABI is ECX header, one request DWORD in actual callee argument
storage, `RET4`, with no semantic result. The public fastcall declarations add
an explicitly ignored EDX argument so the request retains that stack position.
They are new source interfaces, not binary caller replacements. `B22B30` writes
the fresh allocation to its actual request slot at `B22B60` and reloads that
slot at `B22B97` after EBP has served as a row-copy temporary. That slot belongs
to the callee's caller-prepared arguments, not to the header or its owner.

Both entries signed-clamp the request to at least one, compare signed current
capacity, and return immediately when it suffices. The no-growth path does not
read data/count or call an allocator. Growth computes wrapping 32-bit bytes
using stride12 or4. After allocation it reads current count/data and retains
the native per-word load/store order. The count comparison and source-base
load are repeated each iteration. A null computed destination skips that
element's stores. No capacity/count consistency, overflow or extent repair is
inserted into either body.

The current old data pointer is captured only after copying, then freed before
fresh data and the captured requested capacity are published. Count is never
written by either entry, including after provider reentry. Negative current
counts therefore skip copying without being repaired. Neither entry supplies
rollback if an allocation or reached raw access fails.

Saved analysis omitted returning-free fallthrough at `[B22BA3,B22BAD)` and
`[B23701,B2370A)`, following calls `B22B9E` and `B236FC`. Both complete original
spans and source include the missing stack balancing, publication and register
restoration. The worker made no Ghidra flow or global `_free` changes; primary
review/integration handles those separately.

## Concrete allocation boundary

The only adapted instruction operands are four direct calls. Each allocation
call binds a fixed cdecl bridge to
`singleton_lifetime_allocate({object, bytes, bytes})`. Native `BF55BE` forwards
to full `BF681B`; the existing source service calls current CRT `malloc`, then
current `_callnewh` on failure, retries when it returns nonzero, or throws
`std::bad_alloc`. The wrapping native byte count is passed unchanged in both
request size fields. No callback or separate allocator policy is introduced.

Free calls use a fixed cdecl bridge to `singleton_lifetime_free`; native
`BF6989` forwards through `BF65AC` to `_free`. Source backing allocations must
belong to that existing shared heap. All reached raw accesses require valid
readable/writable extents for the actual native arithmetic and lifetime.

The bridges use ordinary Win32 cdecl stack cleanup and preserve nonvolatile
registers, including the fresh allocation retained in EBP/EBX by the reserves.
The complete bridge and linked service code are included in the object/link
proof. Caller-save registers, flags and incidental register return values are
not an interface guarantee. Allocation may throw; the naked reserves add no
EH frame or rollback. This packet does not establish native CRT exception
objects, native SEH/hardware-fault unwinding or original caller ABI compatibility.

## Verification

The strict actual library was built through `scripts/build.ps1` using an
ignored extra-source CMake hook. Both existing CTests passed; all eight fresh
native seed checks matched disk. No permanent test was added.

Fresh guarded queries verified the existing `bsp.gpr` and
`/battlestationspacific.exe`. Six complete spans, 345 bytes, match the unchanged
original PE. All 225 owned instruction bytes (90 instructions) match the actual
archived source sections after accounting for the four declared direct-call
relocations. The original six-byte alignment LEA is retained literally.

One focused ignored fixture links the frozen full library. For each width it
compares the complete original reserve body against the actual source entry,
with the original's two calls bound to the same complete compiled bridges.
Fixture-local malloc/free IAT forwarding calls the real CRT first, then mutates
only fixture storage. After allocation it publishes a different source buffer,
count2 and a changed capacity. After the real free it changes the header again
to another buffer, count3 and capacity-2. Both paths copy the current source,
free that current pointer, publish the fresh allocation/captured capacity4,
and retain the current count3. The untouched remainder of the fresh allocation
stays at its fixture-initialized bytes. A negative request then exercises the
signed-clamp no-growth path without a further allocation or free.

All four raw 124-byte traces are retained. Comparison normalizes only each
trace header's actual data pointer using four explicit per-run identities;
counts, capacities and payload bytes are compared literally. The actual pointer
values and identity table remain in the raw artifacts. All four original-page
CALL operands have checked preimages, targets and postimages. The complete
4096-byte private original page is unchanged after execution.

The proof covers all eight owned COFF sections and their relocations, both
exact linked archive members, ten source/header inputs, 96 mapped sections,
58 actual import providers, and 8,368 bytes of whole runtime code postimage.
It strips every leading `f`/`i` link-map flag when establishing object ownership.
No provider was recompiled or replaced for the fixture; only its executable's
two import cells temporarily forward through the explicit fixture observers.
The real CRT module code and original game are unchanged.

Runtime proof is limited to this valid-extent allocation/free mutation case.
Allocation failure/new-handler execution, null destinations, malformed extents,
C++ exceptions and native hardware/SEH unwinding were not exercised. Original
reserves execute with full compiled source providers, not original CRT bodies.
There is no gather, gameplay or drop-in ABI claim. Exact hashes and the local
immutable bundle paths are recorded in the accompanying audit report.
