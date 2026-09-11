# Native node pool owner and static lifetime

Packet `orch3_node_pool_owner_p`, 2026-09-11. Six complete native bodies are
reconstructed in `src/native_node_pool_owner.cpp`, using the same physical pool
as `native_node_pool_allocation.cpp` and the application's existing
`AllocatorListDomain`. Names are descriptive hypotheses, not recovered symbols.

## Native interfaces and evidence

| Address | Inclusive end | Original ABI |
|---|---|---|
| `00B6E980` | `00B6EA52` | ECX fresh actual 38h owner; EAX same owner; RET |
| `00B6E3D0` | `00B6E459` | ECX actual owner; RET |
| `00B6EA60` | `00B6EAFF` | ECX actual owner; native D62C78 virtual +00; RET |
| `00B6DDF0` | `00B6DDFD` | ECX actual 0Ch table header; RET |
| `00CD7D10` | `00CD7D25` | No arguments; EAX actual atexit status; RET |
| `00CE0E20` | `00CE0E29` | Select 0108FF58; tail JMP B6E3D0 |

Live Ghidra and installed-PE bytes agree across all six spans; lengths and
SHA256 values are in `reports/native_node_pool_owner.json`. Analysis verified
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe` before each batch.
The saved `CG_static_init_00ce0e20` name was wrong: CD7D10 registers CE0E20 as
the exit callback, after constructing the pool. CE0E2A is padding, outside it.

The flow repair restored returning-free tails at B6EA39, B6E3ED, B6E405,
B6EA86 and B6DDFC. All four affected functions have complete stored bodies
and zero remaining call gaps. The three-byte B6EA6D alignment after a jump was
left alone. CD7D10 and compiler handler CC19CE were defined from verified
bytes under the shared write lock; the project was saved. Mutation records
are in the accompanying `*_flow_p.json` and `*_definitions_p.json` reports.

## Actual owner construction and destruction

The 38h owner contains the integer profile at +00, actual allocator-list links
at +04/+08, real Win32 critical section at +0C, signed tracked depth at +24,
slab table at +28, unsigned count/capacity at +2C/+30, and earliest nonfull slab
at +34. Construction prepends this same prefix through the existing E188B4
domain, publishes D62C78, initializes the section, writes depth/table/count/
capacity zero and earliest FFFFFFFF, then reserves 32 pointer cells. Capacity
is published before the 80h allocation. The copy loop rechecks current count
and current backing; it skips stores through a null destination, advances the
captured destination, frees current old backing, then publishes the replacement.

CC19CE..CC19D7 selects descriptor DFA8C4 and jumps to the existing CRT handler
BF6B43. Its DFA8AC unwind map is state 0: base unlink via CC19B0/403970;
state 1: section cleanup via CC19B8/402F70; state 2: table cleanup via
CC19C3/B6DDF0. The constructor's main body advances directly from 0 to 2 after
section and metadata initialization. The rebuilt Win32 function preserves
that cleanup order using SEH termination handling. Original exception dispatch
and allocation-failure execution have not been differentially tested.

Destruction captures whether count is nonzero before writing D62C78. It frees
slabs in ascending index order, reloading current table/count, then frees
nonnull current table backing. It drains only positive signed depth before
deleting the critical section and uses canonical base unlink through the same
list domain. It invokes no payload destructors and does not free the physical
owner. Pointer/count/capacity and the element's own stale links remain unchanged.
The small B6DDF0 helper likewise captures and frees backing without clearing
any table-header field.

## Empty slab compaction

B6EA60 does not take a lock internally. A slab is empty when its WORD at
+2F40 is exactly 32. After freeing it, the routine reloads table/count, moves
the last table entry into the hole, decrements count, and rewrites all 32
trailing slot IDs in the moved slab to the new table index. These DWORDs follow
each 174h payload at stride 178h; payload bytes remain intact. Decrementing
the loop index makes the next iteration inspect the moved slab, including
the unsigned wrap at index zero. This is necessary when several slabs are empty.

After compaction it sets earliest to FFFFFFFF and, using one captured table
cursor with current count bounds, finds the first remaining nonfull slab.
Capacity and unused table cells are retained. The rewritten IDs are essential:
B6E490 resolves a returned slot through its current stored slab index.

## Static binding and CRT exit

The setup helper borrows the actual 0108FF58 storage and the same E188B4 list
domain, and binds D62C78/+00/B6EA60 to the reconstructed trim operation. It
changes no raw owner byte, list link, or CRT callback. There is no replacement
pool or list. Both borrowed objects must survive through the exit callback;
setup occurs once before startup. Rebinding and repeated startup are outside
this contract.

CD7D10's source binding constructs that owner, calls real `std::atexit` with
the CE0E20 source binding, and returns the actual registration status without
rollback. CE0E20 destroys the same owner. The trim dispatch uses the existing
domain's checked current-profile binding; integer native vtable addresses are
evidence identities, not executed function pointers.

## Validation and remaining work

The registered source passed the strict MSVC Win32 build and both existing
CTests. One ignored fixture compares all six original spans with reconstructed
operations, using current allocator/free services, real Windows import cells,
and actual fixture storage/list pointers as explicit rebindings. The original
constructor exception-handler immediate remains original; only its normal path
was executed. No permanent test was added.

The fixture constructs two owners in one list, allocates 65 slots, empties the
middle slab, verifies moved IDs and unchanged payloads, and returns remaining
slots through the updated indices. It also checks repeated empty-slab removal,
retained capacity, real domain trim dispatch, list unlink ordering and stale
links, positive lock-depth drain, and null/nonnull table cleanup. Two child
process modes separately exercise original and rebuilt static hooks through
real CRT atexit. A later-running marker verifies the pool has been destroyed.
Binding the rebuilt static pool preserves all 38h input bytes and the list head.

These are new C++ interfaces, not binary or original exception-ABI replacements.
The shared checkout already supplies the 174h B6F5A0 constructor in
`native_node_construction.cpp`. It accepts a physical slot and currently wraps
`SizedStoragePool` in the semantic `PooledStringStorage`; composing it with this
pool and the actual native string-pool bridge remains work. The retained terminal
binding, full point-effect construction and gameplay are unvalidated here.

## Follow-up packet

Reuse the existing B6F5A0 implementation and connect its string operations to
the existing actual NativeStringStorage bridge, retaining its current semantic
adapter for callers that still use it. Compose allocation/construction with
the same NativeNodeBinding and real terminal dispatch. Claim the relevant
addresses and files first; do not reconstruct a second node-storage type.
