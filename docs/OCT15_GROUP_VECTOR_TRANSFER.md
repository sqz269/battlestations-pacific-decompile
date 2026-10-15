# Actual group-vector transfer and backward swaps

This packet reconstructs complete `006A6D20..006A6E12`, including its
parent-frame catch `006A6DB0..006A6DD7`, transfer dispatcher
`006A70E0..006A7104`, backward swap `006A6E20..006A6E62`, and backward
dispatcher `006A7110..006A714F`. Source is
`src/native_group_vector_transfer.cpp`; declarations are in
`include/bsp/native_group_vector_transfer.hpp`. Actual16B outer headers own
actual16B inner checked-vector headers whose backings contain8B pooled
strings. Existing typed settings projections, host views and fixtures remain
unchanged. Descriptive names remain hypotheses; earlier ledger evidence and
correct library names are preserved.

The saved project is `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. Complete listings, pseudocode, live prototypes,
typed body metadata, disk/live bytes and full instruction starts were
checked through repository tools. The parent has two typed ranges totaling
203 bytes: `006A6D20..006A6DAF` and `006A6DD8..006A6E12`; its40-byte catch
is a separate function. The combined243-byte envelope decodes completely to
87 matching disk/live instruction starts. The other complete envelopes are
37/67/64 bytes and14/26/29 starts. Neither dispatcher is a typed Ghidra thunk.
The typed catch no-return flag is false, although its full listing ends in
the native rethrow transfer; no analysis metadata is repaired here.

| Entry | Original physical interface |
|---|---|
| `006A6D20` | ECX first, EDX last; stack destination, owner word, duplicate word, tag word; RET10h/EAX final destination. Source retains those inputs and adds raw-pool context. |
| `006A6DB0` | Parent EBP, original destination at -18h, completed end at +8, owner at +C; no independent callable Source frame adapter. |
| `006A70E0` | ECX receiver word, EDX unused; stack first,last,destination; RET0Ch; child EAX passes through. Source adds context. |
| `006A6E20` | Cdecl first,last,destination-end followed by three unused words; plain RET. Source is void; native EAX final cursor is residue. |
| `006A7110` | Cdecl first,last,destination-end; plain RET/EAX computed independently after the child. Source retains the three-stack-word interface. |

The transfer captures last and original destination, initializes only +4,
+8 and +C of a private16B empty header to zero, and leaves opaque +0
uninitialized. For each source header, nonzero destination invokes accepted
actual `0055B150` using that empty header as the copy source. Only after a
successful constructor does native EBX capture temporary begin. A zero
destination skips only construction and that cache reload; the following
swaps still access destination fields. Source adds no null-destination guard.

The swaps preserve the full instruction order. For +4, source is read before
destination; destination is written before source. For +8, destination is
read before source, then destination is written before source. For +C,
source is read before destination, destination is written, the destination
register advances16, source is written, and only then the completed-end
spill advances. The source cursor advances16 last. Source keeps a separate
completed end so catch cleanup excludes a constructor that failed before
the current header completed. +0 is untouched. Endpoint comparison and
cursor stepping use DWORD equality/wrap, without extra validation.

On normal completion the native state becomes -1 before manual temporary
cleanup. If cached EBX begin is nonzero, the code destroys the range from
that cached begin to CURRENT temporary end, forwarding the temporary pointer
and final destination as unused metadata to accepted `0055A3A0`, then frees
the same cached begin. This is deliberately not a tidy substitution: a tidy
would read and later free CURRENT begin, and perform extra clears. Normal
cleanup has no remaining temporary-unwind retry if a child fails.

The complete lower-level `006A3200` listing was compared, including its
post-catch normal tail. Its instruction schedule has the same shape, but
the ownership children differ: checked-string-vector constructor `00557BC0`
becomes outer-vector constructor `0055B150`; prefix tidy `00559DC0` becomes
stacked outer tidy `006A6CE0`; normal8B-string range `00432050` becomes
16B-inner-header range `0055A3A0`. This packet composes the deeper actual
services independently; it does not call the lower-level transfer or reuse
its settings projection.

The raw FH3 handler at `00C7F549` loads FuncInfo `00DAC9CC` and jumps to
the frame handler. Its nine DWORDs are
`19930522,5,00DAC9F0,1,00DAC9B8,0,0,0,1`. Unwind map `00DAC9F0` is:

| State | To state | Action |
|---|---|---|
| 0 | -1 | `00C7F530` |
| 1 | 0 | none |
| 2 | 1 | `00C7F538` |
| 3 | 1 | none |
| 4 | 0 | none |

Try map `00DAC9B8` is `[1,3,4,1,00DAC9A8]`; the one catch record is
`[40h,0,0,006A6DB0]`. State2's action reloads current destination twice
from -1Ch/-20h, pushes the two words and invokes RET-only `00401130`;
there is no free or destruction to synthesize. The prefix catch captures
original destination and completed end, loads owner only for a nonempty
prefix, and tidies completed16B headers forward through accepted `006A6CE0`.
It then rethrows. The temporary's state0 action uses current fields through
accepted `0055B110`, including if prefix cleanup itself exits exceptionally.
Source uses nested current C++ catch scopes for those two responsibilities;
it does not manufacture EBP frames or bind the original FH3 implementation.
The accepted child constructor retains its own cleanup behavior. Unconsumed
child EDX inputs are normalized to zero, without a native-residue promise.

The transfer dispatcher duplicates its original destination stack word,
forwards receiver as owner, and clears the low byte of a saved receiver copy
for the final dead tag. Thus child stack words are destination, receiver,
destination, receiver&FFFFFF00h. Receiver is not dereferenced. The new
by-value Source interface excludes native argument/private-stack aliases.

The backward swap initially loads last and destination-end, then first. For
each element it reads source-12 before destination-12, decrements both
cursors16, captures the next comparison, and writes destination+4 before
source+4. It then reads source+8 before destination+8, writes destination
before source, and repeats that read/write order at +C. It does not read or
write +0 and performs no construction, destruction, allocation or pool work.
Equal endpoints skip every field access.

The backward dispatcher pushes destination twice as dead child words and a
saved ambient ECX word with its low byte cleared as the sixth dead word.
The complete six-word child reads none of those three words. Source
normalizes the last dead word to zero rather than adding a semantic ECX
input. Native ECX restoration/residue is outside this interface. The
dispatcher ignores child EAX and independently computes
`destination - (SAR4(DWORD(last-first)) << 4)` using DWORD wrap and an
explicit arithmetic-shift equivalent. No pointer-difference assumptions,
alignment check or generic STL move replacement are introduced.

Exact direct transfer evidence, including incoming sites only, is:

```text
006A6D79 CALL 0055B150
006A6DC3 CALL 006A6CE0
006A6DD3 CALL 00BF6885
006A6DF0 CALL 0055A3A0
006A6DF6 CALL 00BF65AC
006A70FC CALL 006A6D20
006A7137 CALL 006A6E20
00C7F533 JMP 0055B110
00C7F540 CALL 00401130
00C7F54E JMP 00BF6B43
006A7780 CALL 006A6D20
006A77AC CALL 006A6D20
006A784A CALL 006A70E0
006A78C2 CALL 006A70E0
006A78D0 CALL 006A7110
```

Current Source failures stop later cleanup; earlier swaps and ownership
changes remain. A prefix-cleanup failure still reaches temporary cleanup,
and a temporary-cleanup failure can replace the earlier Source exception.
This is a current C++ composition, not proof of the native cleanup-failure
personality. Valid reached raw storage and accepted actual pool/CRT child
lifetimes are preconditions. There is no callback, host container model,
synthetic allocator, added guard, retry or rollback. Raw context/by-value
interfaces, private stack/opaque temporary state, native heap/CRT/handler,
FH3/SEH/fault/exception identities and game execution are distinct limits.
No new native differential, failure-injection or gameplay proof is claimed.

The MSVC Win32 Release build and all three existing suites passed in9.04
seconds. The existing duplicate `spawn_request_id_matches` LNK4006 warning
remains. The transfer checker verified15 report rows and15 Markdown claims,
with no failed or unchecked claims. Incoming transfer sites are grouped
under current enclosing function `006A7690`; no enclosing catch-body or
insertion completeness is inferred. No new tests were added. Transfer
checks and complete byte/start evidence are recorded in
`reports/oct15_group_vector_transfer.json`.
