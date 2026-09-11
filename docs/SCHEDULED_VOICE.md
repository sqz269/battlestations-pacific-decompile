# Scheduled voice admission, start and pending poll

Packet `orch4_scheduled_voice_c`. Read-only Ghidra queries verified existing
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`. The implementation
is in `src/scheduled_voice.cpp`; shared projection types are in
`include/bsp/scheduled_voice_types.hpp`. Names are descriptive hypotheses.

## Corrected manager field and record identity

Manager **+74 is a borrowed pending record pointer**, not a disabled flag.
005B9414 stores the input record pointer directly there. 005B9452 reloads it
after polling and passes it to 005B9300; 005B945D clears it after that call.
None of these operations retains/releases the record. The existing readiness
test at 005B71D0 checks this same field for nonzero: waiting scheduled work
blocks ordinary voice admission.

The producer provenance establishes that this is the same manager/record path:

* 004483F0 loads `[00E188A8]+21DC` as the record-lookup owner and calls 00705E00.
* 0044841B..29 then loads `[00E198C4]+A4` as the voice manager and passes the
  lookup result unchanged into 005B94D0.
* 005B953E passes that same record into 005B93D0. 005B9327..30 later constructs
  `Clip12{00CF0DD0, same record pointer, resource index0}` for real slot startup.
* On the existing-map branch, 00706035..40 returns node+14, an inline map value.
  Its absent-record branch initializes sound ID=-1 and timed keys. The record
  owner and table destruction are external; this packet does not copy that
  value or claim to implement its map/allocator.

`ScheduledVoiceBindings` aliases actual +70/+74/+84/+94 fields. It must never be
backed by snapshots or a second pending queue. `ScheduledVoiceHost::timed_keys_34`
is a side-effect-free view of the vector belonging to the exact supplied
`VoiceClipRecord` identity. Resolving by text or sound ID is incorrect. The
record and timed-key storage must outlive pending work and row references.

This worker leaves the existing shared manager header untouched. Integration
must include `scheduled_voice_types.hpp`, replace `disabled_74` with
`const VoiceClipRecord* pending_record_74`, and bind `slot_index_70`,
`selected_row_84`, and `rows_94` directly. The parent's sequence update owns
additional +44/+88/+8C fields and the final callsites.

## Recovered rows and timed keys

| Native location | Established meaning |
| --- | --- |
| manager +70 | signed slot index; admission writes0 or -1 |
| manager +74 | pending borrowed `VoiceClipRecord` prefix pointer |
| manager +84 | unsigned selected row index |
| manager +94/+98 | begin/end of native34h rows |
| row +00 | widget pointer, consumed by 005BBF10 |
| row +0C | fade countdown; producer copies manager+78 |
| row +10/+14/+18/+1C | four color words; producer copies manager+44..50 |
| row +20 | active timed-key byte |
| row +24 | same borrowed record pointer |
| row +28 | unsigned timed-key index |
| row +2C | scheduled start mission clock |
| row +30 | signed playback-slot index |
| record +30/+34/+38 | native debug-vector header/begin/end for1Ch keys |
| key +00/+08 | NativeString text / callback name |
| key +10/+14/+18 | start float / end float / text flag byte |

005B9549..95A9 checks record's key vector and reads last key+14 into manager+8C.
The full row/key consumer is parent-owned 005BBF10; its accesses corroborate the
two strings, timing floats, flag, row pointer/index and clock fields. Unknown
row +4/+8, padding, exact native vector ownership and original allocator ABI
are omitted. Typed defaults are host conveniences, not recovered constructors.
NativeString storage remains explicitly owner-managed.

## Recovered sequences and callback order

**005B9300 start:** if record+8 is nonnegative, call the actual reconstructed
005B9050 with current manager+70, Clip12 resource index0 and null bank. No extra
bank retain is needed. After that call and all its callbacks/logging, reload
current selected row and slot index. Store row+30; set row+20 from whether the
record's timed-key vector is nonempty. Nonempty sets row+24 to the record,
row+28 to0, then captures mission clock into row+2C. Empty only clears row+20:
it leaves the previous pointer, key index and timestamp untouched.

The row range check happens **after** sound startup. The native calls existing
CRT invalid-parameter helper00BF6713 on an invalid range, then continues if its
handler returns. The typed projection uses `std::vector::at`, which throws;
malformed native layouts and invalid-handler continuation are not reproduced.
No CRT/STL implementation or replacement library name is introduced.

**005B93D0 admission:** only nonnegative sound IDs poll for a free slot. The
loop stride is18h but `CMP index,1` proves only slot0 is visited. A free poll
stores+70=0 and starts; a busy poll stores+70=-1, puts the borrowed record into
+74, and returns. Negative IDs start immediately without changing +70 or
clearing existing +74. The poll is the already-reconstructed effectful
007027B0, not an immutable busy-state check.

**005B9420 pending poll:** if +74 was nonnull at entry, probe slot0 and write
+70=0/-1. When free, reload the current +74 pointer *after* retirement callbacks,
start that record, then clear +74 unconditionally after start callbacks.
It returns true for this entire branch, including a successful start of a
soundless record. A callback that clears +74 during the first poll would make
the native dereference null; there is no synthesized successful skip.

If no record was pending at entry, capture nonnegative +70 and poll that slot.
Busy returns true; completion writes+70=-1 and returns false. Negative +70
returns false immediately. A pending record created by that poll is not
reconsidered within the current call. Nonzero slot mapping uses the existing
slot-start host's exact address mapping/fault contract; it never substitutes
slot0 for an arbitrary index.

The bindings and the row/key view must remain stable through each native-equivalent
field access. Playback callbacks may change fields and selected row; subsequent
reads follow the native order. Structural invalidation of a row while a native
pointer to it is held is outside the valid projection contract.

## Native ABI and evidence

| Address | Original ABI | Inclusive final instruction (length) |
| --- | --- | --- |
| 005B9300 | ECX=manager, borrowed record* stack, RET4 | 005B93CA (3) |
| 005B93D0 | ECX=manager, borrowed record* stack, RET4 | 005B9419 (3) |
| 005B9420 | ECX=manager, no stack args, RET; boolean in AL only | 005B9489 (1) |

Assembly is authoritative over incomplete decompiler prototypes. Start's
Clip12 and bank arguments are explicit stores at005B930F..933F. The pending
reload/clear ordering is005B9452..945D. All three claimed functions have complete
flow:65,34,42 listed instructions, zero gaps. Supporting producer005B94D0 also
has zero gaps. No missing function start was discovered. The `stl_probable`
tag at005B9300 is inconsistent with this recovered game orchestration; parent
may retire that tag when applying names. No Ghidra annotations were changed.

## Validation boundary

`reports/scheduled_voice.json` records build and focused local-probe results.
The local probe exercises deferral, replacement of pending identity during
polling, row selection changes, post-start pending clearing, the pending
branch's true return, and soundless/empty-key stale-field preservation. It
does not emulate FMOD or claim native audio playback. Existing real reconstructed
slot startup/polling are called directly. Original binary ABI/SEH, rendering,
installed-game execution and native differential behavior remain unvalidated.
