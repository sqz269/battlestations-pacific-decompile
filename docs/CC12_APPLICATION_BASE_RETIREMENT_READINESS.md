# Application base retirement prerequisite: 00BEA990

This bounded read-only audit establishes the complete **11-byte, two-instruction**
body at `00BEA990`. It writes native table identity `00D68BC8` to the actual
receiver, then tail-jumps to `00BEA8B0`. Application retirement remains held at
that child. There is no new Source implementation, callable Application table,
owner construction, or reconstruction credit.

The accompanying [report](../reports/cc12_application_base_retirement_readiness.json)
retains the fresh body, the previously captured parent, decoded instructions,
current installed-image comparisons, Source excerpts and hashes, and explicit
boundaries. The starting main was `141f7a163`, preserving the accepted
`d02b25aa298c961b6b20ccb654237ee89e752680` manager-registration audit.

## Scope and target

The only fresh native function queried was `00BEA990`. Its reported body is
`00BEA990..00BEA99A`, below the requested 300-byte gate. The live CLI verified
project `bsp`, program `/battlestationspacific.exe`, language
`x86:LE:32:default`, and image base `00400000`; the configured saved project is
`C:/Users/sqz269/bsp.gpr`.

Only the already captured complete normal `007379A0` destructor shell was reused
from the [Application producer audit](CC12_GAME_APPLICATION_NAME_CELL_PRODUCER_READINESS.md).
Its 93 bytes and 28 instructions were checked against the current installed PE.
There was no fresh parent, child, table, or exception-funclet query. No game or
Source execution, build, test, probe, ledger edit, or Ghidra mutation was performed.

## Physical contract

| Site | Instruction | Established behavior |
| --- | --- | --- |
| `00BEA990` | `MOV dword ptr [ECX],00D68BC8` | Store the native base-table value at actual receiver+0. |
| `00BEA996` | `JMP 00BEA8B0` | Tail-forward the same receiver and caller stack to the child. |

The complete bytes are `C7 01 C8 8B D6 00 E9 15 FF FF FF`. Neither instruction
changes a register, the stack, or arithmetic flags. The jump adds no return
address. The only explicit local memory access is the receiver DWORD store.

The saved prototype `undefined FUN_00bea990(void)` omits the physical `ECX`
input. Pseudocode instead prints a pointer parameter and a child call followed
by return; the actual transfer is a tail jump. This body contains no `RET`, so it
does not independently establish the child's stack cleanup or semantic result.
The captured caller supplies `ECX=ESI`, pushes no additional explicit argument
for this call, and does not consume `EAX` afterward.

`D68BC8` agrees with the base-construction profile already observed at `BEA970`.
That agreement does not supply table contents or a callable Source table. The
write must address the genuine object; a projected host or detached table word
does not establish that identity.

## Captured caller and lifetime ordering

The complete saved `7379A0` normal shell establishes this order:

1. Capture incoming Application `ECX` in `ESI`, then form actual receiver+8.
2. Call `735F30` on that interior vector storage with stack argument zero.
3. Reload current `[receiver+8]` after the helper and pass that data pointer to
   `BF6989`. This free argument is the vector data, not the captured outer `ESI`.
4. Restore `ECX=ESI`, set the caller's cleanup state to `-1`, then call `BEA990`
   at `7379E7`.
5. On normal return, restore the caller SEH frame and saved registers, then
   execute `RET0` at `7379FC`.

Thus vector retirement precedes the base-table reset and child retirement.
There is no intervening publication read or outer-owner free in this captured
shell. Neither that observation nor the local wrapper proves what its children
do, the total outer-storage lifetime, or a successful complete destructor.
`735F30`, the child of `BEA990`, and the parent's FH3 paths remain unexpanded.

## Publication, registration, locking, and failure boundary

`BEA990` does not locally read or clear the Application publication `00E1AE90`,
obtain a manager, remove a registered pointer, acquire or release a lock, change
tracked recursion, or free storage. These are local body statements. The
tail-called child's effects remain part of the complete entry's behavior.

The [prior registration audit](CC12_APPLICATION_MANAGER_REGISTRATION_READINESS.md)
established the constructor-side raw registration contract. It supplies no
converse rule for which pointer retirement removes, whether a publication is
reloaded, which manager or lock is captured, or when publication is cleared.

The older platform-stop document summarizes `BEA8B0` as unregistering `E1AE90`.
That historical summary is recorded as context only; this packet neither reads
nor replays the child's body and does not promote that summary to a complete
current retirement proof.

No local exception frame exists in the two instructions. Faulting on the store,
throwing or failing to return in the child, and the inherited caller cleanup
state cannot be replaced by a successful no-op, rollback, automatic free, or
repeated cleanup. The exact exceptional retirement schedule remains open.

## Current Source readiness

A bounded current search of `src/**/*.cpp` and `include/bsp/**/*.hpp` finds no
address-labelled `BEA990`, `BEA8B0`, `D68BC8`, `D68BC4`, or `CFEAB0` match. This
is a qualified Source search, not proof of whole-program absence.

`GameStartupHost::application_construct` resets projected `ApplicationFrameState`
and its bookkeeping flag; `application_destruct` only logs and clears that flag.
`game_hosts_text.cpp` also explicitly documents the absence of an actual
`E1AE90` owner in that process path. Current raw singleton destruction requires
recovered profiles and actual bindings, otherwise throwing `logic_error`.

Concrete existing Source services are available for later reuse:

| Service | Existing contract | Relationship to this audit |
| --- | --- | --- |
| `get_native_singleton_manager_00415350` | Borrow the actual mutable manager publication; current nonnull fast path, otherwise allocate/construct/publish with captured-allocation cleanup. | Existing manager context; no new child call edge established. |
| `register_native_singleton_object_00bd0c30` | Raw registration using the actual stack argument slot. | Accepted constructor-side dependency; current Source identity pinned again. |
| `unregister_native_singleton_object_00bcfca0` | Clear only the first matching raw slot; preserve length, capacity and later duplicates; null argument touches no owner fields; no synchronization or destruction dispatch. | Potential reusable service only; no child operand/order inferred. |

Those services retain their existing Source CRT, allocation and exception
qualifications. This evidence-only packet provides no new ABI or runtime proof
for them and introduces no Application wrapper or combined factory.

## Next bounded prerequisite and validation

The single next prerequisite is **`00BEA8B0`**, the direct tail target. A separate
packet should start with its ledger, current Source, and body-size gate; inspect
the complete body only when within its agreed limit. Its actual receiver versus
current-publication use, manager/removal operands, lock capture and reloads,
table changes, and normal retirement order must be established without an
automatic descendant sweep.

Fresh 11-byte/two-instruction evidence plus the reused 93-byte/28-instruction
caller all match the current installed PE and decode completely. The JSON pins
eight current Source files, two target guards, and three prior/context files,
with bounded excerpts and normalized base-blob checks. Only this document and
its JSON report are tracked outputs. The result is a static evidence audit;
Application retirement, Source integration, ABI compatibility and game validation
remain separate claims.
