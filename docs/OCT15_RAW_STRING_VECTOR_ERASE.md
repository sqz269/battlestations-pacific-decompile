# Actual pooled-string vector erase and clear

Addresses: `004954F0`, `00498FF0`.

Both bodies now compose accepted actual-header services through the concrete
`NativeStringRawPoolContext`. The old `erase_checked_native_string_storage`
fragment and its original provenance remain separate. No iterator class,
parallel container, configurable allocator, or report/copy callback is added.

| Native body | Source | Coverage | Original ABI |
| --- | --- | --- | --- |
| `004954F0..0049556D` | `erase_native_string_header_vector_range_004954f0` | complete, qualified providers/private frames | ECX header, EDX unused, five stacked pointers, RET14h, EAX output |
| `00498FF0..00499028` | `clear_native_string_header_vector_00498ff0` | complete, qualified providers/private result | ECX header, no logical EDX/stack input, plain RET, no semantic result |

Source/header: `src/native_string_vector_erase.cpp` and
`include/bsp/native_string_vector_erase.hpp`, registered for `bsp_core`.
The established erase library name is preserved. The proposed clear name
`STL_CheckedNativeStringVector_Clear_00498ff0` is a descriptive hypothesis;
its previous value `STL_inst_00498ff0` is recorded before primary annotation.

## Actual layout and borrowed services

The receiver is an existing 10h vector: opaque+0, begin+4, end+8, capacity+0Ch.
Elements use the canonical eight-byte native string header (length+0/data+4).
Producer `00432090`, constructor `0041E870`, the existing NativeString
declaration and preceding packet establish those layouts. No new overlay is used.

The accepted complete `0048DA50` Source provides suffix assignment through
actual raw-pool resize and the accepted current-CRT overlap-copy service.
The complete `00432050` Source destroys ascending actual string headers through
current `00419CC0`/`00BD1510` bindings. It leaves headers untouched and does not
free vector backing. `invoke_native_invalid_parameter_00bf6713` supplies the
existing current-SDK policy; it has no noreturn promise. These are direct real
services, with their documented pool/manager/CRT and zero-copy qualifications.

## Erase validation and physical inputs

Native ECX is the receiver, EDX is unused. The five incoming stack words are:

| Entry ESP | Input |
| --- | --- |
| +04h | output pair address |
| +08h | first owner |
| +0Ch | first element |
| +10h | last owner |
| +14h | last element |

The first owner is captured before validation. A null first owner or unequal
owner values invokes the returning report boundary. There is no check that
receiver equals either owner, no owner dereference, and no added range,
capacity, alignment, or header-null validation. A returning report continues.
The first/last pointer arguments are then loaded in the native body.

An empty first==last range still validates and writes the result pair. It
performs no dedicated header-end read/publication, assignment, or destruction.
The result stores can themselves affect a receiver field when output aliases it.

For a nonempty range the exact order is:

1. Capture receiver+8 as old end.
2. Assign the captured suffix [last,old_end) onto captured first, ignoring the
   assignment helper's EAX.
3. Read CURRENT receiver+8 after assignment as destruction end.
4. Derive new end from captured first/old_end/last:
   `first + SAR32(old_end-last,3)*8`, with DWORD wrap.
5. Destroy [new_end,destruction_end) using the concrete actual-pool child.
6. After normal destruction return, publish the captured new end at receiver+8.
7. Write output owner at +0, then output first at +4; return the output address.

These capture/reload points permit actual header changes during the accepted
pool operations. New end survives destruction even if a reached provider
changes the header again. No count is recomputed from a later header state.

Output may alias vector headers or string/data storage. Both output stores
follow end publication, so they may overwrite the just-published end or other
fields. The output base and first pointer are retained through the first store;
there is no subsequent header read or output-pointer reload from that memory.

The range child receives six physical words: last, captured old end, first,
unused output pointer, receiver bits with the low byte cleared, and last-owner
bits with the low byte cleared. These final three are ignored by the accepted
core. Six words are confirmed by ADD ESP,18h at `00495543`.
The destruction child receives ECX=new end, EDX=post-copy current end and two
unused stacked header/output words; its RET8 removes those words.

