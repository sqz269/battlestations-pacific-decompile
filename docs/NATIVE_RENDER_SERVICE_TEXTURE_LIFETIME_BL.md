# Native render-service texture child lifetime

Addresses: B52270, B52400, B52840. These are the actual CCh child's release,
destructor and scalar deleting wrapper. Source control flow is complete for the
stated concrete profile domains; usable nonzero auxiliary composition depends
on the separately reconstructed post-effect owner terminal and registration.
No arbitrary callback, zero-only fallback or substitute object type is supplied.

| Entry | Inclusive body | Bytes / instructions | Original ABI | Coverage |
| --- | --- | --- | --- | --- |
| B52270 | B52270..B522FE | 143 / 64 | ECX owner, no stack args, RET | Complete ordered release control flow; D61EC8 concrete terminal dependency |
| B52400 | B52400..B52543 | 324 / 116 | ECX owner, no stack args, RET | Complete normal flow and five-state C++ cleanup projection in admitted domains |
| B52840 | B52840..B5285D | 30 / 11 | ECX owner, DWORD flags, EAX captured owner, RET4 | Complete flags/free flow; destructor dependencies apply |

The saved B52400 body and B52840 listing were repaired by the integrator, not
this worker. The continuous B524CC..B52543 tail is **120 bytes inclusive**;
the earlier BL analysis's 118 was a counting error. B52855..B52857 is ADD ESP,4.
Current live flow reports 64/116/11 instructions with zero gaps. BSP CLI verifies
the existing bsp.gpr /battlestationspacific.exe target; original PE is untouched.

B52270 captures the callable CE2220 cell at B52271, then visits current child
fields +30,+68,+88,+B0. B52400 writes D62074 and arms state4 before that call.
After it returns, B52433 captures +18 **before** B52436 captures a fresh CE2220
target. That target serves +18,+3C,+70,+98. Each nonnull captured owner decrements
captured+04; only a zero result permits current profile/slot0 inspection. BD30E0
reloads the current profile and slot4 and passes flags1 without another decrement.
The canonical companion borrows that same count and performs the concrete
terminal. The parent field clears only after return, overwriting callback writes
to that same field; null fields are not written. Later fields are freshly read.

Normal vector destruction is +A4 (24h records), +8C (0Ch), +7C (24h), +24 (24h).
Each state is disarmed before resize0 and the current pointer's shared CRT free.
Pointer/capacity are not cleared. BD30F0 finally restores CEB130. The unwind map
DF8AB8, described by FuncInfo DF8A94 and handler CBFF6A, is:

| State | Next | Funclet | Cleanup |
| --- | --- | --- | --- |
| 4 | 3 | CBFF5C | B523C0 child+A4 |
| 3 | 2 | CBFF4E | B523E0 child+8C |
| 2 | 1 | CBFF43 | B523C0 child+7C |
| 1 | 0 | CBFF38 | B523C0 child+24 |
| 0 | -1 | CBFF30 | BD30F0 base |

An earlier texture/auxiliary failure does not release later raw references.
B52840 calls destruction first and frees the captured CCh allocation through
BF65AC only when flags bit0 and destruction returns. Other flag bits do nothing.
The returned address may already be freed. Cleanup is a C++ projection; native
FH3/SEH identity and fault behavior are not claimed.

## Recovered nonzero auxiliary producers

These fields hold 36-byte (0x24) post-effect owners created by B4E840, whose original
ECX is the allocated receiver, three DWORD arguments are callee-popped (RET0C),
and EAX returns that receiver. B4E87F stamps D61EC8. Its two-word table contains
BD30E0/B4E450; B4E450 calls B4E2F0 and conditionally frees. They are not D61948
textures. B52550's four zero writes establish only the initial state.

| Field | Producer | Constructor call | Publication |
| --- | --- | --- | --- |
| +68 | B52860, fixed two-unit setup | B528D2 | B528DE |
| +30 | B529A0, positive changed parameter | B52B2A | B52B38 |
| +88 | B529A0, changed generated record count | B52DC1 | B52DCE |
| +B0 | B529A0, positive changed parameter | B52F39 | B52F47 |

