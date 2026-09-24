# Raw Group query and canonical identity

This packet implements AC6F70's complete40B query and a separate host
`NativeGuiGroupReference` over a completed actual Group. It adds no call or
registration behavior to native AA12F0. The existing raw factory/default/copy
and scalar destructor remain their own providers.

## AC6F70

The range is `[00AC6F70,00AC6F98)`, inclusive last byte AC6F97. Final RET4 is
at AC6F95, length3; the false branch's RET4 is AC6F90. Native incoming ECX is
ignored; the descriptor comes from the stack. Each comparison reads the
CURRENT next DWORD from F8BFD8,F8BFDC,F8BFE0; equality returns AL1, exhaustion
AL0. The new bool interface does not promise the pointer-derived upper EAX.
The lineage cells are borrowed volatile references, not Group enum2 or a
cached type set. Actual startup must supply their established values; static
Ghidra zero-filled cells do not establish runtime type identities.

Ghidra had no function at AC6F70 during worker analysis. Read-only live40B
bytes match the PE; the PE listing confirms both RET4 endpoints. The primary
may define/annotate the function after the worker releases its lease.

## Admission and domains

AC6F50/AC6FA0 finish a real EC-byte Group in an F0-byte F8BFA0 pool slot;
pool metadata is +EC. Its actual first8 bytes contain D5CB80 and the existing
live atomic count+04. The companion borrows that atomic through
`RenderCommandReference(actual_count)`, whose optional owned count stays empty.
Admission never placement-constructs another native prefix or initializes a
counter. It validates alignment, current Group profile/slots, positive count,
and absence of an existing canonical entry, then binds host registry metadata.
Host allocation/duplicate failure leaves actual storage and creator untouched.

The caller explicitly admits the completed object after construction, before
canonical lookup exposure. AA12F0 itself allocates, constructs, and returns;
it has no native companion/admission call. `NativeGuiTextIdentityReference`
is not reused: it requires a logical `GuiWidgetOwner` and Text deletion layer.

The context borrows one `NativeRenderActualOwnerRegistry`, the existing raw
`NativeGuiWidgetLifetimeContext`, and the actual current D5CB80 first two
cells. The registry must be the same domain used by retained owners/materials
and the actual node companions. `lifetime.nodes.attachments` must resolve
every reached +4C to the SAME actual model companion registered in that domain.
The generic interface has no registry identity field to prove that relationship;
this is an explicit caller contract, not an invented native field or registry.
Parent/child/timed contexts and all callback dependencies must share it.

## Terminal and explicit scalar deletion

Live D5CB80 has slot0 BD30E0, slot4 AC73E0, slot0C AC6F70 and slot20 AA8320.
The canonical release path decrements actual+04 externally, resolves its same
companion only at zero, and calls `release_zero_references()`. The companion
checks bound phase/count0/current slot0. Existing BD30E0 then reloads current
profile for slot4; only the verified AC73E0 target is admitted, with flags1.
Neither helper decrements the count again. Unknown current profiles/targets
have no fallback; the existing terminal noexcept boundary applies.

Explicit `delete_scalar_00ac73e0(flags_slot)` is separate. It requires bound
phase, but does not inspect or decrement +04 or read flags before the real
deleter. AC73E0 stamps D5CB80, calls raw AA9730, then tests ONLY the current
low flags byte. Bit0 returns the same slot through F8BFA0/AC7260; flags0 still
destroys payload. Both outcomes retire the companion. The returned value is
the original numeric identity and is not permission to access a returned slot.

Once bound, explicit child deletion must route through this method, not a
second direct raw-deleter path that leaves stale metadata. Raw lifetime's
current Group0C binding can use the new40B query, Group20 uses existing AA8320,
and current Group4 can resolve this same canonical companion and invoke its
explicit method. Other derived children/timed owners still require real
`NativeGuiWidgetLifetimeBindings`; no generic logical or successful no-op
dispatch is installed. Existing raw A9BD50 supplies valid parent-list removal.

Phase changes are host metadata. After native teardown/possible pool return,
the wrapper sets retired and calls exact-match registry unbind. That operation
only hashes/erases identity and companion pointers; it does not read the native
count or payload. The companion destructor checks only host phase, and its base
destructor does not dereference the borrowed counter. Companion/context must
remain address-stable through dispatch. External synchronization must prevent
slot reuse/admission before registry retirement and permit companion disposal
only after terminal quiescence.

An entered explicit destruction failure is one-shot: retire/unbind metadata,
then propagate the C++ exception. The unreturned slot and partial native state
remain caller-owned, with no retry or implicit cleanup. Terminal delivery
terminates on such a failure. These are source failure boundaries, not native
FH3/SEH equivalence. This packet does not establish a complete application
resource graph or install factory admission into gameplay.

## Verification

Strict MSVC Win32 `scripts/build.ps1` and both existing CTests passed. Exact
live/PE evidence covers AC6F70[40], BD30E0[14], AC73E0[38], and the first44B
of D5CB80. Native scalar call rows AC73E9->AA9730 and AC73FB->AC7260 are checked;
BD30EB is an inspected indirect current-slot4 call. The companion itself is
host metadata and has no invented native call site.

One ignored focused probe compares six AC6F70 original/source query pairs,
including changing a borrowed lineage cell between calls. It then uses actual
Group process startup, pool/default payloads, native count and real registry:
F0-byte unchanged binding, duplicate refusal, count1 zero terminal, explicit
nonzero flags0/1, nested admitted Group child teardown and raw list removal.
A scripted child query callback changes parent flags1 to flags0 during AA9730,
confirming the late low-byte test and caller-owned unreturned slot. Companion
destruction follows terminal pool return, and the actual pool reuses returned
slots. It performs no payload/count reads after a slot return.

The lifecycle portion executes source providers, not original native destructor
bodies. Profile/lineage inputs are explicit fixture publications; other derived
and timed callbacks fail when reached. The test has no model+4C payload, so the
caller node-domain contract is not dynamically certified. The report records
exact `/MD`, `/fp:strict`, `/MANIFEST:EMBED` command, hashes, and an NDEBUG
compile-time rejection. No outer native ABI, full register/FH3/SEH identity,
application admission or game validation is claimed.
