# Native CEnum table insertion and replacement

Addresses: `008F17E0`; initializer fragment `004D3069` inside `004D3040`.

This binds the complete normal table insertion body to its proved nonnull
producer domain: uniquely owned current-CRT `19Ch` CEnums constructed by the
genuine `008F4DD0` Source provider, with `D16508` and scalar target `008F59C0`.
Null mapped payloads are also admitted. Foreign mapped classes and heaps remain
external. The Source directly calls that complete genuine scalar operation;
it adds no virtual-profile dispatcher, deletion callback, guard or default.
This is an explicit CEnum-only producer/heap/lifetime precondition for both
old and new pointers. Original profile/slot loads and general virtual dispatch
are not replayed by the direct Source binding.

| Native extent | Bytes / decoded instructions | Coverage |
|---|---:|---|
| `[008F17E0,008F188C)` | 172 / 65 | Complete normal insertion/replacement, only the admitted CEnum producer domain |
| `[004D3069,004D308B)` | 34 / 9 | Initializer fragment only; containing `004D3040` startup remains incomplete |

Both saved-program byte spans equal the installed PE. Their SHA-256 values are
`9e83aa51d4fea8ac9c75ecdaaaef8cfdd14b956b65af605b47459983af6a0e5a`
and `609c490761e55a54bb4a29b4af24c128f1bdc5066f76e2607c531812a6f1956b`.
The existing `C:/Users/sqz269/bsp.gpr` and `/battlestationspacific.exe` were
verified by each `bsp.py ghidra` batch; the worker made no analysis mutations.

Native `008F17E0` takes its nested-map receiver in ECX and captures it in EDI;
EBX receives the stack key-header pointer. The other stack argument is the new
mapped pointer. The hit and miss tails load the current count at receiver+4
and return EAX, with `RET8` at `008F181D` and `008F1889`. This is observed
register/stack evidence, not a claim of formal original class/EH ABI parity.
The initializer fragment takes the allocated root in EDX, preserves EDI,
stores outer `CE78BC`, nested `CE7514`, zero count and 64 null heads. It has
no standalone native return; the new Source function returns its borrowed root.

The actual owner is `10Ch`: profiles at 0/4, count at 8, heads `0C..108`.
`004D3056` pushes `10Ch`, `004D305B` allocates, and publication to `E1867C`
occurs later at `004D3094`. Only the 34-byte initializer is implemented here.
No allocation, global startup, namespace, or owner-destruction binding is added.

| Call site in `008F17E0` | Actual target and order |
|---|---|
| `008F17EF` | Complete `0048D4E0` table finder, including captured bucket output |
| `008F1807` | Indirect old-mapped virtual slot0, only when nonnull; `PUSH1` |
| `008F1825` | Miss: actual `E175E8` pool through `004E7C00` |
| `008F184C` | Newnode != query: `0041DD40`, current query length, preserve1 |
| `008F1861` | Nonempty after resize: historical `_memcpy` `00BF7680`; `ADD ESP,0C` |

On a hit, `008F1809` clears node+8 only after deletion returns. `008F1814`
publishes the new pointer, then `008F1817` reloads the count. A null old value
skips deletion and the zero store. Key ownership, node address, chain and count
stay unchanged. There is no same-pointer or shared-ownership check: such inputs,
reentry, and aliases are excluded from this binding rather than assigned policy.

On a miss, genuine `14h` table slots come from a separate initialized pool.
Only the fresh key header at 0/4 is initialized; allocator slot+10 survives.
Owning resize precedes the current-header copy. The Source uses current-CRT
`std::memcpy` in the admitted disjoint owning-copy domain, retaining the native
library name without claiming historical CRT parity. Publication is payload+8,
chain+0C, bucket head, count increment, then a fresh count read. There are no
added failure, null-receiver, rollback, overflow or partial-publication defaults.

