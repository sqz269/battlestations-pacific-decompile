# Raw Icon state getter support proposal

Ignored whole AB1230..AB125C field/validation schedule:45B/19ops, actual vector
receiver ECX, DWORD index home, returned EAX address, RET4. Original HIGH STL/CG
name/tag remains unchanged and excluded from new Original/helper/vendor/CG/body
credit. This is ordinary Source storage support, not vendor reconstruction.

The candidate borrows actual0x10-byte F0 vector backing and a real volatile DWORD
index home. It reads first04, tests it, THEN captures the index once. Only nonnull
first reads last08, subtracts DWORDs, arithmetic-SAR6 and compares index unsigned.
Invalid input invokes the EXISTING zero-argument real Source provider
invoke_native_invalid_parameter_00bf6713(); no new callback/context/allocator.
After any normal provider return, current first04 is reloaded and captured index
is shifted6/added modulo2^32. Index-home mutations do not replace captured EDI;
vector repairs do affect the final reload. No input/count cap, nullptr default,
throw-on-invalid substitution, reread index, typed vector/T[n] or state dereference.

Two asm regions preserve all13 non-ABI Native operations. The Native validation
CALL is an ordinary call to the existing provider between those regions; saved
register/PUSH/RET ABI and Source decision/captured-output metadata are distinct.
Actual index-home/vector aliases retain reached reads where C++ lifetimes permit.
Source parameter/control/spill backing is disjoint from borrowed data. Pointer
result bits can be returned even after a returning validator leaves an invalid
address; this getter neither dereferences it nor invents a successful state owner.

The published provider uses the actual current SDK _invalid_parameter_noinfo and
five unused padding words. Its zeroarg/normal-return/handler ABI/lifetime and
failure policy qualifications are inherited. It does not reproduce Original
BF66EF encoded globals/Watson/debugger/SEH/FH3/fault addresses. No noexcept,
noreturn, terminal policy, cleanup/catch/retry, owner/Acquired/Origin or handler
installation is added. No C++allocation/FP or statefield write is present.

Old semantic GuiIconState/stdvector helpers, raw state/vector definitions and
provider bodies remain whole-file pinned. ActualAB24B0, then AB2600/default80,
current88/production callers/custody/normal disposition and activation remain
separate. This precise body is not withheld on unrelated producer or fullCRT
history; Root/peer review, official MSVCWin32 and actualCore checks are pending.
No tracked Source/CMake/ledger/Ghidra/compiler/test/probe/provider/Game action.
