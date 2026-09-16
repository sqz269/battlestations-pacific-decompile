# Animation group and channel lifetime (R28)

The R28 source closes the concrete lifetime path for the R26/R27
`AnimationChannels` storage. `NativeAnimationDeleteCalls` binds the actual
`D62ED4` group and `D632B0` channel profiles to their current slot-4 scalar
deleting destructors. It is directly usable as the `NativeRefCountedDeleteCalls`
service required by the extra-item lifetime context. This is an explicit-service
C++ interface, not a callable native vtable or a binary replacement.

| Entry | Complete bytes | Reconstructed behavior |
|---|---:|---|
| `00B78530` | 195 | Destroy one actual `20h` channel group |
| `00B78D00` | 30 | Group scalar deleting destructor |
| `00B8A3C0` | 112 | Resize actual `28h` key vector |
| `00B8A7D0` | 127 | Destroy one actual `28h` channel |
| `00B8AD60` | 30 | Channel scalar deleting destructor |

The five bodies total 494 bytes. Their complete EH code/data and the two
profile pairs add 197 bytes. All 691 live bytes match the installed PE;
aggregate SHA-256 is
`735ee2317d4a0d9a485f9bef34549751e65a5216534405a44f4c54b2e86acdf8`.

## Group ownership order

`B78530` first stamps `D62ED4`. While current count `+14` is nonzero, it
reloads current vector data `+10` and current count, captures the last cell and
the channel pointer in that cell, then captures that channel's current profile
and slot 4. A nonnull channel receives flags 1 directly; no reference decrement
occurs. The captured cell is cleared only after the scalar deletion returns.
The group count is then reloaded exactly once, tested, and that captured value
is decremented when nonzero. A terminal callback can therefore mutate the
vector or count; the subsequent load keeps the native captured-versus-current
distinction without introducing a second count read.

After the loop, the normal path changes EH state before each cleanup stage:
resize the pointer vector to zero, free its current data, return the raw pooled
name `+8/+C`, then destroy the resource-item base through `B86890`. The name
header, vector pointer/capacity, and reference count remain stale. `B78D00`
always runs this destructor, frees the original group only when flags bit 0 is
set, and returns the original address even after free.

Profile `D62ED4` contains `BD30E0` at slot 0 and `B78D00` at slot 4. The finite
adapter chooses the supplied application table from the captured numeric
profile and reloads the table's current slot 4 for every dispatch. Unknown
profiles or mismatched slots fail explicitly; there is no arbitrary-vtable or
safe-fallback path.

## Channel and key ownership order

`B8A3C0` compares requested count and capacity as signed DWORDs. Capacity
growth uses the existing complete `B76680` reserve. Growth then zeroes ten
DWORDs for each new `28h` key, rereading vector data for every row. Shrink only
decrements the current count and never destroys or clears payload. The final
count is the requested bit pattern.

`B8A7D0` resizes the key vector at `+1C` to zero and frees its current data,
then returns the raw pooled name and calls `BD30F0`. It does not stamp `D632B0`
on entry; the base call leaves `CEB130`. `B8AD60` uses the same flags-bit-0 and
original-return rules as the group scalar wrapper. Profile `D632B0` contains
`BD30E0` and `B8AD60` in slots 0 and 4.

## Exception maps

Group FuncInfo `DFACEC` has max state 3 and unwind map `DFACD4`:

- state 2 calls `CC1CC3 -> B77A10`, draining/freeing the pointer vector;
- state 1 calls `CC1CB8 -> 41DD20`, returning the group name;
- state 0 calls `CC1CB0 -> B86890`, destroying the resource base.

Handler `CC1CCE..CC1CD7` loads `DFACEC` and tail-jumps to `BF6B43`. Thus a
channel terminal failure in state 2 frees the pointer vector and name and
destroys the base, but it does not retry or otherwise delete the channel.
Cleanup failure during C++ unwinding terminates.

Channel FuncInfo `DFBE34` has max state 2 and unwind map `DFBE24`. State 1
calls `CC2858 -> 41DD20`; state 0 calls `CC2850 -> BD30F0`. Handler
`CC2863..CC286C` loads `DFBE34` and tail-jumps to `BF6B43`. A key-cleanup
failure therefore advances through name and base cleanup. A name-pool failure
after the state changes performs only the base cleanup. The source reproduces
these effects for C++ exceptions, not original FH3/SEH or hardware faults.

## Saved-listing repairs completed by the primary agent

The primary cleared four returning CRT-free flow overrides and restored the
complete saved bodies:

| Call site | Required fallthrough/body |
|---|---|
| `B785A9` | clear `CALL_RETURN`; retain `B785AE..B785F2`, physical end `B785F2` |
| `B78D10` | clear `CALL_RETURN`; disassemble `B78D15..B78D17`, physical end `B78D1D` |
| `B8A805` | clear `CALL_RETURN`; retain `B8A80A..B8A84E`, physical end `B8A84E` |
| `B8AD70` | clear `CALL_RETURN`; disassemble `B8AD75..B8AD77`, physical end `B8AD7D` |

The primary also defined the complete ten-byte handlers `CC1CCE..CC1CD7` and
`CC2863..CC286C`. All 22 direct/tail rows now pass the live exact-call checker;
the captured channel slot-4 call remains explicitly indirect and unchecked by
that tool. The worker made no Ghidra, name-ledger, reconstruction-ledger, or
tag-ledger writes.

## Validation and limits

The Release Win32 build passed `/W4 /WX /EHsc /fp:strict /MD` and all three
configured CTests. The ignored `/MANIFEST:EMBED` focused fixture constructs a
real raw string pool, actual groups/channels/keys, and the actual finite scalar
deletion adapter. It invokes `BD30E0` source ref dispatch on the successful
group and later on an orphaned channel. Three successful-graph names and two
failure-path names return to the raw pool. Shrink preserves a key outside count;
growth zeroes it. The only synthetic terminal is an explicit throwing callback:
it verifies state-2 group unwind without pretending deletion succeeded.

Exact bodies, EH data, profile words, call rows, proposed metadata and receipts
are in `reports/native_animation_group_lifetime_r28.json`. The repaired saved
flow passes all 22 exact direct/tail rows. Build, byte, fixture and source-
interface evidence does not establish original binary ABI/FH3/SEH, native CRT
identity, executable admission, runtime integration or gameplay.
