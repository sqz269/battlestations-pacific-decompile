# Actual outer-vector assignment

This packet reconstructs complete owning assignment `006A6680..006A67C5`,
forward range wrapper `006A6C60..006A6C98`, and fill wrapper
`006A6CA0..006A6CC5` in `src/native_outer_vector_assignment.cpp`, declared
in `include/bsp/native_outer_vector_assignment.hpp`. An actual16B outer
header owns actual16B inner checked-vector headers, whose backings contain
8B pooled strings. Existing settings projections, host views and fixtures
are retained separately. Source names remain hypotheses; existing library
names and historical ledger evidence are preserved.

The saved project is `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. Complete native listings/pseudocode and live
typed metadata, bytes, prototypes and caller contexts were inspected through
repository tools. Full disk/live byte equality and complete instruction-start
equality hold for326/57/38 bytes and146/27/18 starts. Each typed body is one
complete range; none is a typed thunk or no-return function. The full native
bodies create no local EH frame, catch, unwind map or rollback scope.

| Entry | Original physical interface |
|---|---|
| `006A6680` | ECX destination, EDX unconsumed; stacked source, RET4/EAX destination. Source retains these words and adds actual raw-pool context. |
| `006A6C60` | Cdecl(first,last,destination), plain RET/EAX precomputed destination+(SAR4(last-first)<<4). Source adds context. |
| `006A6CA0` | Cdecl(first,last,value), plain RET. Source void excludes empty ambient EAX and nonempty last-child residue. Source adds context. |

Self-assignment returns destination before any field read, clear or child
call. Otherwise assignment captures source begin, and reads source end only
if begin is nonzero, to form a DWORD SAR4 element count. Null begin or zero
count delegates to accepted actual `006A6310` clear; there is no extra empty
receiver check. That child retains its original-end capture, unsigned bounds
checks and returning invalid-parameter continuations. The parent invents no
additional validation or diagnostic calls.

For a nonempty source, destination begin is captured once. Size reads end
only if that captured begin is nonzero. Source count and destination size
are compared unsigned. If source fits the existing size, accepted actual
`006A29C0` assigns the initially captured source begin/end to captured
destination begin. The parent then reads CURRENT destination end and
destroys the suffix from the range helper's returned cursor through accepted
`0055A3A0`, forwarding destination/source as its two unused metadata words.
Only after that cleanup does it reload source begin and, if nonzero, source
end, recompute size, read CURRENT destination begin and publish end. It does
not reuse the initial source count or the range helper result for that final
publication. Begin/capacity are not directly changed in this branch.

When source is larger, capacity is computed using the same captured
destination begin and a fresh capacity field read if begin is nonzero. If
capacity suffices, native code repeats the destination-size calculation,
including a second end-field read. The source boundary uses initial source
begin plus that repeated size. Accepted `006A29C0` assigns the existing
prefix; its return is ignored. The parent next reads CURRENT destination end
FIRST and CURRENT source end SECOND. It constructs the suffix from the
captured boundary to that source end, at the captured current destination
end, through accepted `006A4C00`. Only a successful return publishes the new
destination end. No boundary, begin, size or capacity validation is added.

If capacity is insufficient, nonzero captured destination begin triggers
accepted `0055AC60` range cleanup using that begin and CURRENT end. Only
after the child returns does the parent reload CURRENT begin for free. It
then reloads source begin and conditional source end to recompute count,
before accepted `00557250` initializes storage. There is no intervening
clear/tidy substitution. That initializer zeros +4/+8/+C before count
testing, uses the accepted actual allocation/length services, and publishes
capacity then begin then end. Its full DWORD result is retained; the parent
tests only AL. A zero AL returns destination without construction. Otherwise
the parent reads destination begin FIRST, source end SECOND and source begin
THIRD, forwards those captures through `006A4C00`, then publishes end.

Failures retain this schedule's earlier effects. Old backing destruction and
free precede initializer failure; initializer or suffix construction can
leave its already published fields. Earlier prefix assignments remain if a
later child fails. End publication is skipped where a child did not return.
There is no parent catch, rollback, synthetic cleanup, diagnostic retry or
ownership repair. Accepted child C++ cleanup, actual pool/ownership services
and current CRT behavior retain their own qualified contracts. A returning
diagnostic can affect subsequent CURRENT reads; the Source captures/reloads
are not folded together or replaced with a snapshot.

All direct outer header +0 fields remain opaque and unwritten. Counts,
unsigned comparisons, SAR4/SHL4 and cursor arithmetic use DWORD wrap without
C++ cross-object pointer subtraction. Valid reached storage and actual child
ownership/pool/current CRT lifetimes remain preconditions. Source unconsumed
child EDX words are normalized to zero; no native register-residue or private
frame alias equivalence is promised.

The range wrapper computes its returned destination cursor before its first
assignment, independently of child EAX. It captures source end and a
destination-minus-source delta, assigns forward through complete `006A6680`,
and advances source16 using equality termination. Prior writes may affect
later reads under forward overlap. The fill wrapper captures only the value
ADDRESS once on nonempty entry; each iteration reads that current owning
header through assignment. Value aliases, including a reached self-element,
retain those forward effects. Neither wrapper snapshots headers or reverses
the loop. Empty ranges perform no pointed-storage access.

The complete incoming `006A7630` listing resolves a physical-stack trap:
`006A7657 PUSH EDI` saves the caller's nonvolatile register, followed by three
range arguments. `006A7664 ADD ESP,0Ch` removes only the arguments; the saved
register is restored by `006A7678 POP EDI` after suffix cleanup. It is not a
fourth argument to `006A6C60`. The fill callers likewise push three arguments.
Incoming functions remain evidence only, without a completeness promotion.

Exact direct transfer evidence is:

```text
006A66A5 CALL 006A6310
006A66CD CALL 006A29C0
006A66E0 CALL 0055A3A0
006A6748 CALL 006A29C0
006A675B CALL 006A4C00
006A6777 CALL 0055AC60
006A6780 CALL 00BF65AC
006A679E CALL 00557250
006A67B5 CALL 006A4C00
006A6C86 CALL 006A6680
006A6CB6 CALL 006A6680
006A765B CALL 006A6C60
006A7884 CALL 006A6CA0
006A78E0 CALL 006A6CA0
```

These are new context/by-value Source interfaces, not original binary entry
replacements. Native register/stack/private frame, heap/CRT/handler,
FH3/SEH/fault/exception identity and diagnostic-provider identity remain
distinct. Private argument/frame aliases are excluded. No callback, host
container model, allocator substitute or extra guard is introduced. No new
native differential, failure-injection or gameplay proof is claimed.
The MSVC Win32 Release build and all three existing suites passed in7.50
seconds. The preexisting duplicate `spawn_request_id_matches` LNK4006 warning
remains. All14 report rows and14 Markdown claims pass transfer checks, with
no failures or unchecked claims. Detailed validation is in
`reports/oct15_outer_vector_assignment.json`; no new tests are added.

The dependency/build basis is published accepted main
`6249f7f7c080edeabcb936edcac30f2005e595a0`, obtained by fast-forward before
building. The integrator explicitly authorized one recorded CMake `--jump`
turn while the file was unleased behind an idle earlier queue entry. The
worker used it only for this accepted-baseline merge and its own appended
registration, then released immediately before build, without explicitly
removing another agent's queue entry or lease. The receipt records that
limited coordination exception.
