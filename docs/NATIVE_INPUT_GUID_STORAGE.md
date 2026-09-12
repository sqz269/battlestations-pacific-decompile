# Raw input GUID storage host adapter

`append_input_guid_storage` supplies the source storage operation needed by
`NativeInputEnumerationCalls::append_guid_00a97fa0`. It borrows the actual raw
backend's GUID-vector header and owns no separate vector or lifetime domain.
This is a source host adapter, **not** a reconstructed original STL entry,
original ABI replacement, or new address/name-ledger claim. The recognized
library bodies remain evidence; their general insert/iterator implementations
are not ported. The existing provisional A97FA0 name is unchanged.

## Storage and application boundary

The F8h backend's header starts at +E4. Its leading DWORD stays untouched;
begin/end/capacity-end occupy header+4/+8/+C (backend E8/EC/F0). The existing
A982D0 constructor zeros those three words. The existing
`destroy_native_input_backend_guids_00a97b00` frees current begin and zeros them.
The new adapter leaves that destructor as the buffer's sole teardown operation.

The only identified native caller is A98030. It sets ESI=backend+E4 at A981B0,
computes `DIDEVICEINSTANCEA::guidInstance` as instance+4 at A98218, and calls
A97FA0 at A9821E. Its complete live body is A98030..A982AC. The source enumeration
provider can delegate its existing hook to this new function; this packet does
not edit or bind that provider, enumerate SDK devices, or complete application
backend startup.

## Declared domain and behavior

The source interface requires MSVC Win32, a readable actual header, and complete
16-byte GUID elements. Pointer ranges must be valid, have representable signed
32-bit byte differences, and belong to the existing source allocation domain.
Empty storage has three null pointer words; allocated storage satisfies
begin<=end<=capacity-end at whole-element boundaries. The source GUID may be
external or an existing complete element. Partial byte overlaps, overlapping
header storage, arbitrary malformed pointers, concurrent changes, and mutation
of the header from an allocation/new-handler callback are outside this domain.

With spare capacity, copy one complete GUID to captured end and then publish
captured_end+16. Otherwise capture the GUID before allocation. Capacity grows
by 1.5 times, with a minimum of count+1; an overflowing geometric candidate is
discarded before choosing that minimum. The native element maximum is
0x0FFFFFFF. Arithmetic retains DWORD subtraction/SAR4 and unsigned comparisons.
Copy existing elements and the captured value with standard memory operations.
After successful copying, free current old begin, then publish capacity-end,
end, and begin in that order. Preserve the leading DWORD and existing elements.

The source borrows the observed slow-path validation schedule: check captured
begin/end before insertion, capture the insertion index, and check current
begin/end and the resulting element position after publication. The native
iterator-owner identity check is tautological for this same-header append API;
no iterator object or general middle-insertion implementation is introduced.
The real CRT invalid-parameter handler is allowed to return. Later reads remain
current where a callback can intervene; there is no invented success result or
unconditional throw. A bounded returning-handler repair is fixture evidence
only, not general malformed-header recovery support.

## Reused services and exception limits

Allocation uses `singleton_lifetime_allocate` with equal native/host byte sizes
and the existing malloc/_callnewh retry policy. Free uses
`singleton_lifetime_free`. This is the source counterpart of A974D0 -> BF681B
and BF65AC. No allocation or deallocation comes from a new vector allocator,
`delete[]`, or private manager. A failed allocation leaves the old storage and
header intact. The following standard memory copies and free cannot throw C++
exceptions; hardware-fault/SEH equivalence is not claimed.

The length-error service is the existing
`native_singleton_length_error_00bd0590`. Native A97B90 and BD0590 have matching
instruction schedules except addresses/EH handler: the same CE37E0 message/count18,
408720 -> 411700 calls, D69260 profile, and D83F98 throw descriptor. Its source
transport is `NativeSingletonVectorLengthError`, not `std::length_error`.
The allocation multiplication check throws the existing source `std::bad_alloc`
type. No original static-CRT heap, new-handler, RTTI, or FH3 identity is implied.

