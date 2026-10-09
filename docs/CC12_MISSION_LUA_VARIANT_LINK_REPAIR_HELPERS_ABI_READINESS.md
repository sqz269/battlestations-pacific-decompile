# Raw mission Lua variant link helpers: 006EDC80 and 006EDCD0

Both complete bodies have one ECX receiver, one stack argument, no child calls,
and three `RET 4` exits. Incoming EDX is overwritten before use. Each retains
the captured pivot in EDX and initially selected neighboring node in EAX until
return. EBX/BL, EDI and EBP are untouched; ESI is saved and restored before the
root/parent attachment branch. This locally satisfies the six accepted caller
sites' stack-cleanup and register requirements, subject to valid stack backing
and the memory-alias qualifications below.

The link schedules resemble left and right tree rotations. That description
is provisional: the exact reads, stores and branch tests are the evidence;
valid tree structure, ownership and semantic rotation identity are not proved.

## Scope and fixed evidence

Packet `cc12_lua_variant_link_repair_helpers_ABI_readiness`, baseline
`800db7be0a6e05f5d8f154d8eace5ef1a01bba58`. Only this document and
`reports/cc12_mission_lua_variant_link_repair_helpers_ABI_readiness.json`
are changed. The two complete bodies were individually authorized and leased.
No bytes between or beyond them were read from Ghidra or decoded.

| Entry and saved extent | Bytes / instructions | SHA-256 | Physical returns |
| --- | --- | --- | --- |
| `006EDC80..006EDCCD` | 78 / 30 | `acd8854a9bf9c7b03fb8668432e70075be53ebf6a2f6b453f970bfeefd6f5bc5` | `006EDCAF`, `006EDCC0`, `006EDCCB` |
| `006EDCD0..006EDD21` | 82 / 30 | `948bd60eeef4fbedf3d20796401d881873df045f2bdcb0935ea617572e96b856` | `006EDD00`, `006EDD14`, `006EDD1F` |

All 160 admitted bytes match the configured installed PE, and all 60
independently decoded instructions have saved listing entries. The first
body's RVA/file offset is `002EDC80`; the second's is `002EDCD0`.
Each metadata record reports 30 instructions, seven blocks, seven edges and
zero calls. Each complete instruction CFG is acyclic, reaches all 30 physical
instructions, and has six syntactic paths: two flag outcomes times three
attachment exits. These paths do not prove consistent or valid pointed memory.
Each path performs five or six explicit DWORD data stores plus its stack save.

The existing configured `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, language `x86:LE:32:default`, and image base
`00400000` were reused and checked by the live CLI. Configured GPR existence
and project/program identity do not establish a stronger server-side absolute
path claim. Live and saved function counts are 64,729. The installed PE digest
is `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

Saved prototypes remain `undefined FUN_006edc80(void)` and
`undefined FUN_006edcd0(void)` with parameter count zero. The decompiler's
inferred thiscall form and the register/stack evidence are reported separately;
no prototype, body, flow, no-return flag or AddressSet was repaired. No Native
caller, external child, handler, profile, table, string or adjacent body was
opened. There are no external transfer sites in either admitted body.

## Actual frame and machine contract

Let `S` be entry ESP, `R` entry ECX, and `X` the DWORD initially loaded from
`[S+4]`. Use `a(V)=[V+0]`, `p(V)=[V+4]`, `b(V)=[V+8]`, and
`s(V)=byte[V+31h]`. These are offset roles rather than recovered C++ types.

Both bodies first load X into EDX, then load the selected neighbor Q into EAX.
Only **after both loads** do they push incoming ESI at `S-4`. Thus a fault
in either initial load precedes that save. Their only explicit stack argument
is the original `[S+4]` word; it is never reloaded. Incoming EDX and EAX have
no semantic role: both are replaced before their first owned read. There is
no call through which their old values can escape locally.

While saved, ESI supplies temporary link/parent values; ESP is `S-4`.
The root comparison executes before `POP ESI` at `006EDCA4` / `006EDCF4`.
That POP restores from the **current** save slot, restores ESP=`S`, and
preserves comparison flags for the following conditional branch. All three
terminal arms execute with ESP=`S`; `RET 4` consumes the original return
address and the one argument, leaving ESP=`S+8` on a valid normal return.
There is no local allocation, EBP frame, EH registration, x87 instruction,
indirect branch, callback or external provider dependency.

EBX, EDI and EBP are neither read nor written. In particular, the incoming
full EBX and BL are preserved, including the caller's retained pointer high
bits. ESI preservation requires its save slot to survive earlier data stores.
The initial PUSH itself can affect a subsequent aliased data read: Q is
captured before the PUSH, while the first read through Q occurs afterward.
No disjointness between raw node/receiver memory and the stack is inferred.

## Ordered common schedule

The table gives both physical schedules without replacing repeated memory
accesses with cached values. `T0`, `T1` and `P0` denote separate reads.

