# Conditional five-word delivery through the CRT zero arm

This packet composes the historical `00BF6713..00BF6722` outer schedule with
the historical `00BF66EF` zero arm and the admitted `00C04EF3` local body.
It is a symbolic success contract, not an execution, simulation, recovered ABI
or joint fresh caller/callee capture. The baseline is
`605312f9ae9b71a80d4a9406c3234f0a2f08c113`. The machine-readable companion is
`reports/cc12_noinfo_five_word_zero_arm_composition_01.json`.

## Coordinates and applicable execution

Use ordinary legacy x86 with default 32-bit operands/addresses and **SS.B=1**.
Let `U` be outer entry ESP and `S=U-24` be entry ESP at `00BF66EF`.
ESP arithmetic below is modulo 2^32, with every complete four-byte access
independently valid. Ordinary RAM, stable applicable segment/backing mappings,
valid target checks, applicable instruction bytes and normal completion are
conditions, not observations. No fault, interference, MMIO, self-modifying-code
or additional control-protection behavior is modeled. In particular, the leaf
write must leave its following RET and later reached caller code applicable.
The SS.B=0 case is outside these full-ESP equations; it needs separate SP
wrapping and mixed ESP-instruction analysis.

For a listing coordinate `L`, write
`rho(L)=(phi(L)-CS.base) mod 2^32`, where `phi` is an applicable but unobserved
listing-to-runtime-linear mapping. Each transfer needs a compatible mapping.
A CALL pushes a CPU offset such as `rho(00BF670C)`, not an authenticated
numeric listing address.

## Ordered frame construction and conditional continuation

| Stage | ESP / stack effect |
|---|---|
| `00BF6713` XOR EAX,EAX, then five PUSHes | Writes zero in order at `U-4,-8,-12,-16,-20`, establishing argument slots A5 through A1 under compatible ordinary frame conditions. |
| `00BF671A` CALL `00BF66EF` | Writes `rho(00BF671F)` at `S=U-24`. A1..A5 reside at `S+4,+8,+12,+16,+20`. |
| PUSH EBP; MOV EBP,ESP | Saves the incoming EBP word at `S-4=U-28`. |
| PUSH DS:[0109DD64]; CALL `00C04FDE` | Reads the then-current global operand after preceding stack writes; pushes it at `S-8`. CALL writes `rho(00BF66FD)` at `S-12=U-36`. |
| Compatible first-provider return, TEST, POP, zero branch | Requires arrival at `rho(00BF66FD)` with ESP `S-8`; provider outputs/mutations remain arbitrary. Select returned EAX `H=0`. POP reads the current `S-8` word and leaves ESP `S-4`. |
| `00BF6705` PUSH 2; `00BF6707` CALL `00C04EF3` | Writes literal 2 at `S-8`; writes `rho(00BF670C)` at `S-12=U-36`. Define `M2` after both writes. |
| Admitted AND, then RET | Reads DS:[0109EEA8], zeros its four backing bytes, then reads the resulting return word at `S-12`. Normal RET leaves ESP `S-8`; reaching `rho(00BF670C)` is a separate condition. |
| `00BF670C` POP ECX; POP EBP; JMP `00BF65BB` | On that nominated continuation, reads the updated words at `S-8` and `S-4`, leaves ESP `S`, then transfers without a new return word. |

The first provider can alter the original zero arguments, saved EBP and return
words. A compatible return does not silently preserve them. Likewise, the
separate literal-2 slot at `S-8` is not argument A2 at `S+8`. The deepest
return slot written by these owned CALL instructions is `U-36`; unopened
providers' private stack depth is unknown.

## Exact byte predicates

Let `M2` be ordinary-RAM bytes immediately before the leaf AND, including all
earlier provider and prefix effects. Let
`W={backing(DS,0109EEA8+i): i=0..3}` and
`r(L,j)=backing(SS,L+j)`. The normally completed leaf write yields
`M3[b]=0` for `b in W`, otherwise `M3[b]=M2[b]`. Thus the post-write word is

`V3(L)=sum(256^j * (0 if r(L,j) in W else M2[r(L,j)]), j=0..3)`.

The leaf reads the old data operand before this write; RET and the following
POPs read after it. This equation describes the completed write, not its
atomicity or byte-store order. Backing identity controls aliasing; numerical
DS/SS offset equality is neither required nor sufficient.

