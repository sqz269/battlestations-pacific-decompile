# Actual outer string-vector uninitialized fill

`src/native_outer_string_vector_fill.cpp` reconstructs the complete ordinary
effects of `006A6FA0` and `006A7500`, the completed-prefix catch `006A6FFB`, and
the observable absence of cleanup in placement action `00C7F560`. The actual
16-byte outer header has an untouched opaque word at +0, owning begin at +4,
end at +8 and capacity at +C. Its elements are actual 16-byte inner checked
headers, themselves owning eight-byte pooled-string elements. Existing
qualified host/storage projections are separate.

## Original bodies and interfaces

The core has ordinary ranges `006A6FA0..006A6FFA` and `006A7020..006A7032`
(110 bytes, 42 instructions), with the separate 37-byte/15-instruction catch
at `006A6FFB..006A701F`. The complete combined span is 147 bytes. Native ECX
is the destination cursor, EDX is an unsigned count, and the four stacked
words are the fixed value **address**, owner metadata, dead tag3 and dead
tag4. It ends in `RET 10h`. Source retains those words, adds the actual raw
string-pool context and therefore ends in `RET 14h`.

The dispatcher `006A7500..006A7535` is 54 bytes/23 instructions. It consumes
ECX owner metadata and three physical stack words: destination, count and
value address. Entry EDX is unused. The native return is `RET 0Ch`; Source
reserves the two register positions and adds context, giving `RET 10h`.
`006A7524 CALL 006A6FA0` forwards the core's dead words as owner,
value-address bits, and owner
with only its low byte cleared. The ignored high tag bytes are not invented
zero initialization. Retained descriptive library names remain hypotheses.

The core's EAX is ambient register residue: zero count leaves the entry
`FS:[0]` value; nonnull construction leaves the child's result; null skips
leave the previous residue. Source exposes `void`. The dispatcher instead
computes and returns DWORD `original_destination + (original_count << 4)`
after the core, with 32-bit wrap. Native `006A7790 CALL 006A7500` is followed
by storing EAX at `006A77A0` and passing it onward at `006A77AB`;
`006A7867 CALL 006A7500` is followed by overwriting EAX at `006A786C`.
This establishes a semantic dispatcher result without adding a semantic
incoming-EAX parameter to the core.

## Construction and progress

`006A6FE8 CALL 0055B150` binds directly to the accepted actual
`construct_native_outer_string_vector_copy_0055b150`. Each iteration uses
the same value address. The pointed header is read anew by the child; it is
not snapshotted, borrowed or assigned. Native ignored EDX is not a child
semantic input; Source passes zero for that explicitly unused word.
The child's original count-before-clear, +0 preservation, current field
rereads, real allocation, owning pooled strings, self-alias behavior and
cleanup contracts all apply.

The loop compares the unsigned count to zero, not to a signed bound.
Placement state1 is armed before the null-slot branch. A null slot skips
the constructor. After either success or that skip, it decrements count,
advances the DWORD cursor by16, returns to state0 and publishes completed
end. No new range, null, alias, capacity or self-copy guard is introduced.
Address advancement uses unsigned integer arithmetic, not C++ pointer
arithmetic. A count with its high bit set is not treated as empty.

## Catch, placement action and actual cleanup binding

The catch captures initial destination and the published completed end.
`006A700B CALL 006A6CE0` is implemented through the accepted actual
`tidy_native_outer_string_vector_0055aff0`, after comparing both complete
native listings and the concrete accepted Source. `006A6CE0` takes its
header on the stack and ignores the catch's owner ECX. Both routines
capture header+4, read +8 only when begin is nonnull, destroy the captured
16-byte range through actual `0055A3A0`, free **CURRENT** header+4, then
unconditionally clear +4, +8 and +C in order; +0 is untouched. Both send
header/header as the two ignored range metadata words. There is no local
catch, retry or extra null-header guard. Thus no pending group-cleanup or
unaccepted wrapper dependency is needed, and no new entry for `006A6CE0`
is claimed by this packet.

