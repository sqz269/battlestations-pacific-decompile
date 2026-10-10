# CC12 allocation statistics: actual CallSettingFrame gate

The actual `__CallSettingFrame@12` instructions compute the second argument
plus `0Ch`, install it in EBP, and invoke the first argument as an action.
The separately admitted notification path preserves that EBP and the action
address in EAX on ordinary return. This closes the previously conditional
frame-setting arithmetic at this helper's action entry. It does not establish
the upstream `C069A2` SEH-prologue slots, OS-handler entry arguments, failure
policy, or the allocation action's physical epilogue.

The worker fast-forwarded to accepted main
`b5483aa916bef6780edbe35036980f309f814e30` before claiming this packet.
Only this document and `reports/cc12_allocation_stats_call_setting_frame_gate.json`
are changed. The preceding 448-byte frame gate and all earlier constructor and
startup evidence remain immutable. Correct library names and saved listing,
flow, prototype, AddressSet and ownership metadata are preserved.

## Exact admitted bytes and ownership

| Original/live span | Role | Bytes | Instructions |
| --- | --- | ---: | ---: |
| `[C07B10,C07B5C)` | Complete saved `__CallSettingFrame@12` | 76 | 34 |
| `[C16870,C16879)` | Complete saved `__NLG_Notify1`, ending in a jump | 9 | 4 |
| `[C16884,C16898)` | Actual shared suffix inside saved `__NLG_Notify` | 20 | 12 |

The three exact windows match their original PE bytes, and all 50 decoded
instruction starts/lengths match typed metadata. The total fresh scope is
**105 instruction bytes**, with no data read. The first two saved bodies also
have matching complete Ghidra listings. The suffix uses exact bytes plus typed
starts; its enclosing function listing was not requested.

`C16877` has an ordinary unconditional jump to `C16884`, no override and no
fallthrough. The target belongs to saved owner `C16879`, whose complete saved
range is `[C16879,C16898)`, 31 bytes. Only its 20-byte suffix is admitted here.
The excluded owner prefix `[C16879,C16884)` is **11 bytes (`0Bh`)**. An early
handoff called this five bytes; the arithmetic was corrected before freezing.
The range and the 105-byte admitted total never changed. No fresh original/live read covered the excluded prefix,
neighboring bytes, destination data words or enclosing 40-byte notification
interval. No function boundary or flow repair is needed or performed
to preserve and describe this cross-owner jump.

## Notification path closes the immediate register/stack dependency

Let notification entry ESP be `N`, with EAX=`A`, EBP=`F`, EBX=`B`, ECX=`C`.
The entry at `C16870` pushes EBX and ECX, loads EBX with the encoded address
`E16830`, and jumps directly to `C16884`. It therefore carries code in ECX;
it does not load code from the stack before the stores.

The actual suffix performs these DWORD stores, in order:

1. `[E16838] = C` (code, offset `+8`).
2. `[E16834] = A` (destination, offset `+4`).
3. `[E1683C] = F` (frame, offset `+Ch`).

It then pushes EBP, ECX and EAX, pops them in reverse order, restores the
entry ECX and EBX, and executes `RET 4` at `C16895`. Under ordinary return,
EAX, EBP, EBX and ECX equal their entry values, EDX/ESI/EDI are untouched,
and ESP becomes `N+8` after consuming the return address and one code DWORD.
The observed MOV/PUSH/POP/JMP/RET instructions preserve flags. The saved code
DWORD is consumed even though this particular entry takes the value from ECX.

These are encoded stores, not reads or validation of the destination's current
contents, mapping, ownership or initialization. There is no signature store,
lock, validation or callback in the admitted path. Faults or nonlocal exits
can interrupt the ordered stores and stack restoration; ordinary-return
preservation is not a failure-path guarantee.

## Actual frame-setting and action sequence

Let helper entry ESP be `T`, with action `A=[T+4]`, raw second argument
`R=[T+8]` and code `K=[T+0Ch]`. Its own explicit prologue establishes
`B=T-4` in EBP, reserves one DWORD, and saves entry EBX and ECX. At `C07B18`
through `C07B1E`, it computes `(R+0Ch) mod 2^32` in EAX and stores it at
`[B-4]`; `C07B21` then loads action `A` into EAX.

The helper saves its own `B` on the stack, pushes `K`, sets ECX to `K` and
loads EBP from `[B-4]`. It calls `C16870`. The admitted notification path now
establishes that ordinary return preserves EAX=`A` and EBP=`R+0Ch`, while
consuming the stacked code word. It then pushes ESI and EDI and executes
`CALL EAX` at `C07B35`.

