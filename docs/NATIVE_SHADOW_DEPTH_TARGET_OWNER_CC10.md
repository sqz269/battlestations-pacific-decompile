# Actual shadow depth target and singleton lifetime

This packet supplies the complete normal source paths for the actual 2Ch
shadow target, including its singleton base, resource creation and destruction.
It borrows the existing real renderer, texture/surface producers, canonical
owner domain and raw singleton manager. A new optional singleton dispatcher
binding recognizes only final D5B5E8 and invokes A900C0. Production startup does
not construct or activate a target.

## Native storage and boundaries

The live project/program remain `C:/Users/sqz269/bsp.gpr` and
`/battlestationspacific.exe`. BSP wrappers verify each analysis batch; this
worker made no Ghidra changes. All captured ranges equal the installed PE.

| Entry | Half-open interval | Bytes | Last instruction |
|---|---|---:|---|
| singleton base construction | [A8A820,A8A8B1) | 145 | A8A8B0 RET, 1 byte |
| singleton base destruction | [A8A8C0,A8A959) | 153 | A8A958 RET, 1 byte |
| extent construction | [A8A980,A8A9A6) | 38 | A8A9A3 RET8, 3 bytes |
| extent destruction | [A8A9D0,A8A9DB) | 11 | A8A9D6 JMP A8A8C0, 5 bytes |
| target construction | [A8FE30,A8FF2D) | 253 | A8FF2A RET8, 3 bytes |
| one-time resource creation | [A8FF30,A8FFE3) | 179 | A8FFE0 RET4, 3 bytes |
| target destruction | [A8FFF0,A900B4) | 196 | A900B3 RET, 1 byte |
| scalar deletion | [A900C0,A900DE) | 30 | A900DB RET4, 3 bytes |

Native ECX is the actual owner. A8A980/A8FE30 receive width then height;
A8FF30 consumes the raw low enable byte, and A900C0 receives deletion flags.
The constructors/scalar return the original address; the scalar address can
already be freed. These C++ entrypoints are not binary ABI replacements.

The actual producer is application function 73D410: at 73DCD3 it pushes 2Ch,
at 73DCE5 calls BF681B, then pushes the same captured ESI dimension twice
before A8FE30 at 73DD04. The byte F8899D selects 800h versus 1000h through
NEG/SBB/AND/ADD. The public constructor accepts its actual argument cells;
the focused fixture deliberately supplies 32x32 for a bounded provider run.

`NativeShadowDepthTargetStorage` is exactly 2Ch. +04/+08 are dimensions, not a
reference counter. A8A980 writes width, D5B558, height and byteE=0 after its
base call. A8FE30 then writes D5B5E8, clears10/14/18/1C, byte20/C/D and24/28,
and probes current renderer+104/B1FF50 and +F8/B21EC0. Caps+28 below200h skips
format queries. Successful DF16 sets24=17h,28=36314644h,E=0; its failure tries
D16, whose success sets24=17h,28=50h,E=1. Both failures preserve the proven
constructor zeros. Padding0F and21..23 remains untouched.

## Actual publications, imports and retained state

`NativeShadowDepthTargetContext` borrows the exact manager01090AA0,
targetF8BBF0 and rendererF8D394 cells, current CE2218/CE2210 import cells,
original renderer/runtime profiles, and the existing runtime texture/getter
and terminal domains. Its admission checks provider/current-cell identities;
it does not create another native registry, pool, reference counter or renderer.
The import cells contain real callable Win32 bindings, not original numeric
addresses or substitute callbacks. Each enter/leave site reads its current
cell only when the native section branch is reached.

A8A820 stamps D5B554, calls real415350, captures that manager's section+10,
enters it and increments its actual +18. It publishes the receiver, calls
415350 again, then reloads CURRENT F8BBF0 for BD0C30 registration. It decrements
and leaves the original captured section. A8A8C0 follows the same first
capture/entry, calls the second getter, reloads CURRENT F8BBF0 for BCFCA0,
clears that publication after removal, releases the original section and
stamps the original receiver CE3818. Receiver equality with current publication
is deliberately not required. A8A9D0 first stamps D5B558 and enters that base
destruction path.

Before each source entry performs native work, the context attaches an
address-stable operation block. Native child acquisition records, saved
dimensions, captured section/imports/child and publication arguments survive
exceptions, including calls made by singleton drain. The dispatcher never
uses a temporary failed operation. Allocation failure before attachment has
no native effects; the caller still owns its previously allocated raw object.

Failed/running operation destruction terminates. No catch or context destructor
releases a retained lock/resource, retries a call or marks failure settled.
Completed blocks contain metadata and can contain consumed/stale raw pointer
values; they do not retain native objects. The context, actual import cells,
all providers and caller-owned backing must survive native retirement and host
quiescence. This intentionally does not claim native failure cleanup.

## Resource creation and terminal behavior

