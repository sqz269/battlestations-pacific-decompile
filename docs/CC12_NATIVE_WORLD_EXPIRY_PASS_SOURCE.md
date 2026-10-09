# Actual World expiry pass Source at 00903610

This packet supplies the separately authorized actual-pointer Source for
`00903610..0090366C`: 93 bytes and 42 native instructions, with nine internal
branches. The guarded MSVC Win32 naked `__fastcall` entry is
`bsp::release_native_world_expired_objects_00903610(void*)`. Its descriptive name
is a hypothesis, not a recovered symbol. The accepted body audit is
`reports/cc12_world_activation_service_readiness.json`.

The former direct-service prerequisite is now closed by the qualified actual
`retire_native_world_child_subtree_009035e0` implementation. Its primary report,
`reports/cc12_native_world_child_retirement_primary_review.json`, records the
complete 42-byte/16-instruction emitted match, physical self-reference, unique
core-library definition, successful normal Win32 build, and three existing
checks. Current helper source hashes match that report. This packet calls that
physical helper; its actual current virtual slot 0 and hierarchy/lifetime
contracts remain external. This worker does not rerun the prior build evidence.

The current baseline is `a3203d44403369e0415d666943a723ed21c28c78`. The existing
counted `release_expired_world_objects_00903610` projection and its record remain
untouched. No duplicate Original function or byte credit is claimed. This is
written Source pending primary registration, emitted-code review, and build
admission; it is not worker ABI-admitted or game-validated.

## Complete instruction mapping

| Native site | Operation | Preserved purpose |
| --- | --- | --- |
| 00903610 | `PUSH EBX` | Save the caller's EBX. |
| 00903611 | `MOV EBX,ECX` | Capture the actual World for the entire pass. |
| 00903613 | `MOV EAX,[EBX+4]` | Fetch the entry header before saving ESI. |
| 00903616 | `PUSH ESI` | Save the caller's ESI. |
| 00903617 | `MOV ESI,[EAX]` | Fetch the header's first entity. |
| 00903619 | `PUSH EDI` | Save the caller's EDI. |
| 0090361A | `XOR EDI,EDI` | Start with no aging anchor. |
| 0090361C | `TEST ESI,ESI` | Test the entry entity. |
| 0090361E | `JZ 00903669` | Empty chain exits. |
| 00903620 | `MOV EAX,[ESI+6Ch]` | Read the entire counter DWORD. |
| 00903623 | `TEST EAX,EAX` | Set the signed initial gate. |
| 00903625 | `JLE 00903662` | Skip zero/negative values without a store or new anchor. |
| 00903627 | `ADD EAX,1` | Increment modulo 2^32. |
| 0090362A | `CMP EAX,3` | Compare the wrapped value with signed 3. |
| 0090362D | `MOV [ESI+6Ch],EAX` | Store the full DWORD while preserving comparison flags. |
| 00903630 | `JL 00903660` | Take the aging branch when the stored value is signed-below 3. |
| 00903632 | `CMP DWORD [ESI+50h],0` | Test the current child gate. |
| 00903636 | `JZ 00903646` | Zero gate skips child-pointer access. |
| 00903638 | `MOV ECX,[ESI+48h]` | Read the current child pointer for each call. |
| 0090363B | `CALL 009035E0` | Call the qualified actual child-retirement helper. |
| 00903640 | `CMP DWORD [ESI+50h],0` | Reload the parent gate after the complete helper return. |
| 00903644 | `JNZ 00903638` | Repeat using the current head. |
| 00903646 | `MOV EDX,[ESI]` | Fetch the then-current entity vptr. |
| 00903648 | `MOV EAX,[EDX]` | Fetch its then-current slot 0. |
| 0090364A | `PUSH 1` | Pass DWORD 1. |
| 0090364C | `MOV ECX,ESI` | Pass the current entity as receiver. |
| 0090364E | `CALL EAX` | Ordinary call to its actual virtual target. |
| 00903650 | `TEST EDI,EDI` | Test the retained anchor after retirement returns. |
| 00903652 | `JZ 00903659` | No anchor selects a fresh header load. |
| 00903654 | `MOV ESI,[EDI+38h]` | Resume through the current anchor successor. |
| 00903657 | `JMP 00903665` | Test that successor. |
| 00903659 | `MOV ECX,[EBX+4]` | Fetch the same captured World's current header. |
| 0090365C | `MOV ESI,[ECX]` | Fetch that header's current first entity. |
| 0090365E | `JMP 00903665` | Test the new first entity. |
| 00903660 | `MOV EDI,ESI` | Only this aging branch changes the anchor. |
| 00903662 | `MOV ESI,[ESI+38h]` | Skip/age follows the current entity's successor. |
| 00903665 | `TEST ESI,ESI` | Test the selected next entity. |
| 00903667 | `JNZ 00903620` | Continue at its counter read. |
| 00903669 | `POP EDI` | Restore the caller's EDI. |
| 0090366A | `POP ESI` | Restore the caller's ESI. |
| 0090366B | `POP EBX` | Restore the caller's EBX. |
| 0090366C | `RET` | Plain return. |

