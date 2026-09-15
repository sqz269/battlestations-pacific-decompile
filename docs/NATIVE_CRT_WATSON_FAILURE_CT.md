# Complete Watson failure source CT

This packet implements complete BF65BB[252] `__invoke_watson`, BFE120[15]
`__security_check_cookie`, and C185A4[260] `___report_gsfailure` as qualified
MSVC Win32 instruction sources. The new interfaces borrow eight actual storage
bindings from one CRT domain; they do not create that domain or provide a native
cookie checker for unchanged compiler/OS frames. The accepted CO proposal
`ab590121f32b3de58ebbd79840677399f7cd1295` remains frozen and is retained locally.

The public header describes every new argument and capture requirement.
`invoke_native_crt_watson_00bf65bb` adds bindings after the five ignored native
arguments, at entryESP+18h/biasedEBP+2C4h. `check_native_crt_cookie_00bfe120` has
an added stack binding while still requiring the cookie in actual ECX; calling
its declaration from ordinary C++ does not arrange ECX. Its equal path preserves
EAX and all registers, returns the actual CMP flags through F3C3, and its
mismatch path directly tail-jumps the full reporter. Caller cleanup is24 bytes
for Watson and4 for checker/reporter. None is declared noreturn.

Watson retains its biased frame, cookie XOR capture, original register/segment
store order and PUSHFD observation. Its local CONTEXT is not initialized. The
real full CL `fill_native_crt_bytes_00bf79f0` clears only the50h exception record;
its actual feature reference is the fourth argument, with16-byte cleanup. DF=0
is required; no CLD is added. The50h size does not read the feature cell or enter
the vector/SSE2 paths. The local pair and exception fields are then published
in the original order. The five real Win32 APIs retain their original order,
filter/debugger branch and termination status C000000Dh.

The reporter begins with the original PUSH EBP/MOV/SUB ESP,328h. One extra
PUSH ECX at EBP-32Ch supplies scratch beneath the original allocation. All
binding loads and capture stores before original PUSHFD preserve flags, so
the stored EFLAGS remain the SUB result. Original EAX is stored first; saved
original ECX second; remaining integer registers and six WORD segment stores
follow unchanged. POP ECX restores scratch after PUSHFD/POP. The later original
MOV EAX,[EBP] replaces the shim's temporary EAX. Extra scratch is disjoint from
the preserved unused [EBP-320h] read and cookie locals at-328h/-324h.

The shared CONTEXT109E5C0[2CCh] and exception record109E568[50h] retain untouched
bytes, including segment high16 halves, floating state and extended registers.
The actual current shared EIP is reloaded for exception address; codeC0000409h
then flags1 are stored. Cookie and complement reads remain ordered. The
current debugger word109E5B8 is stored, then reloaded after the filter call.
UnhandledExceptionFilter receives the address of the actual borrowed
EXCEPTION_POINTERS object corresponding to D6E1CC, whose current fields point
to the same actual record/context. No local replacement pair is created.

All three hook calls use the existing full two-argument helper: actual hook
word then original reason are pushed, the original POP ECX removes the reason,
and LEA ESP,[ESP+4] removes the binding while preserving its AND flags and EAX.
Watson pushes its actual bindings directly before the real checker call and
uses LEA cleanup. If TerminateProcess and the reached cookie failure service
return, both original return tails remain; the reporter preserves the returned
BOOL in EAX. No abort, fastfail, validation, exception catch or repair is added.

Every current source instruction is mapped to one of122 native instruction
groups in the report. Fresh complete527-byte PE/live/listing captures equal
the accepted CO evidence; six original direct/tail rows and ten original IAT
sites are checked. CO's full226-artifact inventory is checked twice. Current
compiler evidence separately checks every instruction, branch target, direct
source relocation and actual import relocation, plus all eight MSVC reference
member offsets and current SDK CONTEXT/record/pair layout. Current command,
read/write and /Fo bindings, object/archive equality, sole source definitions
and the actual CL/hook archive members are retained. Build and verification
results and any failures are recorded in the machine report. The first strict
Win32 build passed, with all eight seeds matching and both existing CTests
passing. First compiler audit passed: 650 emitted bytes/171 instructions
(Watson289/74, checker19/8, reporter342/89), six source/tail relocations and ten
actual IAT relocations. Unchanged nonbranch instructions retain exact native
bytes; every changed group, branch target and provider operand is qualified.

New pointer loads, scratch and call words introduce distinct source frames,
fault locations and asynchronous observations. Captured EIP/ESP/EBP describe
the source caller, not original binary addresses. Actual state must remain
valid and disjoint from active source/provider frames and binding words.
Current shared contents remain mutable and their ordered reloads are visible.
This is not canonical ownership, native exception/SEH/FS identity or gameplay
proof. In particular C0DC54 calls BFE120 without an added context, with EAX=R
needed after return; this source interface cannot serve that unchanged edge.
No Watson, cookie-failure or termination execution, new tests or runtime probes
are performed. Ghidra and saved analysis are unchanged.