Thus the **actual action-entry EBP is `R+0Ch`** on this ordinary path. No extra
action argument is pushed: the two immediately preceding pushes save ESI and
EDI. Those saved values and the helper's own saved frame remain on its stack.
At action entry, ESP is `T-20h`, including the action return address. This
stack equation follows from the observed notification `RET 4`; it is not an
assumption based on a decorated library name.

After a balanced ordinary action return, the helper restores EDI/ESI, copies
the action-return EBP into EBX and pops its own `B` back into EBP. It reloads
the original code `K` through that frame, saves `B` again, and temporarily sets
EBP to the action-return value held in EBX. It preserves that returned frame
value for the second notification even if the action changed EBP.

If `K==100h`, the second code becomes `2`; otherwise it remains `K`. The helper
pushes that code and calls `C16870` again. The ordinary second notification
therefore publishes the action-return EAX as destination and action-return
EBP as frame, preserving both registers. Finally it restores its own frame,
the entry ECX and EBX, executes `LEAVE`, and returns with `RET 0Ch`.

| Ordinary stack point | ESP relative to T |
| --- | ---: |
| After own frame/local/EBX/ECX saves | `-10h` |
| Before first notification call, after own-frame/code pushes | `-18h` |
| After first notification returns and consumes code | `-14h` |
| At action entry after ESI/EDI saves and CALL | `-20h` |
| After action return, EDI/ESI restores and own-frame POP | `-10h` |
| After second notification returns and consumes code | `-14h` |
| Final helper return, including its three argument DWORDs | `+10h` |

These normal-return equations require the action to return with its call stack
balanced and the required saved storage intact. The helper then preserves
entry EBX/ECX/EBP/ESI/EDI and returns the action's EAX. EDX can carry the
action's changes. Flags are not generally preserved: the initial address ADD
changes them, and the post-action `CMP ECX,100h` supplies the flags that survive
the final straight-line notification/restoration sequence.

## What this closes, and what remains open

The preceding saved unwind spans physically push `103h`, their loaded EBX,
and the map's current action pointer before calling `C07B10`. For that call,
both notifications use `103h`; the `100h -> 2` branch does not apply. The action
gets EBP equal to **that loaded EBX plus `0Ch`**. The arithmetic and ordinary
notification preservation are now actual instruction evidence.

The prior parent prologues publish registration node `S-0Ch`. If the upstream
unwind helper's loaded EBX is that node, action EBP equals parent entry ESP S.
This would align the constructor's saved receiver/guard operands and the
startup allocation-spill operand with their observed locals. The association
still depends on unopened `C069A2` SEH-prologue behavior and the actual upstream
handler argument convention. Neither is inferred from installed headers or
the observed offset alone.

The startup action remains physically open after its saved returning-free
call at `C86A37`. Its existing `CALL_RETURN` override and undefined `C86A3C`
continuation were not changed or reread. This packet therefore does not prove
that action's balanced return, additional cleanup effects, publication policy
or complete physical function. The earlier unwind gap, cleanup descendants,
nested exceptions, OS entry, Native FH3 identity and fault behavior remain
separate holds. There is no blanket original-exception or gameplay claim.

## Existing Source context and verification

The current Source query found the existing naked Win32
`notify_native_crt_nlg_00c16879` leaf. Its Source takes code from the one stacked
DWORD before the same store/restore sequence; its header requires an already
published canonical data owner and separately established EAX/EBP inputs.
That is the distinct stack-code entry `C16879`, while this packet's actual
`C16870` path uses ECX code. The current Source/header and named DJ doc/report
are retained as context. Their old build, static-link, data-owner and artifact
claims were not rerun or promoted to current runtime evidence. This packet
adds neither a `C16870` Source entry nor a CallSettingFrame implementation.

The complete current baseline contains 88 inputs, including 40 Source files;
87 match Git exactly and one previously qualified older report differs only
by CRLF/LF. Two resource-application Source files changed during the authorized
sync, as already identified by Root's accepted frame review; current full
snapshots are retained separately from the immutable older baseline. Thirteen
bounded excerpts, one complete sorted Source query and eleven predecessor
receipts accompany the full previous ZIP. Whole-file copies preserve provenance
without whole-file semantic or old-artifact execution credit.

All 54 accepted typed replies are independently validated at epoch 30; 32
other raw GET replies are retained. Loaded Java CodeSource remains unattested.
Exact original reads reuse admitted PE mapping and stable size/mtime, with no
new header read or whole-image hash. Offline checks replay reply hashes,
identity/schema, exact windows, all 50 instruction boundaries, baseline and
excerpt/query pins, and every ZIP member/CRC. No GPR mutation, save, annotation,
repair, Source/CMake/ledger change, build, test, probe or runtime call occurs.
