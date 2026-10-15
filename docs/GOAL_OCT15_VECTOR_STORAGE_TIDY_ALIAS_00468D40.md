# Opaque storage tidy at 00468D40

The complete ordinary Source entry covers 00468D40 through 00468D69:
42 bytes and 13 instructions. The native library name
`STL_Vector_Tidy_00468d40` is retained. It receives opaque 16-byte storage
in ECX, consumes no EDX or stack argument, preserves ESI and uses plain RET.
There is no semantic return value; incidental EAX differs by branch.

Capture DWORD +4 once. If nonzero, free that captured pointer through the
actual current CRT service. On either returning path write zero at +4,
then +8, then +Ch. The routine never directly accesses +0. No element,
field-role, allocator, destructor or ownership interpretation is inferred.

The entire body matches the previously accepted 00468840 body except the
four-byte relative displacement of its free CALL. Both native calls resolve
to 00BF65AC. Source delegates to that complete implementation, including
its actual current CRT free boundary. There is no native 00468D40-to-00468840
call. The existing translation unit is already registered once.

Actual native CALL 00468D4B targets 00BF65AC. Two incoming cleanup tail jumps
are at 00C6200C and 00C6208C; this packet does not reconstruct their enclosing
private frames or expand its coverage to those cleanup bodies.

All reached raw storage and releases must be valid for the current CRT.
This does not prove original heap, static CRT, register/flags/private-frame,
FH3/SEH, fault or mangling compatibility. No new-entry native differential,
application adoption or game validation is claimed. Default Win32 build,
independent review and whole live/installed-byte checks are pending.
