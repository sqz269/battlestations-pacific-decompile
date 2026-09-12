# Native input action records

Addresses: 00A93940, 00A939C0, 00A93A60, 00A93A80, 00A93B30, 00A93C10.

`native_input_action_records` implements six complete normal schedules and
documented C++ exception cleanup over actual30h records. Names are hypotheses.
The record contains a DWORD-array header at+4, a34h-binding-array header at+10,
scalar floats/bytes at+1C/+20/+24/+28, and an actual retained listener at+2C.
Constructors preserve unwritten bytes and padding. No projected owning arrays
or projected reference counts replace these native fields.

Constructor/destructor receive ECX and RET. Copy takes one source pointer on the
stack and RET4; scalar deletion takes one flags word and RET4. Constructor/copy
return the captured destination; scalar deletion returns its captured allocation
address, including after lowbit1 frees it. Reserve/resize take a signed DWORD
capacity/count and RET4. Explicit contexts change the source API; original ABI,
FH3/SEH and hardware-fault compatibility are not claimed.

A93940 initializes only the native fields. Its otherwise dead branch reloads the
just-cleared listener and conditionally clears its fields; no callback intervenes
and it remains unreachable for valid single-threaded storage. Copy deep-copies
both arrays, copies floats through x87 FLD/FSTP, and retains the reloaded actual
source listener with InterlockedIncrement at listener+4.

A939C0 captures the listener, decrements that same reference count, and on zero
calls the listener's current slot-zero provider. The native A93A01 call has NO
stack flags. The record slot is cleared only after the callback returns. Cleanup
then resizes/frees the binding array and finally the DWORD array, using their
current bases after each resize. Freed pointer/capacity bytes are not normalized.

Its FuncInfoDECAE8 mapDECAD8 transitions state1 to0 throughCB67BB/A938E0
(bindings) and state0 to-1 throughCB67B0/6972B0 (DWORDs). A listener exception
therefore cleans both arrays; a binding cleanup exception cleans only the DWORD
array. Copy's DECB1C/DECB0C map cleans completed DWORD storage when binding copy
throws. Source C++ guards preserve these supported transitions and terminate on
a second exception during unwind.

A93B30 copies records forward, destroys old rows forward using live header
reloads, frees the old allocation, then publishes replacement base/capacity.
Its DECB48/DECB40 state0 unwind invokes CB67F0/00401130 placement cleanup, an
actual no-op: no rollback or unpublished-array reclamation is invented.
A93C10 decrements count before each reverse destruction and reloads current
base/count around callbacks. Listener implementations remain required providers.

Validation: six complete body byte spans matched live Ghidra and the installed
PE;15 direct CALL rows passed the mechanical report audit. Indirect OS imports
and listener argument/profile provenance were checked in the listing. Win32
Release and both existing CTests passed. The focused nonempty owner fixture
matched four original A93940 preimages across all48 bytes, checked nested deep
copies and reserve listener retain balance3 to1, scalar identities, and an
injected listener exception with native nested/base cleanup. The normal fixture
retains an external listener reference; fault injection is not a recovered
listener implementation. Initial linkage used the primary provider archive plus
worker owner source; final shared-drain proof belongs to
`NATIVE_SINGLETON_INPUT_ONLINE`. No permanent tests or gameplay proof were added.

Follow-up: connect canonical action records to the frame/classifier producers
and recover actual listener slot-zero bodies. This packet establishes storage
and lifetime behavior, not real device input or application action processing.