The actual wrapper `008F2B90` forms a genuine CString temporary and calls
insertion at `008F2BCE` with outerowner+4. Library loader `008F6A18` and parser
`008F5F8D` reach it with fresh CEnums. Both `19Ch` allocation/`008F4DD0`
lineages and the live/PE `D16508` slots (`008F59C0`, `008F6790`) are pinned.
The wrapper's private EH graph, second virtual method and declaration/type
namespace remain external. Semantic `PropertyLibrary` first-declaration
retention is unchanged: the ordinary loader reopens an existing table before
this insertion. The replacement fixture is not installed duplicate-table proof.

The active unique fixture is `local/cc11_scene_enum_table_insertion/run02`.
Its recipe is `run_probe.py` in that directory's parent. It freshly compiled
14 production TUs plus one fixture with `/W4 /WX /fp:strict`, then linked an
embedded `asInvoker` Win32 executable. All compile, link and execution exits
were 0. It uses actual raw string construction, real OS critical sections,
genuine two-pool/list/page/slot providers, three actual CEnum constructors,
and this actual registry initializer; no fake map/profile/service was supplied.

Installed `global.enums` supplies `LandVehicleClasses` (22 symbols) and
`SoldierTypes` (6). Initial miss publication leads through the table-word
getter to actual symbol word/membership readers. A fresh disjoint 22-symbol
case-variant replacement deletes its old root once; the deleted root is never
accessed or freed again. Found-null and empty-key miss/hit paths preserve count
and key/node/chain identity. Seven table reads, 56 symbol word reads and 56
memberships produce 119 complete query-temporary pool-prefix restoration checks.
All 50 symbol slots and four table slots are returned; real trim, unlink and
string shutdown pass. Cleanup manually invokes the genuine scalar, key-header
and node-return providers; it is explicitly not the native registry destructor.

The manifest records 44 actual project headers plus 14 CPPs (58 unique
production inputs; 60 including fixture/recipe), 202 compiler headers and 16
searched libraries. Original-pre/copy/original-post and pinned pre/post hashes
agree for all three current main libraries; core is
`6e30179692c98be0a495492cf8a9c356c8f7975141742be8d0d2d3dd1642f383`.
All 1,951 files in seven earlier fixture families remain hash-identical; none
was replayed. PE and installed input hashes also agree before and after.
Run01's initial memmove fixture is preserved; run02 freshly reseals the corrected
current memcpy choice. Every existing run01 file is also hashed before/after.
All 411 run01 files stayed unchanged. The final native verifier checks 12 direct
rows with zero failures; the actual virtual call is reported separately. Both
worktree and staged `git diff --check` pass; full main build is pending root.

The whole Source insertion COFF is 389 bytes / 113 instructions. Its actual
relocations and stores demonstrate finder -> conditional genuine scalar flag1
-> post-delete node+8 zero -> new pointer -> fresh count, and the complete
miss publication order. The initializer is 104 bytes / 29 instructions.
Complete scalar/free COFF is included in the supplementary provider receipt.
These are new Source/compiler interfaces, not original binary replacements.

`reports/native_enum_table_insertion_cc11.json` carries exact spans, calls,
profiles, manifests, hashes and qualification. Original 172-byte execution,
native EH/fault/allocator/historical CRT/ABI, global startup, declaration
identity, traffic and game execution are unclaimed. Actual table clear
`004D0760` uses `E175E8`; `008F4C50` uses another pool and is not its substitute.
The registry lifecycle `004D0760` / `008F4F00` / `004D2620` remains open.

## Primary integration

Whole 172-byte insertion/replacement bound to genuine uniquely owned current-CRT CEnum and null mapped payloads. Direct actual scalar flag1, post-delete node8 clear, new-pointer publication, fresh count; separate real E175E8 pool, owning disjoint current memcpy. One installed 28-symbol plus fresh 22-symbol replacement family restores 119 temporary prefixes and returns 50 symbol/four table slots. Original 172-byte execution, general virtual deletion, historical CRT/EH, registry destruction, namespace/startup/class/world/game are external.

Main `5096df208` passed the full Win32 build and all three existing CTests. The independent primary recipe freshly compiled 15 TUs; its sealed receipt is `local/cc11_table_insert_current_primary/inputs_after.json`. Original class/world/game validation remains separate.
