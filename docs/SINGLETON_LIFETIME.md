# Singleton lifetime manager

The getter00415350 publishes a20-byte manager at01090aa0. On a cold call it
allocates, calls00bd0960 and stores the returned pointer; on a warm call it
returns the existing EAX pointer. The getter itself has no synchronization gate.
Its decompiler void/register-argument signature is misleading: assembly returns
EAX with plain RET. Native allocation failure leaves the global null.

The manager constructor00bd0960 takes ECX=this, returns EAX=this and plain RET.
It initializes pointer-vector begin/end/capacity fields+4/+8/+C to zero, calls
reserve00bd0600 with256 elements, and creates the existing tracked critical
section helper00bd1860 into+10. Offset0 is not initialized by this constructor.
Reserve's omitted post-free continuation retains element count when installing
the replacement allocation; the vector contains four-byte pointers.

Registration00bd0c30 takes ECX=manager and one pointer stack argument, RET4.
It checks end>=begin through the native invalid-parameter path, ignores null,
and appends non-null through00bd0bc0. There is no object AddRef, duplicate check,
or lock inside registration. The append routine uses remaining capacity or
the native vector insertion/growth helper00bd08d0. A future typed port must
preserve ownership and ordering without claiming its allocator/iterator ABI.

The typical caller00b3e730 first checks its own singleton0108fedc, obtains the
manager, optionally enters manager+10, rechecks, creates an eight-byte object via
00b61d50, publishes it, then calls registration. It decrements tracked lock
depth and leaves after registration. This registers the new singleton for
shutdown; no surface pointer is passed to this helper. The surface constructor's
call alone does not prove registration of a surface with the reset list.

## Shutdown order

Destructor00bd0400 takes ECX=manager and plain RET. While count00bcf910 is
nonzero, it reads the final pointer, removes it from the vector, then invokes
object virtual slot0 with deleting flag1 if non-null. It reevaluates live count
after each callback, so newly appended objects participate in the ongoing drain.
This is LIFO, with pop-before-delete; snapshotting the initial list would change
behavior. Count00bcf910 returns zero for null begin or arithmetic `(end-begin)/4`.

Only after draining does it destroy the tracked critical section, free vector
storage, and zero begin/end/capacity. Original pointer validation, reentrancy,
allocator and exception behavior are not yet implemented as a typed owner.
In particular, it is not sufficient to destroy objects in registration order.

WinMain008f8449..008f846c calls this destructor, frees the manager, then clears
global01090aa0 at008f8463. That clear and its ADD ESP,4 are missing from the
ordinary export following free. The manager destructor likewise has omitted
post-free stack cleanup in its listing. Raw installed/saved bytes establish the
continuations; no claim is made that the existing Ghidra bodies are repaired.

## Evidence and implementation status

`reports/singleton_lifetime_audit.json` records complete disk/live hashes for the
getter, constructor, registration, append, destructor and count helper, plus
the bounded WinMain shutdown sequence. Each batch verified `bsp`, program
`/battlestationspacific.exe`. Descriptive names and comments are saved in Ghidra
with previous annotations preserved. These functions are analyzed, not added
to reconstruction coverage. No code or test was needed for this evidence audit.

This manager is an actual startup/shutdown dependency. Its registration of
singleton objects is distinct from the renderer's live resource reset list;
SURFACE_REGISTRATION_AUDIT records the latter's explicit ownership path.