B52860 preserves its receiver in ESI. B529A0 sets EBP from ECX at B529BC and
never rewrites it before the final POP; its three publications use that receiver.
All four allocations request 24h and preserve the native null result branch.
Both methods receive current service+34 at callers B120B1/B120B4 and
B16E1D/B16E22. This packet recovers producer identities, not those large bodies.

The missing concrete terminal dependency is B4E2F0..B4E3C8 (217 bytes) and
B4E450..B4E46D (30 bytes). It releases +18 stream, +14 material, unlinks +0C/+10
nodes, frees +1C, releases +08 frame target and restores the base. Its native
base unwind state and current CE2220 epoch belong to the integrator's separate
post-effect provider. B4E385..B4E38A includes ADD ESP,4 and clearing +1C after
free; the old pseudocode's early return was false. B4E840 source construction
and later B529A0 behavior remain separate implementation dependencies.

The integrator's final `native_post_effect_owner.hpp/.cpp` now supplies that
terminal and its canonical companion; its hashes and independent composition
review are retained in this packet's report. It is a separate integration
dependency, not compiled into this worker branch. The reviewed header uses the
same Win32 LONG IAT function-pointer type as BL. Its 36-byte owner still needs an
already-live atomic from a future source B4E840 producer; no missing constructor
or startup composition is claimed by either lifetime module.

## Canonical ownership and validation boundary

2D loaded textures use existing NativeTextureLoadOwners companions. Nonzero
auxiliaries require the concrete post-effect companion in that SAME canonical
NativeRenderActualOwners registry. BL validates current D61948/D61EC8 slot words
at final zero and checks the resolved companion's exact actual+04 identity.
Current profile tables and the IAT cell are borrowed live views, not snapshots.

The CCh NativeRenderServiceTextureReference is caller-prepared persistent host
storage, bound only after B52550 succeeds and before publication. The companion
itself neither allocates nor retains, initializes or resets the count. It uses transactional
GuiNativeGeometryRegistration; a bind diagnostic leaves the native allocation
caller-owned. Registry binding may allocate host metadata; allocation-free
binding requires separately prepared capacity in that provider. Successful final zero runs B52840, marks host retirement and
unbinds without reading freed storage. External quiescence must precede disposal
of the companion/context/registry. A surviving reference keeps them alive.

The inherited RenderCommandReference terminal is noexcept. A throwing terminal
therefore terminates; it does not provide native exception propagation. Raw
entrypoints expose their separate C++ cleanup projection. Unsupported profiles
are diagnosed, never silently ignored. Concurrent profile mutation, address
reuse before retirement, unrestricted aliasing and arbitrary terminals are
outside the admitted domain. There is no original binary ABI, native differential
fixture, startup wiring or gameplay claim. See the report for current build,
call audit, producer lifetime correction and exact source/dependency pins.

B52550 now begins the actual atomic<int32_t> lifetime by placement construction
with value1 at the original B52578 write. It does not default-initialize the
counter earlier or reset it when binding the companion. Size/alignment are both
statically checked as four bytes. Only MSVC Win32 raw-owner/IAT interoperability
is admitted; no portable alias or binary ABI equivalence is inferred.

Both worker Win32 builds passed; the final build ran its configured
`reconstructed_math` test (1/1 passed). This fresh worktree did not configure
the native differential CTest, so this is not a two-test claim. No test or probe
was added. Both reports pass the mechanical call audit: BL25 and constructor39
rows checked, zero failures; indirect rows retain their explicit evidence.
The corrected B52550 generated function section is byte-identical to its prior
1,141-byte section, with all 35 symbolic relocations unchanged. The sole +04
count1 store remains at object+78; no count0 store is introduced. Extra compiler
template COMDATs make the whole object different; whole-object identity and a
new native fixture are not claimed.
