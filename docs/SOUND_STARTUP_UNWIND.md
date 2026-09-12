# Sound constructor exception cleanup

Addresses: 00A88770, 00A7B190, 00A880E0.

The sound constructor now destroys its completed base when a later startup
stage throws. It uses `destroy_sound_system_base_00a816b0` with the caller's
canonical `SoundSystemShutdownContext`. The derived destructor would release
resources that the native constructor's unwind table leaves alive, so it is
not the constructor cleanup operation. Descriptive names are hypotheses.

| Routine | Coverage | Original ABI |
|---|---|---|
| A88770 | Complete ordered normal path and EH state transitions in the existing throwing-allocation, canonical-state projection | ECX = 178h owner; three stack words, only low byte of first consumed; EAX = owner; RET 0Ch at A88AA6, length 3 |
| A7B190 | Complete existing registration sequence plus guard/root-base unwind | ECX = owner; EAX = owner; RET at A7B220, length 1 |
| A880E0 | Complete existing auxiliary registration sequence plus guard/root-base unwind | ECX = owner; EAX = owner; RET at A88170, length 1 |

This is a C++ exception and storage projection. It does not implement original
SEH/FH3 frames, native allocation identity, or malformed native storage. The
existing base, auxiliary and resource constructor implementations are reused.
The application must retain surviving allocations after failure until its
explicit recovery policy has handled their external resources.

## Constructor frame and caller

The only current direct caller is A88770's call at 0073DAFD within 0073D410.
0073DAC8 pushes 178h; 0073DAD3 calls BF681B and 0073DAD8 cleans four bytes.
On a nonzero allocation, the caller pushes two EDI words and the EDX word whose
low byte was set by `SETZ DL` after comparing global F889A4 with zero. EDI's
zero value is established in the caller's prologue. ECX receives the allocation.
Only the disabled byte is meaningful; the rest of that first word is not a
recovered Boolean representation. A88798 reads that byte at `[ESP+34h]` and
A887B1 stores its negation at owner+70h. RET 0Ch proves the three-word stack
contract. No constructor argument is an FMOD speaker-mode enum.

The constructor saves its owner at `[ESP+14h]` before calling A81480. The FH3
unwind funclet accesses the same owner at `[EBP-1Ch]`; the EH frame EBP is the
saved exception-registration frame plus 0Ch. Its temporary allocation word at
`[EBP+4]` aliases the first caller argument, overwritten at A887BC and A88A6D.
After saving EDI at A887D6, the speaker-mode scratch word `[ESP+38h]` aliases
that same first argument slot. This alias explains the misleading decompiler
prototype; it is not evidence for an input speaker mode.

## Native exception states

A88772 installs handler CB62BE. Its bytes are `B8 78 C4 DE 00 E9 7B 08 F4 FF`:
load FuncInfo DEC478, then tail-jump to BF6B44. The final instruction begins
CB62C3, length 5, ends CB62C7. FuncInfo starts with magic 19930522h, maxState 3,
and unwind-map pointer DEC460. No catch table is present.

| State | Becomes active | Next state | Action |
|---|---|---|---|
| -1 | Entry and while A81480 executes | None | No A88770-owned cleanup |
| 0 | A887A1, after A81480 returned | -1 | CB62A0 loads saved owner; CB62A3 tail-jumps A816B0 |
| 1 | A887C2, after auxiliary allocation returned | 0 | CB62A8 loads temporary allocation; CB62AC calls BF65AC; POP ECX at CB62B1; RET at CB62B2 |
| 2 | A88A73, after resource allocation returned | 0 | CB62B3 loads temporary allocation; CB62B7 calls BF65AC; POP ECX at CB62BC; RET at CB62BD |

State 1 returns to state 0 at A887D9 after successful auxiliary construction.
State 2 returns to state 0 at A88A86 before publishing resource+54 at A88A8B.
The clock, complete FMOD stage and Lua configuration all run with state 0.
The outer C++ guard is armed only after A81480 returns. Its noexcept destructor
matches termination on a second exception while base cleanup is unwinding.

The two allocation funclets are distinct from their caller A88770. Their
BF65AC calls are ordinary calls, not tail calls. CB62A3 is a genuine EH tail
jump. Report fields distinguish these cases even while Ghidra's historical
false no-return information truncates funclets after BF65AC.

## Allocation and nested construction

BF681B is the throwing allocator: BF6833 calls BF9F1A; a null result goes
through the new-handler path C055B1. If that declines retry, BF687F calls
BF6885 with throw information E03CC0 and then has an INT3 at BF6884. BF6885
calls RaiseException through CE2280. Ordinary return at BF683D..BF683E follows
a nonzero allocation. The C++ factories consequently retain their throwing
allocation contract; artificial null-return host factories were not added.

