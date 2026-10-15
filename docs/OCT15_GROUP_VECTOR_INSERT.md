# Actual group-vector InsertCount

`native_group_vector_insert.cpp` reconstructs full `006A7690`, its two
catch scopes, prototype unwind and inline normal cleanup. It also provides
the actual ordinary support entries `0069B360` and `0069F1A0`. Old typed
settings/frontend projections and their fixtures remain separate and
unchanged. Descriptive names remain hypotheses.

The receiver is an actual16B checked header: opaque +0, begin +4, end +8,
capacity +C. Its backing contains16B outer headers; those own16B inner
headers, which own8B pooled-string records. These levels are established
by the actual constructor, transfer, assignment and cleanup producers.
This is not raw8B copying or a borrowed nested-vector model.

## Native interface and body

Native ECX is the receiver, EDX is unconsumed, and the four physical stack
words are dead iterator owner, position, unsigned count and value address.
The return is `RET10h`, without a semantic result consumed by the Resize
caller. Source reserves those words and adds actual `NativeStringRawPoolContext`,
therefore `RET14h` and `void`; private register residue is not promised.

Typed metadata identifies three normal ranges:
`006A7690..006A7804`, `006A7827..006A788D`, `006A78B1..006A7923`.
Together they contain591 bytes/229 instructions. The separate catches are
`006A7805..006A7826` (34 bytes/12 instructions) and
`006A788E..006A78B0` (35 bytes/13 instructions). The full envelope is660
bytes/254 instructions. `006A78E8` is an inline normal-cleanup fragment,
not an additional function. Its41 bytes/14 instructions end at006A7910,
before the ordinary epilogue. Older saved call graphs did not reliably
separate these scopes; current typed ranges and actual instructions govern.

## Prototype, count and capacity

The routine first constructs a private owned prototype using actual55B150
from the supplied value address. Its opaque word is neither initialized
nor evaluated. Receiver begin is then captured before prototype cleanup
is armed. Even count0 constructs the prototype, reads capacity when begin
is nonnull, and later destroys/frees the owned prototype. There is no
early count0 shortcut or header snapshot before the actual constructor.

Capacity is SAR32(capacity-begin,4), or0 for captured null begin. Size uses
the same captured begin but rereads CURRENT end at each original decision.
The unsigned limit test is `0FFFFFFF - size < count`, including DWORD wrap.
No extra malformed-header, position, alignment, self or range guard is added.

If backing must grow, half is the logical `capacity>>1`. Growth becomes0
when `0FFFFFFF-half < capacity`; otherwise capacity+half. Native size is
reread for the comparison and again if the candidate is replaced by
size+count. Original repeated reads and unsigned comparisons are retained.

## Reallocation path and catch

The actual allocation target is69B360, not553300. Its complete81-byte
body matches accepted553300 after **only** three independently verified
rel32 fields. All count/zero/division/return instructions, actual targets,
bad-allocation vtableD6923C and throw metadataE03CC0 remain identical.
The new Source entry reuses that complete actual current-CRT service.
Count0 still requests new0 within the allocator, without a new host limit.

New-backing progress initially equals the allocation start. The first
actual6A6D20 transfer uses CURRENT receiver begin and the input position;
its successful result publishes completed progress before prototype fill.
The fill result is captured, CURRENT receiver end is read, then completed
progress is advanced before the suffix transfer. The suffix result itself
is unused. Original nested backing identities and capacities move through
ordered field swaps; prototype fills are actual owning constructions.

The first catch tidies only previously completed phases through actual
6A7190, frees raw new backing, then rethrows. Each failing child retains
its own partial-phase cleanup. No already-transferred source state is
restored. Cleanup failure stops later work. The receiver begin is captured
before this construction scope is disarmed.

Only the prototype scope remains armed during old-backing cleanup. New
size is original count plus size from that captured old begin and CURRENT
end. Old destruction uses the captured begin but another CURRENT end read;
free reloads CURRENT receiver begin. Capacity, end, then begin are published
in that order. Failure in old cleanup does not gain a new-backing free or
rollback after the construction scope has ended.

## In-place paths

Both paths capture old end and compare unsigned SAR4(old_end-position)
against count. For a short tail, actual6A70E0 first transfers the suffix
to position+count*16. Only after this succeeds does the routine reload
CURRENT end and recompute the remaining fill count. The second guard covers
prototype fill, CURRENT-end increment and the end reread for gap assignment.
Its catch reloads CURRENT end and tidies exactly
`[position+count*16, CURRENT_end+count*16)`, then rethrows. It does not free
the receiver backing. A captured old-end substitute would change this scope.

That guard is disarmed before actual6A6CA0 assigns the existing gap from
the fixed owned prototype address. The dependency is the accepted full
6A6680 assignment chain, not a borrowed-header loop or callback.

For a long tail, actual6A70E0 transfers the last count headers to old end,
and its result is immediately published as receiver end. Actual6A7110
then swaps backward, preserving nested backing identity/capacity, before
actual6A6CA0 fills the gap. There is no short-tail construction guard or
new rollback around these later operations. Published end remains published
if backward movement or gap assignment fails.

## Prototype cleanup and EH

Exceptional prototype cleanup is the actual55B110 Tidy schedule: captured
range destruction, CURRENT begin free, ordered +4/+8/+C clears, +0 untouched.
Normal cleanup is different: capture prototype begin while state0 remains
armed, disarm cleanup, capture end if begin is nonnull, destroy that range,
then free the cached begin without clearing fields. Normal cleanup failures
do not enter the prototype unwind scope. Source preserves both schedules.

