# Checked-tree out-of-range exception ownership

The checked tree's `00B2EF00` invalid-iterator path throws a native 28h
`out_of_range` payload. ThrowInfo `00D863A8` selects cleanup `004412B0`;
the 28h catchable type at `00D863F4` selects copy `00441760`.
These two complete functions total 75 native bytes. Their new C++ interfaces
borrow `NativeLegacyExceptionStorage` and reuse the existing complete logic-error
copy and cleanup bodies. Descriptive names describe the observed tree use;
the cleanup body may be shared by other exception paths.

`00441760..00441779` takes the destination in ECX and a source-owner pointer
on the stack, returns the original destination in EAX, and ends with RET4.
It first calls the complete `004118D0` logic-error copy. Only after that call
returns does it publish native profile `00D6926C`. It adds no local EH state:
copy failure leaves cleanup to the existing copy implementation, and the
out-of-range profile is not published.

`004412B0..004412E2` takes the owner in ECX. It publishes logic-error profile
`00D69248`, tests the current member capacity at `+24` as unsigned against 16,
and frees current data `+10` when the capacity selects heap storage. The
three-byte ADD ESP,4 continuation at `004412C8` must execute after free; the
old Ghidra listing omitted it. Cleanup then writes capacity 15 at `+24`,
length zero at `+20`, and the first inline byte zero at `+10`, before tail
calling base destruction `00BF6454`. That base tests current ownership,
publishes the base profile, and frees the current owned base message. The
owner allocation and stale base message/ownership words remain untouched.

The tree constructs its original payload using the existing `00411700`
logic-error constructor followed by the out-of-range profile store. The
completed temporary string is protected only after its counted assignment
returns. A future owning host exception transport can compose that constructor
with these copy/cleanup entries; the native throw metadata does not make a
host C++ exception ABI-compatible.

Both native bodies and the concrete exception metadata are checked against
the saved Ghidra program and installed executable. Validation and annotation
status are recorded in `reports/native_tree_out_of_range_exception_audit.json`.
No new tracked tests or game/exception-ABI compatibility claim is introduced.
