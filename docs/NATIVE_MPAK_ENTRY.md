# Actual MPAK entry materialization and raw inflater construction

Addresses: 00bb5080, 00bbc1d0

The source implements both complete native bodies over the existing raw owner,
provider, stream and allocator storage. Names are descriptive hypotheses, not
recovered symbols. These are new C++ interfaces; original Win32 ABI/FH3 identity
and gameplay are separate evidence categories. Stock zlib 1.2.1, CRT allocation,
the returning invalid-parameter handler and stream dispatch remain explicit
library/runtime dependencies. No game owner or stream is replaced by a host
container or `shared_ptr`.

| Routine | Native ABI | Bytes | Coverage |
| --- | --- | ---: | --- |
| BB5080 | ECX provider; stack signed index; EAX stream; RET4 | 415 | complete |
| BBC1D0 | ECX raw34h; stack source, descriptor, ignored capacities; EAX owner; RET10h | 334 | complete |

Both contiguous bodies match the installed PE bytes and have no Ghidra listing
gaps. The saved void prototypes and constructor pseudocode's dropped capacity
arguments do not describe the machine interface. Assembly and EH maps settle
the register inputs, stack cleanup and cleanup lifetime.

## Entry producer and caller contracts

The provider/parser producer is already documented and reconstructed in
`NATIVE_MPAK_PROVIDER.md`: provider44h has stream+14, the file-vector allocator
at+1C and begin/end/capacity at+20/+24/+28. File records are24h: pooled name+0/+4,
numeric words+8/+C, flag byte+10, untouched allocator+14, offset-vector
begin/end/capacity+18/+1C/+20. The directory parser writes the second numeric
input to record+8 and the first to record+C. This consumer establishes their
decoded/compressed-length use; it does not invent the record layout.

The only direct BB5080 caller is BB5BB0 at BB5CB3. After its name/device work,
it reuses a matching cached index only when signed provider+40 is greater than
zero, otherwise calls BB4A60. It passes the resulting signed index unchanged
and returns the materialized stream. Negative values therefore reach BB5080's
null result before any provider read. BB5080 itself accepts index zero normally.

For nonnegative indices, BB5080 checks the signed byte difference divided by
24h, compared as unsigned against the index. Its invalid-parameter callback can
return: the actual begin pointer is reloaded afterward. It captures the actual
record and source, writes provider+40, then calls the captured source's current
slot20. Only EAX is retained from the position result; EDX is ignored.

Offset iteration preserves the native validation order and reloads. It captures
the first offset before testing iterator-versus-end, then scans in stored order
for the first unsigned offset greater than or equal to the low position word.
If none matches it uses the captured first offset. The loop captures each end
before a returning validation call; the equality comparison uses that captured
end, while dereference/increment checks reload the current end. BB5141 compares
ESI with itself, making the BB5145 handler call unreachable. No sorting, bounds
repair, nearest-offset calculation or high-position handling is added.

Flag zero calls the existing actual BEF840 with selected offset, high0,
record+8 length, high0. Nonzero flag captures the12h descriptor in order
`{selected offset, record+C compressed length, record+8 decoded length}`,
allocates raw34h and calls the actual constructor below. It then calls the
existing actual BEF750 and decrements the temporary inflater's real+4 count.
Only zero dispatches the inflater's current slot0; the converted result remains
captured across that call.

## Actual raw inflater constructor

BBC1D0 writes reference base/count1, source+0C, then profile D64400 and byte+9=1.
Bytes8, A and B remain untouched. It reads and stores descriptor words
sequentially into+10/+14/+18; an aliasing descriptor is not copied in advance.
Its native stacked capacity values are entirely unused. It allocates two raw
10h headers; each holds begin, buffered-end, allocation-end and cursor at
0/4/8/C. The input allocation is always4000h, the output always10000h. Each
initial cursor/buffered-end equals begin.

