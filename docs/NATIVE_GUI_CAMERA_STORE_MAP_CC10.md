# Raw GUI camera-store lookup and insertion

Addresses: `00AA3280`, `00AA5070`, `00AA4960`, `004BDA70`, `00AA1080`,
`00AA2180`, `00AA2960`, `00AA41B0`; EH boundaries `00CB7080`, `00CB7088`.

These source bodies operate on the actual `88h` manager and its tree at `+8`.
The existing raw `4C12B0` getter and `AA5D70` constructor already acquire that
manager through publication `F8BC5C` and the real singleton-lifetime domain.
Raw `AA52A0` page registration operates on the separate vector at manager
`+14`. None of those operations is replaced with `GuiCameraStoreMap`.
Names below are descriptive hypotheses, not recovered symbols.

| Routine | Bytes / final instruction | Coverage |
| --- | --- | --- |
| `AA3280` lookup | 274; `AA338F RET4`, length3 | Complete raw normal body |
| `AA5070` create/insert | 224; `AA514D RET0C`, length3 | Complete raw normal body; no native EH |
| `AA4960` multimap insertion | 122; `AA49D7 RET8`, length3 | Complete raw normal driver |
| `4BDA70` iterator increment | 99; `4BDAD2 RET`, length1 | Complete alias of raw `869A20` |
| `AA1080` right rotation | 82; `AA10CF RET4`, length3 | Byte-identical to raw `869810` |
| `AA2180` left rotation | 78; `AA21CB RET4`, length3 | Byte-identical to raw `86A2F0` |
| `AA2960` allocate/copy node | 63; `AA299C RET14`, length3 | Complete alias of raw `86AB70` |
| `AA41B0` link/rebalance | 492; `AA4399 RET10`, length3 | Complete alias of raw `86EBE0`; existing host EH transport |
| `CB7080` unwind funclet | 8; `CB7083 JMP4072D0`, length5 | Compiler-frame boundary only |
| `CB7088` handler | 10; `CB708D JMPBF6B43`, length5 | Unimplemented native FH3 boundary |

The five aliases are equal across their complete bodies after only the listed
CALL/JMP displacements and `AA41B0`'s handler-address immediate are normalized.
The report retains every relocation pair. They use the existing physical
tree leaves and `link_tree_node<TreeInsertAccess<14h,15h>>` machinery; no
engine, allocator, container or reference-count implementation was duplicated.
The old `STL_xlen_throw_00aa41b0` label understates that routine: the length
error is a guard before its full allocation/link/rebalance body.

The raw tree is `{opaque allocator, head, unsigned count}`, `0Ch` bytes. Its
nodes are `18h`: left/parent/right at `0/4/8`, raw float-key bits at `0C`, an
unowned store pointer at `10`, color at `14`, nil byte at `15`, and untouched
padding at `16/17`. The existing integer-pointer pair type is reused only as
the identical DWORD/pointer storage; this family never applies its integer
comparison. An insertion result contains owner/node at `0/4`, inserted byte
at `8`, and three untouched padding bytes. `AA4960` publishes byte8, owner0,
then node4. The shared linker publishes node4 before owner0 in its own output.

A store is `24h`: descriptor flags0, near4, far8, scaleC, priority10,
render-order14, actual camera pointer18, actual scene pointer1C and visible
count20. `AA5070` allocates through the same real source CRT provider as the
existing raw objects. It stages defaults using current `D7A2F0`, `CE3804`
and `D7A24C` words in original read/store order. A null allocation is followed
by the original null dereference schedule, not a successful null result.

The original descriptor argument is read at `AA50C7`, after allocation and
defaults. Flags are copied as a DWORD, followed by four separate x87
`FLD/FSTP` copies at descriptor offsets `4/8/C/14`; priority10 is a DWORD
copy. Scene is read from its current caller argument cell between the first
FLD/FSTP (`AA50D2`); camera is read between the final FLD/FSTP (`AA50F2`).
The actual pair's store pointer is written at `AA50DC`, between the second
FLD/FSTP. Scene1C is then stored before camera18 and count20=0. Finally
`AA5106 MOVSS` rereads the original descriptor14 as raw tree-key bits. An
sNaN can therefore be quiet in store14 but remain signalling in the tree key.
Whole-descriptor assignment, float normalization and pointer snapshots would
lose these distinctions.

