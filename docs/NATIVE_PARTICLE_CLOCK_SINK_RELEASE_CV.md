# Raw particle clock sink release (CV)

`004DDB40..004DDB5E` is the complete 31-byte release leaf. The original ECX
container is unused. The actual sink occupies one stack word and the function
returns with `RET4`. Its existing typed full-function record remains; this raw
interface upgrade adds zero native body or byte credit.

The routine captures the original public sink word in ESI, calls the current
InterlockedDecrement IAT cell `00CE2220` with `sink+4`, and tests the returned
count. Only when it is zero does it reload the actual sink profile and slot0,
set sink ECX and call the slot with no stack arguments. In particular, it does
not push a deleting-destructor flag. There are no null/profile/count checks.

The new source interface uses EDX for a borrowed reference to the current IAT
cell. The original sink stack slot, retained ESI, profile reload, virtual call
and `RET4` remain. Only the IAT call encoding changes from absolute `[00CE2220]`
to `[EDX]`; the full generated instruction stream is checked against that
explicit transformation of the installed/live body.

This entry uses actual native storage and a concrete Win32 atomic provider.
It does not interpret a host `ParticleClockOwnedSink` C++ vtable as native.
The extra IAT-cell argument is a new source ABI. Concrete nonempty particle
sink destruction, the full clock shutdown chain, original private EH/CRT
identity and gameplay validation remain separate boundaries. The CU read-only
audit records the remaining container destructor and EH dependencies.

## Primary integration validation

Exact source `3a92529df59b7dbfa8ae1f6e7a82f648570f323c` passed MSVC Win32 Release and both existing math CTests with 2601 unchanged tracked inputs and a clean tree before/after. Complete27-byte generated body equals all original31 bytes after replacing the six-byte absolute CALL[00CE2220] at+9 with two-byte CALL[EDX]. Zero relocations. Sink stack capture+1, sink+4 address+5, actual current IAT call+9, result test+Bh, current profile/slot reload+Fh/+11h, actual sink ECX+13h, no-argument slot0 call+15h, RET4+18h. One focused raw source probe passed the real Win32 decrement and fixture slot0 call only on zero; concrete particle-owner destruction, native parent execution and full clock shutdown remain untested. Existing full-function body credit remains unchanged.
