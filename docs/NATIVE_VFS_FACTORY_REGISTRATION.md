# Native VFS factory registration

`BE0660` appends an actual0Ch node to the manager's factory list. The new
implementation uses the existing CRT allocation and native legacy exception
transport. It does not change the older vector projection in `app_init_tail.cpp`
or any application/GameHost route. Descriptive names are hypotheses.

| Entry | End inclusive | Coverage | Native ABI |
|---|---|---|---|
| `00BE0660` | `00BE0692` | complete51-byte body | ECX manager, EDX unconsumed, stacked factory pointer; RET4; no semantic result |
| `00BDAB20` | `00BDAB52` | complete51-byte body | ECX/EDX unconsumed; stacked next, previous, factory-slot address; RET0Ch; EAX allocated node |

The stored bodies have no listing gaps. Full live Ghidra bodies and their
following16 bytes match the installed PE. The Ghidra project/program were
verified for each CLI batch; the worker made no Ghidra or game-installation
changes. Library helper `BDECE0` is reviewed as a consumed contract, not counted
as a separately reconstructed library routine or renamed in the ledger.

## Layout, capture and publication

Manager producer `BE1DC0` forms the list owner at+30h, calls the existing
sentinel allocator `BDA960` at `BE1E1F`, stores its result at+34h and zeros+38h.
The raw list owner's first word at+30h remains untouched by registration.
`BDAB20` is the node layout producer: it requests exactly0Ch through `BF681B`,
then writes next at+0, previous at+4, and the current supplied factory-slot
value at+8. Its three stack arguments are proved by RET0Ch, and the allocator's
one cdecl size argument is cleaned by ADD ESP,4 at `BDAB27`.

The helper performs an independent null-address test before each store, using
32-bit address addition for+4/+8. Source retains those tests rather than adding
an early null return. The existing throwing allocation service returns valid
storage or throws; synthetic null/high-wrap allocation addresses and arbitrary
caller-stack aliases are not promised by that service.

`BE0660` captures sentinel `manager+34h` in EDI, captures list owner `manager+30h`
in ESI, and pushes captured `sentinel.previous` and sentinel for `BDAB20`. It
passes the address of its actual factory argument, so the allocator reads that
slot only after allocating and writing both node links. The returned node is
captured in EBX before the count helper is called with increment1.

After the count helper returns, `BE0685` writes captured `sentinel.previous`.
`BE0688` then reloads the **current new-node.previous**, and `BE068D` writes
that node's next pointer. It does not reload the manager's sentinel or reuse
the earlier captured previous value for the second publication store. Source
uses raw volatile field accesses in this order. Null and duplicate factory
values are accepted; no AddRef, deduplication, or factory ownership is added.

All five incoming call sites were inspected: `73D675` (FileStore result),
`73D688` (MPKG result), `73D95D` (MPAK result), `BEDA98` (physical result), and
`73BC73` (result from `736D00`). Each supplies manager ECX and one stacked result
pointer. `BDAB20` and `BDECE0` each have only the one direct caller in `BE0660`.
These observations do not establish that an original-game run reaches them.

## Returning library count helper and failure order

The current `STL_xlen_throw_00bdece0` label is retained. Its full147-byte body
actually captures list+8 count in EAX and stacked increment in EDX, tests
unsigned `(3FFFFFFFh - captured_count) < increment`, and on the admitted path
stores captured count+increment at list+8 before RET4. The subtraction and
addition wrap in32 bits, including behavior for already-corrupt counts.

The rejection path constructs a native SBO temporary with capacity15 and
length0, then assigns the16 message bytes `list<T> too long`; the copied bytes
exclude the terminating NUL, which counted assignment writes separately.
State0 is armed only after assignment returns. `411700` constructs the legacy
logic-error payload; its vtable is changed to length-error token `D69260`, then
`BF6885` throws using metadata `D83F98`. FuncInfo `E00A0C`, map `E00A04`, and
cleanup `CC64E0` destroy the completed temporary through `4072D0`.

Source implements this bounded library contract using the established SBO
assignment/destruction and `NativeAliasListLengthError` owning transport. The
existing `grow_native_alias_list_count_004ce780` cannot be reused directly: its
limit is1FFFFFFFh. No separate CRT/STL internals or new exception type are added.
The reused transport has source C++ RTTI/catch identity; it is not the original
native `std::length_error`/FH3 ABI.

Allocation happens **before** the count check. Neither owned native routine
has an EH cleanup frame or a node rollback. Rejection therefore leaves the
fresh node detached and unreclaimed, while count and published links retain
their values. Source intentionally has no cleanup guard for that allocation.
Allocation failure propagates before count checking or link publication.

## Verification boundary

Six live/installed spans totaling376 bytes cover both owned bodies and tails,
the entire count helper and tail, its17-byte literal, and FH3 metadata/cleanup.
The exact PE SHA256, bytes, saved annotations and call-site rows are in
`reports/native_vfs_factory_registration.json`.

The ignored fixture executes copies of the two original owned bodies and the
original count helper's normal path. Three relative CALL displacements connect
the copies and the existing real CRT allocation service. No data operand,
branch, stack instruction, or original EH handler immediate is changed.
The unexecuted rejection branch's library-call displacements are left intact.
Native rejection/FH3 and allocation failure are excluded from native execution.

Five focused normal native/source comparisons cover helper next/previous/value
stores, null values, duplicate factories, forward/backward links, preserved
manager words/sentinel value, the last admitted count and unsigned wrapping.
A source-only rejection checks the exact existing length-error payload and
unchanged manager/sentinel; its detached12-byte node is deliberately retained
until process exit. No permanent tests are added. Exact results, physical input
pins, build checks, and remaining limitations are recorded in the report.
