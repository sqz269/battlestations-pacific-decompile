# Actual string-vector group tidies

This packet reconstructs complete `006A6CE0..006A6D1E`,
`006A75F0..006A762C` and `006A7960..006A799C` in
`src/native_string_vector_group_tidies.cpp`, declared in
`include/bsp/native_string_vector_group_tidies.hpp`. The first consumes an
actual16B outer header owning actual16B inner checked-vector headers; the
other two consume an actual16B group-vector header owning those outer
headers. The inner checked vectors own8B pooled strings. These are complete
actual-header compositions using accepted raw-pool/CRT services. Existing
host views, settings projections and their fixtures remain unchanged.
Source names are hypotheses, and prior ledger names/evidence are preserved.

The saved project is `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. Complete pseudocode, native listings, live
prototypes, bytes and incoming xrefs were checked through `bsp.py ghidra`.
The complete entry envelopes are63/61/61 bytes and19/21/21 instructions.
None creates a local EH frame or catch.

| Entry | Original physical interface |
|---|---|
| `006A6CE0` | Incoming ECX and EDX unused; stacked header, RET4; no semantic result. Source retains both unused words and adds raw-pool context after the stacked header. |
| `006A75F0` | ECX header, EDX unused; no stacked input, plain RET; no semantic result. Source adds raw-pool context in EDX. |
| `006A7960` | Same physical interface as `006A75F0`; Source adds context in EDX. |

`006A6CE0` saves ESI and obtains the header from `[ESP+8]`. It captures begin
at +4; only nonnull begin reads end at +8, destroys that captured range of
16B inner headers through accepted `0055A3A0`, and frees CURRENT begin.
Both stacked metadata words passed to that range are the header, and the
range consumes neither. It then always clears +4, +8 and +C in that order,
including null-begin headers. +0 is opaque and is not written. There is no
null-header guard. A complete comparison with accepted `0055AFF0` shows the
same field accesses, captured range, metadata words, child services, fresh
begin load and ordered stores. Only the header input/prologue/epilogue
mechanics differ, so the new Source interface composes that full helper.

`006A75F0` saves its incoming ECX header with PUSH ECX, then saves ESI. It
captures begin; only nonnull begin reads end, pushes the saved header and
current header as dead metadata, and destroys the captured16B outer-header
range through accepted `006A6EE0`. It reloads CURRENT begin for free, then
always clears +4/+8/+C in order without writing +0. Null begin skips the
end read, range and free, but still performs all three stores. Source uses
DWORD field accesses to retain the captured/reloaded read schedule, with
both dead metadata values equal to the original header. It adds no
ownership, null-header, alignment, capacity or range validation.

`006A7960` is fully equivalent to `006A75F0`: all21 instructions match after
address normalization. Of61 bytes, the53 outside the two rel32 operands are
identical, and both decoded target pairs are `006A6EE0` and `00BF65AC`.
The comparison masks only byte offsets21..24 and30..33, not opcodes, branch
displacements, field offsets or immediate stores. Source therefore composes
the complete `006A75F0` implementation once.

All range and pooled-string destruction uses the accepted actual services,
without new callbacks, host container models, allocator substitutes or
synthetic cleanup. Current Source C++ failures propagate immediately: a
range failure prevents the later backing free and all clears, and a failure
before any later operation skips that operation. Earlier effects remain;
there is no catch, retry, rollback or manufactured rethrow. Current backing
frees use `singleton_lifetime_free` and its existing current `std::free`
domain. No claim is made about original native heap/CRT/handler identities.

Incoming evidence is limited to the sites below. Two native catches pass a
completed outer header on the stack to `006A6CE0`, with an ignored owner word
in ECX. Two unwind actions adjust an owner pointer by +30h and tail-jump to
`006A7960`. Their complete caller bodies, private frames, EH state maps and
catch/unwind binding are not reconstructed by this packet. The live xref
query returned no incoming reference for `006A75F0`; this does not prove that
the entry is unreachable at runtime.

The exact direct transfer sites used for checks are:

```text
006A6CF5 CALL 0055A3A0
006A6CFE CALL 00BF65AC
006A7604 CALL 006A6EE0
006A760D CALL 00BF65AC
006A7974 CALL 006A6EE0
006A797D CALL 00BF65AC
0055B004 CALL 0055A3A0
0055B00D CALL 00BF65AC
006A6DC3 CALL 006A6CE0
006A700B CALL 006A6CE0
00C7F99F JMP 006A7960
00C7FA7F JMP 006A7960
```

The Source interfaces preserve the useful physical inputs but add context;
they are not binary entry replacements. Native EAX residue, saved-register
and private-stack identity, FH3/SEH/fault/exception identity and original
unwind-frame personality are not reproduced by these signatures. Reached
storage and the accepted raw-pool/current CRT lifetimes are caller
preconditions. Existing native fixtures for prior interfaces do not prove
these new interfaces; no new native differential, failure-injection or
gameplay validation is claimed.

The MSVC Win32 Release build passed. All three existing suites passed in
8.56 seconds; the existing duplicate `spawn_request_id_matches` LNK4006
warning is unchanged. The transfer checker verified12 report rows and12
Markdown claims with no failures or unchecked claims. Detailed results are
in `reports/oct15_string_vector_group_tidies.json`. No new tests are added.
