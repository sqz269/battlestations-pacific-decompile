# Raw scalar float by-reference leaves

This packet supplies the complete raw minimum at 00415510 and a public header
for it and the existing maximum at 00415550. Names are descriptive hypotheses.
The minimum contributes 62 newly supplied raw implementation bytes and **zero
new unique-address credit**: its address already had a ledger record pointing
to a value-taking approximation. Maximum is declaration-only reuse.

| Entry | Inclusive extent | Bytes | Coverage | Provider |
|---|---|---:|---|---|
| 00415510 | 00415510–0041554D | 62 | complete raw body | new scalar source |
| 00415550 | 00415550–0041558D | 62 | existing complete raw body | native_traceline_render.cpp |

The exclusive ends 0041554E/0041558E are followed by CC CC padding. Both complete
native listings contain twenty instructions and zero gaps. The earlier 63-byte
description included the exclusive endpoint incorrectly.

The original ABI passes pointers to actual binary32 inputs in ECX and EDX,
returns float in x87 ST0, and uses plain RET with no public stack arguments.
Both public declarations use that exact `__fastcall` pointer signature without
an added `noexcept`. The naked minimum has no compiler prologue or value adapter.
It reserves eight stack bytes, FLD32/FSTP32 round-trips left through [ESP], then
right through [ESP+4]. It reloads left then right, executes FCOMIP ST0,ST1
(`DF F1`, right versus left), pops the remaining comparison value, and JBE
selects the right spill. The existing maximum reverses the comparison load
order and otherwise follows the same selection/spill structure.

Equality, including signed-zero equality, selects the right spill in both
leaves. Masked unordered FCOMIP also selects right. FCOMIP is not FUCOMIP; input
loads and stores precede comparison and may already quiet signaling NaNs or
affect status/trap timing. The return is derived from the selected spill, not
necessarily untouched input bits. Each branch MOVSS-copies that spill through
XMM0 into [ESP], FLD32-loads return ST0, restores ESP and returns. No SSE arithmetic,
control-word guard, status clearing, allocation, callback or constant binding is
introduced. The exact input read/spill order is retained even when pointers alias.

Caller storage and stack must be valid, with room for two additional live x87
entries; successful return leaves one result entry. These bodies do not modify
EAX/ECX/EDX/EBX/EBP/ESI/EDI. XMM0 is overwritten and final ADD changes flags. No
return-flags contract or result-in-XMM0 contract is inferred. Literal source
instructions do not by themselves establish injected caller or hardware-fault
delivery compatibility; generated-byte proof is recorded separately below.

The existing external maximum stays in its original translation unit and existing
startup registration. Its symbol is
`?max_native_float_by_ref_00415550@bsp@@YIMPBM0@Z`. The header adds no second
definition; its coupled-library dependencies remain. The eight internal
value-taking minimum helpers and every old caller remain unchanged. In particular,
the approach helper's `(b<a)?b:a` chooses left for equality/unordered and is not
this raw body. The report preserves the exact old backfill/name records for
primary's later ledger redirect, including the backfill's no-reverification
disclaimer. No ledger or Ghidra metadata is changed by this worker.

The accepted ED admission pins complete native/live/PE bodies and the existing
max's complete 62-byte, zero-relocation emitted section in the previous EA build.
That max object is also an exact member of its pinned library. Those are retained
historical validation inputs, not a build of this new source revision. A clean
coupled Win32 build and complete emitted minimum identity are pending primary
source acceptance and completion of the preceding EC build. Required proof is
all 62 bytes equal, zero relocations, the external pointer ABI and both RET exits.
No new fixture or repository test is added; no runtime/game claim is made.