`AA4960` captures the pair address. For a nonempty tree it reads the incoming
float once, overwrites the original pair-argument DWORD with its raw bits,
and keeps the converted value in x87 through descent. Each node key uses
`FLD/FCOMIP`; `JBE` chooses right for less/equal **or unordered**. Empty trees
skip that early key read. The allocation leaf later rereads the original
pair's raw key and value after allocation, so a callback can change the node
data independently of the key that selected its position. The output pointer
is also reread from its actual caller argument cell after linking.

`AA3280` scans from head.left using the actual checked iterator. It compares
flags as an integer, then offsets4/8/C/14 with ordered x87 equality; priority10
is ignored. Signed zeros compare equal, NaNs fail, and original x87 status
effects remain. Current sentinel checks and store-pointer rereads occur at
the native sites, including before and after the diagnostic. `B6D800` is the
existing actual camera+54 name-header getter. `4254B0` is a verified one-byte
RET, but its caller's current camera/name-data/store reads are retained.
Invalid-parameter handling reuses the existing source CRT service, which may
return; it is not replaced with an unconditional assertion failure.

The caller supplies stable scratch with initialized preimages. Find scratch is
12 bytes. Insert frame14h contains its eight-byte local iterator at0, an
unclaimed return gap8, current output argumentC and pair argument DWORD10.
Create frame40h embeds that frame at0; has saved-register gap14..1B, actual
pair1C, result24, return gap30, and current camera/scene/descriptor cells at
34/38/3C. Native `AA5070`'s post-save ESP is frame+14. Pair/result identities
and the late argument-word overwrite are preserved during the nested call.

The frames have distinct return frontiers. At `AA4960` return, local0..7,
output argumentC and overwritten pair argument10 are meaningful and checked.
After that return, native `AA5070`'s name/log calls reuse this dead nested
stack area: the last CALL writes a return address at frame4, while format,
store and name argument pushes occupy8/C/10. The source retains the insertion
frontier there. Post-create byte parity covers live pair/result1C..2F and
caller arguments34..3F; saved-register, return-address and later private-call
argument bytes are explicitly excluded. The probe checks the nested driver
separately at its own return rather than mistaking these call-stack bytes for
algorithm output.

`AA5070` has no EH handler, camera/scene retain/release or canonical admission.
Its fresh, disjoint caller-owned acquired record preserves the actual store
allocation and insertion-completion state. An insertion exception leaves the
allocation live; no rollback or guessed release is added. The shared
`AA41B0` length guard uses the existing native-layout string/logic-error host
transport. Its FuncInfo is `DED780`, unwind map `DED778`, state0 -> -1 with
`CB7080` destroying the string at EBP-50. Native FH3/SEH remains a boundary.
Raw tree destruction `AA22C0/AA5260` frees nodes, not store/camera/scene
payloads. Populated-store removal `AA4B30` and orchestration `AC59A0` remain
separate ownership work.

Strict MSVC Win32 build and all three existing CTests passed. One ignored
original/source probe uses real CRT allocations and raw manager construction,
six store creations plus one direct insertion frontier, duplicate/unordered
routing, sNaN store/node bits, allocator-time argument/pair/output mutations,
full tree topology, untouched padding, and three lookup/FPU comparisons.
Its malloc-IAT observation is confined to the probe executable, calls the
real allocator, and seeds fresh allocation preimages; it does not replace the
allocator domain or install anything in the game. Camera-name prefixes and
opaque scene pointer carriers exercise only these helpers' borrowed reads,
not camera/scene lifecycle. No broad tracked tests or Ghidra changes were
made. Original binary ABI, FH3/SEH, corrupted/concurrent trees, application
publication and gameplay are unproved.