Cleanup advances by16 until equality and excludes the failing current
construction, whose child retains its own cleanup. A null slot that was
skipped during construction still belongs to the completed prefix: cleanup
does **not** skip it. Safe cleanup therefore requires every accessed prefix
header, including any such slot, to satisfy the actual tidy preconditions.
There is no outer range-allocation free. Each completed header owns its
own inner backing. A cleanup failure stops later work; fault, termination
and nested-exception identity are not claimed.

`006A701B CALL 00BF6885` has two zero arguments and does not return; Source
uses `throw;` after its actual completed-prefix cleanup. Native FH3 handler
`00C7F571` loads FuncInfo `00DACA54` and jumps to `00BF6B43`. Its full tuple is
`(19930522,3,DACA3C,1,DACA28,0,0,0,1)`. The unwind map is
`state0->(-1,0), state1->(0,C7F560), state2->(-1,0)`. The try record is
`(0,1,2,1,DACA18)`; handler record `(40,0,0,6A6FFB)` is catch-all.

Placement action `00C7F560..00C7F570` reads parent words at EBP-1C/-20,
pushes them and performs `00C7F568 CALL 00401130`, then `ADD ESP,8; RET`.
The complete actual target is one byte, `C3`/`RET`. Eliding this action adds
no free, destructor, callback or stub. The catch and action are parent-bound
Source scopes, not independently callable native ABI replacements.

## Evidence and limits

Before analysis/export, the existing project `C:/Users/sqz269/bsp.gpr` and
program `/battlestationspacific.exe` were verified. Pseudocode was followed
by complete assembly, catch/action exports, native child and caller flow,
accepted producer/layout/API evidence and full raw EH records. The saved
listing reports 57 combined core and23 dispatcher instructions with zero
gaps; the live flow tool does not expose typed no-return/fallthrough flags.
The actual branches, catch rethrow and both native returns were read
directly instead of inferring completeness from that gap report.

The installed PE32 section table was used to extract complete disk spans.
Core147B and dispatcher54B exactly match independently read live Ghidra
bytes, without masks. Seven E8 targets, both returns, the full EH maps and
RET-only action target are asserted. Full spans and extraction provenance
are retained in `reports/oct15_outer_string_vector_fill.json`.

All admitted accessed storage, actual pool publications, child ownership
and current CRT contracts must hold; aliases to native/Source private
frames are excluded. Fixed value/slot aliases retain the actual child's
rereads and ownership effects. Source C++ catch/rethrow and added context
do not reproduce native FH3/SEH/RTTI, private-frame/register ABI, exception
or fault identity. No x87 or other floating-point operations occur in these
wrappers. No original/native fixture execution, binary replacement,
startup or gameplay equivalence is claimed. Historical index evidence
also listed `006A7110` as a core caller; the current live caller query only
lists `006A7500`, so that old edge is not used as proof here.

The exact registered Source passed `./scripts/build.ps1` and all three
existing suites in11.06s: reconstructed math, native math differential and
tool tests. JSON7/7 and Markdown7/7 call checks pass with zero unchecked
claims or warnings; `git diff --check` passes. No new test suite or
deterministic ownership fixture is introduced.

The exact-build COFF retains the unsigned/zero test, null skip, fixed value
address load, actual constructor call, then decrement at55/cursor advance
at56/completed-end publication at59. Its catch contains the actual accepted
Tidy call, stride/equality cleanup and two-zero rethrow. The dispatcher
retains the owner mask and forwarded words, then shift/add from original
inputs. New context stack cleanup is `RET14h`/`RET10h`. These are static
Source observations, not original-byte or native EH/register equivalence.
The report records object/source hashes and extraction provenance.

Short CMake leases were separate: `oct15_outer_string_vector_fill_cmake_registration`
resolved the published-main append conflict and was released after merge
`636ebf689`; `oct15_outer_string_vector_fill_source_registration` queued,
claimed its first/unleased turn, appended only this Source registration and
was released before the build. The address lease remains held for review.