It then allocates raw38h z_stream, writes zero only to next_in, avail_in,
zalloc, zfree and opaque, and calls BC96A0 with ECX decoder, EDX=-15, stacked
version pointer D643F8 (`1.2.1`) and size38h. The native library callee uses
RET8. Source calls the fetched stock `inflateInit2_` with those same values;
zlib initializes its own remaining fields and its result is ignored.

The constructor captures the current source+0C reference address before
publishing decoder+28, then calls real InterlockedIncrement. It reloads source,
table, current offset+10 and current slot1C before seek(offset,0,0). It ignores
the seek result. After seek it reloads both sizes, writes remaining compressed
at+2C, position zero at+24 and remaining decoded at+30. Reentrant seek changes
to+14/+18 are therefore visible in the remaining counts.

The observed D64400 table has slot0 BD30E0, slot4 BBC3E0, slotC BB8B80,
slot18 BBBDC0, slot1C BBC060, slot20 BBBE50, slot24 BBC140, slot28 BBC1C0 and
slot30 BBBDD0. Numeric raw-inflater read/seek/refill/lifetime and BEF750 routing
are the integrator's adjacent packet. This packet supplies no successful
fallback for missing numeric dispatch.

## Exception evidence

| Owner / state | Map action | Source effect |
| --- | --- | --- |
| BB5080 state0 | DFDC38 FuncInfo, map DFDC30: CC4360; next -1 | Free captured raw34h only if constructor throws |
| BBC1D0 state0 | DFE904 FuncInfo, map DFE8EC: CC4AD0; next -1 | BB86E0 base cleanup: D5C104 then CEB130 |
| BBC1D0 state1 | map DFE8F4: CC4AD8; next0 | Free current raw10h input header, then base |
| BBC1D0 state2 | map DFE8FC: CC4AE3; next0 | Free current raw10h output header, then base |

BB5080 arms state0 only after allocation returns, and disarms at BB51D1 before
BEF750. Conversion or zero-reference failure has no inflater rollback. Null
allocation would flow through null conversion and then touch null+4; the source
adds no successful null-allocation recovery.

BBC1D0 arms state0 before its last descriptor store. States1/2 surround only
each inline array construction and revert to state0 before publishing that
header. Later failure never frees earlier buffers, z_stream or a retained
source. Only the current failed buffer header is freed, followed by base
cleanup. The original FH3 runtime and simultaneous cleanup exceptions are not
reimplemented.

At initial analysis the free-call overrides truncate CC4360 at CC4368,
CC4AD8 at CC4AE0 and CC4AE3 at CC4AEB. Their disk/live-byte continuations are
respectively CC4369..CC436A, CC4AE1..CC4AE2 and CC4AEC..CC4AED, each POP ECX;
RET. The integrator received those exact repair ranges. No worker mutation,
rename, function definition or project save was performed.

## Verification and remaining evidence

The machine call-site rows, byte hashes and current build/probe results are in
`reports/native_mpak_entry.json`. Strict MSVC Win32 Release compilation passed,
followed by both existing CTests after all eight seed-byte comparisons passed.
The report verifier checked25 direct call rows with zero failures; five
indirect rows carry explicit contracts and are outside that mechanical check.
The local probe passed218 checks across seven original/source entry scenarios
(including negative index), one original/source constructor scenario and one
source-only throwing-seek case. The local probe executes copied original
BB5080 only on its raw-range/negative-index paths and copied original BBC1D0
on its normal path, with explicit source allocation, zlib and stream service
bindings. Native library code is not counted as reconstructed game bytes.
The probe separately exercises a source-only throwing seek to check the
constructor's retained source and base-profile rollback boundary.

Compressed BB5080 conversion/release, original FH3 exceptions, allocation
failure, invalid pointer dereferences, arbitrary native stack-spill aliasing,
concurrent mutation and installed archive/gameplay behavior remain unvalidated
by this packet. No permanent test suite was added.
