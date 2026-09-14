# Native FileBlock production frontier

Read-only audit of loader preparation `00504790`, FileBlock constructor
`00BE0A30`, identifier `00BDF950`, paired VFS block entry/exit, and deletion
used by loader retirement `00506BF0`. No source, annotations, repairs, shared
metadata, executable invocation, or production binding is added here.

Worker baseline: `c2da6ab4d304fa7c9ee5488239312abb15e9feda`. Primary semantic
audit baseline: `96d910d50a2e98317bb45227d830544ef1727a3d`; integration continued
independently. The [report](../reports/native_fileblock_frontier_bl.json)
records exact historical source/image hashes, byte spans, call sites, native
ABIs, live membership gaps, and leases. Its hashes are not later build claims.

## Actual ownership path

`504790..50484B` allocates **1Ch = 28 bytes**, not 28h. The `28h` wording in
the older `FILE_BLOCK_SETUP.md` is a units error. The caller first checks
captured job `+20` for an existing block and job name length `+14` for nonzero.
It allocates before taking substring `(start=13,count=storedLength-18)`;
`469840` supplies its native negative/count behavior. There is no package
prefix validation or added minimum-length check. It calls `BE0A30(temp,1)`,
stores returned EAX at job `+20`, and only then releases the temporary.

`BE0A30..BE0ADA` consumes ECX raw owner and two stack arguments (name header,
flag DWORD), returns that owner in EAX, RET8. It writes:

| Offset | Actual storage |
|---|---|
| `+0` | Base `CEB130`, then FileBlock profile `D68494` |
| `+4` | Reference count DWORD = 1 |
| `+8/+0C/+10` | Zero DWORDs; do not invent stream members |
| `+14/+18` | Owning native length/data identifier header |

After name copy it calls identifier preparation with ECX owner, then reads
**current `0109CEEC`** and calls `BE0980(&owner.name,flag)`. It stores no
manager pointer and opens no byte stream itself.

At `506C07`, retirement calls the captured block's **current virtual +4**
with flags1; it clears captured job `+20` only after that call returns. The
live `D68494` profile is slot0 `BD30E0`, slot4 `BDEBE0`.
`BDEBE0..BDEBFD` calls `BDCB30`, frees outer storage only for flags bit0 after
normal destruction, returns the captured owner in EAX, RET4. Neither it nor
`BD30E0` decrements the object's reference count. Existing actual `BD30E0`
dispatches current virtual +4(1); `BD30F0` only stamps `CEB130`.

`BDCB30..BDCBAB` stamps `D68494`, reloads **current `0109CEEC`**, calls
`BDC9B0(&owner.name)`, releases the current name, then calls `BD30F0`.
Capturing the construction manager for later exit would change native behavior.

## Identifier packet

`BDF950..BDFD7F` (1072 bytes) and `BD20A0..BD216C` (205 bytes) have no complete
native source. Their string/vector dependencies already do: actual native
headers, `NativeStringStorage`, substring/copy/concat/byte operations, and
the `0Ch` `NativeStringVectorStorage` with `8h` elements and deep append.

The identifier replaces the exact invalid-byte set at `D68520` with `_`,
preserving its `strcspn` behavior, including embedded NUL before stored length.
Length at most32 then returns. Otherwise the helper splits nonempty underscore
tokens. For token counts3..11, native `atol != 0` keeps a complete token;
zero keeps its first byte. Underscores separate tokens. Numeric tokens can
leave a result longer than32. Other counts compose substring `(-64,64)`
with `(0,64)` in the original operand order; existing `469840` makes the
first slice empty. No replacement hash, suffix selection, or new length cap
is justified. Cleanup and temporary construction order remain part of the
implementation packet, including the aligned native stack/EH evidence.

## Paired VFS state and concrete observer

Entry and exit each retain their captured manager receiver across callbacks.
Actual `native_vfs_manager_lifetime` already constructs and destroys the
manager's `+7Ch` list, initializes gate `+79=1`, and owns its sentinel at
`+80` and count at `+84`. A separate host gate stack would duplicate storage.

