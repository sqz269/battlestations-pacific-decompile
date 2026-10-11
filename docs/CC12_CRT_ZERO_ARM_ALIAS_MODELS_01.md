# Conditional zero-arm stack alias models

This is an independent paper derivation from six canonical inputs frozen at `605312f9ae9b71a80d4a9406c3234f0a2f08c113`. It adds no Native capture, decoder result, source implementation, executed test, ABI proof, or observed CPU state. Listing labels below identify accepted saved instructions; runtime destinations use the separately conditioned CPU-offset map `rho`.

Let `S` be entry ESP of the saved `00BF66EF` body. A compatible first-provider return and the zero branch place ESP at `S-4` before listing `00BF6705`. The accepted local schedule is `PUSH 2; CALL 00C04EF3; POP ECX; POP EBP; JMP 00BF65BB`. The call's continuation label is `00BF670C`. Assume legacy 32-bit operand/address sizes, `SS.B=1`, successful complete little-endian RAM accesses, stable relevant segments and translations, applicable instruction bytes at all reached fetches, no external/intervening writes, and no extra control-protection effect. Address arithmetic is modulo `2^32`, with every complete access separately valid. These are success-model hypotheses, including the first provider's compatible return and zero EAX, not recovered runtime facts.

## Ordered memory and exact predicates

Use backing-byte identities, not offset equality, to decide aliasing. Each valid ordinary four-byte operand has four distinct byte positions. For a four-byte stack word at offset `x`, write `F(x)[j]=backing(SS,x+j)`, `j=0..3`, and `word(M,F)=sum(256^j*M[F[j]])`. Define:

| Footprint | Stack offset | Role |
| --- | --- | --- |
| `R` | `S-12` | Return operand for the EF3 call |
| `L` | `S-8` | Reused pushed-argument slot, then POP ECX source |
| `B` | `S-4` | Saved-EBP slot, then POP EBP source |
| `C` | `S` | Current caller return word at the final tail |
| `A_k` | `S+4*k`, `k=1..5` | Current outgoing five-word prefix |

Let `N` be memory immediately before `PUSH 2`, `t=rho(00BF670C)`, and `store(M,F,v)` be the ordinary completed four-byte little-endian store. The two writes are ordered:

```
M0 = store(store(N, L, 2), R, t)
W  = {backing(DS, 0x0109EEA8+i) : i=0..3}
M1[b] = 0 if b is in W, otherwise M0[b]
```

`M0` is the EF3-entry memory. The AND first reads its complete four-byte operand and then zeros it; the read, valid access requirements, and lack of an atomicity promise remain part of the model. Using ordered stores avoids assuming that distinct stack offsets protect an earlier literal, saved word, or argument. The countermodels below choose locally distinct stack backing; that extra choice makes `word(M0,L)=2` and leaves `R=t` after the call. More generally, the final store establishes `R=t`, while its alias effects on other footprints must remain in `M0`.

For any footprint `F` and desired value `v`, the exact value predicate after the AND is:

```
P(F,v) iff for every j:
  F[j] in W     => byte_j(v) = 0
  F[j] not in W => M0[F[j]] = byte_j(v)
```

Thus preserving a word's own pre-AND value is equivalent to **every overlapping pre-AND byte already being zero**. Disjointness is sufficient, not necessary. Full footprint coverage forces zero. Partial overlap clears only the affected byte positions. The predicate refers to backing identities and remains applicable when DS and SS offsets differ.

Reaching the intended continuation requires `P(R,t)` as well as valid RET/fetch conditions. Conditional on that return, the two late reads give `ECX=word(M1,L)`, `EBP=word(M1,B)`, and ESP progresses `S-8 -> S-4 -> S`; the direct jump then reaches `rho(00BF65BB)` under the compatible code mapping. Consequently:

- ECX is two exactly when `P(L,2)`. If the pushed literal survives the following CALL, this reduces to its low byte `02` not being cleared; its three high zero bytes may overlap harmlessly.
- EBP equals a desired earlier value `b` exactly when `P(B,b)`. Preservation relative to the immediate EF3-entry word is weaker than restoration of original function-entry EBP: earlier first-provider or alias writes must also be accounted for through `M0`.
- The caller return word is a desired `c` exactly when `P(C,c)`; each outgoing word is a desired `a_k` exactly when `P(A_k,a_k)`.

The AND supplies `CF=OF=SF=0`, `ZF=PF=1`, and undefined AF. RET, the two POPs and the direct JMP preserve those arithmetic flags in this success model. EAX remains the conditioned zero-arm input zero through these local instructions, without establishing a return type or ABI. RET does not restore EBP; the explicit later POP reads whatever the saved slot currently contains.

## Two small countermodels

Choose `S=0x02001000`, valid flat SS/CS with zero bases, identity listing/linear/CPU mapping for the reached code, ordinary distinct stack backing, and code disjoint from writable stack/data. Let `word(M0,R)=0x00BF670C`, `word(M0,L)=2`, and `word(M0,B)=0x11223344`. Choose valid DS base and limit so DS's clear operand maps as stated. No segment base or stack value here is asserted observed.

| Model | DS mapping choice | EF3 return and following POPs |
| --- | --- | --- |
| Literal changes | `DS.base=(S-8)-0x0109EEA8`, so `W=L` | Return reaches `rho(00BF670C)=0x00BF670C`; ECX becomes `0`; EBP remains `0x11223344`; the tail is reached at ESP=S. |
| Saved EBP changes | `DS.base=(S-4)-0x0109EEA8`, so `W=B` | Return reaches the same continuation; ECX remains `2`; EBP becomes `0`; the tail is reached at ESP=S. |

The return footprint is disjoint from W in both models. Therefore proving a correct EF3 return alone proves neither literal-two cleanup residue nor original EBP restoration. These are paper countermodels, not simulations or game observations. They use different valid segment bases to make distinct numerical DS/SS offsets refer to identical backing bytes.

## Initial zeros and first-provider writes

For the outer `00BF6713` schedule, let `U` be entry ESP and `S=U-24` at its child edge. The five zero pushes occupy `A_1..A_5` at that boundary under the corresponding successful ordered-store conditions. They are initial values, not immutable tail arguments. The first provider and intervening local writes occur before `M0`; their effects must be included rather than replaced with the initial zero snapshot.

After EF3, `word(M1,A_k)=0` exactly when every byte of `A_k` **outside W** was zero in `M0`. Bytes inside W are zero regardless of their prior values. If the first provider leaves a nonzero byte outside W, it survives to the tail; if it changes only bytes later covered by W, the clear can restore that word to zero. The AND cannot turn an already-zero word nonzero, but it is not a universal re-zeroing of all five words. No provider behavior or final tail return/cleanup contract is supplied by this derivation.

The frozen-input receipt is `local/cc12_crt_zero_arm_alias_models_01/input_snapshot.json`; its six inputs include the two readiness documents and the accepted same-epoch/joined-body doc/report pairs. Only their bounded Native schedule/contract sections were used for the reasoning. Two inspection displays were truncated; the first incident also displayed embedded Source-boundary/policy prose, which was not used. Their separate incident record and the exact retained `Root_input_scope_qualification_01.json` preserve that qualification. Root's original `Source_payloads_read:false` denotes no Source-filepath access, not absence of embedded Source bytes. The original accepted receipts and six canonical byte copies remain unchanged.
