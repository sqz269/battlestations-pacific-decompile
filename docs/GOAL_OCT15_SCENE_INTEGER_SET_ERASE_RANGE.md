# Actual scene integer-set range erasure

This packet drafts the whole `0046B170` raw range-erasure body. The full native
listing was read independently of the similar record-map routine. Its envelope
is 201 bytes `[0046B170,0046B239)`: 200 instruction bytes plus the unreachable
NOP at `0046B1EF`. Original ABI is ECX tree; five stacked words output,
first-owner,first-node,last-owner,last-node; RET14; EAX current output.
Coverage is complete. Library-role names are retained as descriptive hypotheses.

| Routine | Source API | Coverage |
| --- | --- | --- |
| `0046B170` | `erase_native_scene_integer_set_range_0046b170` | complete |

The incoming first-owner is captured before current head/minimum reads and the
first returnable diagnostic; first-node is read afterward. For a possible full
range, last-owner is captured, current head is captured before its diagnostic,
then current last-node is compared with that captured head. Full match invokes
the exact accepted actual 14h-node clear body: real `00468300` subtree free and
the original current head/reset sequence. Source reuses `00468980` for that
complete inline schedule. It then captures current minimum before current output
and writes tree then minimum. Tree+0 and the sentinel survive this routine.

The partial loop compares captured owner with current last-owner; zero owner
short-circuits the last-owner read before reporting. It compares captured node
against current last-node after the returning diagnostic. A reached iteration
increments the actual outer iterator at argument+4, then prepares the child
arguments in native push-store order: OLD node, OLD owner, private output pointer.
Only after increment returns is child metadata accessed. After real single erase
returns, outer first-node is read before first-owner for the next iteration.
Empty exit publishes the captured owner/node, using current output pointer.

`NativeSceneIntegerSetEraseRangeFrame` binds actual 20B outer cells, separate
private output 8B and a lazy child-frame pointer. Child actual 12B arguments are
needed only on a reached partial iteration. The child alone imposes its separate
savedNODE4 requirement after its initial nonnil-node check; this caller never
dereferences that backing itself. Full/empty paths allow null child metadata and
unbacked private output. Metadata is stable/disjoint from the described cells;
explicit valid payload aliases are admitted, with no implicit physical private
stack coincidence, added validation, snapshot or rollback.

| Containing routine | Site | Native callee | Contract |
| --- | --- | --- | --- |
| `0046B170` | `0046B18A` | `00BF6713` | Initial owner report after head/min capture; can return. |
| `0046B170` | `0046B1A6` | `00BF6713` | Full-range last-owner report after current head capture; can return. |
| `0046B170` | `0046B1BA` | `00468300` | Current root, callee RET4; followed by exact inline head reset, shared in Source through real `00468980`. |
| `0046B170` | `0046B1FA` | `00BF6713` | Partial owner report with current last-owner read only when owner nonzero. |
| `0046B170` | `0046B209` | `00466AE0` | Actual outer first iterator; no decrement-style validation added. |
| `0046B170` | `0046B217` | `0046A740` | Actual child output/OLD owner/OLD node cells; callee RET0C; real child savedNODE4 frame. |

All five live caller setups were inspected. The three tiny destructors at
`0046B530`, `0046C4C0` and `0046CE90` pass captured begin/end and private output.
The scene-database destructor at `0046EFB0` supplies the embedded sets at +164h
then +158h at `0046F002` and `0046F039`. Their caller/private cleanup bodies are
evidence, not newly owned or reconstructed by this packet.

The full clear provider is real accepted Source in the primary staging branch.
The coordinated increment and single-erase dependencies are named, concrete
drafts whose APIs/frames were read; their files must be merged before this
packet is Source-ready or build-final. No stub, callback, nil19h/string body or
success-only narrowing substitutes for those dependencies. Initial status is
therefore complete algorithm draft with dependency merge/build pending.

Live query batches verify configured `C:/Users/sqz269/bsp.gpr` and
`/battlestationspacific.exe`. Complete original bytes, typed transfers and prior
annotations are in the report. Ghidra changes preserve names/comments under
the write lock before project save/export. No new routine tests or candidate
native differential, private CRT/FH3/SEH/binary ABI or gameplay proof is claimed.
