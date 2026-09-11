# Native hardware-layout tree lifetime

Three complete native functions now operate on actual 28h nodes and borrowed
12-byte tree headers. The strict MSVC Win32 build and both existing CTests pass.
One ignored original-instruction differential fixture matches 13,747 behavior
DWORDs, 43 actual node frees, nine handler calls and 14 observed output stores
on each side. The three original function bodies total 309 bytes and execute
without patches. This is bounded reconstruction and fixture evidence, not
binary replacement or game validation.

| Entry | Native ABI | Reconstructed behavior |
| --- | --- | --- |
| B25DC0 | ECX unused; no stack args; EAX allocation; RET | Actual shared allocation of 28h raw bytes, conditional link clears, black1 and nonsentinel0. |
| B230B0 | ECX tree; stack node; RET4 | Destroy current right subtree, capture current left, free node, continue through captured left. |
| B2F3A0 | ECX tree; stack output and first/last iterator pairs; EAX output; RET14h | Full-range reset or checked partial-range increment/erase, then publish OWNER followed by NODE. |

B25DC0 leaves key/value bytes and final padding untouched. The caller sets
the allocated node's sentinel byte to 1. Each original computed-address check
is retained, but the established shared allocator cannot return null
successfully; mandatory later writes require backed storage. No constructor
callback, rollback policy or substitute allocator was introduced.

B230B0 contains an 11-byte continuation at B230D4..B230DE that the current
Ghidra listing omits after the call to returning free. Those bytes clean the
stack, test the captured left child's sentinel, select that child and loop.
The full 53-byte body matches fresh live Ghidra bytes and the installed PE.
The implementation reloads the current left link after right recursion, then
captures it before freeing the original current node. It leaves sentinels,
tree headers and borrowed values alone. The primary integrator owns the
Ghidra flow repair, evidence comments, naming, ledger entries and export refresh.

B2F3A0 preserves checked-iterator identity and instruction order. It captures
the current minimum before first-owner validation, reads the first node after
the handler, and captures the current head before last-owner validation. The
full-range branch then destroys the current root and reloads the current head
for root/extrema reset. It captures the minimum before writing output owner,
so output may overlap the header without changing the selected result node.

The partial branch validates first-owner against last-owner before every
equality check, including the final equal pair. It advances the actual local
first iterator through existing B20DC0, erases its captured old owner/node
through existing B2EF00, then reloads the advanced iterator. The output receives
the captured final owner and node. Returning handlers do not create an added
repair policy, and throwing handlers propagate before subsequent writes.

The private fixture exercises raw allocation and allocation failure, full
right/node/left destruction, sentinel skipping, full and partial erasure,
returning and throwing owner checks, and output overlapping actual head/count.
Partial erasure runs the original increment, erase and rotation bodies over a
valid seven-node red-black tree, including successor transplant and black
fixup. The free observer deliberately changes selected left links to test
post-recursion reload and pre-free capture; these controlled perturbations
are not claims about normal CRT free behavior. A read-only output page and
single-step observer verify the actual OWNER-then-NODE instructions.

All 48 captured spans, totaling 2,429 bytes, were checked live against the
installed executable and checked again against their complete loaded
postimages. Only the recorded 29 absolute relocations, 12 existing service
entry bridges and three existing host FH3 registration adapters are allowed.
The original lifetime bodies remain unchanged. The complete existing native
erase exception closure is loaded; this packet does not repeat its separate
exception-construction validation. The new source links the actual current
tree and allocation implementations, with no new operation callbacks.

Source and public contracts are in
`include/bsp/native_hardware_layout_tree_lifetime.hpp` and
`src/native_hardware_layout_tree_lifetime.cpp`. Exact spans, native ABIs,
source/object/library pins, postimages, provider map lines, fixture artifacts
and behavior evidence are in
`reports/native_hardware_layout_tree_lifetime_audit.json`. Shared CMake and
metadata integration remain the primary agent's responsibility. Global tree
initialization/shutdown and hardware-layout value ownership are separate
packets; malformed graphs, concurrent mutation and game execution remain
unvalidated.
