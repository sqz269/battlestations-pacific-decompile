# Native scene registry mutation adapters, CC10

This packet composes three outer adapters with existing raw pair-vector and
legacy exception providers. It never substitutes a host `SceneNodeRegistry`.
Baseline main for the build is `005e28b10`.

| Exclusive native range | Bytes | Operation | Coverage |
| --- | ---: | --- | --- |
| B83490..B8353A | 170 | Resize boundary vector | Complete normal/returning-handler body |
| B83560..B835CC | 108 | Assign boundary vector | Complete normal/returning-handler body |
| B82D30..B82DC3 | 147 | Checked list-size increase | Complete decision and raw-provider composition; source throw transport |

Native resize is ECX vector with count and inline two-word pair, RET12 at
B83537 (3B; the growth return is B834E2). Assign is ECX vector with count/pair
pointer, RET8 at B835C9 (3B). Checked increase is ECX embedded list with stacked
increment, RET4 at B82DC0 (3B). B82D30 allocates no list entry and performs no
linking. Its failure adds no rollback for an entry allocated by its caller.

## Borrowed frames and current reads

`NativeSceneRegistryResizeFrame` is 60B: the three incoming words and existing
InsertFrame28/VectorEraseArguments20. Growth captures requested count, initial
begin, repeated size and end before a returning BF6713 handler. It then passes
the CURRENT inline pair by address. Shrink captures end, rereads begin after
the first validation, overwrites incoming pair word1 with that begin before
the range check, and passes the SAME pair as the erase-result buffer.

`NativeSceneRegistryAssignFrame` is 72B: two incoming words, local pair/result
pairs, then the two nested frames. Assign captures input pair word1 before
word0. It captures end and compares current begin, then writes local pair0/1
before invoking the first invalid-parameter handler. Erase uses the captured
end and current begin. After erase, assign captures current begin and validates
against current end; only after the handler does it read CURRENT count argument
and compose insertion from its captured pair.

Every nested argument is stored in native push order. Nested scratch is not
initialized by the adapters. Frames carry initialized caller preimages, remain
address-stable and are disjoint from actual vector/backing storage. Assign's
incoming pair can alias live vector data or caller cells. These explicit cells
do not represent arbitrary native private-stack/saved-register aliases.

The existing `NativeSceneRegistryVectorMutationBindings` supplies the genuine
returning BF6713 service plus raw allocation/free/length-error bindings. Each
reached service is read at its call site; there is no default/no-op handler.
A returning handler may repair/change current cells, but all subsequent native
reads, ranges and source-provider contracts must remain valid. Native debug CRT
implementation/exception transport is not reimplemented here.

## Checked count and overflow

B82D30 captures the incoming increment before the current count. Its comparison
is unsigned `(3FFFFFFF - captured_count) < increment`, with DWORD wrapping.
Success stores captured count+increment, without rereading count. Even values
outside the usual list-size invariant follow this arithmetic; no clamp is added.

Overflow initializes only capacity15, length0 and first inline byte0 in caller
`NativeSceneRegistryListGrowthScratch` (28B). It passes the borrowed actual
CE38F8 text, exactly 16 bytes `list<T> too long`, to existing raw408720. Cleanup
is armed only after assignment succeeds. Existing
`NativeHardwareLayoutTreeLengthError` provides raw411700 construction, D69260
publication, actual411940 copying and411780 destruction in an owning source
exception. Its host RTTI/throw transport differs from BF6885/D83F98; no native
private exception-buffer preimage or FH3 compatibility is claimed.

Read-only compiler evidence is included, without a new runtime helper:

- CC22D0..CC22D8 (8B): `LEA ECX,[EBP-50]`; tail JMP4072D0 at CC22D3 (5B).
- CC22D8..CC22E2 (10B): `MOV EAX,DFB554`; tail JMPBF6B43 at CC22DD (5B).
  Ghidra has no function starting at CC22D8; the inclusive raw end is CC22E1.
- FuncInfo DFB554 has maxState1 and unwind map DFB54C `{toState:-1,
  action:CC22D0}`. State0 is written at B82D89 after408720 and before411700.
  The source guard uses the same existing4072D0 cleanup; allocation or
  construction failure before/after that arm follows the documented boundary.

## Verification

Strict `scripts/build.ps1` passed MSVC Win32 and both existing CTests. The 425
body bytes, 18 compiler-boundary bytes and 17-byte text including terminator
match live Ghidra and the installed PE. All native direct call/tail rows are
recorded; the missing-function descriptor jump is separately marked raw.
No Ghidra mutation or tracked test was added.

One ignored original/source sequence starts with actual28 registries and
existing raw vector providers. During growth, a genuine allocation callback
changes incoming count/pair cells; fill still uses the earlier captured pair.
Shrink checks the incoming-pair/result alias. Assign's returning validation
binding repairs live storage, changes the count, poisons the incoming source
pointer and overwrites its original backing pair; the earlier local pair is
preserved and the new count is used. Both executions match current arguments,
local pairs/results, registry bytes, full allocated vector contents and traces.
The original normal B82D30 path also verifies DWORD wrap and untouched scratch.
A source-only overflow check validates D69260, exact16-byte message plus NUL,
unchanged list count, preserved SBO leading DWORD and completed cleanup.

The copied outer bodies share genuine source B82FD0/B82BF0 providers through
explicit adapters. Allocator/free and returning validation are shared bound
services. This checks outer composition; it is not an execution of original
vector internals, original CRT diagnostics or original exception unwinding.
The standalone probe uses `/MD /O2 /W4 /WX /fp:strict /link /MANIFEST:EMBED` and
compile-time rejection of `NDEBUG`; its exact command is in the report.

Frozen evidence is `local/output/cc10_registry_adapters*` and
`local/cc10_registry_adapters_build.log`. Full registry insertion/registration,
scene attachment, AC59A0, binary ABI and gameplay remain outside this packet.
