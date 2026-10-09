# World current-storage tick readiness

The complete native schedules for `00904BF0` and `00904600` are now evidenced
against the installed PE. The smallest complete raw tick scope is their pair:
the 69-byte entity walk unconditionally calls the 1,507-byte matrix-list pass.
The current projected tail is not a usable actual-storage provider. A concrete
pair still needs raw `0042ED50` subtree invalidation and real clock, entity-table
and nonempty-list ownership contracts. This packet writes evidence only;
Source, Original credit, build, runtime and gameplay claims remain zero.

## Verified bodies and ABI

Each live query used `bsp.py ghidra`, whose client verifies project `bsp`,
`/battlestationspacific.exe`, `x86:LE:32:default` and image base `00400000`.
The configured project file is `C:/Users/sqz269/bsp.gpr`. Both complete bodies
were read as pseudocode and assembly. Target-verified byte reads then matched
every byte of the installed executable; independent x86 decoding covered each
span continuously and every encoded non-call branch stayed inside its body.

| Entry | Inclusive span | Bytes / instructions | Native contract |
| --- | --- | --- | --- |
| `00904BF0` | `00904BF0..00904C34` | 69 / 28 | ECX=actual World, one float stack argument, RET 4; ESI/EDI saved. |
| `00904600` | `00904600..00904BE2` | 1507 / 323 | ECX=actual World, one unused float stack argument, RET 4; EBX/EBP/ESI/EDI saved, `2E8h` local frame. |

The live prototype metadata is insufficient: it reports no parameters and
the matrix decompiler chooses fastcall. The assembly proves the stack cleanup
and receiver contracts above. Its signature metric reports 320 instructions,
while the complete live listing and byte decode contain 323. No listing gap
remains, including the three instructions at `00904BCA..00904BD5` after free.
The projected header's claim that those bytes are undisassembled is stale.

Exact Ghidra flow-override/no-return properties remain unverified. The supported
read-only `flow-properties` query returned `Script execution disabled`; its
before/after target checks succeeded. No setting, listing, function, flow
override or project was changed. Complete encoded native control flow is a
separate result from unavailable Ghidra override metadata; this packet neither
requests nor admits an analysis repair. Full byte/control records and that
read-only failure are retained in the paired report.

## Entity walk: exact actual receivers

`00904BF4` reads World+4 once, then `00904BF7` reads that header's first DWORD.
This is the head of an intrusive entity chain, not an inline World field,
index, token or separate payload node. The loop never consults header count,
World+8, pause/dead/cull flags or the active byte at World+4A4.

For each nonnull actual entity, it reads byte +5C. When nonzero, it loads that
entity's current primary table, loads the incoming float through x87, loads
slot +DC from that table, pushes/stores the float through FSTP, sets ECX to the
same entity and invokes the selected method. The exact order at
`00904C06..00904C18` includes a binary32 FLD/FSTP round trip; a plain integer
argument copy does not establish equivalent NaN/FPU behavior.

After the callback, or immediately for an inactive entity, `00904C1A` reloads
next from that same entity+38. It does not save next before the call or restart
at a replaced World header. A callback clearing +38 therefore stops this walk;
a vector snapshot would behave differently. The current entity must remain
valid through that post-call read. Finally, even for an empty chain, the code
does another FLD/FSTP argument copy and calls `00904600` with the original
captured World receiver. The tail ignores its float argument, but the caller's
x87 conversion is still part of the native schedule.

## Matrix pass: actual list and numeric schedule

World+4B0 is the address of a checked-list header; +4B4 is its current sentinel
pointer and +4B8 its DWORD count. A node is `6Ch` bytes: next+0, previous+4,
and a `64h`-byte record at +8. The record holds entity+0, translation+4/8/C,
angles+10/14/18, duration+1C, a 64-byte base matrix+20 and start time+60.
The initial node is `[[World+4B4]]`. EDI retains World+4B0, while comparisons
reload its current sentinel at every validation and loop site.

The pass has 29 CALL instructions: 18 validator calls (one structurally
unreachable after `CMP EDI,EDI`), three rotation builders, four matrix
multiplies, two entity virtuals, subtree invalidation and free. Returning
validator effects must be retained; they are not a new exception-only policy.

For every reached record, including an inactive one, `0090462F` loads the
current mission clock `00F876A4` once. After the first validator site, x87
subtracts the current node+68 start value and stores elapsed to binary32 at
`0090464E`. That captured elapsed survives all later callbacks. Phase uses
that same captured elapsed divided by current node+24 duration, with a second
binary32 spill at `00904677`; it must not recompute clock minus start later.

The lower clamp uses x87 FCOMIP against zero followed by JBE; the upper clamp
uses COMISS against the current `00D7A24C` operand followed by JBE. Unordered
comparisons take both keep paths, so a NaN phase survives. Translation products
are spilled individually to binary32. Each angle product is spilled, reloaded,
negated by FCHS and spilled again before its helper call. Substituting generic
clamp/trigonometry or unspecified compiler arithmetic would not establish the
observed x87/SSE stages, signed-zero or exception behavior.

The three rotation calls use ECX=64-byte destination, EDX=angle pointer and
return that destination in EAX. Their results are copied as sixteen raw DWORDs.
The four `00413920` calls use ECX=left, stack destination/right, RET 8 and
EAX=destination. Their order is `((Rz * Ry) * Rx) * translation * record.base`.
The last right operand remains the actual record's base matrix address, not a
snapshot copied before the preceding operations.

