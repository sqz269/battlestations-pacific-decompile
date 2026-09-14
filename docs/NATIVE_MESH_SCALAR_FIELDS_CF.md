# Native mesh scalar fields (CF)

The native mesh parser at `B944E0` still needs field readers that populate its
actual BCh object. This batch adds its LOD, weight-name and discarded-bounds
dependencies, reusing the existing raw reader, string pool, native string vector
and mesh layout. The earlier decoded-value interfaces remain separate.

| Entry | Bytes | Native contract |
| --- | ---: | --- |
| `B72710` | 14 | ECX mesh; stacked float bits; store at +0C; RET4 |
| `B73270` | 47 | ECX mesh; stacked 16-byte record pointer; append; RET4 |
| `B73D50` | 11 | ECX mesh; stacked counted-string pointer; tail append; RET4 |
| `B93590` | 45 | Stacked mesh/handle; consume four floats; RET8 |
| `B935C0` | 63 | Stacked mesh/handle; consume six floats; RET8 |
| `B93710` | 130 | Stacked mesh/handle; append counted LOD phases; RET8 |
| `B93F90` | 142 | Stacked mesh/handle; append weight-map names; RET8 |
| `BE99F0` | 15 | ECX handle; read current stream slot38; EAX DWORD; RET |

The four field readers ignore incoming ECX. These eight complete ordinary bodies
total 467 bytes. Source interfaces add explicit services and are not native ABI
replacements. Addresses, bytes, original names/comments and validation receipts
are recorded in `reports/native_mesh_scalar_fields_cf.json`; descriptive names
remain hypotheses.

## Actual storage and read order

`B72710` is a bit-preserving `MOVSS` store, with no arithmetic or node access.
`B73270` captures mesh+50, computes mesh+10+count*16 with DWORD wrapping, and
performs four alternating source loads/destination stores. It then increments
the current mesh+50 value. Source and destination may overlap; replacing this
schedule with a 16-byte snapshot copy changes behavior. The physical four-slot
capacity is not checked by the original or the new source.

`BE99F0` captures the current node, its +20 remaining-budget address and +8 reader
before calling the existing complete `BF02A0`. The current stream slot38 and
wrapping actual-byte debit remain owned by that established dependency.

`B93710` reads an unsigned phase count through `BE99F0`. For each phase it reads
the current constants at `CE4ADC` and `CE4970` into the first two local words,
then replaces them with direct `FSTP32` stores after two `BE99D0` calls. Two
`BE9A00` DWORD reads complete the record before `B73270` appends it. Each scalar
reloads the current node. Earlier appends remain if a later read throws; there
is no count, remaining-byte or physical-capacity guard.

`B93590` and `B935C0` call the actual float reader four and six times respectively,
discarding each result with `FSTP ST0`. They do not round through an extra memory
temporary, write mesh bounds, skip/detach the node or stop early at EOF. Explicit
x87 bridges preserve both these discards and the LOD reader's direct stores.

`B73D50` tail-forwards mesh+B0 to the existing `4CDC20` counted-string deep-copy
append. The new adapter borrows the same actual raw pool publication, shutdown
gate and lifetime as the reader. It does not introduce shared string references.
Its established `NativeStringStorage::release` interface is noexcept; lazy pool
recreation failure during that release remains a termination boundary.

`B93F90` repeatedly checks the current handle's node+20. `BEA010` writes a local
eight-byte name, and its **returned header pointer** is passed to append. The
local becomes EH-owned only after that read succeeds. After append, local data
is captured before cleanup is disarmed; current length+1 and the current raw pool
govern its normal return. The next iteration rereads the handle/node. There is
no rollback of names already appended to the mesh.

## Exception evidence and Ghidra

The map at `DFC620` and FuncInfo at `DFC628` establish one action: state0 to -1
through `CC2E50`, which loads the local header at EBP-14 and tail-jumps to
`41DD20`. A failed read before local completion does not arm that action. A
failure during append cleans the completed local name only; a second cleanup
exception terminates in the source boundary.

Ghidra lacked the function at `CC2E58`, the ten-byte compiler handler loading
`DFC628` before its `BF6B43` jump. This batch created that function under the
write lock without clearing bytes or changing no-return flags. The existing
`CC2E50` compiler action and its comments are preserved.

Saved/disk byte equality covers all eight ordinary and both support bodies,
plus both constants and the 44-byte EH map/FuncInfo region. The final audit
verifies 152 executable instruction owners: 148 ordinary and four support.
One unreachable three-byte alignment instruction at `B9372D`, after the jump
to `B93730`, is excluded. Executed alignment in the weight-name loop remains
included. There are 24 direct transfers and no indirect calls in these spans.

## Validation and remaining work

`scripts/build.ps1` compiled the new source into actual CMake `bsp_core` and
passed both existing CTests. Because `cc7` still leases `cmake/startup.cmake`,
an ignored `CMAKE_PROJECT_INCLUDE_BEFORE` hook registers pending CC, CE and CF
sources in this worktree. Tracked source registration and main integration are
not claimed; remove the cached hook when adding tracked registrations.

One controlled-child fixture copies all eight ordinary bodies, relocates 22
direct operands and guards the original EH entry. Five source/original pairs
(ten executions) compare:

- Bit-preserving LOD setting and an overlapping append that propagates the
  current source word through all four stores.
- Four/six discarded bounds reads, untouched mesh bytes, exact stream events
  and the retained trailing DWORD budget.
- Three LOD phases appended after an existing phase, including signed zero,
  subnormal, quiet NaN and infinities; a separate zero-count path.
- Five counted weight names, including an existing seed, empty name, embedded
  NUL and 180-byte name; exact strings, capacity growth and untouched mesh fields.

A source-only failure during the second weight-name payload confirms the first
name remains stored and the node retains its prior budget. The fixture then
uses existing native root/reader, memory-stream and string-vector owners to drain
its storage. Its initial setup omitted the canonical root-reference binding
and terminated during reader destruction; restoring `NativeResourceRootDispatch`
made the full fixture pass without changing CF source.

The copied bodies share established scalar/string/vector/pool dependencies and
the BCh constructor. Original EH, native ABI/SEH/private stack aliases, hardware
faults, complete FPU exception policy, corrupted-header/count overflow, all
callback mutations, append-allocation failure and lazy recreation are not newly
validated. Full native `B944E0` parsing and registered mesh wrappers still require
the actual vertex-format (`B93800`), indices (`B93AA0`), vertex-stream (`B93E60`)
and subset/material (`B941D0`) paths. Executable admission and gameplay remain
unproven.