A8FF30 checks attempted20 before consuming enable/extents. If fresh, it captures
height then width, writes the exact enable byte, chooses8x8 when disabled and
clears createdD. After the current capabilities gate it reads color format24
before the current renderer/profile+88. The real B2A070 call uses
`width,height,1,color_format,10h`, publishes its returned owner at10, then
reads that returned owner's current profile+30 and calls real B3FD80(0,0).
The surface result is published at14. Next it reads depth format28 before
reloading renderer/profile+88; B2A070 uses the captured extents and flags100h,
publishes18, and its returned owner supplies the second getter/publication1C.
Only then is createdD=1. Attempted20 becomes1 even when capabilities or
format24 skips creation. A subsequent call performs no enable/resource writes.

Both creation/getter invocations have independent retained acquired records.
The genuine texture/cache/surface counts transfer normally; no extra retain,
blanket nested registration, copied COM extent or fallback result is added.
`view_native_shadow_depth_target` aliases the actual fields for existing
A8FDD0/A8FD90/A8FDB0 adapters. Their resolver must alias the actual produced
texture dimension fields, not snapshots.

A8FFF0 stamps D5B5E8, captures first surface14, then captures CE2220 once.
It processes14,10,1C,18, reading each later field only after the prior release
returns. A nonnull captured owner is decremented at actual+04; only a zero
result invokes current virtual0. Its CURRENT parent field is cleared after
terminal return. A null field is left alone. A8A9D0 follows; A900C0 calls the
actual shared free only after full destruction returns and flags&1 is set.

The new shared zero-terminal adapter reuses the existing render-resource
lifetime dispatch. It requires an already-zero actual owner, performs pure
canonical lookup first and checks the companion's actual count address.
An existing companion retires normally without another decrement. Otherwise
the explicit genuine-producer domain selects current0=BD30E0 and a freshly
loaded +4, verifying profile-family correspondence before the real surface,
holder or runtime texture deleter. It never installs the optional service
`direct_terminals` pointer. Nested direct owners must remain unbound as the
existing raw deleters require; a profile token alone proves no backing lifetime.

The optional singleton binding appends at source binding offset172; existing
members retain their offsets. Only D5B5E8 selects A900C0. Transient D5B554 and
D5B558, a missing binding, and unresolved partially constructed registrations
retain the existing unsupported-drain failure instead of acquiring a fallback.

## Native EH and returning-free listing audit

The original base handler thunks CB63A0/CB63C0 load FuncInfo DEC5C4/DEC5F8 and
jump BF6B43. Their state0 funclets CB6390/CB63B0 tail412430 (reset the receiver's
profile); state1 CB6398/CB63B8 tail411EE0 for the captured guard. Target
constructor/destructor handlers CB65A8/CB65C8 use DEC82C/DEC858; their state0
funclets CB65A0/CB65C0 tailA8A9D0. The six funclets are existing8-byte Ghidra
functions. The four10-byte handler thunks currently have no function marker;
the report gives inclusive ends and final5-byte JMP boundaries.

These are observed original compiler cleanup paths. The new source retains
partial state instead of pretending to execute these FH3 tables, guard
transport, SEH or hardware-fault paths. It does not admit partially stamped
base profiles to ordinary singleton drain.

The A900C0 function extent includes its finalRET4, but its live listing still
has a gap after CALL BF65AC at A900D0: native bytes A900D5..D7 are
`83 C4 04` (ADD ESP,4), with listing resuming at A900D8. The worker left it
unchanged for primary repair after lease release. The read-only flow-property
query was rejected because bridge script execution is disabled; its exact
error is retained, without enabling scripts or changing callee no-return state.

## Validation and remaining composition

Strict MSVC Win32 build and all three existing CTests pass. No tracked test was
added. The one ignored current-application probe uses `/MD /fp:strict` and an
extracted embedded `asInvoker` manifest. It links69 untouched current application
objects plus one rebuilt current GameSingletonHost source with an observer-only
pointer capture, and the three current libraries. The observer reaches the
existing real manager/deletion bindings; it creates no replacement host/domain.
Only `tools/run_game.ps1` launched it.

The probe supplies an address-stable caller-owned publication/import binding,
constructs the genuine2Ch target, verifies untouched padding and actual manager
registration, and creates both real textures/surfaces. The installed device
selected color23/D16=80/policy1; both returned surfaces have actual count2
(caller plus texture cache), and all nested raw identities remain unbound.
Existing actual-field toggle operations and the attempted20 gate pass. The
target remains registered for the real shared manager drain: the new scalar
dispatch clears its publication and completes its retained operation and lock
record. Final device/API COM counts are0/0; two presents, one skipped, exit0.
This demonstrates the scoped normal provider/lifetime path, not rendered shadows.

The report contains26 live-PE blocks and44 transfer rows. The verifier checks20
complete live caller/target transfers plus four missing-handler targets only;
the20 indirect rows are explicitly separate. Root must update the four raw
handler row kinds if later creating their Ghidra functions. Probe, build,
native/EH/gap evidence and preparation inputs are SHA-indexed in the report.

Production app retention/publication/import wiring remains a separate step;
this packet's fixture cell does not activate it. Cold material compiler/cache
invocation, sampler stack residue admission, bloom fourth-DWORD backing,
distortion cleanup preimages and full B107F0 execution remain unchanged.
