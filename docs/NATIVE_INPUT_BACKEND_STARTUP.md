# Raw input backend reset and poll schedule

Addresses: `00A900F0`, `004B4630`, `00A90490`, `00A918A0`.

`native_input_backend_startup.hpp/.cpp` implements these operations on the
existing canonical F8h backend. It adds no input owner, table projection, lifetime
domain or SDK object. The older typed `input_focus_reset` functions remain.
These are new C++ interfaces, not original callable vtable, FH3 or SEH ABIs.

| Routine | Coverage | Original ABI and body |
| --- | --- | --- |
| A900F0 reset | Complete over valid raw storage and supplied device calls | ECX backend; no stack arguments or semantic result; RET at A90123 (1 byte); body A900F0..A90123 |
| 4B4630 startup callback | Complete | ECX unsigned class, EDX signed index supplied by its consumers; no stack arguments or semantic result; C3 at 4B4630 |
| A90490 requested-count test | Complete | ECX backend; DWORD class on stack; AL bool only; RET 4 at A904D1 (3 bytes); body A90490..A904D3 |
| A918A0 poll schedule | Complete over valid raw storage and supplied calls | ECX backend; float seconds on stack; no semantic result; RET 4 at A91921 (3 bytes); body A918A0..A91923 |

Ghidra has all four starts. A900F0's listing omits the three alignment bytes
`8D 49 00` at A900FD..A900FF. The preceding unconditional jump skips this LEA;
there is no missing execution path or requested analysis repair. The other
three bodies have no instruction gaps. No worker changed Ghidra metadata.

The existing raw A982D0/A91570 producers establish profile D5B72C/D5B5F8,
24 owning device slots at +4, three 24h groups at +68, the policy byte +64,
and callback DWORD +D8. Each group's active-vector header starts at +6C,
with begin/end at header+4/+8. The leading allocator words are not replaced.
This packet neither initializes nor copies these fields.

A900F0 walks three classes of eight slots in class-major order. It tests the
slot for null, reloads the slot, then invokes that device's current profile
slot14. Later slots are read only when reached. It performs no reference-count
operation or cleanup of its own. Both direct callers reload F8BBF4: startup
at 73DDA2/73DDAF and session teardown at 4DAA89/4DAA8F.

A90490 compares the requested DWORD at +68+class*24h with the active count.
A null begin means zero without reading end. Otherwise the DWORD end-minus-begin
wraps and undergoes signed SAR 2. The comparison is equality. There is no added
class bound, span validation or saturation rule; original callers supply 0..2.

A918A0 first captures the backend's current profile for slot10. D5B72C selects
the already reconstructed A97390 bare RET; D5B5F8 selects the real CRT purecall
contract. It then captures each nonnull device, loads/stores the original float
seconds through x87 for that poll, and ignores the poll result. After the call
it rereads the requested and active counts. If they differ, it rereads byte+64.
Only a false byte and class 2 require the activity query. That query uses the
captured polled device even when its former slot changed. Activation receives
the backend, class and fixed-slot index; the separate A91620 operation reloads
the current slot. Thus activity and activation can correctly see different
devices. No exception cleanup is added to these schedules.

| Required call | Native site | Source binding |
| --- | --- | --- |
| Device current-profile slot14 reset | A90111 in A900F0 | Concrete `NativeInputBackendStartupDeviceRuntime` forwards to existing `NativeInputDeviceRuntime` |
| Backend current-profile slot10 prepass | A918AD in A918A0 | D5B72C -> A97390; D5B5F8 -> CRT purecall |
| Device current-profile slot10 poll, one float stack word | A918D9 in A918A0 | Same concrete raw runtime adapter; result ignored |
| Captured device current-profile slot28 activity | A918F8 in A918A0 | Same adapter, freshly captured profile; AL bool |
| A91620 activation, two DWORD stack words, RET 8 | A91902 in A918A0 | Required `NativeInputBackendSlotActivation`; separate raw binding packet |

The adapter borrows the existing SDK-backed finite device runtime. It does not
provide substitute hardware operations. Unsupported backend profiles or callback
identities are explicit source binding errors. The accepted captured outer
slot04 profiles D5B5F8 and D5B72C both select A918A0; that operation separately
reloads the live profile for its prepass. The outer call sites are A92C57 and
the three BECB20 calls BECBD8, BECC1F and BECC9B. Their callers own the publication
and delta-time reloads, including live D7A2F0 in the cursor path.

4B4630 is a real one-byte RET, not an unresolved-call substitute. Startup stores
that exact identity at backend+D8. A91620 loads/tests the callback after active
vector mutation and passes original class in ECX plus the old active count,
captured before append allocation, in EDX at A917CA. A90EE0 likewise loads/tests
the current callback after mutation, passing captured class and index -1 at
A90F86. The dispatcher consumes the captured nonzero identity; it does not
reload +D8 or call a numeric address in the reconstructed process.

`create_and_reset_native_input_backend` is source composition of **only
73DD6C..73DDB3**, not a reconstruction of all 73D410. It calls existing
`create_native_input_backend`, reloads the actual publication for the +D8 write,
then reloads it for reset. Its returned allocation identity is for source
bookkeeping and does not replace either reload. Existing constructor failure
cleanup remains in the allocation wrapper. Once construction completes, a reset
exception leaves that registered allocation live, as native startup does.
Application callers must retain bindings and services through shared raw drain.

Verification on MSVC Win32 Release: `scripts/build.ps1` passed with both existing
CTests; all eight native seed ranges matched before native execution. One ignored
archive-only probe executes copied original A900F0/A90490/A918A0 bytes with
controlled callbacks. The native poll's count CALL targets copied original
A90490; its activation CALL targets a controlled provider. Reset called twice;
poll called six times; activity once; the activation provider three times.
Callback-driven later-slot replacement, live count/policy changes, retained-device
activity and current-slot activation produced equal traces and canonical output.
The probe also checked null-begin and wrapped-SAR counts and exact callback
identity acceptance. Installed executable bytes were unchanged.

Logs, hashes, original fixture ranges and CALL evidence are recorded in
`reports/native_input_backend_startup.json`. The probe made zero SDK calls,
real activation calls or game launches. The factory/publication sequence is
source- and build-checked, not exercised by that probe. Real activation binding,
application construction/frame wiring and hardware behavior remain separate
integration work. Asynchronous mutation and hardware-fault compatibility are
outside the new source ABI.
