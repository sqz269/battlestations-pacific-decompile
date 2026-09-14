# Native CRT nested local-unwind boundary

This discovery recovers the complete physical `00C0DC54` handler (70 bytes, 21 instructions) and `00C16898` cleanup-call primitive (3 bytes, two instructions). Fresh complete PE/live spans agree with the installed executable. The handler has no current Ghidra function; the separately defined `00C0DC9A..00C0DCB5` entry is excluded. No function definition, listing, flags or saved analysis was changed.

Base: `e9c7792df0bc8a213b6b13c88c206575f2b1eb4d`. Every live query used the verified BSP CLI against the configured bsp project, `/battlestationspacific.exe`, x86 language and 00400000 image base. The entire installed PE was rehashed: SHA256 `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`. BR `062331c340b9d8d19bd77c2ba528f20a2f44197a` and BS `4b524e551c0f86956ec118e60b4d304a61ab817d` supply retained complete checker, parent and dispatcher evidence. All 75 BR and 119 BS local artifacts were verified and copied without changing those worktrees.

## Producer and nested registration

Let `S` be ESP at entry to `C0DBC4`, and `F` its inherited EBP. Its three caller-cleaned DWORDs are cookie-word pointer at `S+4`, outer registration at `S+8`, and target level at `S+Ch`. After saving EBX/ESI/EDI, it captures those arguments, then pushes F, cookie pointer, outer registration, target level twice, the literal handler C0DC54, and previous FS:[0]. The resulting registration is `R=S-28h`.

| Registration slot | Exact producer and value |
| --- | --- |
| R+0 | Previous FS:[0], pushed at C0DBDD |
| R+4 | Actual handler C0DC54, pushed at C0DBD8 |
| R+8 | Initially copied target; overwritten at C0DBEB with `[E15590] XOR R` |
| R+Ch | Target level captured at entry |
| R+10h | Outer registration captured at entry |
| R+14h | Cookie-word pointer captured at entry |
| R+18h | Inherited actual frame F |
| R+1Ch/+20h/+24h | Saved EDI/ESI/EBX |
| R+28h | Helper return address |
| R+2Ch/+30h/+34h | Original stacked cookie pointer/outer registration/target arguments |

The actual cookie is loaded once for this nested registration, then FS:[0] is published at C0DBEF, after its cookie field. The outer scope-table decoder separately reloads the original cookie-pointer argument and its current pointed-to word. The loop also reloads the original outer-registration and target arguments. These current argument loads must not be conflated with the earlier copies in R+Ch/+10h/+14h, which the nested handler uses.

The parent stops on level -2, or unsigned current level <= target when target is not -2. Before inspecting a scope's filter word, C0DC21 stores its enclosing level to the outer registration. Nonzero filters skip cleanup. Zero filters trigger actual NLG notification, then the cleanup pointer is reloaded at C0DC3C, ECX has been set to 1 at C0DC37, and C0DC3F calls C16898. Thus an exception from an invoked cleanup occurs after the outer state has advanced. Recursive handling does not automatically retry that already-selected scope. This conclusion assumes valid scope-chain/callback behavior; arbitrary callback writes or corrupt frames are not converted into a recovery policy.

## C0DC54 complete physical handler

At handler entry `H=ESP`, the four stack words are exception record at H+4, this nested registration R at H+8, context at H+Ch, and a writable dispatcher output pointer at H+10h. The third word is not read. Plain RET leaves argument cleanup to its caller.

1. C0DC54 loads the exception record into ECX. C0DC58 tests its DWORD flags with mask 6, then EAX is set to 1. If neither bit is set, C0DC64 jumps directly to the final RET. No registration/cookie/dispatcher-output access occurs on that branch.
2. Otherwise C0DC66 loads R into EAX. C0DC6A loads its cookie field into ECX and XORs it with R. C0DC6F calls the actual ECX-input BFE120 checker. Its complete retained 15-byte body compares actual E15590, preserves EAX on the equal return, and tail-jumps to C185A4 on mismatch. This real success-path EAX preservation is required by the next instructions; a generic status-returning checker is not interchangeable.
3. After normal checker return, PUSH EBP saves the handler's incoming EBP. C0DC75 installs `[R+18h]` as EBP. It pushes `[R+Ch]`, `[R+10h]`, `[R+14h]` in that order and calls C0DBC4 at C0DC81: cdecl `(captured_cookie_pointer, captured_outer_registration, captured_target_level)` with the saved actual frame in EBP. That recursive provider creates its own native registration and performs the remaining eligible cleanup.
4. Only after normal recursive return, C0DC86 adds ESP,0Ch and POP EBP restores the incoming frame register. The handler then reloads R from H+8, reloads the dispatcher output pointer from H+10h, writes R through it at C0DC92, sets EAX=3 and returns. If the checker or recursive provider transfers exceptionally, these later restoration/output operations are not promised to execute.