HandlerC7F5C8 loads FuncInfoDACAF0 and transfers to native FH3. Its full tuple
is `(19930522,5,DACB3C,2,DACB14,0,0,0,1)`. Unwind states are
`0->(-1,C7F5C0)` and `1/2/3/4->(0,0)`. The two try records are
`(1,1,2,1,DACAE0)` and `(3,3,4,1,DACAD0)`. Their catch-all records bind
respectively to6A7805 and6A788E. No extra prototype free is invented.

The actual length target is69F1A0, not556C90. Its complete105-byte body
matches accepted556C90 after only its verified own-handler immediate and
three verified same-target rel32 fields. The exact18 counted bytes are
`vector<T> too long`; no terminator belongs to that counted slice. The
no-init SBO cap15/length0/NUL schedule, counted assignment, state-arm order,
legacy28h logic-error payload, D69260 word and D83F98 throw data match.
Source reuses the accepted owning transport, without generic `std::length_error`.

Its own handlerC7EF28 binds FuncInfoDABDE4/mapDABDDC: one state,
`0->(-1,C7EF20)`, no try/catch records, flags1. The action tidies the actual
completed SBO temporary at parent EBP-50 through4072D0. Full maps and action
match the accepted length transport structurally with actual addresses
retained. Counted-assignment failure precedes that cleanup state.

## Exact transfer evidence

Every ordinary/catch/action transfer below is verified against live listing
and installed bytes; native FH3 transfers are context evidence, not new
Source frame adapters. The final row is the caller's result-use check.

| Site and target | Purpose |
|---|---|
| `006A76BD CALL 0055B150` | Owned prototype |
| `006A76FC CALL 0069F1A0` | Actual owning length throw |
| `006A775D CALL 0069B360` | Actual16B allocation |
| `006A7780 CALL 006A6D20` | Prefix transfer |
| `006A7790 CALL 006A7500` | New-backing fill |
| `006A77AC CALL 006A6D20` | Suffix transfer |
| `006A77D9 CALL 006A6EE0` | Old group range |
| `006A77E2 CALL 00BF65AC` | CURRENT old begin free |
| `006A784A CALL 006A70E0` | Short-tail transfer |
| `006A7867 CALL 006A7500` | Short-tail prototype fill |
| `006A7884 CALL 006A6CA0` | Short-tail gap assignment |
| `006A78C2 CALL 006A70E0` | Long-tail transfer |
| `006A78D0 CALL 006A7110` | Backward movement |
| `006A78E0 CALL 006A6CA0` | Long-tail gap assignment |
| `006A7903 CALL 0055A3A0` | Normal prototype range |
| `006A7909 CALL 00BF65AC` | Cached prototype begin free |
| `006A7810 CALL 006A7190` | New-backing catch range |
| `006A7816 CALL 00BF65AC` | New raw backing free |
| `006A7822 CALL 00BF6885` | First catch rethrow |
| `006A78A3 CALL 006A7190` | CURRENT-end short-tail catch |
| `006A78AC CALL 00BF6885` | Second catch rethrow |
| `00C7F5C3 JMP 0055B110` | Prototype unwind |
| `00C7F5CD JMP 00BF6B43` | Insertion FH3 handler |
| `0069B36D CALL 00BF681B` | Actual allocation service |
| `0069B395 CALL 00BF6340` | Native allocation-failure payload |
| `0069B3AC CALL 00BF6885` | Native allocation-failure throw |
| `0069F1D8 CALL 00408720` | Counted actual SBO assignment |
| `0069F1ED CALL 00411700` | Actual legacy logic-error payload |
| `0069F204 CALL 00BF6885` | Owning length throw |
| `00C7EF23 JMP 004072D0` | Length temporary cleanup |
| `00C7EF2D JMP 00BF6B43` | Length FH3 handler |
| `006A7A03 CALL 006A7690` | Resize ignores EAX |

## Validation and limits

The existing project `C:/Users/sqz269/bsp.gpr` and program
`/battlestationspacific.exe` were verified before each live batch. Full
pseudocode was followed by complete assembly and typed metadata. Sixteen
full live-byte spans exactly match the installed PE32 image without masks.
Capstone5.0.7 matches all354 current listing starts over nine owned/comparison
units. Forty direct targets, both main try maps, all support EH records and
ordinary return conventions are asserted. Comparison masks are limited to
the individually verified address fields described above. All raw captures,
bytes, typed responses, hashes and extraction provenance are recorded.

Assignment dependency e33160ebce62e0812c2d22572d11f908612c7403 was accepted
before build, then merged separately as ce52805f4 after published-main
fast-forward303a0b292. The short queued CMake registration lease was released
before the exact Win32 build. All three existing suites passed in9.06s;
no new test was added. Generated COFF verifies the prototype-before-zero
path, progress guards, actual child services, current reads, publication,
both catches and cached normal cleanup. JSON32/32 and Markdown32/32 transfer
checks pass with zero unchecked claims or warnings; staged diff checks pass.

All reached storage/endpoint, actual child ownership/pool and current CRT
contracts must hold. Private native/Source frame aliases are excluded.
Unconsumed private tag high bytes are qualified and normalized0, rather
than evaluating indeterminate C++ words; meaningful public metadata is
forwarded. Receiver/prototype opaque words are not evaluated or written.
No x87/FP operations occur in this body/support/scopes. Source C++ catch,
owning exception transport, extra RawCtx, private frames and current CRT do
not reproduce original register/FH3/SEH/RTTI/fault/cleanup-failure/heap identity.
No new native execution, fixture equivalence, binary replacement, startup
or gameplay claim is made. Original game and saved Ghidra are preserved;
only the primary performs later annotations and saved export refreshes.