* `Preserve(L)`: the word equals its pre-AND value exactly when every overlapping
  pre-AND byte was already zero. Disjointness is sufficient, not necessary.
* `Match(L,K)`: every resulting byte equals the corresponding byte of `K`.
  This is exactly `V3(L)=K`.
* `Zero(L)`: every **nonoverlapping** pre-AND byte is zero. This is exactly
  `V3(L)=0`; overlapping bytes become zero even if they were previously nonzero.

These predicates do not assume disjoint data/stack operands or freeze stack
words into register snapshots. They require the stated stable mapping and
normal-completion interval. A distinct live stack frame is one sufficient
prefix-establishment condition; where a prefix write affects another slot,
the general equations use the actual later `M2` bytes.

| Word | Offset | Required value and limitation |
|---|---|---|
| Five arguments | `S+4*i`, `i=1..5` | All five `Zero(S+4*i)` conditions plus actual arrival at the tail give five zero slot words there. Initial zero pushes alone do not establish the late conditions. If the M2 words are zero, the leaf preserves zeros even when it overlaps them; disjointness preserves any existing mutations too. |
| Native36 caller return | `S=U-24` | `Match(S,rho(00BF671F))` establishes the word at intermediate tail entry only. A sufficient condition is `V2(S)=rho(00BF671F)` together with `Preserve(S)`. The final target may later change or ignore it. |
| Saved EBP | `S-4=U-28` | `Match(S-4,B_initial)` makes POP EBP restore its saved original value; otherwise POP reads the changed word. |
| Literal-2 child argument | `S-8=U-32` | `Match(S-8,2)` makes POP ECX load 2. When the pre-AND word is 2, low-byte overlap changes it to zero; overlap limited to its already-zero upper bytes preserves it. |
| Leaf return | `S-12=U-36` | `Match(S-12,rho(00BF670C))`, valid ordinary RET and target mapping gate the entire nominated POP/POP/JMP suffix. A different valid popped offset has a different continuation. |
| Outer caller return | `U=S+24` | `Match(U,R_outer_initial)` preserves the original outer return through the leaf. Separate later preservation is needed through final-target behavior. |

## Final-target boundary

On actual entry to the intermediate zero tail after normal leaf/suffix
completion, EAX is zero; CF=OF=SF=0, ZF=PF=1, and AF is undefined.
The leaf AND supplies those arithmetic flags. The following ordinary RET,
POPs and JMP do not modify them. ECX and EBP come from the late memory reads.
This neither recovers a return type nor constrains arbitrary final-target
execution.

Only if the final target reaches `rho(00BF671F)` normally with compatible
mapping/frame and ESP `P` does the outer ADD compute `P+20`, after which RET
reads the then-current word there. For `P=S+4=U-20`, ADD reaches `U` and
normal RET leaves ESP `U+4`. A compatible plain return using the tail's
`SS:S` slot supplies that P; it is not guaranteed. Reaching the original
outer caller additionally requires the late word at U to remain
`R_outer_initial` and a valid target. Final EAX is arbitrary target residue;
the ADD supplies its own arithmetic flags. No nonreturn, termination,
Watson/handler policy, Original ABI or Source UCRT equivalence follows.

## Evidence and scope

The independent candidate was closed before Root's review criteria or peer
material were read: 13908 bytes, SHA-256
`ca45fbfc20153455fa0398fc0f0d9d6daeebac0224b244f4c550edd677881e9b`.
Peer review found that original CALL establishment plus leaf preservation
omits the arbitrary interval before M2. Named `semantic_refinement_01`
replaces that sufficient premise with the expected pre-AND word together
with leaf preservation. The exact Match predicate and all other candidate
values remain unchanged; original candidate and first publication records
are retained.

Six canonical artifacts and the Root freeze receipt were retained without
expanding their references. A separate Root qualification records that earlier
document reads incidentally displayed an embedded historical member body and
Source/library prose. No Source filepath was opened, no Source census was made,
and no implementation or equivalence credit is taken from that display. The
original freeze was preserved.

This packet performs no Native/context/provider/target query, decoder run,
simulation, test, build, probe, runtime action, GPR mutation, C++/CMake/ledger
edit, archive creation or old-helper rerun. Packaging only binds retained
bytes and the symbolic contract. The caller schedules remain historical; the
separately admitted leaf does not make the composition jointly fresh or
game-validated.
