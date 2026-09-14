# Raw publication/base cleanup and owner unwind (CX)

The complete `004B4F10..004B4F20` leaf unconditionally clears F8D420, then
writes CE3818 into the actual complete owner's first DWORD, and returns.
Original ECX is that owner; there are no stack arguments. The public raw
interface adds the actual publication cell's address in EDX. Its instruction
stream preserves the two stores and their order, replacing only the absolute
publication destination. It introduces no manager unregister, free, admission
lookup or operation state. There is no whole-game binary ABI claim.

This body already exists in orchestrator 5's `native_sampler_owner_lifetime`
implementation (source commit `abfc4761e44ab10204871d37618fca226fabb9bd`, reviewed
branch head `1bbc16628a278b9c8b023266664e4874eb45a368`). That implementation uses
an explicit owner/operation context. CX exposes the original two-store leaf
through a smaller raw call contract with the actual owner and publication cell.
It grants **zero new native function or body-byte credit** and preserves the
existing Ghidra name `BSP_SamplerOwner_ClearPublicationBase`. All names remain
descriptive hypotheses.

The B1B680 owner destructor's native handler CBC798 loads FuncInfo DF4B04.
Its single unwind map entry at DF4AFC changes state0 to -1 and invokes CBC790,
which loads the retained complete owner from `[EBP-10h]` and jumps to 004B4F10.
The native body arms state0 before the secondary destructor call at B1B6B4.
Consequently a failed secondary destruction still clears the publication and
stamps CE3818. Its normal tail at B1B6BD/B1B6C7 performs the same stores inline.
This agrees with the already-existing orchestrator 5 source exception cleanup.

The report pins complete installed/live body and EH spans, the cross-branch
source identity, and the generated-code transformation. The CU/CW branch-local
readiness audits did not include that external implementation; their claim that
the full raw container shutdown was absent was incomplete. Cross-branch review
is required before implementing further overlapping dependencies. This packet
does not claim to reconstruct the native private FH3 frame, validate a complete
mixed-owner shutdown, or run gameplay.
