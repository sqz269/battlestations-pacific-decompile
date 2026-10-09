# Qualified Source tick subnode reparent

Candidate `reparent_native_tick_subnode_00876020` composes the complete
`00876020..00876112` Native body: 243 bytes and 80 operations. Root's full-body
gate records identical Original PE/live bytes and all 80 saved starts. The
worker read all 80 operations. This four-file packet provides a plain C++
may-throw void entry; it adds no CMake registration, compiled candidate,
consumer, Source admission, Original ABI or runtime credit.

## Actual borrowed storage

The caller supplies the actual receiver address, actual volatile incoming-node
word by reference, actual volatile F=`00F878CC` and M=`01090AA0` publication
cells by reference, and eight live bytes of caller-owned Source guard backing.
The numeric cell names identify established Source contracts; no Original
global binding or private publication snapshot is created. The guard is real
borrowed Source storage for this invocation, never a claimed Native EBP slot.
It has a profile DWORD at +0 and pointer word at +4; no complete guard,
receiver or node type is fabricated or default-initialized.

Individual aligned volatile `uint32_t`, `int32_t` and `void*` views express the
required word accesses. Storage/lifetimes must support those types and valid
offset arithmetic. Receiver mutation accesses +1Ch/+20h/+24h, requiring backing
through +27h. Every invocation reads captured node+4; insertion writes node+8
and +Ch, and only a nonnull initial head causes one signed node+18h key read.
Reached positions/neighbors and the old-parent child require their actual
backing under the existing child contracts. A captured raw section contains
the 18h-byte Win32 `CRITICAL_SECTION` prefix and tracked DWORD at +18h.

Keep the guard, reached raw storage, actual publication cells and their real
owner/domain/section lifetimes valid throughout all normal and failure calls.
Aliasing is retained where these C++ typed-storage requirements are compatible.
Arbitrary Native control-stack aliases, saved registers, invalid addresses,
hardware faults and concurrent mutation are not thereby made equivalent.

## Entry and child binding

Private cleanup control initially holds the actual guard address with
`armed=false`. Call the real
`get_native_pending_registry_00875280(F, M)` with both actual publication
references, then read its result+4 section once into captured K. There is no
new result-null guard. Write profile DWORD `00CE37FC` to actual guard+0, then
captured K to actual guard+4. If K is nonnull, call actual SDK Enter(K), then
increment the current tracked DWORD K+18h modulo 32 bits.

Only after those operations, read the incoming node cell once into N. Read
current N+4 into oldParent, then arm cleanup before the old-parent branch.
The initial getter, guard publication, Enter/depth increment and both late
reads precede arming. This matches the logical Native state-zero point after
the old-parent read/TEST, without constructing a Native state word or FS frame.

A nonnull oldParent equal to the receiver skips mutation and proceeds to
normal captured-section cleanup. If oldParent is different and nonnull, create
a separate actual volatile child-input word initialized from captured N and
call the real
`unlink_native_tick_subnode_00875960(oldParent, 0u, child_word, F, M)`.
That new word represents Native `PUSH ESI` copying N into the child argument.
It does not reuse or reread the original incoming cell. The real F/M references
remain direct. Zero is only the child's unused Source EDX placement value;
there is no claim about Original EDX residue.

The existing child is a naked fastcall Source adapter with ECX receiver, EDX
placement and three stack references, ending in `RET 0Ch`. It reads the child
word only after its own real getter and optional Enter/depth increment. Its
raw unlink and subsequent current-node parent clear retain their existing
contracts and failure limits. The parent continues with its own captured N.
There is no new child stub, callback or copied publication.

## Ordered reparent mutation

If mutation is selected, write N+4=receiver after any required child unlink.
Read receiver+1Ch head once. Only when nonnull, capture signed N+18h once, then
walk current positions: stop when signed current position+18h is greater than
the key; otherwise load current position+Ch and continue while nonnull. Equal
priorities are passed. No cycle detector, count bound or tree/list validation
is introduced.

