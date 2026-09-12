# Raw input action tick and listener state

Addresses: `00A92C40`, `00A91A50`.

The new `native_input_action_tick` interfaces operate on the existing actual
24h action owner, its 30h records and actual 24h listener allocations. They
introduce no owning state, vector copy, typed listener projection or lifetime
domain. Older typed implementations and ledger records remain intact. Required
backend/rebind/poll/post-tick services are explicit; this packet does not supply
substitute device or game callbacks.

| Routine | Coverage | Original ABI / verified body |
| --- | --- | --- |
| A92C40 owner tick | Complete within the raw storage/provider domain | ECX owner; one float stack argument; RET4 at A92D0F, length3; body A92C40..A92D11, 70 instructions, no gaps |
| A91A50 listener state | Complete raw field and arithmetic behavior | ECX listener; float delta then two DWORD argument slots whose low bytes are previous/current; RET0C at A91BB2 and A91BBB, each length3; body A91A50..A91BBD, 129 instructions, no gaps |

Incoming EDX is not an argument to either. Native callers ignore results;
A91A50's final EAX incidentally equals its derived +0C byte. These are new C++
interfaces, not original callable/FH3/SEH ABI replacements. No Ghidra writes
were made, and both owned function starts exist.

Producer evidence is independent of consumer offsets. A93DA0 initializes the
24h action owner with profile D5B630 and raw action base/count/capacity at +4/+8/+C.
A93940 initializes each 30h record, including enabled byte+1, previous
float/latch +1C/+20, current float/latch +24/+28 and listener pointer+2C;
its padding remains untouched. A92B70 allocates exactly 24h at A92B74/78,
sets profile D5B610 and refcount+4=1, zeros twelve flag bytes +8..+13 and
four float timers +14/+18/+1C/+20, then replaces record+2C after releasing the
old listener. D5B610's slots are BD30E0 and A92150. This packet neither allocates
nor releases listeners; their separate owner packet supplies that lifecycle.

A92C40 loads delta through x87 before capturing F8BBF4 and the current backend
profile. Its slot04 call receives the binary32-spilled value. Afterwards it
reloads the publication, captures and clears only byte+D4, and calls A922A0 on
the original action owner if that captured byte was nonzero. It then reads count
before base to establish the initial action endpoint.

Each enabled record is captured across A92370. The listener pointer is loaded
after that call. `NEG/SBB/TEST immediate F8BC00` is only a nonnull test, not a
read of a global enable flag. Current latch/hold is tested first, then previous:
nonnull latch and ordered COMISS greater than the freshly loaded D7A218 value.
The original delta is separately loaded/spilled through x87 for A91A50.
After each record, owner count and base are reread to form the next endpoint,
then the retained record pointer advances by 30h. No snapshot, capacity bound
or altered malformed-storage termination rule is added.

The listener implementation preserves the native x87 operations, binary32
spills, individual byte stores and comparison branches. Its header/refcount
remain untouched. The live E12F20 timing threshold is loaded at all four
original sites; E12F24 and E12F28 remain separate live repeat threshold/step
reads. Defaults are 0.25f, 0.5f and 0.1f, supplied by the caller's storage.
The +14 timer measures time since release, +18 since press. Quick edge and
expiry/repeat tests are strict; held is inclusive. Only one repeat step is
subtracted per continuing-down update. Derived +09 is +08 AND NOT +0A;
+0C is +0B AND NOT +0D. No wholesale bool conversion replaces stored raw bytes.

| Required service | Native CALL site | Contract |
| --- | --- | --- |
| Captured backend slot04 | A92C57 | Same raw receiver and captured profile, one float stack word; parent can route to raw A918A0 |
| A922A0 rebind | A92C71 | Same actual action owner, no stack args; separate raw binding packet |
| A92370 poll | A92C90 | Captured actual 30h record, no stack args; separate raw binding packet |
| A91A50 state | A92CEA | Captured listener, original delta and low-byte previous/current; implemented here |
| Captured F8BBFC callback | A92D0D | Load/test once at tail, no semantic arguments/result; explicit required game service |

The only known F8BBFC writer is 4DD71B, which installs 6965A0. That missing
function start has a complete verified 11-byte body: 6965A0 loads ECX from
E188A8, then 6965A6 tail-jumps 4D8CD0. Inclusive end6965AA, exclusive6965AB.
4D8CD0 records game action deadlines; no fake empty callback is provided.
This unowned dependency and its raw game binding remain outside this packet.

All 33 live tick xrefs were inspected. Thirty have stored containing functions;
three unowned sites 5E737A, 5EC84C and 5EC921 have no containing Ghidra body.
Their exact 18-byte argument blocks are independently verified: FLDZ, reserve
and spill one float, CALL4BEC00, MOV ECX,EAX, CALLA92C40. They are recorded as
unattributed native-byte evidence, not assigned invented caller boundaries.
Most tick callers pass zero; 4E07DD/4E07F3 read live CE7628; 4E4A72 forwards
its incoming float. A91A50's other three sites in A92A20/A92D40 include an
explicit FLD1 delta at A92E4C. All agree on the complete stack contract.

MSVC Win32 Release build and both existing CTests passed after all eight native
seed ranges matched. One ignored archive-only probe compared copied original
A91A50 bytes against 32 raw whole-storage cases, including noncanonical low-byte
arguments, signed zero, finite values, infinity and a quiet NaN. Two copied-native
tick cases matched traces and raw preimages under controlled mutation of the
backend publication, byte+D4, owner base/count, enabled flags, listener pointer,
timing cell and tail callback. Empty-table/null-tail behavior also matched.

Exact byte spans, hashes, CALL evidence and remaining analysis gaps are in
`reports/native_input_action_tick.json`. The fixture used controlled providers;
no SDK, real device poll, force, ShowCursor or game execution occurred. Native
hardware faults, asynchronous mutation, unmasked floating exceptions and full
application/frame behavior are not claimed. Callback exceptions propagate with
partial effects intact and without added cleanup.