The mask-zero branch returns EAX=1 and flags from TEST: CF/OF/SF zero, ZF/PF one, AF undefined. The recursive-return branch returns EAX=3; final arithmetic flags come from ADD ESP,0Ch. EBP is restored only on normal recursive return; EBX/ESI/EDI preservation relies on the exact providers. The handler itself does not write FS:[0]; the recursive provider performs its own publication/unlink. No new catch, finally retry, output prepublication or generic unwind substitute is justified.

## C16898 complete instruction primitive

The bytes are exactly `FF D0 C3`: CALL EAX, RET. EAX on entry supplies the actual executable cleanup target. EBP, ECX and every other register arrive from the caller; this primitive adds no setup, save, validation, stack arguments or result normalization. If primitive entry ESP is P, the actual cleanup receives its return word at P-4; on a balanced normal return, RET resumes the primitive's caller. The primitive leaves the cleanup's EAX, flags and register effects unchanged. Exceptional/nonlocal exit does not execute its final RET by guarantee.

Both current direct callers were inspected. C0DBC4 explicitly sets ECX=1. The complete 132-byte `C167C9..C1684C` caller instead loads a scope's enclosing level into ECX at C16811; actual C16879 notification preserves ECX, and C16830 only reloads EAX before C16834 calls this primitive. Therefore ECX=1 is a caller-specific setup, not the C16898 contract. There is no basis for adding a constant argument or generic C++ callback wrapper.

## Listing attribution and source readiness

The current live C0DBC4 body ends at C0DC53 and contains two direct calls, to C16879 and C16898. Indexed callees additionally list BFE120 and C0DBC4 because the index folds in the adjacent undefined handler. Fresh handler bytes place those calls at C0DC6F and C0DC81. Both sites have no live containing function; function-based disassembly at C0DC54 also reports none. This does not prove instruction-level flags or code-unit membership, which remain unqueried/unknown. The defined C16898 listing contains both of its instructions.

A future separately authorized definition proposal would create a handler entry at C0DC54 with exactly the physical 70-byte control-flow span through C0DC99, preserving the existing C0DBC4 and C0DC9A bodies and all names/comments. It must first record current code-unit/flow/body properties and choose disassembly/definition only from those observed properties. No old override/no-return value is guessed here. The proposal is not an applied metadata change.

The smallest independent source candidate is the complete three-byte C16898 primitive with an explicitly documented EAX-input native entry and balanced actual-funclet contract. A naked Win32 instruction entry can preserve those bytes; an ordinary C++ call declaration alone cannot establish EAX or a valid native frame. It supplies neither a cleanup body nor native exception/frame ownership.

The nested handler algorithm is fully recovered but not source-closed: it needs the actual BFE120 checker and its C185A4 failure owner, complete recursive C0DBC4 plus this handler as its installed provider, actual compatible cookie domains, FS/thread registration and real NLG descriptor/cleanup targets. BS already establishes C0DBC4's complete normal body and names those dependencies; no substitute owner is introduced here. The approved epilog/filter source primitives do not provide that domain. C0DCE6/C2F25C global unwind and reader/heap work were left untouched.

The report contains seven direct/tail rows: the two newly recovered handler calls and five retained/current caller/checker rows. PE instruction checks establish all seven targets. The standard live call verifier is expected to reject the two undefined-handler sites; that explicit discovery failure is retained, not suppressed or counted as indirect success. CALL EAX at C16898 is separately qualified. All packet-local evidence, scripts and failures are inventoried twice with SHA256/SHA512. This is discovery only: no source, build, tests, native execution, Ghidra mutation or push.
