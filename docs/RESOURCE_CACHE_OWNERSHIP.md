# Resource cache ownership

Addresses: 00b7f220, 00b7f290, 00b7f6a0, 00b7ff20, 00b7ff80, 00b803b0, 00b805d0, 00b80f10, 00b81150.

The resource-name cache owns its key strings and tree nodes, and borrows the
mapped resource pointers. Normal insertion adds no resource reference; both
single-node erase and full-cache destruction free keys/nodes without releasing
resources. The initial reference supplied by the known factory belongs to the
load caller. A cache hit separately adds a caller reference, as established by
the prior load audit.

[The audit report](../reports/resource_cache_ownership_audit.json) records nine
complete function bodies, the load caller's insertion-to-return window, three
data spans, original ABIs, old/proposed names, and full decoded instructions.
All spans matched the installed PE with SHA-256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Live reads used `bsp.py ghidra`, verifying project `bsp` and program
`/battlestationspacific.exe` for each CLI batch. The previous
[classification/cache erase](RESOURCE_CLASSIFICATION_AND_CACHE_ERASE.md) and
[resource ownership](RESOURCE_ITEM_OWNERSHIP.md) files remain unchanged.

## Insertion and caller references

| Function | Established role |
| --- | --- |
| `00b7f290` | Construct temporary owned-name/raw-resource pair |
| `00b803b0` | Insert unique name or return existing node with inserted=false |
| `00b7ff80` | Allocate, link, and rebalance a new node at the chosen position |
| `00b7f6a0` | Allocate 1Ch-byte node and invoke its constructor |
| `00b7f220` | Copy the key string and raw resource value into the node |

`00b7f290` receives the destination pair in ECX, resource pointer in EDX, and
a by-value native-name length/data pair on the stack. It copies the key, stores
the resource at pair+8, tears down the incoming temporary name buffer, and
returns the destination pair in EAX with RET8. It does not modify the resource
reference count.

The node constructor `00b7f220` receives ECX node plus left, parent, right,
source pair, and color on the stack, returning the node in EAX with RET20. It
copies the key into node+Ch/+10h, then copies source pair+8 directly into
node+14h. This value copy has no AddRef, release, or item method call. Tree links
are at +0/+4/+8 and color/sentinel bytes at +18h/+19h.

`00b7ff80` rejects tree size at or above `15555554h` through a length-error
branch. Its normal path allocates the node, increments size, links/rebalances,
and returns an iterator with RET16. Its old name `STL_xlen_throw_00b7ff80`
describes only the error branch; the report proposes
`BSP_ResourceNameTree_InsertAt` and preserves the observed old name.

The known factory/base constructor initializes a new resource reference count
to 1. The verified insertion path does not add a cache reference. The previous
`00b80720` load audit separately establishes an interlocked increment on a cache
hit, which supplies that hit caller's reference. These conclusions apply to
the known factory route; arbitrary factories still need an explicit initial
reference contract.

## Equivalent names and rejected insertion

Unique insertion compares length-zero empty markers first, otherwise using CRT
`stricmp`, consistent with the already audited find/erase operations. An
equivalent key returns the existing iterator and an inserted byte of zero,
without replacing the mapped resource, allocating a node, changing tree size,
or retaining/releasing either pointer. The result consists of owner at +0,
node at +4, and inserted byte at +8. Different name keys are not rejected merely
because their resource pointer values are equal.

The matched caller window `00b80908-00b80a4a` calls unique insertion at
`00b80975`, then cleans its temporary names without testing that inserted byte
or adopting the returned node. At `00b809e0` it reads manager.currentResource
at +24h; at `00b80a2e` it returns that pointer. Consequently, if the miss path
encounters an equivalent name after creating another resource, the old mapping
remains and the new current resource is still returned.

The prior resource destructor erases by name without mapped-pointer identity
validation. Combining those two contracts implies that a rejected new
resource's eventual destruction can remove the old equivalent-name cache
entry. This is a consequence inferred from the verified bodies; no concurrent
or reentrant duplicate scenario was reproduced.

The insertion helpers can store null raw pointer values without dereferencing
them. Their source name/pair addresses must be valid, and nonempty names need
valid string data. The prior cache-hit caller dereferences resource+4, so a
null mapped value is outside the complete load/hit contract.

## Manager destruction and full-cache clear

Vtable `00d63128` slot +0 points to scalar deleting wrapper `00b81150`. It calls
field destructor `00b80f10`, conditionally frees the manager when deleting flag
bit 0 is set, and returns the original manager pointer with RET4.

The field destructor first invokes virtual +0 with flag 1 on nonnull manager+4.
It then clears the resource cache at manager+14h through `00b805d0`, passing
the complete begin/end range. That helper recognizes the whole-tree case and
calls `00b7ff20` on the root. It resets the head's parent/left/right links to the
sentinel and size to zero. The manager then frees the sentinel itself and
zeros its head/count fields.

`00b7ff20` recursively destroys each right subtree, saves the left link, tears
down the current node's key through `00419cc0`/`00bd1510`, frees that node, and
iterates into the saved left subtree. Sentinel flag +19h terminates traversal.
The complete routine never reads mapped slot +14h, changes a resource count,
or calls a mapped-resource method. The saved left link precedes node free;
the initially truncated pseudocode omitted the subsequent left traversal.

The manager next clears a separate tree at +8 through `00b80500`, frees that
sentinel, zeros its head/count, resets singleton pointer `010901c4`, and installs
base vtable `00ce3818`. The manager+4 object's destruction and secondary-tree
cleanup remain external. There is no direct release of current factory +20h
or current resource +24h in the complete manager body.

Resource references held elsewhere can survive cache destruction. Singleton
shutdown order and the behavior of those resources' later name-removal
callbacks remain outside this audit; cache nonownership is not a claim that
all other manager components have the same lifetime policy.

## Analysis state and verification boundary

Initially, `_free` calls at `00b7ff5c`, `00b80f71`, and `00b81160` had stale
`CALL_RETURN` overrides. Tail call `00b80f9f` existed in the matched PE bytes
but had no Ghidra instruction until the earlier truncation was repaired.
During this packet the primary repaired analysis; final worker read-only
checks saw instructions with `NONE` overrides at all four sites. Initial and
final observations are recorded separately in the report. This worker made
no Ghidra changes.

This packet writes only its new document and report. Native name allocation,
allocator failure/exception cleanup, general tree helper internals, secondary
parser cleanup, arbitrary factories, and shutdown ordering remain external.
No C++ build, fixture, native ABI compatibility, or game validation is claimed.