| Step | `006EDC80` body | `006EDCD0` body |
| --- | --- | --- |
| Capture pivot | `006EDC80`: EDX=X=`[S+4]` | `006EDCD0`: EDX=X=`[S+4]` |
| Capture Q | `006EDC84`: EAX=Q=`b(X)` | `006EDCD4`: EAX=Q=`a(X)` |
| Save ESI | `006EDC87`: PUSH ESI | `006EDCD6`: PUSH ESI |
| First moved-link read | `006EDC88`: ESI=T0=`a(Q)` | `006EDCD7`: ESI=T0=`b(Q)` |
| First link store | `006EDC8A`: `b(X)=T0` | `006EDCDA`: `a(X)=T0` |
| Moved-link reload | `006EDC8D`: ESI=T1=`a(Q)` | `006EDCDC`: ESI=T1=`b(Q)` |
| Byte test | `006EDC8F`: compare `s(T1)` with zero | `006EDCDF`: compare `s(T1)` with zero |
| Conditional parent store | `006EDC95`: `p(T1)=X`, only if zero | `006EDCE5`: `p(T1)=X`, only if zero |
| Late pivot-parent read | `006EDC98`: ESI=P0=`p(X)` | `006EDCE8`: ESI=P0=`p(X)` |
| Q-parent store | `006EDC9B`: `p(Q)=P0` | `006EDCEB`: `p(Q)=P0` |
| Late receiver-header read | `006EDC9E`: ECX=H=`[R+4]` | `006EDCEE`: ECX=H=`[R+4]` |
| Root comparison | `006EDCA1`: compare X with `[H+4]` | `006EDCF1`: compare X with `[H+4]` |
| Restore ESI | `006EDCA4`: POP ESI | `006EDCF4`: POP ESI |
| Attachment branch | `006EDCA5`: unequal -> `006EDCB2` | `006EDCF5`: unequal -> `006EDD03` |

The nonzero flag branches at `006EDC93` / `006EDCE3` skip only the conditional
parent store and join at the late pivot-parent read. Any nonzero byte skips;
the body does not require one, read a color byte, or treat the flag as a
pointer/null validation. T1 is dereferenced for its flag without a guard.

All initial Q and X register captures survive the subsequent stores. H is
read only after the Q-parent store; its value is retained for the root test
and root store. P0 is read after the conditional T1-parent store. The second
read through Q follows the first pivot-link store. These orders are retained
even if overlapping storage makes later values differ.

## Attachment and final stores: 006EDC80

If the comparison at `006EDCA1` is equal, the root arm executes, in order:

1. `006EDCA7`: `[H+4]=Q` through the captured H; no fresh `[R+4]` read.
2. `006EDCAA`: `a(Q)=X`.
3. `006EDCAC`: `p(X)=Q`.
4. `006EDCAF`: `RET 4`.

If unequal, `006EDCB2` reloads **current** `P1=p(X)` into ECX, after the
earlier `p(Q)=P0` store and the ESI restore. `006EDCB5` compares X with
current `a(P1)`. Equality selects `006EDCB9: a(P1)=Q`, followed by
`006EDCBB: a(Q)=X`, `006EDCBD: p(X)=Q`, and `006EDCC0: RET 4`.
Inequality selects `006EDCC3: b(P1)=Q`, followed by
`006EDCC6: a(Q)=X`, `006EDCC8: p(X)=Q`, and `006EDCCB: RET 4`.
The fallback does not check that `b(P1)` previously equals X.

## Attachment and final stores: 006EDCD0

If the comparison at `006EDCF1` is equal, the root arm executes, in order:

1. `006EDCF7`: `[H+4]=Q` through the captured H; no fresh `[R+4]` read.
2. `006EDCFA`: `b(Q)=X`.
3. `006EDCFD`: `p(X)=Q`.
4. `006EDD00`: `RET 4`.

If unequal, `006EDD03` reloads **current** `P1=p(X)` into ECX.
`006EDD06` compares X with current `b(P1)`. Equality selects
`006EDD0B: b(P1)=Q`, then `006EDD0E: b(Q)=X`,
`006EDD11: p(X)=Q`, and `006EDD14: RET 4`.
Inequality selects `006EDD17: a(P1)=Q`, then
`006EDD19: b(Q)=X`, `006EDD1C: p(X)=Q`, and `006EDD1F: RET 4`.
The fallback does not check that `a(P1)` previously equals X.

## Returns and raw backing

At every local return EAX is the originally captured Q and EDX is the
originally captured X, regardless of later link aliases. ECX is captured H
on root exits, and freshly loaded P1 on either nonroot exit. ESI is the word
already popped before branch selection. The void decompiler form does not
erase these physical register values or imply that EAX has a public semantic
return contract.

Root exits preserve arithmetic flags from the X-vs-`[H+4]` comparison;
nonroot exits preserve flags from X-vs-`a(P1)` / `b(P1)`. POP, MOV and RET
do not replace them. Root and matching-parent exits therefore return with
ZF=1, CF=0 and OF=0; fallback exits have ZF=0 and the other arithmetic flags
of the actual DWORD subtraction. The earlier flag-byte comparison does not
survive the unconditional root comparison. No incoming condition flags are
used and no Original-to-Source flags equivalence is claimed.

