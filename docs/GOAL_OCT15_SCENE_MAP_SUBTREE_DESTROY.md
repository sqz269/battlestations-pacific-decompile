# Actual scene map subtree destruction

Address: `0046AD10`.

The complete ordinary C++ body in
`src/native_scene_record_map_subtree_destroy.cpp` recursively destroys the
current right subtree, releases the current node's pooled key and actual CRT
node allocation, then continues with its captured left subtree. It uses the
existing concrete raw-pool getter/return and current-CRT free service. It adds
no semantic owner, allocator substitute, host callback, copied node or stub.

| Preserved native library name | Body | Coverage | Original ABI |
| --- | --- | --- | --- |
| `STL_Tree_Erase_0046ad10` | `0046AD10..0046AD61`, 82 bytes | complete, qualified actual-storage Source | ECX saved and forwarded only to self; stack root pointer; `RET 4`; no semantic result |

The subtree interpretation is a body/layout/caller-derived hypothesis rather
than a recovered original symbol. There was no current indexed reconstruction
or complete Source under another spelling. Existing `seg6_cc13` code delegates
this leaf through a host contract; it is not this actual raw body.

The public Source API omits unused ECX and receives the existing concrete pool
context:

```cpp
void destroy_native_scene_record_map_subtree_0046ad10(
    void* actual_root, NativeStringRawPoolContext& strings);
```

Native ECX is copied to EBX and forwarded to recursion; neither the receiver
nor its fields is accessed at any recursion depth. Every original caller still
sets it to the tree. Omitting it is an explicit new-interface qualification,
with no original binary ABI or private-frame claim.

The established layout authority is the constructor/insertion evidence already
read in this continuing stream and `docs/SCENE_RECORD_MAP.md`: actual1Ch node
links left+0/parent+4/right+8, pooled native8h key at +0Ch/+10h, mapped word+14h,
color+18h and sentinel byte+19h. The owned body only reads sentinel, right,
left, key data and conditionally key length. It never reads or frees the mapped
word, whose nominal type/ownership is not inferred from this generic leaf.
There is no new structure, conflicting field declaration or pointer owner.

The full native listing establishes this order:

1. Read current node+19h; nonzero returns without recursion, key reads, pool
   lookup or free. No null-root guard is added.
2. Read CURRENT right+8 and recurse. The current node pointer survives the
   child call; no parent link is used to rediscover it.
3. After recursion returns, read CURRENT key data+10h. Then capture CURRENT
   left+0, before any length read, getter, pool return or node free. A changed
   left link from recursive/provider effects is therefore visible here, and
   the captured value is retained across subsequent provider effects.
4. For captured nonnull data only, read CURRENT key length+0Ch, add1 with DWORD
   wrap and resolve the actual pool. Pending native arguments are block,
   size,1; the getter consumes none of them. Return the captured block/size
   through the actual pool, whose `RET 0Ch` discards those words. The getter
   still runs for large blocks and disabled small returns. Null data skips
   both length and pool operations. No positive-size restriction is added;
   FFFFFFFFh length yields returned size0, under the existing provider domain.
5. Free the current node through the actual current-CRT free service; caller
   `ADD ESP,4`. Test captured left+19h and continue there while zero. No freed
   current node, left link or key header is reread after free.

The Source uses volatile raw DWORD/byte reads and retains that full schedule.
It does not call the accepted single-header string destructor: that helper
would put the left capture on the wrong side of the key-data read or reread
the data. Concrete getter/return composition preserves the actual interleave.
No explicit key-header zeroing, map-head or count reset, color write, rollback
or local cleanup is introduced. Getter exceptions escape before current-node
free/left continuation; completed right-subtree changes remain. There is no
new local exception handler, implicit owning destructor or error conversion.

| Native call site | Native target | Concrete Source contract |
| --- | --- | --- |
| `0046AD27` | `0046AD10` | Current right child, same complete Source recursion; original child `RET 4` |
| `0046AD3F` | `00419CC0` | Existing actual publication/raw-manager getter, no explicit native arguments; every nonnull key return resolves it |
| `0046AD46` | `00BD1510` | Existing actual raw-pool return, captured block/wrapped size; ignored third native word1, child `RET 0Ch` |
| `0046AD4C` | `00BF65AC` | Existing `singleton_lifetime_free`, genuine current `std::free`; native thunk JMP `00BF9DC8`, caller `ADD ESP,4` |

The pool bodies and actual current-CRT service were inspected before contract
selection. The complete owned assembly settles the interleaved key/left reads,
captured pointer lifetimes and cleanup. There is no new dependency dispatch.

All live direct call sites were checked: self recursion plus `0046B5BA` in
`0046B570`, `0046BDDA` in `0046BD10`, `0046B709` in `0046B680`, and `0046B46C`
in `0046B460`. Each external site pushes the root from current tree+4 head,
head+4 root and sets ECX to that tree. Each resets its tree header separately
after the helper returns; this helper does not absorb those caller writes.
Containing-function boundaries and argument/result contexts were checked live.

The companion report retains original live bytes/SHA, eight exact call rows,
prior names/comments, annotation and verification states. Every live client
checks project `bsp`, program `/battlestationspacific.exe`, x86 language and
image base against configured `C:/Users/sqz269/bsp.gpr`. Annotation goes through
the write lock, preserves established names/comments and old values, saves the
project and refreshes the owned export.

Compilation and existing regression checks are distinct from original-byte
native execution. No new routine tests are added. Valid taken backing and
compatible allocations/current-CRT/raw-pool state are inherited requirements.
Original binary/private-frame/SEH/fault-address identity, application adoption
and game validation remain unproved; mapped-type uncertainty is preserved.
