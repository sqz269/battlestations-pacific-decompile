# Actual-storage World activation flush: Source refinement of 00903670

This packet implements the complete reviewed `00903670..009036A8` body as
`bsp::flush_native_world_activations_00903670(void*)` in the guarded MSVC Win32
`native_world_activation_flush.hpp/.cpp` pair. The function follows actual
receiver/header/entity/parent storage and directly calls the existing concrete
`mark_native_entity_destroy_state_00922fd0` helper. No application wiring is added.

The body retains all 22 native instructions: 21 assembly mnemonics and six byte
directives encoding the single native `LEA EBX,[EBX+00000000]`. Every instruction
is tagged with its original address. Static written-source checks cover the
complete sequence, branch labels and concrete call intent. The integrator owns
registration, build and full emitted-body/external-relocation/binding review.

The projected `flush_entity_activations_00903670(WorldTickState&, WorldTickHost&)`
and its counted canonical record are unchanged. This worker claims zero new
Original functions/bytes or ABI/game-validation credit; a Source refinement must
not duplicate the existing function count.

## Baseline and accepted evidence

- Published baseline: `141f7a16360139a982382b9c5f8bfecc67d202e6`.
- Worker merge baseline: `340e0ea30125dcab712e885dcef9eea19e91bc53`, preserving
  readiness commit `7697456017d1d8ed054ca3d16874d41c103821fe`.
- The accepted readiness report pins all 57 bytes / 22 decoded instructions,
  SHA-256 `1711b2cb01a1341d6bbecc164337978395150304375ea23838060ec319e769b1`.
  Its live Ghidra bytes equal the installed PE span. This packet rechecks that
  complete accepted capture without a new Ghidra query or native execution.
- The concrete callee remains the qualified 66-byte / 25-instruction helper.
  Its current header/source hashes match the published primary report, which
  records whole resolved-body equality with the PE, a unique core definition,
  a normal Win32 build and three existing checks. Those results are prior
  integrator evidence; this worker does not rerun them.
- The companion report includes the complete caller bytes and written mapping,
  canonical current input pins, accepted audit pins and owned LF file hashes.

## Preserved actual-storage behavior

The first instruction captures `[ECX+4]` before saving `ESI`. The next pointer load
captures that header's first entity once. A null first entity takes the native
empty path; a null header is still dereferenced. Header first and receiver `+4`
are never reloaded. The function does not read a World vtable or header count.

Each current entity is called only after these ordered, short-circuit predicates:
byte `+5E != 0`, full DWORD `+6C == 0`, and either parent `+3C` is null or the
captured parent's byte `+5E == 0`. No active-byte, vector-index, parent-membership
or null-identity guard is inserted. Every nonzero `+6C` pattern skips the call.

The sole direct CALL names `mark_native_entity_destroy_state_00922fd0` after
loading `ECX` with the current entity. The callee performs its actual flag/mark,
recursive child and late current virtual `+84` operations. On complete return,
the flush reads the same entity's current `+38` successor. A callback's changed
successor is therefore observed; a saved next pointer or vector walk would differ.
Later marks and parent fields are read only when those entities are reached.

The body preserves the six native LEA bytes `8D 9B 00 00 00 00`, which leave EBX
and flags unchanged. It ends with `POP ESI` and plain `RET`. There are no explicit
stack arguments, meaningful incoming `EDX` input, local field stores, globals,
floating-point operations, allocation, cleanup or default-return substitution.

## Qualification and validation limits

The caller supplies actual readable receiver/header/entity/parent storage plus
the existing callee's real callable `+84` and hierarchy contracts. No object
overlay, context, global, table, callback substitute or owner is created.

The receiver/header must permit their initial loads. Every current entity must
survive all reached predicates and its post-call `+38` read; a nonnull parent
must survive its reached `+5E` read. The callee's own child `+44` requirements and
compatible stack/callee-saved-register return behavior also apply. There are no
later receiver/header accesses in this body, but its caller may require a longer
lifetime. No new permission to free caller-needed storage is implied.

A cyclic `+38` chain can loop even after every mark becomes nonzero. Invalid
storage can fault, with earlier callee effects retained. The helper adds no
guard, visited set, catch, rollback, exception translation or `noexcept` promise.
Native exception/unwind/fault compatibility, concrete virtual target behavior,
application runtime, binary replacement and gameplay remain unproved.

Worker validation covers the full accepted 57 bytes, contiguous 22-instruction
decode, all 22 original-address annotations, six branch destinations, six-byte
LEA encoding, single concrete external call and final restore/RET. Canonical
inputs, unchanged concrete provider, owned staged LF hashes, JSON and the diff
are checked. Written instructions are not emitted-code evidence. No CMake,
ledger, GPR, tests, probes, standalone build or native execution was performed.
The integrator must inspect the full emitted caller and resolve its external
CALL to the positive concrete helper definition before admitting the refinement.