The native defensive null branches are nevertheless explicit: A887C7 skips
the auxiliary constructor but still reorders the manager after the current
F8BBE8; A88A77 skips the resource constructor, A88A82 zeros EAX, and +54 is
then assigned zero before Lua initialization. These branches are not separately
projected by `std::make_unique`, nor claimed as allocation-failure fixtures.

The 10h auxiliary owner is allocated at A887B4; A88650 is called at A887CB.
Its own constructor registers the singleton through A880E0, then allocates the
tree head through A88270. DEC408/DEC400 invokes A88180 if head construction
throws after registration. The existing factory performs that unregister,
then releases its partial C++ allocation; A88770 next unwinds the sound base.
If registration itself throws, the global auxiliary word may point to the
now-freed partial allocation, just as native EH leaves the published word.

The 18h resource owner is allocated at A88A65; A858F0 is called at A88A7B.
Its DEC080/DEC068 unwind releases the temporary pooled path and options before
base-cache A85500. The existing factory implements these steps through its
required resource host, and frees the partial C++ allocation afterward.
It has not assigned owner+54 unless the complete factory returns.

## What survives a later exception

A816B0 destroys DSP pointer storage, configured group storage, the embedded
listener owner, classes, entries, reverse pointer80 and pointer74 arrays, types,
then singleton base A7B230. Existing shutdown code implements this ordering.
It does not call A7F560 or A7E240, free field174, destroy resource+54, release
EventSystem+48, or unregister the completed auxiliary singleton F8BBE8.

Thus an exception from the clock leaves the completed auxiliary registered.
An exception during FMOD startup leaves any written +44/+48 handles intact.
An exception during Lua configuration also leaves the assigned +54 resource
owner intact. Base-owned configuration storage is still destroyed. C++ owner
storage must not be discarded before an explicit host recovery policy deals
with its surviving external objects; running A882C0 after this base unwind
would destroy the base twice and is not native constructor behavior.

The shutdown context must use the same lifetime domain, global slots, entries,
class ownership, string storage and retained-reference services as startup.
No second lifetime domain or placeholder destructor is created by the wrapper.

## Registration can fail before the outer guard is armed

A7B190 uses handler CB4FE0, FuncInfo DEAEEC, map DEAEDC. State 1 first calls
the captured guard destructor through CB4FD8/CB4FDB -> 411EE0; state 0 then
uses CB4FD0/CB4FD3 -> 412430. A880E0 has the same sequence via handler CB6210,
FuncInfo DEC37C, map DEC36C, guard CB6208/CB620B, and base CB6200/CB6203.
411EE0 decrements/leaves the captured critical section; 412430 only writes
the root vtable CE3818. Both helpers now reset that profile after their scoped
guard unwinds and rethrow. They do not clear their published global word or
undo a registration. A81480 has not completed when A7B190 throws, so the outer
startup guard does not invoke A816B0. This native leftover publication is not
evidence that registration or the full owner constructor completed.

## Enabled FMOD branch audit

The existing fragment's enabled branch agrees with the inspected assembly:
driver count at A8886D; count-down driver indices at A88887..A888BE; speaker
scratch initialized to 1 at A88890; six pushed arguments at A888A6..A888B5
for getDriverCaps; unchecked caps-call return; surviving frequency and speaker
outputs from driver 0. QAG FMOD thunks use stdcall cleanup. The two frequency
fields are then overwritten with 1 and 440000 at A888EB/A888F1. There is no
setDriver call. This audit required no fragment source change and supplies no
hardware audio or gameplay validation.

## Verification

See `reports/sound_startup_unwind.json` for checked direct-call sites, exception
funclet boundaries, previous interpretations and verification results. Ghidra
was read only in this packet; the integrator owns annotation/body repairs.

`scripts/build.ps1` passed Win32 Release and both existing CTests after disabling
MSBuild node reuse; shared reused nodes had failed to write this worktree's
build logs. Seed bytes matched the installed image. The report checker accepted
14 direct-call rows, with zero failures, and one separately marked clock virtual.
The local `sound_startup_unwind_probe.cpp` injects one diagnostic exception
scenario into both registration helpers. Both preserve the published word,
restore the captured lock recursion and CE3818, and propagate the same exception.
The probe links the actual reconstructed archive and has an embedded manifest.
This worker did not run an outer-constructor failure fixture, native EH
differential test, enabled hardware audio, or gameplay; installed failure-path
composition belongs to the application runtime integration packet.
