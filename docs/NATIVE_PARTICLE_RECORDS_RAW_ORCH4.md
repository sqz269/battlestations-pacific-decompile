# Raw particle record append and clear

Addresses: 00b00ee0 00b00f30

## Reconstructed behavior

| Address | Coverage | Original ABI | Raw source interface |
| --- | --- | --- | --- |
| B00EE0 | Complete, 73 bytes through B00F28 | ECX descriptor, stacked record, RET4 | `append_native_particle_type_record_00b00ee0(descriptor, record)` |
| B00F30 | Complete, 50 bytes through B00F61 | ECX descriptor, RET | `clear_native_particle_type_records_00b00f30(descriptor)` |

The new overloads call the existing fixed-CRT B00C20 reserve implementation directly.
The existing host interfaces share the same append/clear kernels and remain available.
The descriptor is the actual 0Ch header: pointer at +0, count at +4, signed capacity at +8.
Its 1Ch records come from the existing B00880 producer and B01150 constructor.

Append tests count against the captured capacity. Equality doubles capacity with wrapping
32-bit arithmetic, then clamps the signed result to at least one. After reserve it reads
the current count and data pointer, skips the copy only when the computed destination is
null, copies seven DWORDs forward with REP MOVSD, and increments the current count.
The forward copy deliberately retains overlapping-source behavior. It does not make an
aliased source safe across reserve/free. The direct call at B00EF9 targets B00C20.

Clear calls B00C20(descriptor, 0) at B00F3B only for negative capacity. It decrements a
positive current count to zero and stores zero for nonpositive counts too. Storage and
capacity remain retained, including the allocation of minimum capacity one in the negative
capacity case. Neither body has an FH3 frame or an independent cleanup obligation.

## Evidence and validation

The complete 123 bytes match the installed PE and live Ghidra program
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`. Every direct call is recorded in
`reports/native_particle_records_raw_orch4.json`; prior annotations and ledger rows are retained.

Strict MSVC Win32 build and all three existing CTests passed. An ignored, manifested native
probe compares both complete original bodies against the raw overloads in eight cases:
append without growth, growth, zero capacity, overlapping forward copy, null destination,
positive and negative clear counts, and negative capacity. Its original reserve body uses
the same genuine fixed-CRT allocation/free providers. Initialized record bytes and descriptor
count/capacity/storage retention agree. No permanent tests were added.

These are new source interfaces, not drop-in binary replacements. Native invalid-pointer,
OOM/CRT exception identity, concurrency and gameplay behavior remain unvalidated.

## Follow-up

Compose B01350 through the raw record helpers, actual texture-name/atlas helpers, actual
renderer B319B0 texture-cache domain, and its canonical texture-owner release operation.
Nested particle parsers and the whole particle resource parser remain separate work.
