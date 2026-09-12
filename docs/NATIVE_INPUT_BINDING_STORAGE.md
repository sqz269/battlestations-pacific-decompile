# Native input binding storage

Addresses: 0086A220, 0086A430, 00696D80, 00696E70, 00696DE0, 00697220, 00A92E70, 00A92EE0, 00A93100, 00A93220, 00A93500, 00A93440, 00A937E0.

`native_input_binding_storage` implements thirteen complete normal schedules over
the actual Win32 `{base,count,capacity}` headers. These engine arrays are not STL
layouts. Names are hypotheses. Three entries (A93100/A93220/A93500) already have
value projections in `input_binding_install`; those interfaces remain intact.
This additional storage implementation preserves native bytes and ownership.

All entries receive ECX storage and one DWORD stack argument, returning with
RET4. Assign and copy routines return the captured destination in EAX; reserve,
resize and append have no semantic result. The new explicit-service C++ APIs
are not binary replacements. Valid native ranges and the existing CRT allocation
boundary are required; arbitrary overflow, hardware faults and original FH3/SEH
compatibility are outside the supported source domain.

The two DWORD reserve/resize specializations retain distinct entrypoints.
Reserve clamps requested capacity to at least one, allocates, reloads current
base/count, copies the current elements, frees the current old allocation, then
publishes base/capacity without changing count. Resize initializes new DWORDs
to zero. Assignment clears destination count first, including self-assignment;
it captures each source element before possible growth and reloads source count.

Modifiers are 14h bytes: construction writes FFFFFFFF at+0, zeros DWORDs+4/+8/+C,
and clears only byte+10. Copy transfers all five DWORDs, preserving the final
word beyond that constructor byte. Bindings are 34h bytes with two modifier
headers at+18/+24. Construction clears bytes+0/+1, writes FFFFFFFF at+4, zeros
DWORDs+8/+C/+10, clears only byte+14, initializes both headers, and copies the
live D7A24C default scale to+30. A93500 reads that constant only on growth, once
before its loop (A9352F/A93531 guard A93533). Shrinking performs no such read.

Binding copy transfers the full DWORD+14, deep-copies each modifier array and
uses x87 FLD/FSTP for the scale. Untouched constructor padding remains untouched.
Reserve copies records forward, destroys old records forward, frees old storage,
then publishes replacement storage. Destruction releases the forbidden array+24
before required+18. Resize decrements live count before each reverse destruction.
Record loops reload live count/base at the same callback boundaries as native.

The A93100 state0 unwind (DEC9D0/DEC9C8, CB66D0) destroys the completed first
modifier header through6977D0 if the second copy fails. A93220's map atDEC9F4
also contains a first-header cleanup and a placement cleanup. The placement
target00401130 is an actual RET/no-op. A failed reserve-copy therefore does not
gain rollback or free its unpublished replacement/completed records. Source
C++ guards represent the supported unwind order; a second exception terminates.

Validation: every owned native body was read through its final RET and its live
bytes matched the installed PE. The mechanical report audit checked34 direct
CALL rows with zero failures. Win32 Release compilation and both existing CTests
passed. A focused nonempty owner fixture exercised34h bindings, both14h arrays,
padding/full-word preservation and independent copies. At that stage it linked
primary provider archives plus the worker owner source; final archive-only raw
manager drain evidence is recorded separately in `NATIVE_SINGLETON_INPUT_ONLINE`.
No permanent tests were added and no application input behavior was validated.

Follow-up: connect the same raw arrays to action installation and frame consumers;
do not substitute a second owning vector or assume a value projection has this
layout. See `NATIVE_INPUT_ACTION_RECORDS` for the owning30h records.
