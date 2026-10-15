# Actual outer-vector erase range and clear

This packet reconstructs complete ordinary entries 006A5BF0 and 006A6310 in
`src/native_outer_vector_erase_range.cpp`. They operate on actual 16-byte outer
headers owning 16-byte inner checked headers, which own 8-byte pooled strings.
Existing typed settings projections remain separate. Descriptive names are
hypotheses, not recovered symbols.

The complete native bodies are 88 bytes/37 instructions and 57 bytes/26
instructions. Both full live spans equal the installed executable; all 63
current export instruction starts match disk Capstone. Fresh typed metadata
identifies the exact complete ordinary ranges. Neither body establishes a
local EH frame. The paired JSON report records hashes, all six direct CALL
sites, original interfaces and validation limits.

## Erase schedule and interface

006A5BF0 receives the actual header in ECX and five stacked words: output
iterator address, first owner, first position, last owner and last position.
Its RET14h discards those five words. EAX is the output iterator address after
the two final stores; Source exposes this stable machine result. The Source
reserves EDX and adds an actual raw-pool context, changing the physical ABI.

The routine captures first owner, checks it for zero or inequality with last
owner, and invokes accepted actual BF6713 diagnostic policy on failure. It
does not require either owner to equal the receiver. A returning diagnostic
continues the schedule. It then compares the first and last positions. Equal
positions skip receiver end/range accesses but still write the output iterator.

For unequal positions it captures the receiver end and performs accepted
006A29C0 forward assignment from last position through that captured end into
first position. This is owning assignment into existing headers, preserving
native forward overlap effects. It retains the returned new end, reloads
CURRENT receiver end for accepted 0055A3A0 suffix destruction, then publishes
the new end only after successful cleanup. No allocation or receiver begin or
capacity update is introduced. Child failure preserves earlier effects.

Finally it writes captured first owner to output+0 and first position to
output+4, in that order, and returns the output address. Output aliases retain
those stores after receiver cleanup/end publication. There is no new null,
receiver-owner equality, range, alignment or self-alias guard, snapshot or
rollback. The actual children retain their field reads, clears and ownership
effects; receiver opaque+0 is not directly changed unless an admitted output
alias writes it.

## Clear schedule and interface

006A6310 receives the actual header in ECX, consumes no stacked input and
returns with plain RET. Incoming EDX is unused. Source places the added pool
context in EDX and exposes void; native EAX points into its private output
iterator frame and is not promoted as a semantic result.

It captures the original end before comparing CURRENT begin with that end
using unsigned DWORD ordering. After any returning diagnostic, it captures
begin again and compares this captured begin with CURRENT end. The second
diagnostic also may return. It passes those retained begin and original end
captures to the complete erase routine, with the receiver as both iterator
owners and a private two-word output iterator. It does not reread endpoints
after the second diagnostic or replace this path with unconditional tidy.

## Validation limits

The current SDK diagnostic policy supplied by accepted BF6713 differs from
the original encoded handler/debugger/Watson selection. All actual taken raw
accesses, reachable endpoints and child ownership/pool/current-CRT contracts
must be admitted. Native and Source private-frame aliases are excluded.
Added context, current C++ and CRT interfaces do not establish native register
or stack ABI, FH3/SEH/RTTI, faults or handler-failure identity. No local catch
or cleanup retry is added. Existing native math differential tests cover prior
verified seeds; these entries have no new native execution or gameplay proof.
The report records the exact build and existing-suite results separately.
