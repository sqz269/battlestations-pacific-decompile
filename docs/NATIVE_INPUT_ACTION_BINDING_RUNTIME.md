# Native input action binding runtime

Addresses: A922A0, A91E80, A92370, A91D60, A92090. Packet
`orch4_native_input_action_binding_runtime_ag`. Names remain hypotheses. Source:
`native_input_action_binding_runtime.hpp/.cpp`. Existing typed records are
preserved; the new ledger rows are additive raw fragments.

| Entry | Original inputs/result | Coverage, inclusive end |
|---|---|---|
| A922A0 | ECX actual24h owner; no stack input; RET, no consumed result | Complete, A922D2 |
| A91E80 | ECX actual30h action; no stack input; RET, no consumed result | Complete reachable flow, A9204B |
| A92370 | ECX actual30h action; no stack input; RET, no consumed result | Complete, A926E0 |
| A92090 | ECX actual30h action; no stack input; RET, AL0/1 | Complete, A920D4 |
| A91D60 | Two stack floats primary/paired; no ECX/EDX input; RET8, ST0 float | Complete, A91E1D |

End-exclusive addresses are respectively A922D3, A9204C, A926E1, A920D5,
A91E1E. No owned missing entry or excluded execution tail exists. A91E80's
listing omits A91F1A..A91F1F: verified bytes `8D 9B 00 00 00 00` are an
alignment LEA EBX,[EBX], bypassed by A91F18's jump to A91F20. No flow repair or
Ghidra mutation was performed by this worker.

## Producers and lifetime

Existing `native_input_action_owner`, `native_input_action_records`, and
`native_input_binding_storage` produce the actual allocations. Owner+4 is the
12-byte `{base,count,capacity}` action header, stride30h. Action+10 is a binding
header, stride34h. Binding+18 and+24 are modifier headers, stride14h; these
headers contain counts, not checked-STL end pointers. The backend instead has
actual10h active-pointer headers at+6C+class*24, with begin+4/end+8.

A93500 initializes binding resolved+0/curve+1, class+4=-1, index+8, cache+C,
code+10, scale-override byte+14, the two modifier headers and scale+30. A93750
writes source class/index/cache/code/flag and scale, then A937B7 calls A91E80.
A93940 produces the action's previous value+1C/previous byte+20/current
value+24/current byte+28, plus retained listener+2C. These same fields and
allocations are used here. No new arrays, owning typed records, singleton
domain, device references or listener lifetime are introduced.

## Rebinding and raw predicates

A922A0 calls A91E80 for every action, including disabled records, then reloads
owner count/base before comparing the advanced pointer. A91E80 captures the
primary class before clearing resolved. ClassFFFFFFFF skips lookup without
clearing the primary cache. Other classes use the currently published F8BBF4,
with unsigned index checks against the actual active-pointer vector. The
redundant native validation remains the real returning CRT call; a later slot
read reloads begin. No invented class bounds or null-backend check is inserted.

After storing a nonnull primary, required then forbidden modifier caches are
resolved sequentially. F8BBF4 is reloaded for every modifier. The first missing
pointer stops that traversal, preserving later caches. A fresh endpoint decides
whether traversal completed, and only completion of both arrays marks resolved.
All outer endpoints likewise reload from the actual headers.

A92090 scans cached primaries even on unresolved bindings, ignores modifier
state, and returns canonical AL1 on the first nonzero vslot1C result. Every
query captures receiver, current profile and code in native order. A92370 uses
raw byte predicates: required vslot20 accepts any nonzero; forbidden rejects
exactly1. A returned byte2 therefore has different consequences in these two
arrays. A fresh modifier endpoint is compared even after a rejected predicate.

## Poll order and arithmetic

A92370 shifts previous/current with the native FLD/FSTP value copy and byte
copy, then clears current state once. It runs A92090 before scanning all
binding+14 bytes for the action-wide unit-scale override; neither scan requires
resolved. For each resolved row whose modifiers finish, it calls vslot24 and
immediately spills ST0 to binary32.

