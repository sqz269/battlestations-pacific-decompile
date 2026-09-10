# Input-settings segment red-black tree helpers (smoke packet)

Addresses: 006a01f0, 006a0770

Two `FUN_` routines in segment 31 (`00696470-006be980`, keywords `inputsettings`,
`sensitivitysettings`, `savedata`). Both are MSVC `std::_Tree` template members, each from a
*different* instantiation (their node layouts differ). Ghidra holds no committed prototype for
either (`Signature: undefined FUN_...(void)`); every convention below is read off the assembly.

Shared container shape, recovered from the callers at 006a1230 and 006a1aa0:

| Offset | Tree object          | Offset | Node                       |
| ------ | -------------------- | ------ | -------------------------- |
| +0x00  | allocator/comparator | +0x00  | `_Left`                    |
| +0x04  | `_Myhead`            | +0x04  | `_Parent`                  |
| +0x08  | `_Mysize`            | +0x08  | `_Right`                   |
|        |                      | +0x0c  | `_Myval`                   |

Iterators are the checked two-word form `{_Mycont, _Ptr}`; 006a1aa0 compares an iterator's
first word against the tree pointer and calls `LIBCRT_unmatched_00bf6713` on mismatch.

## 006a01f0 - provisional `STL_Tree_InsertUnique_IntKey_006a01f0`

Evidence for the name: it descends the tree comparing `*param_3` against `_Myval` at node+0x0c
as a signed int, returns the existing node with a `false` flag on an equal key, and otherwise
delegates to the node-insert helper and returns `true` - the exact unique-insert contract.

- **Convention.** `__thiscall`, `RET 0x8` at 006a026d. `ECX` is the tree. Two callee-cleaned
  stack arguments: a hidden 12-byte return slot for `pair<iterator,bool>`, then a pointer to
  the value. The bool is written as `MOV byte ptr [EAX+0x8],0x1`, so the pair is
  8-byte iterator plus a byte flag.
- **Arguments.** `param_1` = tree (`this`, ECX); `param_2` = return slot; `param_3` = pointer
  to the value whose first dword is the int key. Only the key is read here; the whole value
  pointer is forwarded to the insert helper.
- **Reads.** `_Myhead` at tree+0x04, then per node `_Left` (+0), `_Parent` (+4), `_Right` (+8),
  the int key (+0x0c) and `_Isnil` (+0x15). Node payload is 8 bytes (0x0c..0x14), so `_Color`
  sits at +0x14 and `_Isnil` at +0x15.
- **Writes.** Nothing in the tree on the duplicate path; it only fills the caller's return slot
  with `{tree, node}` and a zero flag. On the insert path all mutation happens inside
  0069f400, which is called `__thiscall` (ECX = tree) with a scratch iterator slot, an
  add-left flag, the parent node and the value pointer, and returns a pointer to the new
  iterator in EAX.
- **Callers.** One: 006a0e90 at 006a102d. That caller is the hint-taking overload; it validates
  a hint iterator and falls back here, copying only the first 8 bytes of the returned pair and
  discarding the flag. This is the strongest single piece of evidence for the pair return.
- **Callees.** 0069b190 (iterator decrement, tagged `stl_probable`) and 0069f400.

### Finding outside this packet

0069f400 carries the ledger name `STL_xlen_throw_0069f400` and the tag `stl_throw_site`. Both
call sites here and in 006a0e90 use it as the tree's node-insert helper: it takes a return slot,
an add-left flag, a parent node and a value, and returns an iterator. A length throw is at most
an internal branch of it. The address is not in this lease, so nothing was renamed; flagging it
for whoever owns that band.

## 006a0770 - provisional `STL_Tree_EraseSubtree_PooledPayload_006a0770`

Evidence for the name: it recurses on `_Right`, walks left, and frees each node plus its
pooled buffer without ever rebalancing - the destroy-whole-subtree helper, not a single erase.
Its caller at 006a1230 follows the call by resetting head, size and both extremes, which is
`clear()`.

- **Convention.** `__thiscall`, `RET 0x4`. `ECX` is the tree, kept in EBX only to pass to the
  recursive call (`MOV EBX,ECX` at 006a077b, `MOV ECX,EBX` at 006a0785); the allocator is
  otherwise unused. One callee-cleaned stack argument: the subtree root node.
- **Arguments.** `param_1` = node pointer. The pseudocode drops the `this` register entirely
  and renders the function as plain `void FUN_006a0770(void *)`.
- **Reads.** `_Isnil` at node+0x29, `_Right` at +8, `_Left` at +0, and a payload pair at +0x0c
  (element count) and +0x10 (buffer pointer). `_Isnil` at +0x29 makes `_Myval` 28 bytes, a
  different instantiation from 006a01f0.
- **Writes.** No tree links are updated. Per node it releases the payload buffer, then
  `free`s the node itself through the thunk at 00bf65ac.
- **Callers.** 006a1230 at 006a123c (`clear()`), 006a1aa0 at 006a1aea (range erase, which
  inlines the same clear sequence when the range is the whole tree), and itself at 006a0787.

### The pseudocode for this one is wrong; use the assembly

Ghidra stops disassembling after the `free` call at 006a07ac, so 11 bytes inside the function
body (006a07b1..006a07bb) are undefined data and never reach the decompiler. The exported C
therefore frees a node and returns, leaking every left subtree. The raw bytes decode to the
loop back-edge:

```
006a07b1  83 c4 04        ADD ESP,0x4
006a07b4  80 7e 29 00     CMP byte ptr [ESI+0x29],0x0
006a07b8  8b fe           MOV EDI,ESI
006a07ba  74 c5           JZ 0x006a0781
```

With `ESI = _Left` (loaded at 006a0791) and `EDI` the node being freed, the real shape is the
standard loop: recurse right, step left, free the old node, repeat while not nil. The gap is a
no-return annotation artifact on the `free` thunk, exactly the case AGENTS.md says to check the
assembly for.

### Payload release is a two-call idiom, not two calls with the same arguments

```
PUSH 0x1 / PUSH count+1 / PUSH buffer / CALL 00419cc0 / MOV ECX,EAX / CALL 00bd1510
```

`BSP_SizedStoragePool_GetSingleton_Provisional` (00419cc0) takes no arguments and returns the
pool in EAX; its ledger note already records that caller-pushed release arguments stay on the
stack. They belong to `BSP_SizedStoragePool_ReturnBlock_Provisional` (00bd1510), which is
`__thiscall` on that pool with `RET 0x0C`. The decompiler's rendering of both calls taking
`(buffer, count+1, 1)` is an artifact. The released size is `count + 1`, so the payload is a
NUL-terminated string of `count` characters held in pool storage.

## Uncertainty

Both names are hypotheses. The element types are not recovered: 006a01f0's key is a signed int
at node+0x0c with a 4-byte mapped value, and 006a0770's payload is a 28-byte value containing a
pooled string at +0x0c/+0x10. Neither function was renamed, commented or otherwise mutated in
Ghidra; this packet was read-only.