| Raw role | First body accesses | Second body accesses | Address span where used |
| --- | --- | --- | --- |
| Receiver R | Read DWORD `+4` | Read DWORD `+4` | At least 8 bytes; no `R+0` or `R+8` access |
| Captured header H | Read and conditionally write DWORD `+4` | Same | At least 8 bytes |
| Pivot X | Read/write DWORD `+8`, read/write DWORD `+4` | Read/write DWORD `+0`, read/write DWORD `+4` | 12 bytes first body; 8 second |
| Captured Q | Read/write DWORD `+0`, write DWORD `+4` | Read/write DWORD `+8`, write DWORD `+4` | 8 bytes first body; 12 second |
| Reloaded moved node T1 | Read byte `+31h`; write DWORD `+4` only for zero flag | Same | 50-byte highest-address span; not a proved object/allocation size |
| Reloaded parent P1 | Read DWORD `+0`; write `+0` or `+8` | Read DWORD `+8`; write `+8` or `+0` | Up to 12 bytes, path-dependent |

These are directly used offsets, not implicit C++ object construction or a
requirement that all roles be distinct. No null, alignment, capacity, parent
membership, allocation, lifetime or ownership check is supplied. There is
no allocation or release and no direct node flag/color store. Both bodies
are finite locally, but reaching RET still requires every selected memory
access and the current return slot to remain usable.

A data store can overlap a subsequent link, header or parent read. It may
also overlap the ESI save slot before its POP, or the return/argument area.
Restoring ESI before final attachment stores means later writes to its former
save slot cannot change that already restored register, while writes to the
current return slot can still affect RET. The pivot argument is captured
only once; later argument-slot changes are not reloaded as X. Earlier
completed stores remain after a later fault; no transactional rollback,
exception cleanup, atomicity or all-alias behavior guarantee is introduced.

## Accepted caller contract and evidence boundary

Only the accepted `006EE5D0` report was read. Its function evidence remains
**637 saved bytes**, a separately authorized **54-byte continuation
candidate**, and **13 trailing INT3 bytes**. The combined 691-byte / 234-op
candidate excludes those INT3 bytes and is not a changed Ghidra function
extent. This packet does not reopen any of those Native bytes or revise
that classification.

| Accepted call site | Helper | Argument word | Caller continuation |
| --- | --- | --- | --- |
| `006EE773` | `006EDC80` | P | Reload W from `b(P)` |
| `006EE7A2` | `006EDCD0` | W | Reload W from `b(P)` |
| `006EE7BC` | `006EDC80` | P | Continue to final Y color store |
| `006EE7D3` | `006EDCD0` | P | Reload W from `a(P)` |
| `006EE817` | `006EDC80` | W | Reload W from `a(P)` |
| `006EE82F` | `006EDCD0` | P | Continue to final Y color store |

Every site supplies ECX=R and one pushed word. The three RET4 exits in each
helper confirm the local cleanup requirement. The caller retains R in EBP,
P in ESI, Y in EDI and its byte constant in BL; helper machine behavior
supports those requirements under the save-slot qualification. Child EAX
does not supply the accepted caller's semantic result at these sites.
Caller color stores occur around argument preparation; arbitrary aliases
can change the actual pushed word before CALL. The helper therefore uses
the actual `[entryESP+4]` value, not a presumed earlier P/W value. No whole
repair-loop, allocator, destructor, runtime or gameplay equivalence follows.

## Source readiness and replayed inputs

There is no exact admitted Source implementation of either helper at the
fixed baseline. Their complete local schedules need no external child
provider, but a Source admission still needs a deliberately chosen raw
receiver/stack/register interface, actual backing and alias contracts, and
separate emitted-code/caller review. No implementation or ABI bridge is
created here. No recovered symbol name or tree type is asserted.

All 82 inherited canonical pins replay at their recorded revision and match
the current baseline exactly: **zero current input drift**. This includes
the actual typed Source wrapper, raw allocator cleanup and typed legacy
owner files inspected in the preceding packet, their headers and receipts,
and the ledger shards. The prior 12 historical Source-artifact replays,
six current cleanup Source-pin replays and two historical runtime snapshots
remain qualified by their recorded revisions. This packet rechecks their
current repository files and report pins; it does not repeat those historical
artifact searches, compiled artifacts or fixtures.

In particular, the typed legacy `destroy_base` caller retains `noexcept`,
while the raw cleanup declaration does not. Current Source UCRT and compiler
EH/terminate behavior remains Source evidence, not Original runtime/EH
identity. Those contracts belong to the inherited chain, not new external
dependencies of these leaf helpers. Later Source or ledger changes are
outside this report's fixed baseline and require fresh drift qualification.

The report pins 84 current canonical/physical inputs, the accepted prior
document, both complete Native captures, the local CFG records and this
document. Verification is static evidence validation and diff/scope checks.
No Source/CMake/ledger/Ghidra modification, build, test, probe, annotation,
new Source function/byte credit, Original ABI execution or gameplay credit
is claimed.