Nonzero/unordered value, no action-relative query, class2 and code3C..3F select
the paired-axis branch. The native four-way chain is retained literally: an
unsuccessful query may change the binding code before the next comparison, and
the paired value call reloads the cached receiver/profile after that query.
Replacing this chain with a once-captured XOR1 code loses observable behavior.

A91D60 preserves absolute-value bit masking; FCOMIP/JBE choice on equality or
unordered; binary32 reciprocal/normalized-component/sum/root/product spills;
and the x87 sum-of-squares intermediate. It invokes the existing recovered
`native_crt_sqrt_st0_00bf7030` using a required borrowed CameraAxesCrtAccess.
The final cap is only above+1; negative/unordered results are retained. No CRT
implementation, square-root fallback or arithmetic sanitization is added.

Curve/global references are borrowed at their actual read points. Verified image
values: CE3800=float0.5, D7A208=float-0, D7A24C=float1; D7A348=double0.25,
D7A280=double0.5, CE42E0=double0.875, D04380=double0.125. The quarter/outer
curve, multiplication, signed/unordered accumulation and selected-value spills
follow the x87/SSE schedule. Current-down OR short-circuits its final vslot20
query and stores canonical0/1 even if a callback supplied a different byte.

No allocation or owning EH state exists in these five bodies. Provider failures
propagate with preceding writes retained. Current record/range backing must
remain addressable, including during callbacks. Source ABI, source callback
transport and CRT binding differ from native vtables/FH3/SEH; arbitrary faults,
asynchronous mutation and original stack-frame aliasing are not covered.

## Providers, callers and unresolved boundary

`NativeInputActionBindingCalls` requires captured-profile1C/20/24 methods on
actual device pointers. Primary's finite NativeInputDeviceRuntime supplies
them; its new captured-profile1C overload avoids selecting a replacement
profile. No SDK polling or force method is called by this packet. Existing
keyboard/mouse, XInput and joystick source bodies supply these cached queries;
common gamepad pure slots retain their established dispatch boundary.

All known callers were inspected. A922A0 is called at A92C71. A91E80 is called
at A922B8/A937B7. A92370 is called at A92C90, A92A29 and A92E03; the older
sole-caller description is obsolete. A91D60's caller is A92556. Assigned
A92090 callers are A92390, 51EAB9, 51EAD0, 64B8D6 and67C60C. The tick/listener
packet owns A92C40/A91A50 and must call these services on the current raw action.

One additional A92090 incoming site is explicitly unassigned:60BC03 has no
Ghidra containing function. Raw bytes `E8 88 64 48 00` prove the call; aligned
60BBF5..60BC08 shows getter4BEC00, ECX=[EAX+4]+1E90, call, TEST AL. The nearest
function609BD0 ends60ABCC and is not its owner. No wider function bounds are
claimed. This site is kept separately in the report, outside mechanical body
membership auditing, as agreed with primary. Every owned CALL is in the audit.

## Validation

Win32 Release and both existing CTests passed; all8 native seeds matched. The
report includes7 owned direct CALLs,10 indirect CALLs and every assigned
incoming call. Indirect site membership was checked live; original byte/float
slot signatures and captured inputs remain explicit.

One ignored manifested differential executes all five verified native spans
over actual records from the existing producers and compares source rebind/poll
results. Only7 direct calls and10 absolute operands are relocated; branch bytes
are unchanged. Both sides use the same recovered CRT sqrt and controlled raw
device callbacks. The focused case matches13 ordered device calls, output bits
3EE00000, previous byte2/current byte1, paired code/receiver mutation, forbidden
byte2 acceptance, endpoint shrink, preserved unresolved cache and untouched
later modifier cache. Snapshot comparison excludes unwritten padding.

Runner `local/run_native_action_binding_probe_ag.ps1 [-PrimaryWorktree path]`
links only that tree's headers and archives. This fixture covers finite values
under the default masked FP environment; signaling NaNs, exception flags and
all control modes were not exercised. No permanent tests, game, hardware,
window, SDK poll, cursor or force effects were run.
