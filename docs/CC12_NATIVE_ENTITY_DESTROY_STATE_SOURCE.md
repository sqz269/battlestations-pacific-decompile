# Actual-storage entity destroy state: Source refinement of 00922FD0

This packet implements the reviewed complete `00922FD0..00923011` body as
`bsp::mark_native_entity_destroy_state_00922fd0(void*)` in the guarded MSVC Win32
header/source pair `native_entity_destroy_state.hpp/.cpp`. It uses the actual
receiver and current hierarchy/table storage. No application binding is added.

The Source function contains the native 25-instruction schedule: 24 assembly
mnemonics and three byte directives encoding the single native `LEA ECX,[ECX+00]`
instruction. The recursive `CALL` names the same real Source function. Every
instruction has its original address beside it. Written-source inspection is
complete; registration, build and emitted-code review belong to the integrator.

The existing projected `scene_node_kill_00922fd0(SceneNodeFlags&)` and its counted
canonical reconstruction record are unchanged. This worker claims zero new
Original function/byte or ABI/game-validation credit. The integrator will record
the refinement and fix the old name-ledger `+5D` store-site label only after the
complete emitted body is reviewed.

## Baseline and native evidence

- Published baseline: `4b8c9780ad27ad881ff99528c79d1168d712086e`.
- Worker merge baseline: `699e2c38a874e771365eed989b0bc52fd52541ea`, preserving the
  reviewed readiness commit `f50917e86d6f36192a75f1146e4a2c5c14f68016`.
- Accepted evidence: `CC12_ENTITY_DESTROY_STATE_CURRENT_STORAGE_READINESS.md` and
  its JSON report, with read-only verified Ghidra project `C:/Users/sqz269/bsp.gpr`,
  `/battlestationspacific.exe`, `x86:LE:32:default`, base `00400000`.
- Full native body: 66 bytes / 25 instructions, SHA-256
  `29eceff51de711a90ebe3201b0f270a6cdc3febb62b1532857b5fba6301d382d`.
  The earlier live capture equals the installed PE span. This implementation
  packet rechecks that accepted capture; it makes no new Ghidra query or write.
- The companion report embeds the complete native span and written-source
  instruction mapping, canonical input hashes and owned Source/document hashes.
  Native bytes and written instructions are not emitted-code evidence.

## Preserved current-storage schedule

`ECX` supplies the receiver; no incoming `EDX` or explicit stack argument is read.
The function saves `ESI`/`EDI`, captures child `+48` and tests it before four stores:
byte `+5E = 1`, byte `+5D = 1`, byte `+5C = 0`, then full DWORD `+6C = 1`. The MOVs
preserve that initial TEST's flags for the leaf branch. Already-marked receivers
still receive every store and the final virtual dispatch. Byte `+5F` is untouched.

The child walk recurses only when the current child's **full DWORD** `+6C` is
zero. Every nonzero pattern skips recursion. After a recursive invocation and its
terminal virtual method return, the parent reads that same child's current `+44`
sibling link. Skipped children use the same reload. There is no saved successor,
head restart, vector index, projected Boolean or added child flag gate.

The receiver's table and slot `+84` are loaded only after all reached children
have finished. A child callback can therefore alter both the followed sibling
link and the ancestor's eventual dispatch. After restoring `EDI` and `ESI`, the
last instruction remains `JMP EDX`, with `ECX = receiver`, `EAX = current table`
and `EDX = current target`. There is no local RET, default return, extra cleanup
or call-then-return substitution. The real target uses the original caller's
return address; a recursive target returns directly to the parent's `+44` reload.

## Caller qualifications and remaining limits

The helper borrows actual readable/writable receiver and child storage, current
hierarchy links, and a real callable current slot `+84` target compatible with
the recovered register/stack dispatch. It supplies no object overlay, full-size
claim, context, global, table, callback substitute, allocator or owner.

The receiver must survive through its late table load; each current child must
survive through the parent's post-return `+44` read. The World caller additionally
reads the same entity at `+38` after return. That obligation is pinned through the
accepted peer destructor evidence; no World code is changed here. Although this
body reads nothing after its terminal jump, the callee cannot invalidate storage
that a continuing caller still requires.

Mark-before-descent can suppress child backedges to a marked ancestor, but sibling
cycles can still loop and callbacks can clear marks. There is no added cycle
guard. Null storage faults at the first `+48` access; later failures leave earlier
stores and descendant effects intact. No new `noexcept`, catch, rollback, default
success or failure policy is introduced. The target's complete ABI, incidental
return values, native exception/unwind/fault behavior and binary replacement
compatibility remain unproved.

Production entity tables and virtual target bodies, actual hierarchy producers,
callback mutation/lifetime guarantees, application wiring and complete World
destruction remain separate work. A callable-table precondition qualifies this
leaf; it is not evidence that those production providers now exist.

## Validation boundary

Worker checks cover the exact cached 66 bytes, contiguous 25-instruction decode,
25 ordered original-address annotations, all three branch targets, native LEA
bytes, the sole self-call symbol, and the terminal dispatch/register schedule.
Canonical input and owned-file hashes, JSON parsing and the staged diff are also
checked. No new test, probe, standalone build, GPR, CMake or ledger mutation was
performed. The integrator must build the registered Source and inspect every
emitted instruction and its recursive relocation before accepting ABI claims.

## Primary registration and emitted review

The integrator registered this unit in bsp_core. The normal MSVC Win32 build and all three existing checks passed. Complete physical COFF review finds exactly 66 bytes / 25 instructions and one self-call relocation: operand 41 targets the same physical symbol index8, section3, value0. Resolving its zero addend to `D3 FF FF FF` makes the entire body equal the pinned live capture and installed PE, including `8D 49 00` and the terminal `FF E2` jump. The unique whole Core member and positive own definition are retained.

The existing counted projected function record is preserved. This is one additional actual-storage fragment with zero new Original function/byte credit. The name-ledger correction records +5D at00922FE1 and +5C at00922FE4 while preserving the older evidence. The public root is absent from the game map. Real current tables, hierarchy/lifetimes, Native entry/exception/fault execution and gameplay remain open.

Evidence: `reports/cc12_entity_destroy_state_primary_review.json`; complete retained objects/library/map/build receipts under `local/cc12_entity_destroy_state_primary`.
