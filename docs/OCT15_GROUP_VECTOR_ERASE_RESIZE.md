# Actual group-vector erase and owning-parameter resize

This packet reconstructs complete ordinary entries 006A7630 and 006A79A0,
plus the parent-bound C7F5E0 ownership unwind, in
`src/native_group_vector_erase_resize.cpp`. The actual 16-byte group header
owns 16-byte outer headers, which own 16-byte inner checked headers containing
8-byte pooled strings. Existing settings projections and fixtures remain
separate. Descriptive names are hypotheses, not recovered symbols.

The complete bodies occupy 88/253/8 bytes and contain 37/95/2 instructions.
All 349 live bytes match the installed executable, and all 134 owned current
export starts match disk Capstone. The report records complete typed ranges,
hashes, 11 body CALLs, the ownership tail JMP and raw 44-byte EH data. Separate
read-only native and Source reviews accompany the build receipt.

## Erase

006A7630 takes the receiver in ECX, ignores incoming EDX, and receives five
stacked words: output iterator, first owner, first position, last owner and
last position. RET14h discards those words. EAX returns the output iterator
address after its two stores. Source reserves EDX and adds actual pool context.

The only owner check is nonnull first owner equal to last owner. It does not
require owner equality with the receiver or validate positions. A diagnostic
through accepted actual BF6713/current SDK policy may return, after which the
position comparison continues. Equal positions skip all receiver field reads,
assignment and destruction, but still publish the output iterator.

Otherwise it captures receiver end and invokes complete 006A6C60 forward
owning assignment from last position through that captured end into first
position. It retains the independently computed range result, reloads CURRENT
receiver end for actual 006A6EE0 suffix destruction, and publishes the retained
new end only after cleanup succeeds. The native PUSH EDI before the three
assignment arguments is a saved nonvolatile register, not a fourth argument.

Finally output+0 receives captured first owner and output+4 receives first
position, in that order. Aliases retain those stores after receiver end
publication. No new owner-receiver, null, range or self guard, allocation,
snapshot, retry or rollback is added. Child failures retain earlier effects.

## Resize and ownership

006A79A0 takes the actual receiver in ECX, ignores EDX, and receives requested
count followed by an owned by-value 16-byte outer header. Its RET14h discards
five physical words and it supplies no semantic result. Source transfers the
four ownership words into a single private canonical header; it does not
invoke another argument copy constructor. Opaque+0 remains a bit pattern.

The first receiver begin read precedes ownership-state0. Growth computes the
initial and repeated current sizes using that retained begin, captures end
before a returning diagnostic, and invokes complete 006A7690 insertion with
the retained endpoint and count difference. Even equal/shrink paths consume
the fill parameter.

Shrink retains its second receiver-end capture across all diagnostics. It
reloads begin after the first report, compares it with CURRENT end, and retains
that begin through later reports when deriving the first erase endpoint.
The final bounds test reads end before begin and skips the begin read if the
first comparison already fails. The private output iterator's second word
receives the retained begin before the final report. The full erase service
receives those captures; it does not substitute freshly reread endpoints.

Normal cleanup captures fill begin while the owner remains armed, disables
ownership, destroys from cached begin through CURRENT fill end with actual
0055A3A0, and frees that same cached begin without clearing fields. It executes
outside the Source owner catch, preserving the absence of a cleanup retry.
Exceptional exits instead compose accepted actual 0055B110 Tidy on the
canonical header, including its CURRENT-begin free and ordered +4/+8/+C clears.

Native handler C7F5E8 loads FuncInfo DACB6C. The sole unwind-map pair at
DACB64 is state0 to -1 with action C7F5E0; there are no native local try blocks.
The exact eight-byte action is LEA ECX,[EBP+8]; JMP0055B110. Source receives
the owned header explicitly and does not supply a native EBP/FH3 frame adapter.

## Evidence and limits

The paired JSON report keeps native bytes, physical interfaces, raw EH maps,
Source review hashes and build results separate. Added RawPoolContext,
canonical private storage and current C++/CRT services change physical Source
interfaces. All taken actual storage, child ownership/pool/current-CRT and
reachable endpoint contracts must be admitted; native or Source private-frame
aliases are excluded. Current SDK diagnostic policy differs from original
handler selection and may return. Native register/stack ABI, FH3/SEH/RTTI,
fault/cleanup-failure identity, runtime and gameplay equivalence are unproved.
Existing native math tests cover prior verified seeds; these entries have no
new native differential execution claim.
