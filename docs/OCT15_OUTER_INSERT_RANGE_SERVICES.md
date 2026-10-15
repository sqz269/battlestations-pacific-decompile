# Actual inner-vector transfer, swap and uninitialized fill services

This packet reconstructs six complete ordinary entries and five parent-bound
cleanup scopes in `src/native_outer_insert_range_services.cpp`. These services
operate on actual owning 16-byte checked headers: opaque word at +0, begin +4,
end +8 and capacity +C. Their elements are actual owning 8-byte pooled strings.
The Source uses the accepted constructor, destruction and allocation services;
it introduces no replacement container, callbacks or fabricated child bodies.

| Native entry | Source behavior and original physical interface |
| --- | --- |
| 006A3200 | Construct empty destination headers, then transfer ownership by ordered field swaps. ECX first, EDX last, four stacked words destination/owner/tag3/tag4, RET10h; EAX advanced destination. |
| 006A4B50 | Transfer dispatcher. ECX owner, three stacked words first/last/destination, RET0Ch; EAX child result. |
| 006A3300 | Backward raw field swaps. Six cdecl physical words, only first three consumed, plain RET. Source exposes the stable final-destination machine EAX; its dispatcher ignores it. |
| 006A4B80 | Backward dispatcher. Three cdecl words first/last/destination_end, plain RET; independently returns destination_end minus SAR32(last-first,4) times16. |
| 006A43A0 | Uninitialized repeated inner-header construction. ECX destination, EDX count, four stacked words value/owner/tag3/tag4, RET10h. Source has no semantic EAX promise. |
| 006A5170 | Fill dispatcher. ECX owner, three stacked words destination/count/value, RET0Ch; independent EAX original destination plus original count times16. |

All descriptive names are hypotheses, not recovered symbols. Added actual pool
context arguments, reserved Source EDX parameters and current C++ cleanup change
the physical interfaces; these are not drop-in native binary entries.

## Transfer and overlap effects

The private empty header initializes only +4/+8/+C to zero; its opaque word is
neither initialized nor consumed. Each nonnull destination invokes the actual
00557BC0 copy constructor from this empty header. A null destination skips that
construction but still participates in the following raw accesses. There is no
new null guard, self-alias shortcut or rollback.

The +4 swap reads source then destination and writes destination then source.
The +8 swap reads destination then source and writes destination then source.
The +C swap reads source then destination, writes destination, advances the
destination cursor, writes source, publishes the completed end and then advances
the source. The opaque words remain untouched. Existing inner allocations and
capacities transfer with those fields. A source equal to destination still gets
the native constructor's clear-before-swap behavior.

006A3290 cleans completed destination headers in ascending 16-byte steps and
rethrows; the failing current constructor is excluded. The actual 00559DC0
per-header cleanup has the accepted 00432440/004324A0 storage schedule. Unwind
action 00C7F260 tail-jumps to actual 004324A0 for the private empty header. The
Source invokes that exact accepted destruction service. The nested C++ catches
preserve prefix-then-private-header cleanup for admitted child exceptions.

Normal private-header cleanup is outside those catches, after the native state
is disabled. It tests the cached empty begin, which starts at zero and is
reloaded only after a successful destination construction, destroys its range
using the current empty end and frees the cached begin. It does not replace
that free with a current-header begin reload.

006A3300 walks backward by 16 bytes, reads the two begin fields before cursor
decrements, and then swaps +4/+8/+C. It performs no constructor, assignment or
allocation. Its dispatcher ignores child EAX and computes its own wrapped
distance result. Native 006A4B80 forwards ambient incoming ECX high24 bits in an
unused sixth child word. The child never reads that word; Source normalizes it
to zero and does not invent a semantic ambient-register input.

## Fill and exception scopes

006A43A0 retains the value address, allowing the actual constructor to reread
the current value header at each nonnull destination. It then decrements count,
advances the cursor by16 and publishes the completed prefix. A null skip joins
that prefix. Catch 006A43FB has no null skip: it tidies each completed header
and rethrows, without freeing a range allocation. The failing current child is
excluded; no source-header snapshot or borrowed assignment is substituted.

Transfer handler C7F279 loads FuncInfo DAC444: five unwind states at DAC468,
try states1..3 and catch state4 at DAC430, handler record DAC420 selecting
6A3290. Fill handler C7F381 loads FuncInfo DAC684: three unwind states at
DAC66C, try states0..1/catch2 at DAC658, handler record DAC648 selecting
6A43FB. Placement actions C7F268 and C7F370 pass their parent words to actual
00401130, whose entire body is RET. Their verified no-op effects are elided;
no fake deallocator or independently callable scope API is introduced.

## Evidence and limits

The paired JSON report records complete typed body ranges, byte hashes,
instruction counts and all 13 direct CALLs plus the one cleanup tail JMP.
Fresh typed metadata verifies the saved project and program; disk Capstone
starts match the forced current export for all 252 owned instructions. Catches
are counted separately from ordinary body ranges. Full live/disk and raw EH
checks and the exact build receipt are recorded in that report after review.

Every taken raw access and actual child ownership/pool/CRT contract must be
admitted; equality-stepped endpoints must be reachable. Aliases to native or
Source private frames are excluded. Current CRT and C++ exception handling do
not establish native FH3/SEH/RTTI, private frames, register ABI, faults or cleanup
failure identity. Existing seed tests do not execute these new native entries;
no game startup or gameplay equivalence is claimed.
