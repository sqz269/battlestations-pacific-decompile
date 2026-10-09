# CC12 squadron launch-task cleanup ABI readiness

This read-only audit establishes the complete normal-flow instruction contract of
`007F1E70..007F1ED2`: 99 bytes, 26 instructions, three basic blocks, three block
edges, three direct calls and a plain `RET`. It adds no Source implementation,
Original ABI compatibility, startup or gameplay credit. Baseline:
`a158157ae5379e4a51ded478818528d8559f0092`.

The last owned root-profile store precedes every child call. The member-cleanup
state store writes **one byte**, preserving the current upper 24 state bits. Every
normal path reaches the base cleanup, whose residual EAX becomes this body's
residual EAX. Neither a semantic return value nor a guaranteed returned task
pointer follows from this body.

The companion [report](../reports/cc12_squadron_launch_task_cleanup_ABI_readiness.json)
records all 26 decoded instructions, exact memory widths, both branch arms,
stack offsets, three child boundaries, exact Source excerpts, dependency-pin
replays and the Root Astra whole-body approval. Native scope is this one body;
child bodies, handler, profile/table words, callers and neighboring bytes remain
unopened. Ghidra, Source, CMake and ledgers were not changed.

## Complete byte and control-flow evidence

The existing project `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, was verified before live analysis batches. Live and
snapshot counts both report 64,729 functions. Fresh live bytes/listing equal the
owned Root-approved capture and the complete file-backed original PE span:

| Evidence | Value |
| --- | --- |
| Native body | `007F1E70..007F1ED2`, 99 bytes |
| Exact body SHA-256 | `c87c2a32db8f6b74332559fed6393c4501089bef0af42b1ffec175617c664731` |
| Original image SHA-256 | `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6` |
| PE section / RVA / file offset | `.text` / `003F1E70` / `003F1E70` |
| Normal blocks | `1E70..1EA1` -> `1EA7` if zero, otherwise `1EA2`; `1EA2..1EA6` -> `1EA7`; `1EA7..1ED2` -> return |
| Coverage | All 99 bytes and 26 instruction positions; no padding, undecoded bytes or x87/SSE operations |

The displayed `undefined FUN_007f1e70(void)` is analysis metadata, not a recovered
ABI declaration. The Root Astra gate approved the complete register/control/EH
instruction review before this packet; only its owned `007F1E70` record supplies
Native evidence here.

## Frame, late captures and ordered state writes

Let `S` be entry ESP and `T` entry ECX. The body pushes state `FFFFFFFF` to `S-4`,
handler immediate `00C8F3E3` to `S-8`, then the prior `FS:[0]` value to `S-0Ch`.
It publishes `FS:[0]=S-0Ch`, pushes entry ECX at `S-10h`, and saves incoming ESI
at `S-14h`. `007F1E87` captures ECX into ESI; `007F1E89` rewrites the local-this
word at `S-10h` from ESI. The handler target is unopened.

With the stated child-preservation requirements, active ESP is `S-14h` through
all three call sites:

| Site | Owned operation and ordering |
| --- | --- |
| `007F1E8D` | Store `DWORD[T+0]=00D08AE4`, the sole explicit task-field store. |
| `007F1E93` | Capture `C=current DWORD[ESI+34h]` into ECX after the profile and frame stores. |
| `007F1E96` | `TEST ECX,ECX` selects the optional first child using captured C. |
| `007F1E98` | Store full `DWORD[S-4]=1`; MOV preserves the TEST flags. |
| `007F1EA0` | JZ skips only `007EE620` when captured C is zero. |
| `007F1EA2` | Nonzero arm: call `007EE620` with ECX=captured C, no pushed arguments. |
| `007F1EA7` | Compute ECX=`current ESI+20h` by LEA, before the next state write. |
| `007F1EAA` | Store **BYTE** `[S-4]=0`, retaining the current upper 24 state bits. |
| `007F1EAF` | Call `00653390` with the captured member receiver, no pushed arguments. |
| `007F1EB4` | Copy current ESI to ECX before the full state reset. |
| `007F1EB6` | Store full `DWORD[S-4]=FFFFFFFF`. |
| `007F1EBE` | Call `00875B30` with captured ECX, no pushed arguments. |

The state-one write may alias the endpoint's backing without changing the
already captured ECX or TEST condition. Neither the JZ nor first CALL reloads
Task34. Likewise, the member LEA and base MOV capture their receivers before
their corresponding state stores. LEA performs address calculation and does not
read member storage. The zero branch still performs both later cleanup calls.

The byte write equals a full zero state only if the current upper state bytes
remain zero. The optional first child can change those bytes through aliases;
replacing the byte store with `DWORD0` would lose the observed contract. The
sequence `FFFFFFFF -> DWORD1 -> BYTE0 -> DWORDFFFFFFFF` alone does not recover
the Native handler's cleanup actions.

## Three child boundaries

Each CALL pushes its return address to `S-18h`. With zero explicitly pushed
argument words, each child must return normally to `ESP=S-14h`: consume its own
return address with **zero argument cleanup**. This is a plain-RET-equivalent
requirement, not proof of any unopened child's literal return instruction.

Each child must preserve ESI=T for the intended later receiver calculations, and
EBX/EDI/EBP for the parent's nonvolatile contract. This body saves/restores ESI;
it has no owned EBX/EDI/EBP writes. Code, return-stack, selected storage and FS
frame backing must remain usable. Hidden inputs, reentrancy, side effects,
exceptions and nonnormal control are not established by the call sites.

| Child | Supplied registers / flags | Fresh metadata only |
| --- | --- | --- |
| `007EE620` | ECX=captured nonzero C; ESI=T; EAX=initial prior-FS capture; EDX=incoming EDX. TEST flags: CF=OF=0, AF undefined, ZF=0, SF/PF from C. | 70 bytes, 24 instructions, 8 blocks, 11 edges, call-count metric 1; named callee `BSP_MissionEntity_Kill` at `00926D90`. |
| `00653390` | ECX=captured current ESI+20h. Nonzero arm inherits first-child EAX/EDX/flags; zero arm retains initial EAX/EDX and TEST-zero flags (CF=OF=SF=0, ZF=PF=1, AF undefined). LEA/MOV preserve flags. | 88 bytes, 24 instructions, 3 blocks, 3 edges, call-count metric 2; named callees `00695870` callback-owner destroy and `006952A0` unregister pair. |
| `00875B30` | ECX=captured current ESI; EAX/EDX/flags are `00653390` residuals across the intervening MOVs. | 84 bytes, 30 instructions, 8 blocks, 11 edges, call-count metric 2; named callees `00875280`, `00874E60`, Enter/LeaveCriticalSection. |

Callee names and graph metrics establish no child-body order, conditions, field
offsets, exception policy or register ABI. The internal call-count metric must
not be equated with the count of internal plus imported callee names. In
particular, the first receiver's raw value does not identify an entity class or
establish that the named kill operation is the complete child behavior.

## Normal exit, flags, aliases and faults

`007F1EC3` reads the **current** saved-chain word at `S-0Ch` into ECX before
`007F1EC7` pops ESI from the current `S-14h` word. The pop advances ESP to
`S-10h`; `007F1EC8` writes captured ECX to `FS:[0]`. `007F1ECF` adds `10h` to
ESP, reaching S under the normal stack requirements. The plain RET at `007F1ED2`
reads the current return slot at S and leaves ESP=`S+4`.

Every normal path calls `00875B30`, and the owned body never writes EAX after
the initial FS read. Final EAX is therefore the last child's residual, with no
universal semantic result or guaranteed task pointer. EDX is also last-child
residual; ECX is the late saved-chain capture. Restored ESI comes from the
current save slot, which need not still contain incoming ESI after aliases or
child effects.

Final arithmetic flags come from `ADD ESP,10h`. For pre-ADD 32-bit x and
`r=(x+10h) mod 2^32`: CF is `x>=FFFFFFF0h`; OF is
`7FFFFFF0h<=x<=7FFFFFFFh`; AF is zero; ZF is `r==0`; SF is bit 31 of r; PF is
even parity of r's low byte. Plain RET preserves these flags. Under the normal
frame contract x=`S-10h` and r=S.

Task fields, local/frame state, saved chain, saved ESI, child return slots,
parent return slot, FS storage and child-visible backing can alias. The audit
preserves read/write timing and captured values without assuming universal
disjointness. `00D08AE4` is the last **owned** Task0 store, before all children;
the final root profile is not proved. There is no established task-null check,
noexcept guarantee, rollback, idempotence or reentry safety. Faults or nonnormal
child returns can stop the described normal schedule. Handler `00C8F3E3`, its
funclets and unwind metadata remain unopened.

## Accepted constructor, member and wrapper composition

The accepted [parent136 audit](CC12_SQUADRON_LAUNCH_TASK_CONSTRUCTOR_ABI_READINESS.md)
and [member92 audit](CC12_SQUADRON_LAUNCH_TASK_MEMBER_CONSTRUCTOR_ABI_READINESS.md)
are pinned and reused without reopening their Native bodies. Under the accepted
placement, the member is Task+20h; its member+14h store maps to Task34h. This
cleanup reads that location's **current** word after intervening caller, child,
frame and alias effects. Equality with the original constructor endpoint is
conditional.

The direct Task34h read ends at Task37h, giving selected extent `38h`, consistent
with the accepted combined constructor coverage. The parent/member combination
directly writes 51 distinct bytes, leaving `12h`, `13h`, `31h`, `32h`, `33h`
unwritten. This cleanup explicitly writes only Task0..3; its Task+20h LEA is not
a memory access. None of this proves an allocation size, complete class,
complete initialization, endpoint identity or sufficient backing for children.

The accepted [60-byte scalar-wrapper audit](CC12_SQUADRON_LAUNCH_TASK_SCALAR_DELETION_READINESS.md)
supplies the `007F1EE0` caller boundary without new Native caller reads. If its
entry ESP is P, it calls this body at wrapper ESP=`P-4`; cleanup entry S=`P-8`.
This body's plain RET returns to `S+4=P-4` when children balance. The wrapper
then reads its current low flag byte, optionally frees current ESI, and returns
current ESI with RET4. Wrapper EAX identity is distinct from this cleanup's
residual EAX and does not prove the returned address is live. Aliases can change
the late-read flag or saved-register words.

## Actual Source interfaces and remaining binding work

At the pinned baseline, the exact `src/include/tests` search finds no provider
address match for `007F1E70`, `007EE620` or `00653390`. `00875B30` appears only
as the pure virtual `NativePilotBotTaskOwnerCalls::base_cleanup_00875b30()` and
its invocation in another owner's destruction path. A concurrent Root-owned
base-cleanup candidate is outside this audit and receives no credit here.
Text-search absence does not exclude differently named or generated code.

The two metadata-listed `00653390` children do have actual Source methods on
`NativeObserverLifetime`: `unregister_pair_006952a0(first, callback_owner)` and
`destroy_callback_owner_00695870(owner)`. The former uses the retained lifetime
to lock, locate/invalidate a pair, decrement references and conditionally remove
it. The latter sets its Source profile, queries count under nested locking,
detaches when needed, and frees allocation on normal or caught/rethrown paths.
These concrete Source methods do not implement or recover `00653390`'s complete
Native schedule or unwind behavior.

The retained lifetime requires the actual borrowed manager access, mutable
lock/dispatch cells and services. Existing game runtime/singleton construction
and binding expose that domain; they do not bind this selected launch task to
it. The generic `NativeObserverOwnerStorage` is a Win32 `10h` prefix containing
profile and edge pointer/count/capacity. Raw correspondence with Task20 does
not create the full `18h` member, its Task30 flag, Task34 endpoint, or a register
ABI bridge.

The actual selected Source path remains the `PlaneSquadronRegistry::records_`
vector's typed `PlaneSquadronHostRecord` fields: `launch_task_live`, timer, deck
and send count. Production host construction/tick/termination update those
fields. Seven exact current excerpts preserve that evidence; there is still no
proved raw-task allocation, member registration, cleanup dispatch or endpoint
lifetime binding. Actual CMake registrations for the existing observer/runtime
and other-owner providers are pinned.

Both accepted current Source primary receipts replay **65/65 input pins**. Their
prior successful Win32 build, three existing checks, 11 captured objects and 13
positive Core roots remain scoped Source receipts; the new three roots were
reported absent from the application map. No fresh compiled/library/EH-byte
inspection or runtime execution was performed here. The accepted parent and
member dependency sets differ only at `CMakeLists.txt`, whose exact delta is two
added Source208 registrations. All eight wrapper inputs match: five raw hashes
are equal, and three differences are proven to be LF/CRLF conversion only by
reconstructing each accepted raw hash and size from the current text.

Validation comprises whole-byte equality, complete instruction/stack/CFG
accounting, capture/state-width/tail/flags checks, dependency hashes and 22 exact
Source excerpts. No new build, tests or probes were added for these two audit
files. Actual raw-task and endpoint lifetime, all child contracts, Native unwind
and selected production wiring remain separate reconstruction obligations.