The accepted contiguous body contains no NOP, alignment LEA, or padding
instruction, so the Source adds none. Its sole direct call opcode is at body
offset 43; the four-byte relative operand begins at offset 44. The native operand
is `A0 FF FF FF` (displacement -96), resolving from `00903640` to `009035E0`.
Primary review must bind the external Source reference to the concrete qualified
helper and compare all 93 bytes, rather than relying only on the declaration.

## Arithmetic, storage, and continuation

The initial signed test skips zero and all negative bit patterns. Positive inputs
are incremented with native wrapping arithmetic and stored before any child or
virtual call. `1` becomes `2` and ages; `2..0x7ffffffe` increments and takes the
release path. `0x7fffffff` becomes `0x80000000`, is stored, and ages because the
subsequent comparison is signed. On a later visit that negative value is skipped.
Keeping the store between CMP and JL preserves the flags on which this depends.
The source therefore makes no signed-C++-overflow assumption.

Only aging updates EDI. A skipped node never becomes the anchor, and a released
node never replaces it. Callback or link changes can cause revisits through the
retained anchor or fresh head, so this body does not promise once-per-pass visits
or universal two-step release timing.

The child gate is the entire DWORD at `+50h`: every nonzero pattern enters the
helper loop, with no null-child guard. The `+48h` pointer is reread for every call,
and the same parent's gate is reloaded after the complete helper return,
including the child's own virtual call and final POP/RET. The entity's current
vptr and slot 0 are fetched only after this child loop finishes.

After its own virtual call, the body never dereferences the released current
entity. With an anchor it reads that anchor's current `+38h`; without one it
reloads `+4h` from the same captured World and reads the resulting current
header's first pointer. It does not reuse the entry header or select a new global
World. The skip/age path retains its ordinary current-entity successor read.

## Qualified ABI and lifetime boundaries

ECX supplies the actual World. Incoming EDX and explicit incoming stack arguments
are not consumed, and the source asserts no typed return value. EBX holds that
World, ESI the current entity, and EDI the aging anchor. All three are saved and
restored by this body. The helper and indirect methods must preserve compatible
callee-saved register behavior. The entity's virtual boundary receives ECX as
receiver, EDX as its table, EAX as its target, and DWORD 1 on the stack. Its target
must consume four argument bytes because this caller performs no stack cleanup.

The captured World and every reached header must remain readable for their loads.
The parent must survive counter storage, child returns, repeated gate/head reads,
and late own dispatch. A retained anchor must survive retirement callbacks through
its later successor read. Retirement may invalidate the current entity only as
the external object contracts permit; no blanket deallocation permission follows
from the absence of a local post-call receiver read. Real hierarchy mutation must
leave coherent count/head/anchor state and provide the desired traversal progress.

Null World/header values fault when dereferenced; a null first entity is an empty
chain. A nonzero gate with a null child is forwarded to the helper, where the
first gate read faults. Cycles or nonprogressing mutations can loop, recurse, or
reach invalid storage. Earlier counter stores and child/virtual effects are not
rolled back. Native EH, unwind, hardware-fault, and runtime equivalence remain
unproven.

The source introduces no Host, callback, default, pointer token, overlay, context,
owner, table, guard, synthetic `-1` counter, unlink/free policy, cleanup,
`noexcept`, or application wiring. There are no local global/FPU accesses or EH
frame. The only local object write is the native full counter store.

## Validation and primary handoff

The report retains the full accepted native span (SHA-256
`469147a3ef98d0e543c4194ce2392ab735ab64e64a49058bf406a52e5befec59`), all 42 source
mappings, nine branch destinations, accepted historical pins, current canonical
pins, current helper primary proof, and exact source/document output hashes.
Static checks cover the complete installed-PE body, contiguous decode, source
schedule, dependency hash agreement, exact four-file scope, JSON, and whitespace.
They do not establish emitted-code identity.

No worker CMake, ledger, Ghidra, build, test, or probe changes occur. Primary
integration must register this source, perform the complete 93-byte emitted and
external-binding review with the complete 42-byte concrete helper proof, build
Win32, and run the relevant existing checks before admission. Full World
construction, actual virtual methods, application wiring, ownership, and gameplay
remain outside this qualified body.
