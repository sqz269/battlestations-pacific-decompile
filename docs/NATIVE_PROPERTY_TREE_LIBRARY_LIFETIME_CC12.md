# Property tree library lifetime (CC12)

`008F4C50` and `008F56A0` now have complete ordinary C++ bodies in
`native_property_tree_library_lifetime.cpp`. These descriptive interfaces borrow
concrete canonical providers through `NativePropertyTreeLibraryLifetimeContext`.
The context contains the existing bag lifetime context and a distinct outer-node
pool pointer from the same physical process's `E17540` owner. Explicitly initialize
both `E175B0` and `E17540` before constructing the context. Neither construction nor
the lifetime bodies starts a pool or accepts a replacement callback/list.

## Recovered behavior

The complete `008F4C50` span is 254 bytes: 74 reachable instructions and one
unreachable three-byte alignment instruction. Original input is the actual map in
ECX, with no stack arguments and a plain RET; normal EAX is the original map. The
new `clear_native_property_library_map_008f4c50` returns that same map as `void*`.

For each of 64 physical heads, clear captures mapped `node+8` and then next
`node+C` before retirement. A nonnull mapped bag goes through the existing fixed
scalar destructor with flag 1; only after return is mapped `+8` physically zeroed.
The existing raw-string destructor retires the eight-byte key, then the existing
whole node-return provider returns the genuine outer 14h slot to `E17540`. The walk
advances using only the captured next pointer. Each head becomes zero after its
chain, including an initially empty chain; count at map `+4` becomes zero last.

The complete `008F56A0` body is 88 bytes and 22 instructions. Original ECX is the
actual outer 10Ch root, with no stack arguments and a plain RET; no meaningful
return value is established. The new void destructor physically writes `D1650C`
at root `+0`, clears the same map at root `+4`, physically writes `D162C8` at that
map's `+0`, then unconditionally clears that same map again. It does not free the
outer root. The second clear remains present in emitted code.

## Ordinary domain

Require genuine live 14h outer slots from the canonical `E17540` owner, intact
allocator membership, coherent 64 heads/count, canonical pooled eight-byte keys,
and actual mapped bags satisfying the complete existing bag/record contracts.
The inner `E175B0` owner remains distinct. Graphs must be finite, acyclic and
unshared; outer nodes/keys are disjoint from mapped ownership. A phase word alone
does not establish a valid object or allocator provenance.

Keep the same canonical string contexts, both pools and shared `E188B4` list live.
Externally serialize shared-list mutations and retire all keys, bags and nodes
before either pool retires pages. Existing raw-string return-gate and generation
contracts apply. Concurrency, reentry, malformed membership, aliasing, provider
failure, faults, overflow and insufficient recursive stack are outside this
ordinary domain. Calls to fixed whole providers preserve the admitted normal
behavior; Original virtual dispatch, register/FS/EH transport, class identity and
game behavior are not established by these C++ interfaces.

## Verification

The first strict MSVC Win32 compile passed with `/O2 /Ob2 /Oy- /W4 /WX /EHsc /Gy
/GL- /fp:strict`. Source bytes and modification stamps match the precompile pins.
Complete COFF inspection accounts for all nine code sections, 380 bytes and 146
instructions, including all helpers: zero gaps or unreachable bytes, seven code
relocations and no FP/SIMD instructions. The clear body is 200 bytes/71 instructions;
the destructor is 72 bytes/25 instructions. The emitted context constructor binds
both parts to the same supplied physical process. Independent complete Source/COFF
review passed without a correction. The original required repository build passed
both existing tests (50.00 seconds total); its unrelated existing LNK4006 duplicate
spawn-request-symbol warning is retained in the companion JSON report.

This worker's repository build does not register this new translation unit;
strict compilation is its new-code evidence. Root owns shared CMake registration,
ledger/Ghidra metadata and integration. No new tests, target process or graph probe
were added or run here. A genuine populated Source exercise awaits complete
`008F29A0` insertion/producer work, which is a validation dependency rather than a
direct callee of these lifetime functions.
