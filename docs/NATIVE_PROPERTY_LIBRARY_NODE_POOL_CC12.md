# Canonical outer property-library node pool (CC12)

The complete Original initializer `00CC8A70..00CC8A85` is 22 bytes/six
instructions: initialize E17540 through 00411050, register CD9270 with real
atexit, and return its registration result. The complete ten-byte/two-instruction
CD9270 tail wrapper destroys the same owner through 00410A60. No repeated
startup, alias callback registration, rollback or failure/EH parity is admitted.

The additional Source binding supplies a distinct aligned 38h E17540 owner for
14h outer-library nodes, using the existing canonical E188B4 allocator list and
unchanged CE37A4 trim/initialize/allocate/return/destroy algorithms. It aliases
neither inner-record E175B0 nor sibling E175E8 nor physical-provider0109DBF0.
Source context/state fields are appended after the canonical process's old
fields. No new process, allocator list, generic pool implementation or callback
profile is created.

Explicit `initialize_property_library_node_pool_once_00cc8a70()` commits one
Source attempt before binding/construction, caches the real registration status
(including nonzero), and forbids retry after a thrown attempt. This is Source
host policy; the Original initializer has no once guard. The guarded
`property_library_node_pool_storage_00e17540()` returns the same borrowed
mutable owner only after startup returned. Nonzero registration retains an
initialized owner without the callback; it is not repaired or rolled back.

Constructing the canonical process precedes explicit callback registration, so
reverse CRT order keeps its owner cells/list alive through their callbacks.
Externally serialize all shared-list startup, trim and teardown; separate
owner mutexes do not supply cross-owner synchronization. Retire mapped114h bags,
their inner records/keys/nodes, then outer keys/nodes before either pool's pages
are released. A raw pool callback does not destroy a populated library for you.

E17540 startup/access supplies no populated-node provenance, library lookup
E18560 role, root constructor, application wiring or game/class/Original ABI
admission. E18560/string-process work remains separate. Strict compilation and
selected complete COFF inspection of the new functions, the required existing
worker build and future registered Main evidence are reported separately.

The first strict MSVC Win32 compilation of both changed translation units passed
(`/O2 /Ob2 /Oy- /W4 /WX /EHsc /fp:strict`). Complete selected emitted sections
cover all five new APIs and the affected process constructor, accessor,
destructor/callback and generated exception fragments. The worker's required
repository build also passed its two existing checks (86.20 seconds). This is
worker Source/build evidence; current registered Main validation is separate.
No new test or reconstructed lifecycle process was executed for this packet.
