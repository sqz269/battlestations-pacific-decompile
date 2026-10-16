# Actual hidden-record destruction

This packet reconstructs all152 bytes/53 operations at00469300, all30 bytes/11
operations at004693A0, and the qualified11-byte/3-operation action00C61AB0.
The old Seg6Host projections remain separate. New explicit raw Source APIs do
not establish Original entry, object/vtable, private-frame or binary ABI identity.

The destructor receives the record in ECX, has no stacked argument and plain
RET. Entry ESP=Q gives saved-this atQ-10h. Both native writes to this cell precede
the CE5640 publication; preserve both even when payload aliases the cell. This
literal is a borrowed original image word, not an invented Source vtable.

Read current record+4 once, then arm the Source cleanup mapping. A nonnull bag
loads its actual vtable and slot0 and calls that actual code with ECX=bag and
stacked1, ignoring its result. No callback metadata, shadow vtable, kind switch
or direct-target hypothesis replaces this indirect call. Clear record+4 only
after return. Then read current+8, conditionally use the matching current CRT
free service and zero+8; repeat with the fresh+0C word. Do not capture the second
string before the first free or zero fields before their calls return.

Read current+58 before disarming. Only a nonnull captured block reads current+54
and captures length+1 modulo32, before resolving the actual current pool and
returning the captured block. No normal header reset, helper cache, retry,
record free or second bag release is added.

The saved-this cell remains live for exceptional cleanup. The sole action reads
its CURRENT word, adds54 modulo32 and calls the real raw0041DD20 header
destructor. HandlerC61ABB selects FuncInfoD890F4: maxstate1 and mapD890EC
state0->-1/actionC61AB0. Normal state0 is armed after bag capture; header-data
capture precedes the native disarm at469367. Source C++ guard transport remains
qualified and does not reproduce Original FH3, SEH or fault identity.

The deleting wrapper receives ECX=record and one LIVE stacked flag word, RET4.
Capture record bits, call the raw destructor, then read only the flag BYTE and
test bit0. When set, use the actual matching host CRT free service; return the
captured record DWORD even after release. Do not replace the late read with a
copied flag value or treat the returned bits as a surviving object. At wrapper
entry ESP=P, actual flags4=P+4 and child's saved-this4=P-18h are distinct native
regions. Binding metadata and pool context stay stable/disjoint from payload;
payload aliases with those regions retain their reached schedules. Implicit
compiler/native argument, register, spill, FS and FH3-state aliases are unproved.

The complete existing singleton_lifetime_free provider calls std::free; both
BF6989 and BF65AC are genuine JMP thunks, ultimately to OriginalBF9DC8. This
packet preserves those different original transfer sites while reusing the
published host CRT service. It does not reconstruct Original heap/TLS/handler
or allocation-failure transport. Native callable-slot ABI, valid reached raw
storage and matching host heap release contracts must hold. All dynamic target
bodies, constructed Source-vtable dispatch compatibility, Original ABI/failure
transport and game execution remain unproved. GENERAL SOURCE-READY IS FALSE.

Seven direct call/jump claims are checked separately from the one unresolved
indirect CALL EDX at469337; no resolved-callee claim is made for that slot.
Whole live/installed bytes and typed flow are captured in the packet report.
Independent full193B/67-op Source/header review passed. Registered MSVC Release Win32 build atefe9cc1079ab3aa1b854040ae00df88f7c5c53c4 passed all three existing CTests in 7.51s. The unique archived COFF object confirms actual ECX/stack1 dispatch and late BYTE/current-field schedules. Compiler child inlining and private guard/EH writes have no Original frame identity credit. Receipt: reports/goal_oct15_scene_lifetime_primary_review.json. The existing Ghidra CALL_RETURN override on action JMP C61AB6 is preserved; no listing/flow repair was made. No new tests are introduced.