For insertion before a position, first capture current position+8 into previous
and write N+8=previous. If previous is nonnull, read its current +Ch only after
that write, publish it to N+Ch, then write previous+Ch=N. Otherwise freshly
read receiver+1Ch, publish that to N+Ch, then write receiver+1Ch=N. Next freshly
read N+Ch after those stores. If nonnull, write that next node+8=N; otherwise
write receiver+20h=N. The selected position pointer is not substituted for
these later current-memory reads.

Only when traversal exhausts, read current receiver+24h. If nonzero, read
current receiver+20h and write that tail+Ch=N without a new null-tail guard.
If zero, write receiver+1Ch=N. Then freshly read receiver+20h into N+8, write
receiver+20h=N, and write N+Ch=null. A zero count does not force N+8 to null:
it still receives the late tail value. After either insertion path, increment
the then-current receiver+24h modulo 32 bits. No count, link or empty-list
normalization is added.

## Normal and failure cleanup

Normal cleanup uses the original captured K, regardless of current guard+4.
If nonnull, decrement current K+18h modulo 32 bits, then call actual SDK Leave(K).
Cleanup remains armed through both operations. Disarm only after Leave returns,
or on null-K normal completion. This preserves the logical Native state-zero
interval across normal release.

On C++ unwind reaching the armed private destructor, call only the actual
`destroy_native_singleton_guard_00411ee0(actual_guard_prefix)`. That provider
captures the current guard+4 pointer before writing the profile, then on a
nonnull captured pointer decrements its current +18h DWORD and calls Leave.
The current guard pointer may differ from K after compatible alias writes.
A C++ failure during normal Leave can therefore reach this cleanup with the
normal decrement and earlier mutations retained. No catch, retry loop, rollback,
repair, free or registration operation is inserted.

The private destructor is explicitly `noexcept`; cleanup throwing from it
terminates under current C++ policy. The actual provider itself has no explicit
`noexcept` or local catch. Current SDK calls and synchronous C++ compilation do
not establish Original exception/fault delivery or successful OS lock release.

The selected Native EH edge is already accepted: handler `00C964E8` selects
descriptor `00DC864C`, map `00DC8644`, action `00C964E0`, and previous state -1.
The action computes ECX=actual runtime EBP-14h and tails to `00411EE0`. Root
replayed the selected 18 code bytes/four operations and 44 data bytes, plus
the existing 25-byte/eight-operation provider. The shared 54-byte FH3 wrapper
is accepted separately; `00C07991` remains unopened. This C++ borrowed guard
and cleanup control do not prove Original interpreter/frame identity.

## Evidence and admission boundary

Native takes receiver in ECX and one stack node value, restores saved registers
and FS, then `RET 4`; no common semantic EAX result is supplied. This Source
entry instead uses five explicit arguments and ordinary C++ EH. It reproduces
neither the Native call frame nor register/flag residuals. Descriptive fields
and function names remain reconstruction hypotheses.

The report pins both Root Native gates, accepted caller/EH/provider evidence,
the actual Source providers and current Source109 receipt
`reports/cc12_native_lua_variant_pair_retreat_primary_review.json`. That build
ended 2026-10-09 19:05:05 UTC with 109 inputs, four artifacts, 30 whole objects,
34 public Core definitions and three passed existing checks. Root's later
readiness gate verifies the actual guard provider's 31 compiled bytes/12
operations and whole-object identity in Source109. Older Source107/105/97/57
artifacts are historical. These existing build facts do not compile this entry.

Worker validation is bounded Native/source inspection, provider and receipt
pin replay, JSON parsing and diff/scope checks. No new build, test, probe,
fixture, consumer or Ghidra/ledger mutation occurs here. Independent Root must
register, compile, inspect and admit the actual new entry and cleanup control.
Production storage/publication binding, Original ABI, startup and gameplay
remain unproved.