## Receiver reloads and expiry/erase selector

| Native sites | Required capture or reload |
| --- | --- |
| `00904659..60` | Load current record entity and read its +5C byte for the apply gate. |
| `009046A1` | Capture the entity in EBX before the following returning-validator sites. |
| `00904A1E`, `00904ADD..E3` | Capture that entity's table, retain its slot-88 address across the four multiplies, then load the slot value and call with ECX=captured entity and one matrix-pointer argument. |
| `00904AE5..B13` | Validate against current sentinel; reload record entity; capture child head at +48; clear that entity's +C8/+10C bytes; call `0042ED50` on each child and reload that child's +44 after the call. |
| `00904B15..2A` | Validate again; reload record entity, its current table and slot+D8; invoke with ECX=that current entity and no argument. |
| `00904B2C..54` | Validate again; reload record entity and active byte. Inactive expires immediately; otherwise validate, reload current duration and compare captured elapsed against it with FCOMIP/JA. Only ordered strict greater expires. |
| `00904B56..62` | Retained record: validate, then reload node.next and loop. |
| `00904B89..B95` | Removal: capture the node in EBP before a returning validator; subsequently capture its current next in ESI. |
| `00904B95..BAD` | Check the retained header address; compare captured node with current sentinel, validate on equality, then recheck. If it is still the sentinel, continue without unlink/free/decrement. |
| `00904BB3..BD1` | Write previous.next, reload both node links, write next.previous, free the captured node, decrement the current header count, and continue with the previously captured successor. |

The header-address test occurs before the successor load, but its validator
call occurs after that load. The slot-88 call consumes its one pointer argument;
slot-D8 consumes none. Entity pointers are deliberately reloaded after slot-88
and before slot-D8/expiry; caching one receiver for the entire record is wrong.
Similarly, a callback can change duration before the expiry test, while elapsed
remains captured. Equality retains an active record; unordered comparison also
retains it. A clock change during a callback is visible to the next record.

Removal frees the matrix-list node, not its entity. It does not call entity
slot zero or the separate entity-chain drain. Link reads after the first unlink
store and the count read after free are observable native ordering. There is
no count clamp, one-expiry limit, early return after free or vector compaction.

## Current Source interfaces and genuine preconditions

The existing `update_world_entities_00904bf0` consumes `vector<bool>` and index
callbacks. `run_matrix_interpolator_pass_00904600` consumes a vector of projected
records and one by-value clock. Its GameWorld binding treats entities as unit
indices, returns true for `entity_active`, and records rather than performs
subtree invalidation and refresh. Those interfaces cannot supply raw receiver,
clock, header/node identity, current-table dispatch or allocation semantics.

Useful concrete dependencies already exist:

- `allocate_native_world_chain_headers_009037f0` supplies the two real 0Ch
  first/last/count headers through the singleton allocation domain.
- `create_native_world_matrix_sentinel_004c3080` supplies raw 6Ch sentinel
  storage with self-links. It does not publish a World list, populate records
  or create a complete World owner.
- The current raw X/Y rotation functions and current-global Z overload in
  `native_particle_axial_loading.hpp` retain native arithmetic against actual
  matrix storage. `multiply_native_camera_matrices_00413920` supplies the raw
  multiplication ABI. Their current-global operands must be borrowed from real
  retained storage; the projected `std::sin`/`std::cos` copies are not equivalent
  evidence for this packet.
- The real singleton free boundary and returning invalid-parameter callback
  interface exist. Node release is valid only for nodes genuinely produced in
  their matching allocation domain; an arbitrary vector element or token is
  not such an allocation.

No concrete actual-storage `0042ED50` provider was found in the audited Source
interfaces. Its current uses are abstract callbacks or logging. A canonical
live mission-clock reference is also absent from the audited raw tick surface;
the projected host's float value does not establish that process owner.

Supplied entities must have genuine callable Source tables for slot+DC, and
matrix-record entities additionally need slot+88 and slot+D8. Table pointers
must be loaded at the native capture points. Actual entity fields, child
subtrees, intrusive links, list sentinel and records must remain valid through
every native post-callback read. Qualified entity allocation/deletion and a
stable nonempty-list producer remain separate obligations. Read-only PE table
addresses, empty sentinels and fake callbacks cannot establish these owners.

Root owns `009041A0`; this audit only reads its recorded dependency contract.
It confirms the same World+4 header/entity+34/+38 chain domain, while its
destruction and reload schedule remains distinct from this update loop. No
drain address was claimed or mutated here.

## Smallest justified follow-up

The body/ABI evidence supports a future actual-storage implementation of
`00904BF0` plus `00904600`, preserving the stages above and reusing the existing
raw matrix helpers. First qualify/bind real `0042ED50`, the clock, returning
validator/free boundary and entity-table contracts. A 69-byte loop around the
projected tail would not close the tick. A separated expiry/advance/erase
selector is only a Source fragment of `00904600`, not a newly completed native
function or a substitute for the active-record path.

This recommendation creates no World table, application caller or ownership
model. Complete World construction/destruction, current-table producers,
nonempty matrix-list production, graph lifetimes, partial effects on exception
and application wiring remain open. Native EH/SEH/private-frame and binary-ABI
compatibility are not established by a new explicit C++ interface. Descriptive
names remain hypotheses. No Source/GPR/config/ledger edits, build, tests or
execution probes were performed. JSON parsing and diff checks validate the
evidence files in addition to the byte/decoding checks described above.