| Entry `BE0980..BE0A28` | Exit `BDC9B0..BDCA74` |
|---|---|
| Allocate saved-gate node, then check/increment list count and link node | Notify current observer before changing exit state |
| AND current gate with argument byte | Clear current name and decrement depth |
| If `+78`, invoke captured observer's current virtual +8 with input name | If input name is empty, observer +0C receives manager's current-name header |
| Increment depth `+14`, copy name into `+0C/+10` | Restore last saved gate, fetch current last node again, erase it |

Exit leaves an empty current name; it does not restore the outer name. Empty
list validation occurs after observer/name/depth effects and can return from
`BF6713`; it is not an empty-stack no-op. Both diagnostics are the existing
RET-only `4254B0`, with their original argument reads retained.

The missing gate-list closure is only `7F8390` (51 bytes), `7FA3A0`
(145 bytes), and `BDAF40` (94 bytes). The node is `0Ch`: next, previous,
and gate byte at+8. Its producer reads the gate pointer **after** allocation.
Entry allocates before checked count growth; do not add rollback on overflow.
Growth uses bound `UINT32_MAX` and returns updated count in EAX. Existing
`NativeAliasListLengthError` provides the owning legacy payload transport;
its existing lower-bound growth wrappers cannot replace this function.
Erase frees the captured node before decrementing current list count, then
writes the actual output iterator. Its decoded continuation is outside current
Ghidra membership.

Production `GameNativeVfsRuntime::register_archive_tail` writes the actual
PAK registry to manager `+88`, and sets observation byte `+78` from cached-load
configuration. Registry profile `D64190` slots **+8/+0C are BB5770/BB5910**.
Both source bodies are missing. They do real archive mount/unmount work;
an inert tracing dispatcher would omit required behavior when `+78 != 0`.

Their bounded missing closure is:

| Entry | Contract |
|---|---|
| `BB5670..BB5760` | ECX output header, EDX input, RET/EAX output. Test the **first**, case-sensitive `.mpak` substring position against storedLength-5; append suffix otherwise. This also preserves the length4/no-match edge and native self-alias stores |
| `BB5770..BB590A` | ECX captured actual registry, stack name, RET4. Clear current VFS+20, resolve mutable name; on success select device, increment registry ordinal, mount under `.`, append returned raw provider, increment current VFS+1C/set+20. Finally recompute current+20 from signed+1C |
| `BB5910..BB5AB0` | Same ABI. Ignore resolver Boolean, remove all matching raw vector entries without releasing them, then unmount once. Decrement ordinal/current VFS+1C only if unmount succeeds; recompute current+20 |
| `BD9200/BD9210/BD9220/BD9FA0` | 15/10/11/12-byte source-missing leaves over the actual VFS counter+1C and byte+20; current publication is reloaded separately before calls |

Existing actual registry owner/reserve, `BDF4C0` resolution, `BDD850` device
selection and `BE1890` mount are reusable. The entry append does not reject
a null mount result after a returning failure path. Do not add a private
registry, origin-manager capture, or restored gate on an exceptional return.

`BE0750..BE0915` (454 bytes) is the remaining independent name-unmount body.
ECX is actual manager; stack arguments are `(systemName,mountPrefix)`; AL is
success, RET8. It canonicalizes/trims the prefix, walks the actual `+3Ch`
tree, and removes the first node matching provider name and mount prefix.
It decrements the captured provider's actual refcount+4, calls its **current
virtual0** at zero, then uses existing `BE0080` erase. The tree helper owns
node/key storage and does not release the provider. Existing source supplies
canonicalization, trimming, iteration, erase and finite provider deletion;
the new packet must explicitly dispatch the current zero-reference target.

## Failure storage required for a viable owner

The fresh constructor/destructor FH3 maps contain no try blocks and no
compensating VFS leave:

