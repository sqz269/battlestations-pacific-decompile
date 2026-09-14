# Post-effect frame binding and count access (BU)

B4E3D0..B4E40A assigns the actual post-effect receiver's frame target at +08.
B4CC00..B4CC03 returns its current +20 count. The packet proposes two complete
native bodies covering 63 bytes; descriptive names are reconstruction hypotheses.
The source uses the existing frame-owner provider and actual atomic IAT cells.
Independent review and an exact committed combined build are pending in this
source snapshot; the final integration report records their eventual results.

The setter takes ECX receiver and one stacked incoming pointer, preserves ESI,
and returns with RET4. It captures the old +08 pointer before comparing identity.
An identical pointer returns without touching any counter, IAT cell or profile.
For a change it publishes incoming first, then calls current CE221C
InterlockedIncrement on incoming+04 if nonnull. Only after retention returns does
it read current CE2220 and decrement captured old+04, if nonnull. It does not
reload old from the receiver, clear the receiver after release, or roll back
publication when a later source provider throws.

If the decrement returns zero, the native code reads current old vtable+0.
The concrete D5E600 profile resolves BD30E0. That provider rereads the current
profile and dispatches deleting slot +4 with flags1; the slot is B1FCF0. The new
source validates those current selectors and calls the existing complete
`delete_native_frame_target_owner_00b1fcf0` implementation. That provider uses the
same surface context, frame vector and CRT allocation domain as construction.
No extra host reference companion, count or registry is introduced. Other final
zero profiles are outside this concrete provider contract.

Four calls in B529A0 occur at B53FE1, B5400D, B54034 and B54074. Each follows
geometry population and precedes B4DEF0; the current children at parent +30,
+68, +88 and +B0 receive the captured common frame argument. The count getter is
called at B5318C and B53B73; both callers shift its returned DWORD right by one.
The getter returns raw bits without normalization or ownership effects.

The getter uses a naked one-input fastcall leaf, preserving the physical ECX
input, EAX result and RET. The setter's new C++ context and the broader program
do not claim drop-in native ABI. Live actual storage and counters, current
callable atomic targets and shared frame/surface/CRT domains are prerequisites.
There is no local native EH frame in the setter. Source exceptions retain prior
side effects and propagate; native hardware faults and private compiler-frame
aliases are outside the claim.

The installed PE, complete native functions, frame vtable, BD30E0/B1FCF0 bodies
and atomic import identities are recorded in
`reports/native_post_effect_frame_binding_bu.json`. Existing compilation and
generated-code inspection are the planned checks. No new test or native fixture
is introduced for this routine assignment. Setter execution, final-zero runtime
disposal, injected failures, SEH and gameplay remain unvalidated here.

## Primary integration at7da09ce9

The combined library passes strict MSVC Win32 compilation, eight native seeds, both CTests and32 numeric call rows. Fresh Ghidra and installed-PE captures match52 spans/12295 bytes. Independent review of all1125 native body lines resolved descriptor count/base capture, world-flag versus width ordering, and the live-one read between decoded component1 FLD/FSTP. Seventeen names/comments were saved and read back, including four new descriptive names; previous comments were preserved.

The existing full original/source fixture compares both4096-byte banks in fallback and optimized skinning variants. It executes27 original bodies/6704 bytes, with shared source B48CE0 and host memmove/invalid-parameter boundaries. Actual texture-source providers and source retained-failure/replay checks also pass. Nonempty renderer/shadow binding, full compiler/pass/debug parents, private ABI/FH3/SEH and game execution remain unvalidated. Worker source6336f486 remains historical; final corrected source is7da09ce9.

Immutable checkpoint: local/checkpoints/7da09ce9/native-material-constant-build-wave/validation.json; SHA256 40e2d9986cf8c3e53cf84d715a1d9aa87081c7df36d8dde6639c4359e8e69b76; 3516 artifacts and7 actual mapped runtime modules. It retains source/build/probe/native-byte/annotation/review inputs and outputs, compiler dependencies, searched libraries and tool/runtime closure.
