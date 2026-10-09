# CC12 squadron-task scalar deletion readiness

The two constructor-identified slot-zero targets have matching complete live
Ghidra and Original PE bodies: `0071C4D0` and `007F1EE0`, 30 bytes each.
Both return with `RET 4`. Their owned instructions meet the child-list
caller's stack-cleanup shape on ordinary return, conditional on child
preservation and balanced stacks. Cleanup effects, allocation provenance,
Source dispatch and execution remain open.

| Target | Receiver cleanup child | Owned operations | Listing omission |
| --- | --- | --- | --- |
| `0071C4D0` | `00875B30` | 11 physical / 10 saved | `0071C4E5: ADD ESP,4` |
| `007F1EE0` | `007F1E70` | 11 physical / 10 saved | `007F1EF5: ADD ESP,4` |

Each wrapper first pushes ESI and captures entry ECX in ESI. It calls its
cleanup child with ECX unchanged and no pushed arguments. Only after normal
child return does it test bit zero of the current byte at entry stack+4.
Other flag bits are ignored. The argument can be changed by child effects;
the owned code does not capture it before cleanup.

If that bit is set, it pushes current ESI and calls `00BF65AC`, then executes
the physically present `ADD ESP,4`. Both paths move current ESI into EAX,
restore ESI from the current saved stack word and execute `RET 4`. The wrapper
does not restore ECX or EDX. EBX, EDI and EBP have no owned writes. Child
nonvolatile-register preservation and stack-word aliases are explicit
requirements; no blanket preservation guarantee is inferred from labels.
The returned pointer on the free path is an opaque value, not a live object.

On the no-free path the last owned flag writer is `TEST byte,1`; MOV, POP
and RET preserve those flags. On the free path the last writer is
`ADD ESP,4`, after the child returns. Physical stack cleanup requires the
receiver-cleanup child to balance its zero-argument call and the free child
to leave its pushed argument for the wrapper. No owned EH registration,
throw translation or unconditional no-throw policy is present.

Ghidra lists ten starts per wrapper, while both complete 30-byte raw windows
decode to eleven operations. Separate instruction-context queries confirm
that no saved instruction exists at either ADD. The read-only flow-property
query was rejected by the server with `Script execution disabled`; before
and after project/program verification succeeded. The report retains that
actual response. No function or instruction flow flags are asserted, no
script execution was enabled, and no listing or no-return property was
changed. `_free` remains its correct current library label. Its body,
Native CRT implementation and failure policy were not opened.

The accepted profile-word audit identifies `00CFD99C -> 0071C4D0` and
`00D08AE4 -> 007F1EE0`. The accepted `00874F00` caller freshly loads a
current node's profile and slot zero after unlink effects, then calls that
pointer with ECX=node and flag 1. These wrappers resolve the selected
RET4 shape only when that current profile word still selects one of them;
they do not establish all node profiles or complete deletion effects.

Current bounded Source searches find neither wrapper nor cleanup provider.
They find only `NativePilotBotTaskOwnerCalls::base_cleanup_00875b30`, a
required pure virtual method called by its member-destruction facade. That
contract is not a raw receiver cleanup implementation or a task-profile
dispatch binding. No Source wrapper, fallback free, child stub or consumer
is added. The Source singleton dispatcher has a separate finite domain.

This packet owns 60 code bytes, 22 raw operations and eight current input
pins. A separate Root Astra gate of the 92-byte member constructor shares
the ignored diagnostic capture file but is excluded from this report's
owned code. Every live batch verifies project `bsp` and program
`/battlestationspacific.exe`; whole PE hash and live-byte agreement are
retained. There are no Source, Ghidra, build, test, probe or coverage-credit
changes. The actual two cleanup bodies, member/EH contracts, owner lifetime,
allocator provenance and runtime validation require their own evidence.
