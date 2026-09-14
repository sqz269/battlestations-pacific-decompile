# Native FileBlock gate-list helpers

This packet reconstructs three complete logical bodies over the actual Win32
list at VFS manager `+7Ch`: node creation `007F8390`, checked count growth
`007FA3A0`, and iterator erase `00BDAF40`. It adds no FileBlock observer,
entry/exit state, host gate stack, runtime registration or production binding.
The [report](../reports/native_fileblock_gate_list_bm.json) records exact source,
dependency, binary-span and fixture hashes. Worker base is `1ada9f4c`.

## Storage and interfaces

The list's sentinel pointer is at `+4` (manager `+80h`) and its unsigned count
at `+8` (manager `+84h`). A node occupies `0Ch`: next pointer `+0`, previous
pointer `+4`, and gate byte `+8`. Bytes `+9..+0B` remain untouched. Existing
`native_vfs_manager_lifetime` owns this list and its sentinel.

| Native body | Original ABI | Reconstructed interface |
|---|---|---|
| `007F8390..007F83C2`, 51 bytes | Incoming ECX unused; stack `(next,previous,gate pointer)`; EAX node; RET0C | `allocate_native_fileblock_gate_node_007f8390`, stdcall |
| `007FA3A0..007FA430`, 145 bytes | ECX list; stack unsigned increment; EAX new count; RET4 | `grow_native_fileblock_gate_count_007fa3a0` |
| `00BDAF40..00BDAF9D`, 94 bytes | ECX list; stack `(output,iterator owner,node)`; EAX output; RET0C | `erase_native_fileblock_gate_iterator_00bdaf40`, with explicit CRT callbacks |

The source uses actual pointer/DWORD/byte offsets and volatile reads/stores.
It retains captured arguments and rereads memory where the original assembly
does. The growth and erase interfaces use new C++ calling conventions. The
producer retains the native stack shape, but is not certified as a binary hook.
All descriptive names are hypotheses.

## Ordering and error behavior

The producer calls the existing `singleton_lifetime_allocate` for `0Ch` before
reading the pointed-to gate byte. It preserves the three individual destination
address guards at node `+0`, `+4`, and `+8`, rather than making allocation a
nullable success path. Its input next/previous pointers remain captured across
allocation. The established allocator implements the throwing CRT new boundary.

Growth captures count, tests unsigned `UINT32_MAX - count < increment`, and
otherwise stores and returns the sum. This permits a count of `UINT32_MAX` and
zero increment there. The alias/registry growth helpers have smaller bounds,
so their error transport is reused without delegating their count predicate.
The native FileBlock caller allocates before growth and links afterward. These
separate helpers add no rollback ownership for a previously allocated node.

Overflow assigns exactly 16 bytes from `00CE38F8` to an actual legacy SBO
temporary. After assignment completes it arms temporary cleanup, constructs
the existing owning `NativeAliasListLengthError`, and throws that prvalue.
No count store occurs on the error path, including after allocations that
change the current list count. The payload's native vtable DWORD is `00D69260`;
the host catch type and generated RTTI/EH remain distinct from native
`std::length_error`. No modern `std::length_error` is introduced here.

Fresh FH3 evidence: handler `00C8F9A8` loads FuncInfo `00DC05AC` and jumps to
`00BF6B43`. The single state0 unwind entry at `00DC05A4` transitions to `-1`
through `00C8F9A0`, which takes the temporary at EBP-50h and jumps to `004072D0`.
There are zero try blocks. Assignment failure has no completed temporary in
this caller's state; exception-owner construction failure does. Native ThrowInfo
`00D83F98` names `00411780` as destructor and `00D83FD4` as catchable array.
This packet reuses the established owner/copy/destructor implementation.

Erase validates a null iterator owner and an iterator equal to that owner's
current sentinel through the supplied returning `00BF6713` callback. It does
not compare iterator owner with the receiving list. Captured owner/node remain
captured after callbacks; a returning callback on a null owner still reaches
the native null-owner dereference. No recovery or empty-list no-op is added.

After validation, erase compares the node with the receiving list's current
sentinel, then captures node.next. A non-sentinel node is unlinked: previous.next
is written first, and node.next/node.previous are reread for next.previous.
It frees the node, decrements the receiving list's **current** count, then writes
output.next at `+4` and output.owner at `+0`. A sentinel skips free/count change
and still produces output. The returned pointer is the supplied output.

## Evidence and verification

Every live query/export used `bsp.py ghidra` wrappers verifying configured
project `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, x86
language and image base `00400000`. Autostart was disabled for the final evidence
batch. Nine fresh live-byte spans match the installed PE, including all 290
routine bytes, the growth handler/funclet/map, error message and ThrowInfo.
The report records all seven direct CALL sites. No Ghidra mutation was made.

`00BDAF84..00BDAF8A` remains decoded but outside Ghidra function membership.
Its `ADD ESP,4; ADD [EDI+8],-1` continuation is included in the reconstruction.
The report carries a future membership-repair proposal; no no-return flags,
listing bytes or function membership were changed.

The new translation unit passed MSVC Win32 `/std:c++17 /EHsc /O2 /MD /W4 /WX
/fp:strict`. One ignored actual-storage fixture linked that object against a
frozen existing library and fixture allocation/free hooks; its legacy string
and exception code came from the library. Source/copy/source library SHA256
matched at capture: `134578367593f74415bb6fd12b51d103dfc6575dcb94d2f692640dde69c78251`.
The probe linked with `/MANIFEST:EMBED` and passed:

- Gate-byte mutation during allocation and preservation of node padding.
- Maximum-count return, zero increment, overflow owning payload, temporary
  cleanup, and owner-allocation failure including the SBO allocator's retry.
- Preservation of the preallocated node and callback-written count on overflow.
- Foreign iterator owner, returning end validation, callback-visible link/output
  ordering, and current count reload after free.

The fixture's counters are volatile observations across exception destruction;
fault injection rejects both primary and fallback allocations. Initial fixture
attempts were corrected for these harness requirements; no source change was
needed. Its hooks are inspection instrumentation, not production dependencies.
No permanent tests or CMake entries were added. Full registered build/CTest,
production composition, native exception ABI, and game validation remain with
the integrator. This is source-, TU-build-, and fixture-verified helper coverage.
