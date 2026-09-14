# Canonical native CRT failure entries DK

This packet reconstructs four complete original-entry bodies against the
accepted DD canonical owner at base
`6c44a02505c4fff9faf5e8b2d01a70697cc2f1ba`. The new naked Win32 entries have
the original stack and register inputs. Existing CT `NativeCrtWatsonBindings`
interfaces, the two-word hook helper, CL source and DD owner are unchanged.

| Original entry | Complete span | Native entry contract |
|---|---|---|
| BF65BB `__invoke_watson` | BF65BB..BF66B6, 252 bytes / 65 instructions | Five ignored cdecl diagnostic words; caller removes20 bytes |
| BFE120 `__security_check_cookie` | BFE120..BFE12E, 15 bytes / 4 instructions | Raw ECX cookie, no stack arguments; EAX preserved; equal F3C3, mismatch real tail |
| C185A4 `___report_gsfailure` | C185A4..C186A7, 260 bytes / 53 instructions | No stack arguments; original frame and ordered register/context capture |
| C04EF3 `BSP_Crt_ClearDebuggerHookState` | C04EF3..C04EFA, 8 bytes / 2 instructions | Original ignored reason word, cdecl caller cleanup; AND fixed word0, RET |

The hook name remains a provisional descriptive hypothesis, not a recovered
library symbol. All three existing CRT library names remain intact. Fresh
installed PE and live database bytes agree for all535 bytes and124 native
instructions. The entire122-byte native memset, actual canonical data and
all five original IAT identities are also retained. Every Ghidra CLI batch
verifies configured project `C:/Users/sqz269/bsp.gpr`, project name `bsp`,
program `/battlestationspacific.exe`, x86 Win32 and base400000h. Database
bytes are image evidence, not observations of a running game.

The original PE is 12,223,752 bytes, SHA256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
All six direct/tail edges and ten IAT call sites are resolved from complete
physical instructions. Indexed historical graph edges do not expand this
closure. C04EF3 was added only after reporting the extra-binding provider
gap and receiving root authorization; no DD data metadata lease was taken.

## Native Watson frame and returned termination

Let S be entry ESP. PUSH EBP and LEA EBP,[ESP-2A8h] establish EBP=S-2ACh.
SUB ESP,328h reserves down to S-32Ch; PUSH ESI saves ESI at S-330h. The five
words at S+4..S+14h are never read. Native stack objects are:

| Object | Address and extent |
|---|---|
| Local exception record | S-32Ch, 50h bytes |
| Local exception pair | S-2DCh, 8 bytes |
| Local CONTEXT | S-2D4h, 2CCh bytes |
| Encoded guard | S-8, four bytes |
| Saved EBP / return word | S-4 / S |

The actual E15590 cookie is read, XORed with biased EBP, stored as the guard
and then captured as EAX. ECX/EDX/EBX/ESI/EDI are the original other integer
inputs. Low16 SS/CS/DS/ES/FS/GS are written in order. PUSHFD captures the
cookie XOR flags, not the incoming arithmetic flags. ContextFlags becomes
10001h; EIP=[S], ESP=S and EBP=[S-4]. No CONTEXT clear or substitute capture
API appears. Untouched bytes and segment high16 halves remain untouched.

BF6647 calls the actual complete CL memset with only the original
`(record,0,50h)` words. The original ADD ESP,Ch remains. The record is cleared,
then the pair is published record first and context second, with exception
code C000000Dh and captured return PC as exception address.

Real IsDebuggerPresent is captured in ESI. SetUnhandledExceptionFilter(NULL)
precedes UnhandledExceptionFilter(&local_pair); the previous filter is
discarded. Only filter result0 and captured debugger result0 call hook(2).
GetCurrentProcess and TerminateProcess retain status C000000Dh and the exact
push/call schedule. If termination returns, the saved guard is decoded into
raw ECX, ESI is restored, and the real canonical checker is called without
extra words. ADD EBP,2A8h; LEAVE; RET remain. Final arithmetic flags are from
that ADD; no abort, fastfail, catch or noreturn pruning is added.

## Native checker and shared reporter capture

The checker is precisely CMP ECX,[E15590]; JNZ mismatch; F3 C3; JMP reporter.
There is no PUSH EAX, binding load or scratch. Equality preserves every
register and the actual CMP flags. The mismatch tail retains the same raw
registers, return word and stack for the complete canonical reporter. The
ordinary C++ declaration does not arrange raw ECX for a source caller.

The reporter begins PUSH EBP; MOV EBP,ESP; SUB ESP,328h, with EBP=S-4. It
stores incoming EAX/ECX/EDX/EBX/ESI/EDI directly into actual109E5C0 CONTEXT,
then the same six WORD segments and the actual SUB flags through PUSHFD/POP.
No extra scratch push or accessor call occurs. Its later fields are
EBP=[S-4], EIP=[S], ESP=S+4. The unused [EBP-320h] read is retained.

CONTEXT writes are B0/EAX, AC/ECX, A8/EDX, A4/EBX, A0/ESI,9C/EDI;
C8/SS,BC/CS,98/DS,94/ES,90/FS,8C/GS (two bytes each); C0/EFLAGS,
B4/EBP,B8/EIP,C4/ESP and00/ContextFlags. Source assertions bind every offset
and the real current Win32 record/pair sizes to SDK types. Neither shared
record is cleared. Current shared EIP is reloaded into exception address,
then code C0000409h and flags1 are stored. Actual cookie then complement
are read into the original -328h/-324h locals, in that order.

