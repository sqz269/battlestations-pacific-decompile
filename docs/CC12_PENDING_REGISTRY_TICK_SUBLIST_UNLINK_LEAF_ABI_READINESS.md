# CC12 pending-registry tick sublist unlink leaf ABI readiness

This read-only audit owns `00874E60..00874EB2` (83 bytes). It establishes a
two-input, zero-call unlink leaf. The architectural inputs are list address
`L=entry ECX` and node address `N=DWORD[entry ESP+4]`, captured once in EAX.
Both paths reach `RET 4` with EAX still N. No source implementation, recovered
symbol, producer binding, virtual-slot binding, or Original-ABI credit is added.
The descriptive list/node/previous/next labels below are layout hypotheses.

The complete machine-readable evidence, all 29 instructions, 11 basic blocks,
15 edges, and current input hashes are in
[`cc12_pending_registry_tick_sublist_unlink_leaf_ABI_readiness.json`](../reports/cc12_pending_registry_tick_sublist_unlink_leaf_ABI_readiness.json).
The worker baseline is `cc2ce843bc3af96242a0beb901dc33f4b82bc7c2`.

## Target, boundary, and input gate

Project-aware live queries verified `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, x86 little-endian 32-bit, base `00400000`.
The live/snapshot function count is 64,729. The live metadata has 83 bytes,
29 listed instructions, 11 blocks, 15 edges, complexity 6, zero recorded
parameters, and zero resolved calls. The decompiler recognizes thiscall
list/node inputs; the first load, ECX accesses, and `RET 4` establish them
independently of the zero-parameter metadata. There is no incoming hidden
data register, x87 operation, listing gap, or out-of-span branch target.

All 83 live bytes equal the configured original PE's file-backed `.text`
at RVA/file offset `00474E60`. The body SHA-256 is
`a83a36e95a4ff5d860bd6334753ab50c03fc0c7b4c7b5aedaec5f97bd3a04652`;
the original image SHA-256 is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Independent full-span decoding accounts for 29 reachable instructions, five
conditional branches, two internal jumps, no CALL or tail transfer, and one
`RET 4`. There is no padding, inline table, omitted byte, or unreachable span.
Static traversal checks all 13 syntactic entry-to-return paths; some branch
combinations are data-infeasible, so this is an over-approximation, not execution.

Current index caller metadata names `00875960`, `00875B30`, and `00876120`;
all retain `FUN_` names. Their Native bodies were not read. Current exact-address
searches under `include/bsp` and `src` found no source implementation. Neither
adjacency nor these callers assigns a containing object or deletion domain.

## Exact access order

The apparent list has head `+0`, tail `+4`, and count `+8` (minimum touched
extent 0xC); the node has previous `+8` and next `+C` (minimum touched extent
0x10). There is no explicit access to node `+0` or `+4`. Aliased addresses can
make a list/neighbor store touch those bytes, so this is an access description,
not a promise that those bytes can never change.

| Address | Operation and timing |
| --- | --- |
| `874E60` | Capture N from entry stack `+4` into EAX; never reload that argument. |
| `874E64` | Capture P from `[N+8]` into EDX. |
| `874E67..6E` | If P is nonzero, proceed; otherwise read `[N+C]` and proceed if nonzero. |
| `874E70..74` | Only when those two link tests are zero, read `[L+8]`; signed count greater than 1 skips all mutation. |
| `874E76..79` | TEST the captured P; PUSH incoming EDI; JZ uses the TEST flags because PUSH preserves them. |
| `874E7B..81` | P nonzero: freshly read `[N+C]`, then store it to `[P+C]`. |
| `874E83..86` | P zero: freshly read `[N+C]`, then store it to `[L+0]`. |
| `874E88..8D` | After that first write, reload current `[N+C]` into EDX and test it. |
| `874E8F..95` | Current next nonzero: freshly read `[N+8]`, then store it to `[current next+8]`. |
| `874E97..9A` | Current next zero: freshly read `[N+8]`, then store it to `[L+4]`. |
| `874E9D` | Store zero to `[N+C]`. |
| `874EA4` | Store zero to `[N+8]`. |
| `874EAB` | Read/modify/write the then-current `[L+8]` using `ADD dword ptr [ECX+8],-1`. |
| `874EAF..B0` | POP EDI from the current spill slot, then common `RET 4`. |

The first splice can change the fields read by the second splice. Both clears
precede the count read/modify/write; either clear can change that count under
aliasing. Capture P once, preserve each fresh read, and clear `+C` before `+8`.
There is no entry count-zero guard, list-membership check, null check, lock,
allocation, freeing operation, virtual lookup, callback, or loop.

The skip condition is exactly captured P=0, the subsequent `[N+C]` read=0,
and signed `[L+8]>1`. This count range is raw `00000002..7FFFFFFF`.
Counts zero, one, or negative proceed to mutation when both links are zero.
There is no underflow or saturation guard; the count update wraps modulo 2^32.
The skip path performs no field or spill write and never pushes/pops EDI.

## Registers, stack, flags, and alias limits

ECX remains L and EAX remains N at the return instruction on every normal
control-flow path. Incoming EAX and EDX are overwritten before data use.
EBX, ESI, and EBP have no owned write. On the skip path EDX is zero and EDI
is unchanged. On the mutation path final EDX is the next value captured at
`874E88` if nonzero; otherwise it is the previous value read at `874E97`.
Later clears and the count update do not modify that register.

Let S be entry ESP. The mutation path saves EDI at `[S-4]`, restores it from
the current `[S-4]`, and reaches the common return with ESP=S; the skip path
reaches it directly with ESP=S. `RET 4` pops the current return address and
discards one DWORD argument, leaving ESP=S+8. Ordinary EDI preservation
requires field writes not to overwrite its spill. PUSH itself can affect
later list/node reads if their backing aliases `[S-4]`. Field writes can also
overlap the return address or argument slot; a valid normal return additionally
requires intact control-stack memory. The argument slot is never reread after
capture. The static stack result proves instruction deltas, not valid memory
or an intact return address under arbitrary overlap.

On the skip path the final arithmetic flags come from `CMP count,1`.
For c in the signed skip range and r=c-1, CF=OF=SF=ZF=0, PF is even parity
of r's low byte, and AF=1 exactly when `(c & 0xF)==0`.

On the mutation path let k be the raw current count immediately at `874EAB`
after both clears, and r=(k-1) modulo 2^32. The actual ADD gives CF=1 iff
k is nonzero; OF=1 iff k=`80000000`; AF=1 iff `(k & 0xF)!=0`;
ZF=1 iff r=0; SF is r bit 31; PF is even parity of r's low byte.
These carry/auxiliary-carry results differ from SUB or DEC. POP and RET do
not change these flags. Other EFLAGS and exceptional returns are not claimed.

No owned exception handler or hardware-fault recovery is present. Invalid
backing can fault on the first stack/node read or any later access. Earlier
writes need not be undone after a later fault. Stack, object validity,
concurrency, and memory-fault equivalence are not established by this audit.

## Relation to the accepted 100-byte cleanup and remaining scope

The accepted `00874F00..00874F63` cleanup report establishes the same link
splice/reload/clear/count sequence in its inlined mutation phase. It contains
no direct call to this leaf. That cleanup first tests count for zero, captures
the current head as its node, and after either its mutation or signed-skip
path dispatches through that node's current profile slot zero with ECX=node
and stack argument 1; it then checks count again and loops.

This leaf instead receives ECX=list and stack argument=node, performs no
profile access or dispatch, and returns once. Shared `RET 4` does not make
those input roles interchangeable. Instruction-sequence similarity does not
prove compiler provenance or a caller edge. This candidate therefore does
not close the concrete subnode producer/profile/slot-zero deletion target.
The accepted outer `00875490` destructor remains a 32-byte, zero-CALL body
with its sole tail into `00874F00`; its Native body was not reread here.

All 18 direct file pins from the accepted cleanup report and all 44 constructor
source-input pins were rechecked and match at this baseline. The admitted
constructor remains Source evidence for its own node layout and registration
path; it does not establish this leaf's producer or the sublist deletion target.

The leaf's independently recovered raw list/node contract can support a later
bounded Source candidate after primary review and a separate assignment.
This packet changes only this document and its report. It performs no Source,
CMake, ledger, or Ghidra mutation; no build, probe, new test, or runtime run;
and grants no Source, Native, Original-ABI, integration, or game-validation credit.
