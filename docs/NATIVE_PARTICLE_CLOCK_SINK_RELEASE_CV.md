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