BF6713 -> BF66EF calls the original decoded global invalid handler with five
zero arguments and can return. The adapter uses actual
`_invalid_parameter_noinfo()`, as other raw singleton source services do.
UCRT's thread-local-then-global handler selection differs from original global
109DD64; source Watson, handler exceptions/registers, and fault sites remain
explicit library boundaries.

For native growth, A97CB0's state0 catch at A97E20 frees the new allocation and
rethrows via BF6885(null,null). Its FuncInfo is DECDEC and first catch descriptor
is DECDBC. The host adapter does not introduce a throwing GUID constructor,
general insertion callback, or transcribed FH3 machinery just to mimic this
unreachable C++ copy-exception case. Reentrant allocator mutation is excluded.

No existing concrete raw GUID append was found. The typed enumeration vector
has separate ownership; singleton raw vector operations use four-byte elements;
resource/VFS/action vectors have different layouts, growth, or deep ownership.
They cannot be cast or substituted for this header.

## Library evidence and verification

All native bodies below are evidence only; no original functions are claimed.

| Entry | Inclusive body | Original ABI / observed boundary |
|---|---|---|
| A97FA0 | A97FA0..A98022 | ECX header, stack GUID pointer, RET4 at A98020 |
| A97F10 | A97F10..A97F9A | ECX header, four stack words, RET10 at A97F98 |
| A97CB0 | A97CB0..A97EF2 | ECX header, owner/position/count/source on stack, RET10 |
| A974D0 | A974D0..A97520 | ECX element count, EAX allocation, RET0 or throw |
| A97790 | A97790..A977C7 | ECX first/EDX last, four stack words, EAX destination-end, RET10 |
| A97880 | A97880..A978B7 | ECX destination/EDX count, four stack words, RET10 |
| A97A20 | A97A20..A97A55 | ECX header, destination/count/source on stack, EAX end, RETC |
| A97B90 | A97B90..A97BF8 | no input; final throw CALL A97BF4 length5 |
| A97B00 | A97B00..A97B29 | existing actual-header destructor, ECX header/RET0 |

A97FA0 has53 listed instructions/zero gaps; A97F10 has57/zero gaps. A97CB0 has
two pre-existing live listing gaps after returning free calls:
`[A97DF4,A97DF7)` is ADD ESP,4; `[A97E29,A97E35)` is ADD ESP,4; PUSH0; PUSH0;
CALL BF6885 at A97E30 length5. Installed-disk bytes establish those instructions;
this packet does not repair Ghidra or claim the absent CALL as a live listing row.
The existing separate `Catch_All@A97E20` body is A97E20..A97E28 and owns the
A97E24 free CALL; the mechanical audit corrected its initial attribution to
the encompassing A97CB0 span. No analysis metadata was changed.
General middle-insert paths A97E35..A97EF2 are outside this append-only adapter.
The JSON report separates live CALL evidence from this gap evidence.

Validation results are recorded in `reports/native_input_guid_storage.json`.
Strict Win32 `scripts/build.ps1` passed with /W4 /WX /fp:strict; both existing
CTests passed, all8 native seeds matched, and all26 read-only CALL rows passed.
The initial Lua HTTPS download reset was resolved by verifying the existing
cached tarball against the configured SHA256 and copying only that archive into
this worker's build cache; no dependency or primary source was changed.

The ignored archive-only probe passed6 appends at capacities1,2,3,4,6,6,
5 buffer replacements,2 complete-element aliases, and preserved leading/header
and backend canaries. A real UCRT invalid handler received five zero arguments
once and returned after repairing a bounded header case; that append passed.
Both cases finished through actual A97B00 teardown, clearing the pointer triple;
HeapValidate passed. The runner preserves /MANIFEST:EMBED and records archive,
adapter-source, and log hashes in the report. It compiles no production source
separately from the built archive.

This does not prove an allocation-failure replay, allocator free counts, native
ABI compatibility, physical enumeration, or game behavior. No new permanent
tests are introduced.
