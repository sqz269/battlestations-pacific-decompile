# Actual renderer resource release

Addresses: 00b262c0

`release_native_renderer_resources_00b262c0` reconstructs the complete468-byte
B262C0..B26493 normal body over the actual renderer. The original interface
receives ECX, no stack arguments, and returns with RET. Its descriptive name is
a hypothesis. The new C++ context borrows the existing binding, resource and
full cache-clear domains; it adds no resource owner, registry or mirrored cache.

Optional guard entry occurs before the ready-byte read at renderer+1D8B.
Native state0 is armed after that read. When ready is nonzero, the routine
clears it before twenty texture unbinds, four vertex calls with index zero,
and one index unbind. Every dispatch rereads the current renderer profile/slot.
It then releases the current optional depth surface, four current color
wrappers, the live query list, texture list and registered surface list.

Query iteration rereads its signed count before each next index and reloads
the array before each owner. Texture and surface iteration preserve their
captured raw cursor, then reload count before base after each callback to
compute the next end. Relocation does not reset that cursor. The separate
2Ch record walk reads count/base and possibly count again, but performs no
owner load or callback. It must not be replaced with B29670's record release
callbacks. Full B241C0 runs on the cache at renderer+34 last.

Current D5F0A8 slots130/134/138 select full B24710/B24840/B24B00.
Resource profiles D619A0, D62AD0 and D61948 select full B3D510 surface,
B5FE20 query and B3DD30 2D-texture release. The genuine cube/volume B33F10
callback is RET. Each nested provider retains its own captured/current COM
and ownership schedule. Numeric original addresses select these concrete
implementations; they are never executed as host pointers.

The only native unwind action is CBD120, LEA ECX,[EBP-14] then JMP B21110,
under FuncInfoDF581C and state mapDF5814. Normal completion reads current mode,
disarms, then conditionally leaves using the saved guard. Exceptional cleanup
executes B21110 without rolling back resource work; a second C++ cleanup
exception terminates. Native-valid mode changes must not expose an uninitialized
guard when entry was skipped. Hardware faults and full FH3/SEH interoperability
are not implied by these source scopes.

Nine fresh saved-Ghidra/installed-image spans cover the full468-byte routine,
EH data and current dispatch cells. Strict owned-source Win32 compilation
passed. The report records later combined build, independent review and any
full-parent original-byte fixture separately. Leaf fixtures and cache tests do
not themselves prove this parent. B29670 device recreation, complete render
composition and gameplay remain unverified.

## AV independent review

Independent complete-listing review confirmed the native ready/state0 order,
current resource traversal, callback-free record walk and cache/guard sequence.
The final leave path now loads the full saved guard DWORD after disarming,
matching the native access width even though B33B00 ignores that argument.
The final report distinguishes parent fixture coverage from leaf evidence.
