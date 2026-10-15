# Actual input pooled-key subtrees

This packet reconstructs complete `00558310..0055837C` and
`00558E60..00558ECC` in `src/native_input_pooled_key_subtrees.cpp`, declared
in `include/bsp/native_input_pooled_key_subtrees.hpp`. These are whole
subtree entries for actual28h nodes with nil byte25h,8B pooled key at+C,
and mapped raw-vector header at+14. Existing AV populated-tree/full-range
storage contracts, frontend projections and native fixtures remain separate
and unchanged. Names are hypotheses, not recovered symbols; existing names
and correct library identities are retained.

The saved project is `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. Complete pseudocode/listings, live prototypes,
typed body metadata, bytes, xrefs and caller contexts were inspected through
repository tools. Both complete109-byte envelopes decode to46 instruction
starts, all equal between disk and live listings. Each typed body is one
complete range, is not a thunk and has no no-return flag. Complete native
bodies have no local EH frame, state map, unwind action or catch. No new
Ghidra action or caller body is promoted by this packet.

Both native interfaces are ECX tree, EDX unconsumed, stacked node, RET4,
with no semantic result. The tree value is preserved for recursive calls but
never dereferenced. Source retains these physical input words and adds raw
pool context, changing stack cleanup. Incoming native register residue is
not exposed as an extra semantic input or result. Recursive Source EDX is
normalized to zero because the complete callee consumes none of it.

The leaf reads only nil byte+25 on entry. Any nonzero value skips the node;
there is no null-node guard, and the nil sentinel is not freed. For each
nonnil current node it captures right+8 and recursively destroys that entire
subtree. Only after successful right return does it read mapped backing+18
FIRST, test that captured pointer, and capture left+0 SECOND. If mapped
backing is nonzero it frees that captured pointer, without reloading +18.
It always clears +18, +1C and +20 in that order. Mapped opaque+14 is retained.

After these stores, the native body reads CURRENT key data+10 once. Nonzero
data gates the CURRENT length+C read and DWORD length+1, both captured before
the actual getter and pool return. A null key skips length, getter and return.
Key fields are not cleared or reread after getter effects. Accepted raw
`destroy_native_string_header_0041dd20(node+C,context)` has these exact ordered
effects: data once, conditional length+1, real current `00419CC0`,
then `00BD1510` with those captures and the real return gate. It resolves the
pool even for large blocks or disabled small returns. It adds no key write
or noexcept storage bridge, and getter failures propagate.

Only after successful key return does the leaf free the captured CURRENT
node address. It reads the captured left node's nil byte AFTER that free,
then updates its current cursor and repeats. This is right recursion with
iterative left continuation, not generic container traversal. It does not
free the tree head, reset tree root/count/extrema, or inspect mapped elements.
Mapped word versus descriptor element interpretation therefore contributes
no additional destructor behavior in either complete native body.

The complete normalized instruction streams agree. The only differing bytes
are the four external rel32 operands; their byte offsets are41..44,75..78,
82..85 and88..91. All93 other bytes agree, INCLUDING the self-call operand.
The five call roles at offsets25/40/74/81/87 are self, current CRT free,
actual getter, actual return, current CRT free. Internal branch targets and
self-recursive targets normalize to the same body offsets. Source therefore
reuses the complete word-vector implementation for the descriptor entry.
This does not promise native recursive frame/callstack identity.

The older AV generic subtree captures left before entering its payload
helper; its raw-vector helper reloads begin for free. Those accesses differ
from these exact leaf schedules. The new Source body performs its own
ordered field reads and captured-pointer free rather than relabeling that
older actual fixture domain. It does not call the noexcept
`ActualNativeStringPoolStorage` bridge or install a termination adapter.

If right recursion fails, current mapped/key/node work is skipped. If key
getter creation fails, mapped backing has already been freed and cleared,
but current node free and left continuation are skipped. Earlier effects
remain. There is no local catch, retry, rollback or synthetic cleanup. These
are current Source C++ and current pool/CRT effects; original FH3/SEH/fault,
heap/CRT/handler and cleanup-failure personalities are not established.

The exact direct transfers, including incoming evidence only, are:

```text
00558329 CALL 00558310
00558338 CALL 00BF65AC
0055835A CALL 00419CC0
00558361 CALL 00BD1510
00558367 CALL 00BF65AC
00558E79 CALL 00558E60
00558E88 CALL 00BF65AC
00558EAA CALL 00419CC0
00558EB1 CALL 00BD1510
00558EB7 CALL 00BF65AC
0055874C CALL 00558310
0055915A CALL 00558310
005593E9 CALL 00558310
0055940C CALL 00558E60
00559A0A CALL 00558E60
00559C79 CALL 00558E60
```

Clear/full-range callers pass their captured head.parent as node; the catch
sites pass native parent-frame node/tree captures. Their surrounding states,
iterator algorithms, exact frames and completeness remain outside this
packet. In particular whole record `0055B690` remains unclaimed and unready
pending its other child contracts and parent EH recovery.

Valid reached node/nil/payload/key storage and actual ownership/pool/current
CRT contracts remain preconditions. Source adds no callback, provider,
global pool, host container, guard or allocator substitute. Extra context,
by-value inputs, register/private-frame identities and original binary ABI
remain qualified. No new native differential, failure-injection or gameplay
proof is claimed. Existing AV fixtures are not promoted to these interfaces.

MSVC Win32 Release compiled the new Source; all three existing suites passed
in8.01 seconds. The preexisting duplicate `spawn_request_id_matches` LNK4006
warning remains. All16 report rows and16 Markdown claims pass transfer checks
without failures or unchecked claims. Detailed byte/transfer/build evidence is recorded in
`reports/oct15_input_pooled_key_subtrees.json`. No new tests were added.