| Operation | Verified unwind map |
|---|---|
| Constructor `E00DB0/E00DA0` | State1: `CC6748` destroys current name through `41DD20`, then state0 `CC6740` stamps base through `BD30F0` |
| Destructor `E00650/E00640` | State1: `CC6218` destroys current name, then state0 `CC6210` stamps base; no exit retry |
| Preparation `D922D0/D922B8` | State1 cleans the flagged substring through `C68F9B`, then state0 `C68F90` frees the captured allocation. The map also contains state2 -> the same temporary action -> -1 |

Thus an entry observer failure can leave a linked gate node and changed gate
without depth/name advancement. The constructor unwinds its own name/base;
the preparation caller frees the failed raw allocation. If exit throws, the
destructor cleans name/base and propagates; scalar deletion never reaches its
outer free. Do not infer balanced nesting or automatically call exit again.

A viable source implementation owns the real `1Ch` allocation and keeps
preparation/observer mutable headers at stable addresses. It must retain these
frames while a nested `NativeVfsNameResolutionAcquired` borrows them. Unknown
provider failures require retained outer state or a terminal policy before
unwind; only modeled native-EH-complete paths may execute the cleanup above.
Use an explicit phase owner, not a borrowed projection or a new scoped-block
destructor that hides partial state. A successfully published job block stays
alive until the concrete current-profile deletion completes.

Proposed source contract: a `NativeFileBlockContext` borrows the actual
`0109CEEC` publication cell, actual string storage, and the complete source
VFS entry/exit dispatch domain. The constructor itself accepts caller-supplied
raw `1Ch` storage. A loader `NativeFileBlockPreparationAcquired` owns the
captured allocation and substring header until normal publication to job+20;
it records construction/publication/failure phase and does not repeat native
cleanup after an unknown boundary. Each `NativePakRegistryBlockInvocation`
owns its mutable `.mpak`, device-selection and dot headers plus any nested
resolver acquired object. Those invocation objects outlive all borrowed
headers on failure. These are proposed interfaces, not new stubs or a claim
that other stack-allocated FileBlock callers are integrated.

## Disjoint next packets

All addresses/files were unleased at the report's final check; recheck before
dispatch. Exact owned header/source/doc/report paths are in the JSON.

| Packet / new source stem | Addresses | Readiness |
|---|---|---|
| `native_fileblock_identifier` | `BDF950,BD20A0` | Ready independently; use Astra for aligned-stack/EH/listing review |
| `native_fileblock_gate_list` | `7F8390,7FA3A0,BDAF40` | Ready independently; use Astra for post-free body membership |
| `native_vfs_unmount_name` | `BE0750` | Ready with explicit actual-provider zero-ref dispatch; Sol is suitable |
| `native_pak_registry_block_callbacks` | `BB5670,BB5770,BB5910,BD9200,BD9210,BD9220,BD9FA0` | Depends on unmount and actual current VFS source composition; retain nested resolver frames |
| `native_vfs_fileblock_scope` | `BE0980,BDC9B0` | Depends on gate-list and concrete observer dispatch |
| `native_fileblock` | `BE0A30,BDCB30,BDEBE0,504790` | Depends on identifier and complete paired VFS domain; includes real owner/preparation lifetime |

Each worker uses `fork_turns:none`, `reasoning_effort:xhigh`; primary owns
runtime/CMake/shared metadata integration. `BE7130` removal is already complete
in the separate `ee981882` result. Loader owner/work storage and retirement
source remain separate ownership assignments.

Current no-function intervals are `BDFD6A..BDFD6C`, `BDAF84..BDAF8A`,
`BDEBF5..BDEBF7`, `C68F99..C68F9A`, and retirement `506C3E..506C46`.
The constructor/destructor/preparation handler thunks themselves also lack
Ghidra functions. Old claims that `BDAF84` belongs to the erase body are stale.
This audit records decoded installed/live bytes without repairing membership.

Validation is static. The report pins fresh installed/live spans and exact
direct/tail call checks: 29 routine spans and 17 data/EH spans matched;
`verify_report_calls.py` passed all 143 direct/tail rows. It claims no C++ build,
fixture, native ABI replacement, original executable execution, or gameplay
result.
