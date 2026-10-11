# C04FD5 direct-target entry metadata and first physical instruction

The accepted C1737B body row 9 at `00C174C7` contains `E8 09 DB FE FF`.
Its signed rel32 is `-74999`; `00C174CC + (-74999) = 00C04FD5` modulo 2^32.
Both accepted BODY/PRIMARY physical ranges and selected call rows bind this edge.
The nine named original Root positive artifacts were copied and rechecked against
the accepted PRIMARY pins. The original selected Root manual and fresh decoder
records agree; their earlier C174B8 IMUL flag difference remains recorded as one
prior comparison failure. Neither earlier producer was rerun.

Paired exact-entry typed metadata initially exposed a two-byte instruction at
`00C04FD5`, with independent default and effective FALL_THROUGH records, no flow
override, no explicit targets and fallthrough `00C04FD7`. All five call/jump/
conditional/computed/terminal flags are false. All four listing aliases identify
the same exact `InstructionDB`, length 2, kind instruction, `defined=null`.
The exact/containing owner is `00C04FD5`, saved name `__encoded_null`, with one
complete nine-address range `00C04FD5..00C04FDD`. Saved no-return, thunk and recursive
thunk flags are false, direct thunk target is null. These are saved metadata,
not authenticated library/service/ABI identity or permission to read the body.

Root reviewed the complete original payloads, then admitted only
`00C04FD5..00C04FD6` and context zero at the start. All 197 named Root artifacts
(186 worker preimages, 10 listed top files and the admission control) were copied
and rechecked before the first phase-two Native command. The original gate failure,
minimal retry, three Root inspection failures and separate driver preparation
failure are retained. The seven original failed-gate files include full child
stdout/stderr; Git child streams not written before the original assertion remain
explicitly unavailable.

Paired physical captures are `6A 00`; both original context-zero records say
`PUSH 0x0`, with empty before/after contexts. The closing complete typed payload
matches both initial originals byte for byte. The admitted physical decode is:

| Field | Result |
| --- | --- |
| Address / length | `00C04FD5`, 2 bytes |
| Prefix / opcode | No prefixes; `6A` at offset 0 |
| Immediate | One byte at offset 1: `00`, unsigned 0, signed 0 |
| Effective operand | Sign-extended imm8 to operand32 numeric zero |
| ModRM / SIB / displacement | Absent |
| Arithmetic successor | `00C04FD7`, unqueried |

For this individual instruction, renew the predicate that operand size is 32,
the actual current SS stack-address size is 32, and execution completes normally.
If `S` is ESP immediately before it, ESP becomes `(S-4) mod 2^32`, and four
little-endian zero bytes are written at actual current `SS:((S-4) mod 2^32)`.
Other general registers and arithmetic flags are unchanged on normal completion.
SS descriptor/base/limit, translation, mapping, aliasing, the incoming stack value
and fault outcomes remain unknown. No earlier-call or caller-frame state is assumed.
The numeric zero and conditional stack write establish no parameter, null-pointer,
service, library or ABI contract.

The independently written manual fields and pinned artifact were completed before
the first fresh Capstone 5.0.7 x86-32 execution. All manual physical fields matched
that actual decode on its first attempt. Capstone's ancillary register, flag,
operand-access and group fields are retained separately. Strict archive replay
does not import or execute Capstone.

Eight positive repository files were frozen at
`0253ea72bce6ef303805ff7f23ffd071ff87a25d`, with their working LF bytes, index and
committed blobs equal. Fourteen actual Git objects bind the commit, five selected
trees and eight blobs; `src` and `include` are subtree identities only, with no
Source contents read. Eleven original Native CLI commands passed, with 33 separate
physical producer reads before and 33 after. Original controls reported 64730
functions and typed reads epoch 98. These historical captures make no claim about
later live state. No further Native query is part of final closure.

The 461-member finite evidence ZIP and distinct byte-identical copy both passed
strict replay on their first attempts: exactly one ZIP read, one pinned helper
execution and one exclusive receipt write, with no denied audit event. The JSON
report includes full typed payloads, manual and fresh actual decode, named pins,
original failure history and strict receipts. Final delivery adds the actual
commit/tree/blob objects and original publication/commit streams after this report
is written; final receipts remain separate to avoid circular self-pinning.

Current worker Native/helper/setup failures are zero. One worker inspection
failure and its one display truncation are retained separately from the one
pre-packet routing display truncation. Missing original inspection streams and
preexecution pins are explicitly unavailable, never reconstructed as originals.
Root gate failure 1, Root pre-gate inspection failures 3, Root display truncations 0,
Root separate driver preparation failure 1, and prior accepted Root IMUL comparison
failure 1 retain their own scopes.

The seven bytes `00C04FD7..00C04FDD`, next cursor `00C04FD7`, and owner-end-plus-one
`00C04FDE` remain unqueried. There is no Source reconstruction, census, GPR write,
callee/provider/cell probe, build, test, native execution, OS/audio observation or
runtime validation. This entry packet awaits independent Root final review.
