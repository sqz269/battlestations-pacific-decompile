# Native CRT global unwind and import thunk

This packet supplies the complete `00C0DCE6 _EH4_GlobalUnwind` (26 bytes)
and `00C2F25C RtlUnwind` import thunk (6 bytes) in
`src/native_crt_global_unwind.cpp`, with declarations in
`include/bsp/native_crt_global_unwind.hpp`. Both are naked MSVC Win32
instruction interfaces. The source retains the direct wrapper-to-thunk call
and thunk-to-real-IAT jump. It adds no exception, frame, handler, cookie, FS
or callback owner.

Base: `e9c7792df0bc8a213b6b13c88c206575f2b1eb4d`. BS discovery
`4b524e551c0f86956ec118e60b4d304a61ab817d` supplies the prior contract and
provider evidence; all 119 local artifacts were verified by complete path
set, size, SHA256 and SHA512 before reuse. That worktree remains unchanged.
Before source edits, fresh complete PE/live spans and the actual IAT matched
the retained evidence. Each live batch verified `C:/Users/sqz269/bsp.gpr`,
`/battlestationspacific.exe`. The installed PE SHA256 is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

`reports/native_crt_global_unwind_bu.json` records full spans, ABI, call
checks, compiler relocations, SDK/import identity, archive membership and
the whole-local double-hash inventory. Existing library names are preserved;
no Ghidra annotation/listing changes were performed.

## Native arguments, stack and continuation

The global-unwind entry receives the actual target registration in ECX,
with no stacked arguments. It pushes EBP, copies ESP into EBP, then pushes
EBX, ESI and EDI. It pushes ReturnValue zero, ExceptionRecord zero, its own
restoration TargetIp, then ECX as TargetFrame. A direct CALL invokes the
separate native import thunk; in the source it invokes the separate compiled
`rtl_unwind_import_00c2f25c` symbol.

If wrapper-entry ESP is `S`, the four saved registers occupy `S-10h..S-4`.
The four API arguments occupy `S-20h..S-14h`; the direct call places its
return address at `S-24h`. The actual stdcall service must supply the state
required by the restoration continuation, whose ESP is `S-10h`. POP EDI,
POP ESI, POP EBX, POP EBP and RET restore the caller's nonvolatile values
and return with ESP=`S+4`.

The original TargetIp is `00C0DCFB`, the first POP EDI and the direct call's
normal fallthrough location. The source uses the address of its own
`restore_unwind_registers` label. It never passes that original numeric PE
address as a source continuation. The compiler evidence resolves this label
to offset `15h` in the same compiled wrapper, exactly the first restoration
instruction and the compiled direct call's fallthrough.

The wrapper does not normalize EAX or modify arithmetic flags around the
real service. Its PUSH/MOV/CALL/POP/RET instructions leave the actual API's
EAX/flags as the resulting state; no Boolean, success code or error status
is invented. ECX and other volatile state are whatever the real API leaves.
No explicit FS read/write occurs in these two source bodies; the actual
Windows service owns its real unwind operations.

The import thunk receives four native pointer arguments under stdcall and
tail-jumps through the actual import. It preserves the caller's existing
return address and argument stack for the Windows callee, adding no frame,
argument conversion, validation, catch, result transformation or substitute
unwind implementation. Its exception-record type is the actual SDK
`_EXCEPTION_RECORD`, forward-declared in the public header.

## Actual Windows provider

The original PE imports IAT `00CE21B8` as
`KERNEL32.dll!RtlUnwind`; the native thunk is `FF 25` followed by that IAT
address. The retained current Windows SDK declaration is VOID NTAPI with
TargetFrame, TargetIp, PEXCEPTION_RECORD and ReturnValue pointers.

The compiler read trace includes that same SDK header. Its matching Win32
`kernel32.lib` contains one short code-import member for `_RtlUnwind@16`,
machine `014Ch`, DLL `KERNEL32.dll`. The complete import library, exact
member, member metadata and hashes are retained. The source COFF relocation
is to `__imp__RtlUnwind@16`; this is the real Windows provider, not a host
CRT callback or locally invented service. Static import evidence does not
prove execution of an OS implementation or a running process's frame state.

## Compiler and build evidence

The strict full MSVC Win32 build passed with `/W4 /WX /fp:strict /MD
/std:c++17`; eight native seeds matched and both existing CTests passed.
No tests were added, and neither primitive was executed. Existing math
tests do not validate this native unwind contract.

The emitted wrapper remains 26 bytes and the thunk 6 bytes. Only three
four-byte operands differ from the original spans:

| Function offset | Relocation | Verified meaning |
| --- | --- | --- |
| Wrapper `+0Bh` | DIR32 | Same compiled wrapper's restoration label at `+15h` |
| Wrapper `+11h` | REL32 | Direct call to the separate compiled import-thunk symbol |
| Thunk `+2` | DIR32 | Real `__imp__RtlUnwind@16` IAT reference |

The label's actual COFF symbol section/value and encoded addend resolve to
the wrapper's restoration instruction. The direct call keeps its five-byte
E8 form; the import thunk keeps its six-byte FF25 form. All remaining bytes
are identical to the original PE/live bodies. No instruction substitutions,
private writable sections or compiler-generated frame policy are present.

The current object matches one unique member of the current `bsp_core.lib`;
each of the two source symbols has one archive definition. The actual
compiler metadata/binary hashes, complete target command/read/write tlog
records, source/header inputs, object, full archive and extracted member
are retained. The entire local inventory was verified twice with SHA256
and SHA512. The one native direct-call row passes the call checker; the
thunk's indirect IAT jump is qualified separately.

## Preconditions and limits

The caller must supply the actual valid target registration and compatible
native exception/unwind domain, with the stack state required by this
continuation. The source creates none of that state. Arguments are forwarded
unchanged; no null or validity test, private handler, callback, synthetic
frame, cookie repair or catch policy is added.

The restoration sequence describes the actual continuation path. An API
fault, exception or nonlocal exit need not perform an ordinary return
through that path; no rollback or alternate return behavior is fabricated.
An ordinary C++ call with unrelated host/native frame state does not prove
these preconditions. The source symbols and relocated instruction addresses
do not establish original exception/fault-PC or OS unwind equivalence.

The complete `00C17653` wrapper, `00C07C90` dispatcher, prolog, NLG,
cookie/check/failure and canonical frame/FS owners remain separate. This
packet is source/build/archive evidence, not binary drop-in, native unwind,
running-process or gameplay validation.
