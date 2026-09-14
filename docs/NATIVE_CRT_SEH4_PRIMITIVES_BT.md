# Native CRT SEH4 instruction primitives

This packet supplies the complete 20-byte `00C07C45 __SEH_epilog4` and
23-byte `00C0DCB6 _EH4_CallFilterFunc` bodies in
`src/native_crt_seh4_primitives.cpp`, with declarations and preconditions in
`include/bsp/native_crt_seh4_primitives.hpp`. They are exact naked MSVC Win32
instruction primitives. No prolog, dispatcher, cookie, FS-chain owner,
RtlUnwind, NLG or filter-body implementation is included.

Base: `e9c7792df0bc8a213b6b13c88c206575f2b1eb4d`. Retained inputs are BR
`062331c340b9d8d19bd77c2ba528f20a2f44197a` and BS
`4b524e551c0f86956ec118e60b4d304a61ab817d`. All 75 BR and 119 BS local
artifacts were checked by complete path set, size, SHA256 and SHA512 before
reuse. Those discovery worktrees remain unchanged. Fresh complete spans from
verified `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe` match the
retained bytes and original installed PE, SHA256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

Exact native/compiled bytes, source symbols, compiler input/output records,
unique archive membership and whole-local double hashes are recorded in
`reports/native_crt_seh4_primitives_bt.json`. Existing CRT library names are
preserved; no Ghidra annotations or listing repairs were performed.

## Epilog: native frame preconditions and order

At entry, EBP must be the actual compiler frame `F`; ESP must be `P-4`,
holding this helper's continuation. `[P]` is the saved cookie, followed by
saved EDI, ESI and EBX. `[F]` holds the incoming EBP, `[F+4]` the enclosing
function's return address, and `[F-10h]` the previous FS:[0] link. The actual
current FS:[0] registration must be at `F-10h`. These are caller-owned native
frame/thread preconditions, not state constructed by this source.

The first two instructions load the old link into ECX and publish it to
FS:[0]. Only afterward does POP ECX obtain the helper continuation. The first
POP EDI discards the cookie; the second restores the real saved EDI. POP ESI
and POP EBX restore the remaining saved registers. MOV ESP,EBP; POP EBP;
PUSH ECX; RET restores incoming EBP and returns to the helper continuation
with ESP=`F+4`. The enclosing function then owns its own return.

EAX and arithmetic flags are unchanged; ECX retains the helper continuation.
No cookie comparison occurs. The first POP EDI remains a real instruction,
including its transient register value and memory access. This cdecl void
source declaration is a symbol/interface for deliberate compiler-frame
entry. Calling it as an ordinary C++ cleanup function does not satisfy the
required stack/EBP/FS state.

The initial frame read occurs before unlinking. A fault during subsequent
stack restoration occurs after the old FS link has already been published;
there is no compensating relink, catch, return normalization or invented
failure policy. Native instruction and memory-access order is retained,
while original exception/fault-PC and running-thread equivalence are not
established merely by matching bytes.

## Filter call: actual native funclet contract

The source naked fastcall declaration takes the actual native filter entry
in ECX and actual establisher-frame pointer in EDX. It pushes incoming EBP,
ESI, EDI and EBX in that order, sets EBP to the incoming EDX frame, and uses
XOR to clear EAX, EBX, EDX, ESI and EDI. CALL ECX invokes the supplied actual
funclet without stacked arguments. If helper-entry ESP is `S`, the filter's
return word is at `S-14h`; its four saved-register words follow.

At the filter call, EBP is the frame, ECX the actual entry, and those five
registers are zero. CF/OF/SF are zero, ZF/PF one and AF undefined from the
final XOR. After a balanced normal funclet return, POP EBX/EDI/ESI/EBP and
RET preserve the filter's raw EAX and arithmetic flags. ECX/EDX remain its
incidental volatile outputs. The emitted fastcall symbol and exact byte
sequence verify ECX/EDX inputs and plain RET with no stacked-argument cleanup.

The caller must provide a real executable native funclet and valid frame in
the same operating domain. Original PE addresses in the report are evidence,
not automatically callable pointers in a rebuilt program. This interface
creates no filter, scope table, synthetic frame or general C++ callback
service. It adds no null check, argument validation, EAX normalization, catch
or FS manipulation/repair. If the actual filter faults or exits by a native
exception transfer, the normal POP/RET sequence is not guaranteed to run;
this helper provides no substitute unwind policy.

## Compiler and evidence validation

The strict full MSVC Win32 build passed with `/W4 /WX /fp:strict /MD
/std:c++17`; all eight native seeds matched and both existing CTests passed.
No tests were added and neither frame primitive was executed. Existing math
tests do not validate these frame/exception contracts.

MSVC diagnoses the required FS:[0] store as C4733. A warning push/disable/pop
is confined to this epilog because it restores the caller's existing link;
it does not install a new handler. All other `/W4 /WX` and linker policies
remain intact. The initial rejected build log is retained. This diagnostic
suppression neither registers a SafeSEH handler nor establishes a valid native
FS chain; the exact FS instruction remains unchanged.

All 43 compiled bytes are identical to the complete original PE/live bodies.
There are no relocations, substituted instructions, added global/code-address
bindings or compiler-generated prologs/epilogs. Native FS:[0] remains explicit.
The filter has exactly one
CALL ECX at offset 10h; the epilog has no calls. The object has no writable
sections and defines exactly these two source symbols. Each symbol has one
definition in the one unique archive member, which is byte-identical to the
current build object.

Retained evidence includes the actual compiler metadata/binary hashes,
complete source-specific command/read/write tlog records, actual header and
source inputs, object, full archive and extracted member. The packet-local
inventory was checked twice using SHA256 and SHA512 with the entire path set
verified. Zero owned direct calls yield zero rows in the direct-call checker;
the filter's indirect call is qualified separately by full native/compiled
instructions and its explicit interface contract.

The exact instruction bodies do not supply `00C07C00`, `00C07C90`, actual
cookie/check/failure ownership, a native scope/frame producer, NLG or OS
unwind services. The complete 197-byte `00C17653` wrapper remains open. No
ordinary C++ caller usability, native exception/fault/unwind equivalence,
drop-in binary replacement or game/runtime validation is claimed.
