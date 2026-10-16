# GOAL15W: shader global readers and production storage

Two whole tagged entries are reconstructed: `00B343F0..00B343F5` and
`00B34490..00B34495`, each six bytes, `MOV EAX,[absolute cell]; RET`. Neither
consumes register or stack arguments. The former returns the DWORD at
`00E13078`, the latter the pointer bits at `0108FE94`, without dereferencing
the pointer, testing it, creating an owner or guarding access. Names and pointer
interpretation are provisional Source descriptions rather than recovered types.

The installed PE and fresh target-verified Ghidra reads agree on both complete
bodies and image cells. `00E13078` is initialized writable data `4D000000`;
`0108FE94` is the loader-zero tail of writable `.data`. These are image/analysis
preimages, not observed game-process values.

## One production authority per cell

The production `GameNativeShaderProcess` already owns a live volatile register
limit, borrowed by the compiler, material pass, header and gather contexts.
The renderer application's existing publication slot is borrowed by the actual
registry constructor, compiler and destructor contexts. The change moves those
two existing values to canonical writable static Source storage and changes
their old member slots to references to that same storage. It introduces no
second production value or substitute provider. Generic low-level contexts may
still borrow caller-supplied fixture/domain storage as before.

The scalar has literal static initialization to 77 and the publication has zero
initialization. Both are available before any dynamic production owner exists.
Binding an owner reference does not reset a value written earlier. The getters
read the cells directly, without calling the lazily allocating process getter.
Both original member slots remain four bytes on the target. Full before/after
member maps agree: `GameNativeShaderProcess` is 1192 bytes with all 14 members
unchanged in offset; renderer `Impl` is 9952 bytes with all 57 members unchanged.
The real registered owner objects contain calls to the respective canonical
cell accessors, and the registered `bsp_game` links successfully. Production
publication/clear routes are established by Source inspection; constructing and
destroying the real renderer was not executed by this packet.

## Complete emitted and linked evidence

Final Source is `fdf5d3dce82a59247db18c7593deb18b16e0a276`; the combined T/W
registered build uses `262406140`. Both strict C++17 `/O2 /Gy /W4 /WX`
and actual registered Win32 objects contain exactly four whole six-byte leaf
functions: the two Original read operations and two new Source cell-address
interfaces. The latter are not counted as recovered Native functions.

All 24 executable bytes, all eight mutable data bytes, all four real DIR32
cell relocations and the absence of undefined providers/dynamic initializers
are checked. The initialized cell is actual `.data=4D000000`, the other actual
four-byte `.bss`. The read operations differ from Original bytes only in the
absolute addresses bound to the real Source storage. This is qualified Source
storage rebinding, not exact Native placement or an arbitrary masked operand.
The whole registered object equals its unique actual archive member. All four
whole contributions in the linked fixture refer to the actual same two cells.

The registered build passes all three existing checks. One focused fixture links
only the real registered core archive and checks 77/null before production owner
construction, four DWORD patterns including high-bit/all-bit values, publication
of an actual object pointer, stable cell addresses, and restore/clear. It does
not execute captured Native code or production owner lifecycle. No new registered
test or broad suite was added. The earlier layout-only header compile used C++20;
the final strict implementation compile and effective registered target use C++17.

The local artifact bundle is `local/goal15w_shader_register_limit/`: full layout
logs/objects, `class_layout_proof.json`, strict object, frozen registered object,
archive and owner objects, live reads, focused fixture/map, build log, and
`audit.json`. `reports/goal15w_shader_register_limit.json` carries hashes.

## Limits

This closes two complete Source read operations under canonical Source storage
bindings. Original global placement, live-process adoption, downstream caller
adoption, complete production lifecycle execution, fault-address identity, and
game validation remain unproved. Register/flag normal effects follow the full
MOV/RET bodies; no claim extends to Native address faults or different owner
initialization/lifetime domains. Existing larger compiler, registry and traffic
parents retain their own unresolved contracts.