## Clear composition and continuation

Clear captures header+8 before its first check. It compares current begin
against that captured end and reports/continues on unsigned greater-than.
It then captures current begin and compares it against a fresh current end,
again reporting/continuing. It delegates to exact erase with both owners equal
to header and with those captured endpoints. It does not normalize malformed
storage, retry the checks or replace the captured end with the latest one.

Its local two-DWORD result is the native private output shape, with no iterator
object or ownership. Five pushed words and the child RET14h establish the
interface. Incoming EDX is not consumed, including after a report; pseudocode
extraout_EDX is not another logical input. Source supplies zero for the child's
ignored EDX while explicitly borrowing the actual context.

Normal clear retains begin/capacity/backing. A returning handler can change
header fields, so the exact composition is retained instead of unconditionally
assigning end=begin. Empty clear has no dedicated end store beyond handler
effects and its unexposed private output. Pool/resize/destruction exceptions
escape before subsequent operations; neither body has a local native EH frame
or rollback. Source adds no catch, noexcept, noreturn or exception conversion.

## Caller and call-site evidence

All 31 known direct erase sites across 19 functions were inspected. Thirty
external setups were read through bounded exact instruction-context receipts;
the owned clear's complete listing supplies the remaining setup. They include
empty clears and the size-shrink suffix setup. All four clear sites across its
two callers were inspected, including the three global vectors in `009105F0`.

The report records all owners and sites; native containing-function queries
confirmed the six owned child sites. Exact direct-call receipts follow:

```text
00495505 CALL 00BF6713
00495535 CALL 0048DA50
00495551 CALL 00432050
00499000 CALL 00BF6713
0049900D CALL 00BF6713
0049901D CALL 004954F0
006A7D32 CALL 004954F0
0049E103 CALL 004954F0
006B9501 CALL 004954F0
007FDD3F CALL 004954F0
0053294E CALL 004954F0
0050E995 CALL 004954F0
0050A867 CALL 004954F0
006279E2 CALL 004954F0
00627A28 CALL 004954F0
00627A71 CALL 004954F0
00627BAA CALL 004954F0
00627BF0 CALL 004954F0
00627C37 CALL 004954F0
00B6CFF0 CALL 004954F0
0049FD1E CALL 004954F0
0049FD50 CALL 004954F0
0049FD95 CALL 004954F0
006269E8 CALL 004954F0
00626A31 CALL 004954F0
00626A77 CALL 004954F0
00626BB0 CALL 004954F0
00626BF9 CALL 004954F0
00626C3F CALL 004954F0
005C58B3 CALL 004954F0
00629E01 CALL 004954F0
008EC817 CALL 004954F0
007FE1DC CALL 004954F0
00A9E58C CALL 004954F0
00AA06B1 CALL 004954F0
00B6D0F8 CALL 004954F0
00506CA5 CALL 00498FF0
00910649 CALL 00498FF0
00910653 CALL 00498FF0
0091065D CALL 00498FF0
```

## Source interface and limits

Source erase retains header/unused-EDX/five pointer inputs and adds the actual
context. Source clear receives that extra context in EDX. The native parameter
slots are reread at several points, including first-owner after destruction and
output after child operations. Source by-value arguments do not expose mutation
of native private argument slots or aliases into compiler frames. Actual
external header/data/output aliases and ordered stores are preserved; original
stack/register/FH3/SEH and fault-address identity are not claimed.

Every live query uses the `bsp.py` verified client for project `bsp`, configured
as `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, language and
image base. Ghidra was read only. The earlier qualified fragment has not been
overwritten or upgraded to these new function claims.

MSVC Win32 Release and all three existing CTest suites passed. The JSON report
and document each pass all40 exact call-site checks with zero failures or
unchecked claims. Whitespace results are recorded in the JSON report.
No broad tests or new native
differential probe are added; the existing math differential suite covers its
existing seeds. Original CRT/manager policy, zero-copy omission, valid taken
raw backing and private-result limits remain explicit.

This closes the actual erase/clear dependency for future `00506C80` composition.
Vector assignment's remaining Buy/throw service is still separate and incomplete;
this packet adds no vector-assignment or production-cache/gameplay claim.
