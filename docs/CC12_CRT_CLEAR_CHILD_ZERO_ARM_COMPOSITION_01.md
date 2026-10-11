# Conditional CRT zero arm with the admitted clear child

When the historical `00BF6705..00BF6712` zero arm actually calls the admitted
`00C04EF3..00C04EFA` body and reaches its recorded continuation, the direct tail
has **EAX=0 and the child AND's arithmetic flags**. ECX and EBP are late reads of
updated stack memory. Reaching that continuation requires an exact return-word
preservation condition, in addition to successful execution and valid fetched
code. This is an offline conditional composition, not a new joint caller/child
capture or runtime observation.

The [report](../reports/cc12_crt_clear_child_zero_arm_composition_01.json)
retains the immutable candidate and finite input identities. The historical
Native36 schedule and the child's recorded epoch-10371 admission remain separate
evidence. The saved target name supplies no debugger purpose, function prototype,
literal-two meaning or runtime failure policy.

## Conditions and initial state

Use legacy non-64-bit x86 with default 32-bit operand/address sizes and **SS.B=1**.
Let S be the Native36 routine-entry ESP. Require the first provider's compatible
return to its recorded cleanup, with EAX H=0, followed by the existing TEST,
POP ECX and taken zero branch. At `00BF6705`, ESP=S-4 and EAX=0. Other registers
and all prior frame/provider memory effects remain arbitrary; EBP is not reset
before the second CALL. In particular, no first-provider preservation contract
is inferred.

Let M0 be ordinary backing-byte memory at this zero-arm entry. Relevant segment
descriptors, translations and backing stay stable, complete accesses and control
transfers succeed, and there are no intervening writers or asynchronous changes.
Each four-byte operand has distinct backing bytes in this valid-word model;
different operands need not be disjoint. Pointer updates are modulo 2^32, but an
operand crossing a limit is not silently wrapped into a valid access.

Every reached instruction must actually fetch/decode its recorded encoding.
The PUSH and CALL stack writes, as well as the AND write, can alias future code
backing. Their effects must leave the stipulated encodings applicable at the
reached fetches. Code/data separation would suffice but was not observed. Faults,
device memory, self-modifying-code visibility and additional enabled
control-protection/shadow-stack effects are outside this ordinary success model.

Listing coordinates remain distinct from CPU offsets. Let phi(L) map a listing
coordinate to its runtime linear code address, and let
`rho(L)=(phi(L)-CS.base) mod 2^32` be its legal CPU offset. The mapping must respect
recorded instruction lengths and relative transfers. In particular, the CALL
requires `rho(00C04EF3)=rho(00BF670C)+59367 mod 2^32`. None of these runtime mapping
premises was checked.

**SS.B=0 is outside this whole-arm derivation.** The child-only 16-bit stack
discussion does not justify substituting SP into the following ESP equations.

## Ordered writes and reads

Define four-byte backing vectors A at SS:S-8, R at SS:S-12, B at SS:S-4 and D at
DS offset `0109EEA8`. The stack starting offsets use the stated modular pointer
arithmetic; all four-byte operands must still be valid in full. Let W be the set
of D's backing bytes, `c=rho(00BF670C)`, and `c_j` its little-endian byte j.
`Load32(M,V)` reads the word from V's four backing bytes; `Store32(M,V,x)` writes
the four bytes of x and leaves other backing unchanged. These are final ordinary
instruction effects, without a byte-store-order or atomicity guarantee.
The abstract alias algebra does not claim that every possible assignment of
backing identities is realizable by x86 address translation.

| Reached listing | Ordered ordinary effect | ESP afterward |
| --- | --- | --- |
| `00BF6705` PUSH 2 | M1=Store32(M0,A,2) | S-8 |
| `00BF6707` CALL child | M2=Store32(M1,R,c); transfer to rho(`00C04EF3`) | S-12 |
| `00C04EF3` AND | Read X=Load32(M2,D), then zero every byte in W, yielding M3 | S-12 |
| `00C04EFA` RET | Read Q=Load32(M3,R); load CPU EIP=Q after checks | S-8 |
| `00BF670C` POP ECX, if reached | ECX=Load32(M3,A) | S-4 |
| `00BF670D` POP EBP, if reached | EBP=Load32(M3,B) | S |
| `00BF670E..00BF6712` direct JMP, if reached | Transfer to rho(`00BF65BB`) | S |

The AND's read remains part of its read/modify/write semantics, even with zero
as its immediate. It reads **after** the two stack stores, so aliases can make X
depend on those stores. There is no LOCK prefix or established atomic RMW
guarantee. The child's `C3` performs no extra stack cleanup or CS pop.

## Exact return-word condition

The CALL is the last pre-AND writer of every R byte and leaves Load32(M2,R)=c.
The AND then changes only return bytes whose backing lies in W:

```text
q_j = 0 if r_j is in W, otherwise c_j
Q   = sum(j=0..3, 256^j * q_j)
Q=c  iff  every j with r_j in W has c_j=0
```

Disjoint return/data backing is sufficient, not necessary. Partial overlap
preserves the offset exactly when every affected return byte was already zero.
Full overlap gives Q=0, which equals c exactly when c=0. A zero offset does not
establish a valid transfer or runtime linear address zero.

This equivalence concerns four-byte offset equality under the model. Successful
return checks and valid continuation fetches remain separate requirements. A
normal RET to Q other than c does not establish the stipulated `00BF670C`
continuation, so the following POPs and direct tail are not claimed in that case.

## State at the direct tail when continuation occurs

The final ordinary state has ESP=S, EAX=0, ECX=Load32(M3,A),
EBP=Load32(M3,B), and EIP=rho(`00BF65BB`). EDX, EBX, ESI and EDI retain their
zero-arm-entry values, which are arbitrary first-provider outputs. Memory is M3:
the RET, POPs and direct JMP add no ordinary RAM store.

EAX stays zero because none of the composed instructions assigns it. This
specializes the historical schedule's unopened second-provider boundary; that
older record remains unchanged. The child AND sets CF=0, OF=0, ZF=1, SF=0 and
PF=1, with AF undefined. The RET, two POPs and JMP preserve those arithmetic
flags. Their match with the earlier zero TEST flags does not define AF or make
the TEST the source of the final flags.

The POP operands are late reads. ECX need not equal two, and EBP need not equal
either its zero-arm-entry value or the original caller's EBP. M2 includes the
ordered stack writes, and the AND may modify either slot through backing alias.
No general caller-frame, saved-register-word or outer return-word preservation
is inferred. If an earlier write succeeds before a later access, return check or
fetch fails, the earlier effects may remain; no transaction or rollback follows.

## Finite evidence and scope

The worktree fast-forwarded only to frozen
`605312f9ae9b71a80d4a9406c3234f0a2f08c113`. Root's freeze receipt and exactly six
selected canonical documents/reports were retained and hash-verified, with no
transitive evidence expansion. The separate Root input-scope qualification is
also retained alone. Native36 reading stopped at lines 1–125; the selected
Native16 document remained opaque and supplies no semantics to this packet.

Historical documents can contain embedded Source text. Opaque canonical
retention is not a Source-absence proof. This worker displayed no Source section
or member body and opened no Source filepath. One joined-contract display
truncation was resolved through a bounded reread and is recorded separately.

The packet holds zero Native addresses and changes only its owned doc/report.
There is no Native, context, provider, target or Ghidra query; decoder import;
Source-file access or census; C++/CMake/ledger change; test, simulation, build,
probe, runtime action, GPR mutation, archive expansion or old-helper execution.
No ABI, startup or gameplay validation is supplied.
