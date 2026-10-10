# Allocation-statistics base destructor failure actions

The actual `BE27F0` descriptor selects two complete eight-byte actions.
For state 1, `CC6AA8` passes the actual guard address to `00411EE0` and
transitions to state 0. After an ordinary return, `CC6AA0` loads the saved
receiver for `00412430` and transitions to `-1`. This closes the previously
unopened destructor action identities and bounded ordinary cleanup schedule.
It does not establish OS delivery of the parent registration node, Native FH3
identity, hardware-fault behavior, cleanup-failure policy or a Source destructor.

`CC6AB0` is correctly classified as an **unowned defined handler stub**, not an
unwind action. No function was created, name changed, body extended, flow
override repaired, project saved or Source implementation added. Exactly this
document and its report are changed.

## Bounded discovery and admitted reads

All fresh typed replies independently validate `C:/Users/sqz269/bsp.gpr`,
program `/battlestationspacific.exe`, x86 little-endian 32-bit language,
image base `00400000` and modification 35. The initial entry query showed a
defined five-byte instruction at `CC6AB0`, falling through to `CC6AB5`, with
no exact or containing function. Exact context-zero listing then showed a
descriptor load followed by the saved five-byte handler jump. The prototype
query reports no function; no function prototype is invented for that stub.

Root separately admitted each following step after its prerequisite was
reported. There was no physical tail probe.

| Stage | Exact Original/live windows | Evidence that admitted the next step |
| --- | --- | --- |
| Handler and descriptor | `[CC6AB0,CC6ABA)` 10 bytes; `[E0123C,E01260)` 36 bytes | Two exact saved instruction starts and their lengths identify the complete stub; its immediate supplies the descriptor address. |
| Unwind map | `[E0122C,E0123C)` 16 bytes | The actual descriptor declares **two** entries at `E0122C`. Collection stopped for Root's separate admission; no one-entry shortcut or constructor-derived count was used. |
| Actions | `[CC6AA0,CC6AA8)` 8 bytes; `[CC6AA8,CC6AB0)` 8 bytes | Both actual map targets independently have complete single-range eight-byte saved function bodies. Root admitted only those bodies. |

