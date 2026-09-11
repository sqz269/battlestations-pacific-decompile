# Native hardware-layout tree static lifetime: independent fixture

Independent review found no defect in the frozen primary initializer and
shutdown source. Eight separate Win32 processes compare the complete original
128 instruction bytes with the actual primary library. Returning from `main`
lets real CRT `atexit` processing invoke the registered tree callback.

The four native/source pairs match 5,597 behavior DWORDs, 19 observed header
stores, 17 actual node frees and three real tree-callback registrations per
side. The strict MSVC Win32 fixture build passes with `/W4 /WX`. No production
source, Ghidra analysis, shared metadata or tracked tests were changed.

| Native body | Full span | Original contract |
| --- | --- | --- |
| CD7960 | CD7960..CD799F, 64 bytes | Allocate and initialize actual head; return real `atexit` result. |
| CE0C50 | CE0C50..CE0C8F, 64 bytes | Erase captured full range; free current head; clear head/count; normal EAX0. |

The initializer publishes the allocated head before setting its sentinel byte,
reloads current head separately for each self-link, and clears count before
registration. The shutdown captures head/minimum, calls the complete range
helper, frees the current head and clears the actual header after free returns.
The original CE0C7C..CE0C8F continuation is included. Header word0 is preserved.
The new shutdown interface is a void callback; the fixture does not claim its
unspecified EAX matches the native zero result.

Both sides use one actual relocated `108D530` header with persistent storage
and invalid-parameter domain through process exit. The native initializer
registers the actual relocated CE0C50 pointer; the source registers the exact
linked `destroy_static_native_hardware_layout_tree_00ce0c50` function. An import
observer validates that pointer and forwards it immediately into the actual
CRT `_crt_atexit` function. No private callback collector or manual shutdown
invocation is used. Validators registered before and after initialization
bracket the genuine tree callback in real CRT LIFO order.

The cases cover an empty initialized tree, a populated seven-node tree, current
head replacement at returning free boundaries, and allocation failure before
registration. Populated teardown frees nodes in right/node/left order and then
the sentinel. Controlled free observers prove that shutdown frees the current
replacement head and clears header words after the actual head free. The
superseded old head stays fixture-owned until cleanup. These deliberate
perturbations are not claims about normal CRT free behavior.

In the failure case, one malloc40 request is forced to fail. The existing actual
shared allocator invokes the installed new handler, which throws a marker.
Both initializers propagate it before header mutation or callback registration.
Real registration exhaustion was not forced; no-rollback registration-failure
behavior follows from the reviewed complete instructions and source order.

A read-only header page and single-step observer record actual write PCs and
post-store storage. Original publication/count writes occur at CD796A/CD798F;
the final original clearing writes occur at CE0C81/CE0C86. Corresponding source
writes are inside the linked primary static object's function ranges. The
trace also retains full live-node links, key/value bytes, color/sentinel and
padding, with only explicit pointer identities normalized.

All seven spans, totaling 152 bytes, match fresh live Ghidra and installed-PE
bytes. Every process's complete loaded postimages were verified: twelve
absolute static-body operands plus CRT initializer pointer CE3504 are rebased;
four captured service entries bridge exact complete production sentinel/range
helpers, shared free and real `std::atexit`. Static relative calls and all other
body bytes remain original. These helpers are shared dependencies whose own
native comparisons are documented separately; this fixture isolates static
sequencing. It invokes the initializer explicitly and does not reproduce the
whole original CRT initializer dispatcher or CRT global state.

The reviewed source is frozen at commit `7ed261e`. The copied primary library
SHA-256 is
`c90fe4e54a5191e71c8c8dcca1b52021153fb579e4baeaf0f5234de3856c5e7e`.
Fourteen source pins, fourteen provider map entries, byte evidence, all process
postimages, real CRT module pins and reproducible local artifacts are recorded
in `reports/native_hardware_layout_tree_static_fixture.json`. No source changes
were needed. This establishes bounded static-lifetime and real-exit behavior;
binary replacement, complete game startup and gameplay remain unvalidated.