IsDebuggerPresent writes actual109E5B8. Hook(1) runs before clearing the
unhandled filter. UnhandledExceptionFilter receives actual RO D6E1CC, whose
current pair addresses are109E568 and109E5C0. Its result is ignored. The
current debugger word is reloaded; if zero, hook(1) runs again. Termination
uses the real current-process handle and status C0000409h. If it returns,
LEAVE/RET preserves its EAX result. Existing Ghidra noreturn metadata is not
treated as evidence that these physical return instructions can be omitted.

## Actual providers and bounded CL compatibility

The DD owner is a precondition. It already maps and retains actual RW cookie,
failure/context/debugger and hook pages, plus the RO exception pair. These
entries use fixed original data addresses directly; they do not call the
owner, allocate storage, initialize it, pass a view or introduce a shadow.
The original data initial state is checked separately from the owner's
accepted controlled-child runtime evidence, which is not repeated here.

The original hook is exactly `83 25 A8 EE 09 01 00 C3`: AND DWORD[109EEA8],0;
RET. It ignores the original reason word and preserves all registers. AND
sets CF/OF/SF=0, ZF/PF=1, AF undefined; other flags survive. Callers retain
their original POP ECX reason cleanup. The prior helper instead requires a
second word and reads it; that interface remains available and unchanged.

The complete existing CL body declares a fourth feature binding. Its actual
emitted machine code first tests count, low fill byte and count>=100h. Only
zero fill with count>=100h can read that fourth word. Watson's zero fill and
50h count force the scalar branch before this access. The source assembly
therefore supplies the original three words, preserving the native caller
frame and ADD ESP,Ch. The full scalar CFG, all alignment paths, original
operands and cdecl RET are statically compared in the current object and
linked image. The actual reached path has no fourth-word access, feature
cell read, SSE2 or nested call. DF=0 and valid writable nonwrapping extents
disjoint from active source/provider frames remain required. This does not
establish general original three-argument memset ABI compatibility. An
earlier root summary mentioned80h; fresh code proves the actual gate is100h,
and the retained contract uses100h throughout.

All five services use real KERNEL32 imports with current SDK stdcall types:
IsDebuggerPresent, SetUnhandledExceptionFilter, UnhandledExceptionFilter,
GetCurrentProcess and TerminateProcess. No injected service or terminator
is substituted. Static source assertions check those exact function types.

C0DC54's read-only boundary shows EAX=registration, ECX=[EAX+8] XOR EAX,
CALL BFE120 with no added word, then EBP=[EAX+18h]; it requires preserved EAX.
BF66EF's complete36-byte boundary calls hook(2), removes the reason, restores
EBP and tail-jumps Watson when its other branch is not selected. These are
incoming-call observations only; neither enclosing native path is rebuilt
or executed by DK. DJ NLG and CU cleanup are outside this packet.

## Validation and limits

The companion report records strict Win32 build/seeds/CTests and the complete
current compiler, instruction, context-layout, COFF relocation, archive and
link/provider audits. The local static audit image forces the four entries
and accepted owner into one link and is never executed. Its map identifies
the real canonical failure, CL, DD owner, RO mapper, bootstrap and cookie
members; imported services are checked in its actual PE import table. All
535 owned instruction bytes are checked, allowing only the six real source
call/tail operands and ten actual IAT operands to relocate. Every remaining
byte, internal branch, fixed data operand and return is compared exactly.
The settled strict MSVC19.51 Win32 build passed with /W4 /WX /fp:strict;
all eight seeds and both existing CTests passed. The full static audit passed
all535 bytes/124 instructions, six direct/tail and ten real IAT relocations,
and every reached linked scalar CL instruction. No source access wrapper,
extra argument or stack scratch is present in the canonical entries.

The first source attempt used bare absolute-memory assembly syntax, which
MSVC rejected. Explicit DS syntax compiled but added redundant3E prefixes,
so the strict byte audit rejected that second attempt. The settled source
uses exact byte emissions only for these fully decoded fixed-address
instructions, with each mnemonic and operand printed beside it; source
calls and branches remain symbolic assembly. This retains native addressing
without new prefixes. Both rejected sources, compiler outputs and the entire
second object/archive/link attempt are preserved. No rejected attempt is
counted as matching the original native body.

The ordinary game can omit these uncalled archive members. Availability in
bsp_core and a static force-link audit are separate from runtime integration.
No original or owned Watson, cookie checker, failure, hook, termination,
cleanup, unwind or game entry is executed. The existing math tests retain
their ordinary scope; no new runtime tests are added. DD remains accepted
source/controlled-probe evidence. Original code addresses, live native
SEH/unwind closure, asynchronous observation, debugger integration and
gameplay remain unvalidated.

Owned Ghidra evidence is appended under the write lock, preserving prior
names, library comments and borrowed-interface evidence. Old values and
save/readback/refreshed exports are retained. Canonical reconstruction
records use distinct source names, preserving the CT and borrowed-hook
records. Only address-specific rows and one CMake registration are changed.
The complete recursive local evidence and explicit external artifact set
are frozen twice with SHA256/SHA512; manifest self-files are separately
pinned in delivery. All rejected attempts remain in the bundle.