The five semantic windows total 78 bytes: 26 code and 52 data. Original and
live bytes match for every window. Each of the three capture stages separately
streamed the complete 12,223,752-byte installed original for SHA-256 before
and after the selected reads, with unchanged size/mtime throughout. Every hash
is `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Those six streams are integrity checks, not additional semantic-byte credit.

VA-to-raw mapping is reparsed from the complete retained DOS/COFF metadata,
image-base/header-size fields and section table. Their full pins are verified,
and their original whole-image identity equals all fresh before/after hashes.
Thus code maps through `.text`, descriptor/map through `.rdata`; no new
header, relocation-directory, neighboring descriptor, parent or callee window
was opened. The retained-header identity is stated explicitly rather than
presented as freshly collected headers.

## Handler and exact descriptor fields

The handler bytes are `b8 3c 12 e0 00 e9 89 00 f3 ff`:

| Address | Saved length | Complete physical decode |
| --- | --- | --- |
| `CC6AB0` | 5 | `MOV EAX,00E0123C` |
| `CC6AB5` | 5 | `JMP 00BF6B43` |

Both instructions remain unowned. At `CC6AB5`, default flow is an
unconditional jump; the existing `CALL_RETURN` override makes effective flow
a call terminator. Both expose the same direct target `BF6B43` and no
fallthrough. Physical interpretation retains the encoded jump separately from
that saved analysis override. The shared handler's bytes are not reread.

Only the previously accepted x86 absolute-pointer, four-byte-packed FuncInfo
layout is used to interpret the descriptor. Full raw bytes, signed and unsigned
DWORD values, offsets and field VAs are retained in the report.

| Offset | Field | Raw DWORD |
| --- | --- | --- |
| `+00` | magic and bbt flags | `19930522` |
| `+04` | maxState / unwind count | `00000002` |
| `+08` | unwind-map pointer | `00E0122C` |
| `+0C` | try-block count | `00000000` |
| `+10` | try-block-map pointer | `00000000` |
| `+14` | IP-map count | `00000000` |
| `+18` | IP-to-state pointer | `00000000` |
| `+1C` | exception-specification pointer | `00000000` |
| `+20` | EH flags | `00000001` |

The low 29 magic bits are `19930522`, with high-three bbt flags zero. EH flags
remain raw `1`; no new runtime policy is inferred. This interpretation does
not identify the current Program's data type by name or attest the loaded Java
CodeSource. The admitted layout's prior source/receipt chain remains retained.

The complete map is:

| State / record VA | Raw bytes | Signed previous state | Action |
| --- | --- | --- | --- |
| 0 / `E0122C` | `ff ff ff ff a0 6a cc 00` | `-1` | `CC6AA0` |
| 1 / `E01234` | `00 00 00 00 a8 6a cc 00` | `0` | `CC6AA8` |

## Complete actions and preserved metadata

| Action | Complete eight-byte body | Physical operations |
| --- | --- | --- |
| `Unwind@00cc6aa0` | `8b 4d e8 e9 88 b9 74 ff` | `MOV ECX,[EBP-18h]; JMP 00412430` |
| `Unwind@00cc6aa8` | `8d 4d ec e9 30 b4 74 ff` | `LEA ECX,[EBP-14h]; JMP 00411EE0` |

Each exact function is non-thunk and `no_return=false`, with one complete
eight-byte saved range and two saved instructions (three plus five bytes).
Both metadata prototypes are `undefined Unwind@...(void)`; they do not describe
the recovered frame-register input or establish an ordinary callable ABI.
The correct `Unwind@` names remain unchanged.

The jumps at `CC6AA3` and `CC6AAB` also retain `CALL_RETURN`: default jump and
effective call-terminator flows share their actual targets. Typed nested target
metadata identifies returning non-thunk `BSP_SingletonBase_ResetProfile` and
`BSP_SystemSingletonGuard_Destroy`; their bodies were reused from retained
accepted receipts only. Full instruction bytes, signed relative displacements,
resolved direct targets and memory displacements are indexed. These decoded
address references are not a claim about PE relocation-table entries.

The retained seven-byte `00412430` leaf stamps `CE3818` at receiver `+0` and
returns. The retained 25-byte `00411EE0` leaf first captures the guard's current
`+4` section pointer, then stamps `CE37FC`; if nonnull it decrements that
section's `+18`, calls `LeaveCriticalSection`, and returns, preserving guard
`+4`. The new actions contain no unregister, stats-publication clear, allocation
free, retry or rollback. Their use of the actual guard address must not be
replaced by a stale copied section pointer where mutable-guard aliases matter.

## Conditional parent and return coordinates

Offline re-decoding of the retained complete 153-byte `BE27F0` body gives
entry ESP `S`, registration node `S-0Ch`, and quiescent ESP `Q=S-20h`.
Its captured incoming receiver is stored at `Q+08h=S-18h`; the actual guard
is at `Q+0Ch=S-14h`, with its section pointer at `Q+10h=S-10h`; the state
word is at `Q+1Ch=S-4`.

The previously accepted wrapper/dispatcher/prologue/frame-setting composition
gives action EBP equal to forwarded raw argument two plus `0Ch`. The unwind
routine's own FS registration node is distinct. **Only if that forwarded raw
argument is the destructor's parent node** does action EBP equal `S`.
Under that substitution, state zero reads the current saved-receiver slot;
calling it the original receiver additionally requires that spill to remain
intact. State one passes the address of the actual parent guard. The new action
bytes establish these operands, not the OS source of the forwarded argument.

For the retained observed unwind-to-`-1` path, the loop writes the predecessor
state before calling each nonzero action. Starting at state 1 therefore stores
0 and calls `CC6AA8`; only after its ordinary return does the next iteration
store `-1` and call `CC6AA0`. A cleanup escape does not prove that the remaining
action runs. Both new actions tail-jump without changing ESP. Conditional on
the retained leaves' ordinary returns and valid saved return storage, their
returns reach the frame helper's `C07B37` with ESP `T-1Ch`, where `T` is
helper-entry ESP. This is stack algebra, not Native execution or an ABI test.

The destructor's `D685E0` profile store at `BE2810` is **before** its full
state-zero store at `BE2816`. This differs from the constructor's previously
accepted state-zero-before-profile order. The guard/profile/section writes and
conditional enter/increment precede the state-one byte store at `BE2841`;
state one precedes the second getter/current-stats unregister and remains
visible across normal publication clear and captured-section release. A future
Source destructor must preserve its own arm order, not copy the constructor's
ordering mechanically.

## Source implications and evidence closure

This gate now supports an explicitly qualified ordinary Source C++ failure
composition using the genuine guard and base-reset providers. It supplies no
destructor/scalar-delete implementation, permanent stats publication cell,
deletion dispatch binding or startup handover. Normal destructor publication
clear still belongs after successful current-stats unregistration; cleanup
actions do not manufacture rollback. A second cleanup failure, hardware/SEH
escape, original frame-spill aliases, OS routing and Native FH3 behavior remain
separate obligations. There is no claim that retry or continued drain after
partially completed failure is safe.

The packet preserves eight complete typed captures (25 responses, all epoch
35) and 45 other complete raw HTTP replies: 43 GETs and two context-zero
read-only POSTs. The expected no-function signature error for the handler is
retained in full. No script capability, availability fallback, disassembly
creation, Ghidra write or save was invoked. Original/live binary windows,
fresh integrity receipts, raw layout words, metadata/default/effective flows,
complete code decodes, parent/leaf receipts and exact command arguments are
frozen alongside this document/report.

Selected Source files are frozen from accepted main
`e9396b4883232059bdad28753bc95403536403bf` (Source749). Its complete build
context remains untouched with Root; this gate does not rebuild or relabel that
evidence. Baseline `baabfc15074b9209de22575cb09741ddcdae4a2a` supplies prior
documents and tooling. The entire preceding immutable readiness archive is
retained with its original full pins. `verify_evidence.py` replays local pins,
full Git files, strict typed validation, raw HTTP hashes, byte comparisons,
complete code/data interpretation and ZIP payloads entirely offline. Archive
and manifest hashes are recorded separately in `final_verification.json` to
avoid self-reference. No compilation, test, Native/Source execution, startup or
gameplay evidence is added.
