# Whole preset pair construction integration

Both ordinary entries 0069E6E0 and 0069E7A0 are reconstructed: 198 Native bytes
and 74 instructions. E6 takes separate key and mapped pointers; E7 takes one
pair. Both compare identity before clearing length and data, resize through the
actual RawCtx with preserve=1, reread CURRENT source length, and capture CURRENT
destination length, source data and destination data in that order before the
actual BF7680 copy. Each mapped word moves as a separate four-byte representation
before the next read. There is no new owner, guard, key cleanup, rollback, self
repair, default feature state or adapter.

Forty-two primary checks passed. All eight CODE sections match the frozen worker
bytes and four actual provider REL32 bindings: 328 bytes / 137 instructions
(public 238B/93, helpers 90B/44). Both emitted predicates compare before the header
resets. Their private volatile bools reuse a dead argument home's high byte while
EDI retains the original pointer. The extra argument-slot accesses, private
frames, flags and fault order remain qualified. Each public body retains five
interleaved DWORD read/store pairs. Eight weak BSS bytes are unreferenced compiler
compatibility data, unrelated to the actual feature cell.

All 12 and 10 provider fingerprints remain exact. Two actual ledger append
conflicts were resolved with the repository union tool. Each address's frozen
name, evidence and ordinary records are verified after integration, with all old
records preserved. Each Core unit appears once. Four JSON transfers and four
Markdown claims pass. Two locked annotations preserve name/comment preimages
and save the project; two exports are refreshed.

The combined default MSVC Win32 build passes all three existing suites in
7.83s. There are no new tests, probes or owned-entry execution. Native
RET8/4 versus Source RET10h/0Ch, current Raw resize, CRT, heap, feature lifetime,
DF0, SSE2, private frames, FH3/SEH, faults, Original ABI and gameplay remain
qualified. Node reconstruction follows E7 publication; factory, catch, logical
action and parent insertion are not promoted by this batch.
