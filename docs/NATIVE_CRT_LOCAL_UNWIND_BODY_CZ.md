# Complete native local-unwind loop proposal (CZ)

Address: 00C0DBC4, original saved name `__local_unwind4`. Complete physical span
C0DBC4..C0DC53 inclusive: 144 bytes, 48 instructions, two direct calls, no
padding or missing listing instructions. Fresh PE/live bytes and the current
Ghidra body agree. Saved `undefined __local_unwind4(void)` omits three actual
stack words and inherited EBP; it is not the recovered calling contract.

This is a proposal only. The full body is understood, but unchanged native
closure remains conditional on the real handler/checker, canonical cookie,
NLG entry/state and native cleanup/frame domain. Accepted CT adds a context
word and cannot satisfy the original handler checker edge. Accepted CU closes
only the exact three-byte cleanup-call primitive. Canonical-owner work belongs
to CY and is not duplicated here.

## Entry and nested registration

Let S be entry ESP, H inherited EBP, P=[S+4] a scope-decoding cookie pointer,
F=[S+8] the actual caller registration, and T=[S+Ch] the raw target level.
The caller cleans 12 bytes. No argument validation, EBP setup or allocation is
performed. EBX, ESI and EDI are pushed, then P/F/T are captured into EDX/EAX/ECX.
Pushes H,P,F,T,T,handler,FS:[0] create N=S-28h:

| N offset | Written value |
|---|---|
| +00 | previous FS:[0] |
| +04 | actual nested handler C0DC54 |
| +08 | first T copy, then overwritten by actual E15590 XOR N |
| +0C | target T |
| +10 | F |
| +14 | P |
| +18 | inherited H |
| +1C/+20/+24 | saved EDI/ESI/EBX |
| +28/+2C/+30/+34 | original return word / P / F / T |

C0DBDD reads/pushes the old FS link. C0DBE4 reads the fixed original E15590
cookie, XORs N, stores N+8, then C0DBEF publishes FS:[0]=N. Do not publish
before the cookie or replace the fixed cell with the first argument: P is a
separate pointer, even when concrete callers pass E15590. The saved +18 H is
for the nested handler; the normal epilog skips it, never POPs EBP.

## Every loop path

At C0DBF6 reload F from original argument N+30. Load encoded table F+8 into
EBX, reload P from N+2C, XOR with the current DWORD at P, then load current
level ESI=[F+Ch]. Stop immediately for level FFFFFFFE. Otherwise reload T from
N+34: target FFFFFFFE means unwind all levels; any other T stops when level
is unsigned <= T. The compare is JBE, not a signed comparison.

For a continuing level, wrapping x86 LEAs form table+10h+12*level. Load its
enclosing level at +0 and publish [F+Ch] before examining filter +4. Nonzero
filter skips the handler and restarts the entire reload sequence; no filter
callback is invoked. Zero filter is a finally/cleanup record: push code101h,
load cleanup +8 into EAX, call C16879, set ECX=1, reload cleanup +8 again, and
call C16898. After ordinary return restart at C0DBF6. Neither key, registration,
level nor table is cached across cleanup. Actual mutations of source-visible
fields therefore affect the next iteration. There is no rollback, retry of
already-published level, range/cycle check or compensating cleanup.

| Site | Actual callee | ABI and cleanup |
|---|---|---|
| C0DC32 | C16879 / __NLG_Notify | EAX=record+8 cleanup, EBX=record, EBP=H; one stacked101h. Actual callee preserves registers/flags, writes shared descriptor code/rawvalue/frame and RET4 consumes the word. |
| C0DC3F | C16898 / saved LIBCRT_unmatched name | EAX reloaded from current record+8 after notification, ECX=1, EBP inherited H, no stack args. Exact CALL EAX; RET forwards actual cleanup effects; no synthetic callback. |

The caller relies on actual NLG preserving EBX for the following +8 reload.
CU itself saves nothing. A cleanup's balanced return and native frame/register
discipline are external obligations; in particular future callbacks still
need the intended inherited EBP. The loop restores EBX/ESI/EDI from its own
stack on normal exit; it does not restore a cleanup-damaged EBP. No general
callback ABI, exception translation or typed host closure is inferred.

## Exit, exceptions and native handler dependency

