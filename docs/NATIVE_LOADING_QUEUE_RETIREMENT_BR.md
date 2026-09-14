# Native loading queue front retirement (BR)

Address: `00506BF0` (131 bytes, through `00506C72` inclusive).

`src/native_loading_queue_retirement.cpp` reconstructs the complete ordinary
body over actual 20h loader and 24h job storage. Native ABI is ECX loader, RET,
with no specified semantic return. Names are descriptive hypotheses.

The function reads the initial front job without an empty guard. If job+20
contains a FileBlock, it reads the current table and deleting target at +4,
calls it with flags1, then clears that initial job's +20 only after return. The
source binds the reached BDEBE0 target to the BQ implementation. Table bytes
must be readable; an unsupported reached target throws at 506C07 before any
clear, and original numeric code is never executed.

Next, reacquire the current FileStore factory, form the initial job's name+14
after that getter, acquire its provider, and remove that name. Only after removal
does the native body reload the **current array and current front**. A nonnull
reselected job is destroyed through 5051A0, freed through BF65AC, and its captured
slot is cleared. The loop shifts with current array/count reads each iteration,
then decrements the current count. Capacity and the vacated trailing slot are
unchanged. No work-result release, thread stop, wake, join or handle close is
introduced.

Ghidra's CALL_RETURN override at 506C39 excluded the ADD ESP and slot-clear
instructions at 506C3E..506C46 from the function even though other control flow
already reached the later shift loop. The repair cleared that one override,
recreated the function without deleting code, restored prior evidence, verified
all 50 instruction starts belong to 506BF0, saved, and refreshed exports. A
zero-gap or min/max body report alone would not prove this ownership.

The explicit retained frame records reached call failures and owns the nested
BQ FileBlock invocation. Native 506BF0 has no local FH3 frame. The source still
inherits BQ's retained outer-failure boundary instead of native FileBlock cleanup,
and callee-internal string/provider/stream limitations remain. Active/failed
frame destruction terminates; callers must retain frame and borrowed storage
after failure. There is no replay or invented rollback.

Strict MSVC Win32 compilation and an actual-service fixture passed: two jobs,
submission/growth, prepared FileBlock and native gate list, a resident FileStore
node retaining a concrete memory stream, retirement to zero stream counters,
front shift, unchanged capacity/trailing slot, final job removal and pool cleanup.
This fixture uses no worker thread. Byte/call/ownership receipts are in
`reports/native_loading_queue_retirement_br.json`; this is not production queue
or gameplay validation.
