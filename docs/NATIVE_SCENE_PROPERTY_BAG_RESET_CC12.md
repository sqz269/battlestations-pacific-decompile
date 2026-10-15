# Whole ordinary property-bag Reset (CC12)

Original `008F5470..008F54E4` is a complete 117-byte, 45-instruction function.
Its ECX receiver is the outer bag; the initial `008F3F30` call receives `bag+4`.
The new context-bearing C++ entry preserves the ordinary operation schedule:

1. Clear the actual destination map through the canonical recursive lifetime.
2. For a nonnull source, allocate a real twelve-byte iterator and begin at its
   actual `source+4` map. This allocation also occurs for an empty source.
3. Clone the node's record through full `008F4F60`, then obtain the key through
   `00484D20`, insert through full `008F33F0`, and advance through `0047E480`.
4. Free the iterator once after the normal loop.

The destination owner at `+110h` and phase words are preserved. Reset retains
the ordinal counter at `+10Ch`; full insertion can increment it when stamping
incoming ordinal-zero records. Reset adds no second ordinal copy, root initializer, fallback
cell, invented head, callback, allocation-failure guard or exception cleanup.
The key helper's existing static empty string supplies its E186ED ordinary
stable-NUL role; the clone-specific F89450 role is not substituted.

Distinct input and destination ownership graphs must be disjoint. Exact
self-reset executes clear first and then the empty iterator path, preserving
counter and owner; it has no early self-alias shortcut. Other ancestor/descendant
overlap is outside the contract because clearing can destroy the input.

The same concrete clone/insertion/lifetime context supplies canonical key-pool,
property-node-pool, allocator-list and current-heap ownership. All selected clone
constructor and FP contracts apply. Retirement must finish before pool teardown.
This is ordinary Source with a new ABI; it does not establish Original register,
RET4/FS/SEH/OOM transport, private CRT, class dispatch, World or game equivalence.
Strict MSVC Win32 compilation and the existing repository build passed. The
repository build ran its two existing tests; Reset is not yet registered in
CMake. The report separately records the accepted full-clone dependency proof
and leaves focused Reset runtime and registered-build admission pending.