At C0DC46 POP FS:[0] restores the saved previous link first. ADD ESP,18h skips
handler/cookie/T/F/P/H, then POP EDI, POP ESI, POP EBX, RET. EAX holds the F
reloaded by the terminating iteration; this is raw behavior, not a meaningful
new ownership result. ECX holds P at the terminal test; EDX need not be loaded
with T on the level=-2 path. Final arithmetic flags come from ADD ESP,18h;
DF is neither changed nor normalized. EBP is inherited, not owned.

If notification or cleanup faults, throws, or exits nonlocally, there is no
ordinary C++ finally/catch in this body. The already-published level is visible;
FS may still point at N until actual native dispatch/unwind processes it.
C0DC54 uses ExceptionFlags&6, checks N+8 XOR N through native BFE120 while
requiring EAX=N on return, sets EBP=N+18, recursively calls this loop with
P=N+14/F=N+10/T=N+C, then publishes N through the actual dispatcher output and
returns disposition3. Non-unwind returns1. Complete CP retained evidence is
reused; no new handler definition or flow repair was performed here. CT's
extra stack binding is absent from this handler's original call and cannot
be inserted silently. The context-bearing reporter is not a canonical owner.

## Concrete incoming call evidence

Fresh xrefs report five physical calls. Defined caller queries report four;
the missing containing function is the expected undefined C0DC54, not a missing
physical CALL. C0DC9A installs EBP from its dispatcher context, pushes context
+1C/+18/+28, calls at C0DCAA, ADD ESP,C, restores EBP, RET4. C0DD00 installs
EBP from first stacked word, pushes EDX/ECX/second stacked word, calls C0DD0B,
ADD ESP,C, restores EBP, RET8. This is a register-plus-stack wrapper, not three
ordinary wrapper arguments. __fsopen at BF8354 and __wfsopen at C04D45 push
T=-2,F=EBP-10h,P=E15590 and ADD ESP,C. The fifth call is CP's C0DC81 recursive
handler site. These are bounded caller setup observations, not reconstruction
of their enclosing I/O or dispatcher bodies.

## Smallest complete source proposal and blockers

A future naked MSVC Win32 three-word entry may declare
`void __cdecl unwind_native_crt_local_scopes_00c0dbc4(const volatile uint32_t* P, void* F, uint32_t T)`.
The declaration does not supply inherited EBP or validate actual registration,
FS chain, encoded scope table, callback targets or stack. Preserve all 48
instructions and original stack offsets. Two REL32 operands bind actual NLG
and CU; an actual handler address immediate and fixed canonical cookie load
must resolve the real corresponding owner. This is not ready as a standalone
three-body C++ composition just because descriptive source names exist.

Conditional relocation plan: +15h handler immediate, +21h canonical cookie
address, +6Fh NLG REL32, +7Ch CU REL32. The other128 bytes are invariant if all
native layouts/ABIs are retained. Do not replace FS operations with an ordinary
C++ guard or substitute a borrowed-view notifier/context checker. Required
providers: complete original C0DC54 and no-extra-argument EAX-preserving BFE120
with its actual failure domain; canonical E15590; actual C16879 and its shared
E16830 descriptor; exact CU C16898 and reached original cleanup funclets.
CY owns investigation of actual mutable storage. No ready owner is invented.

Proposed validation, only if root authorizes source later: strict Win32 build,
existing seeds/CTests, complete native/current instruction and relocation
mapping, genuine FS/stack layout and source/provider compiler/archive binding.
No native loop or cleanup execution is warranted by this discovery. Complete
bytes alone do not prove original exception/frame/runtime/gameplay validity.

## Evidence and qualification

`local/native_crt_local_unwind_body_cz` retains all fresh commands/results,
144B PE/live, full decode/listing, caller windows, target config, all523 accepted
CP artifacts plus its doc/report, and exact accepted CT/CU source snapshots.
Each live command uses BSP Client.verify against bsp.gpr,/battlestationspacific.exe,
x86:LE:32:default, image400000, bridge8089. Autostart was disabled. No saved
Ghidra mutation, function definition, prototype/flow repair or source/build/test
execution occurred. Saved name __local_unwind4 remains unchanged. A local
mkdir failure before any live query is retained. The report inventories every
local file twice with SHA256/SHA512; the inventory is outside local to avoid
self-hashing. Listing verification covers the two owned direct rows, while
undefined external-handler membership is separately qualified.
