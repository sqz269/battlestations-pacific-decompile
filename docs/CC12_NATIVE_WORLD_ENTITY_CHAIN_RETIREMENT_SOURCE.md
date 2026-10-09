# Complete machine-level World entity-chain retirement

`retire_native_world_entity_chain_009041a0(void*)` supplies the guarded MSVC
Win32 naked fastcall body at `009041A0..00904203`. All 100 emitted bytes,
43 complete physical instructions and ten branches match fresh live bytes and
the installed PE without relocation rebinding. One exact complete core-library
member and one unique positive public definition are reviewed. The normal
Win32 build and three existing checks passed.

The existing projected `drain_entity_chain_009041a0` function and typed
`drain_native_world_entity_chain_009041a0` actual-storage C++ fragment remain
unchanged. This separate machine-level refinement adds one fragment and zero
new Original functions or bytes. The descriptive names are hypotheses.

## Complete schedule

ECX supplies the actual 12-byte `{first,last,count}` header. The body saves
ESI/EDI, retains that exact header in ESI, and sets EDI to zero. A full DWORD
count equal to zero exits before reading the head; every nonzero pattern enters.
It consumes no incoming EDX or explicit stack argument and returns by plain RET
with no typed EAX result.

Each iteration fetches the current head into ECX, then current previous `+34h`.
A nonzero previous enters unlinking. Otherwise it reads current next `+38h`;
a nonzero next also enters unlinking. Only when both links are zero does it
compare the current header count with one using signed JG. A value above one
skips unlinking; high-bit counts are signed negative and take unlinking. This
is neither an unsigned comparison nor an entry count-positive gate.

Unlinking preserves the native memory-access order:

1. Read current next and write previous.next or header.first.
2. Reload current next after that write, then reload current previous and write
   next.previous or header.last.
3. Zero entity.next, then entity.previous.
4. Subtract one from the current full header count with wrapping DWORD ADD.

The body then fetches the entity's current vptr into EDX and current slot-zero
code pointer into EAX, pushes DWORD one and CALLs EAX with the same entity in
ECX. It does not push a replacement receiver, call a projected Host or invoke
the typed C++ lifetime dispatcher. The actual target must consume four argument
bytes and preserve compatible callee-saved register behavior.

After the whole virtual return it reads the same retained header's current
count. If nonzero it fetches that header's current head on the next iteration.
There is no returned-entity dereference, cached successor, forced count reset,
stalled exit or synthetic cleanup. Ordinary effects preceding a nonlocal exit
are not rolled back here.

## Whole-body and listing evidence

The native initial JMP at `009041AB` skips `009041AD..AF`. Those three bytes
are exactly `8D 49 00`, an unreachable alignment LEA ECX,[ECX+0]. Current Ghidra
has 42 listed instruction starts and omits this one start. Independent complete
decoding has 43 instructions and covers every byte. Source explicitly preserves
the three bytes; no listing/function repair was necessary or performed.

The emitted function has zero relocations and no named external graph boundary.
The complete object, physical symbol/section, every instruction and branch,
unique positive core definition and exact whole member are retained in
`local/cc12_world_chain_retirement_primary/whole_object_and_indexed_core_proof.json`.
The saved project/program and unchanged 64,729 total function count were verified
before fresh capture. Ghidra's provisional name and old comments are preserved
with appended evidence and a refreshed export.

The primary report is
[cc12_native_world_entity_chain_retirement_primary_review.json](../reports/cc12_native_world_entity_chain_retirement_primary_review.json).
It pins the new source, unchanged earlier interfaces and contract evidence,
build inputs, complete live/PE bytes, physical emitted body and library proof.
The build receipt, log, source copies, whole object/library, game map/executable
and existing test log remain under `local/cc12_world_chain_retirement_primary`.

## Remaining caller contracts

The genuine header must survive every count/head read and virtual return.
Every reached entity, reciprocal link and current table/code pointer must be
valid through its actual accesses. Real hierarchy mutation and callbacks must
provide coherent state and progress; an isolated node with count greater than
one is still dispatched without local unlinking. Numeric Original profile words
are not supplied as callable Source tables. The entity's actual scalar lifetime
and allocation/free domain remain external.

This root is absent from the game map. It supplies no owner, table, context,
global, token, guard, free policy, exception wrapper, `noexcept` or application
wiring. Whole emitted/static/build evidence does not establish Native ABI entry,
fault/SEH/EH behavior, runtime equivalence, startup or gameplay. No new test or
probe was added.
