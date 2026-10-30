# Raw wide construction from narrow bytes

The new raw-context overload of `construct_native_wide_string_header_004c5e60`
covers the complete32-instruction normal constructor (87B, RET4/EAX=this).
It uses the caller's SAME AA8/AA4/AA0 pool context through genuine raw4C53E0.
The existing storage overload and all existing providers remain unchanged.

It clears actual raw8 LENGTH then DATA before scanning unsigned source bytes
through NUL. Wrapping scan subtraction supplies the preserve1 resize request.
After resize it captures CURRENT destination DATA once; nonnull data receives
streaming unsigned BYTE-to-WORD stores through NUL. This loop rereads source
bytes and is not bounded by the earlier measured length. Header/source/backing
aliases retain the reached store/read order; no semantic text or locale
conversion, wchar_t cast, output Frame or storage adapter is introduced.

Header/WORD accesses use fenced byte copies; BYTE reads are volatile/fenced.
Caller supplies valid reached extents/lifetimes and retains actual pool/cells,
provider graph and all partial failure debt. Fresh constructor storage is
expected: old header backing is abandoned without release. No NULL-input
guard, local cleanup, catch, retry, rollback or forced allocation success exists.
Empty input follows actual clear/scan/equal-zero resize/null-DATA behavior.

This is an ordinary MSVC Win32 interface. Native registers, stack aliases,
FH3/SEH/hardware timing and current allocator/CRT faults remain qualified.
Raw resize retains its existing overlap-copy and zero-byte-call omission.
No startup, loaded manager, consumer permission, ABI/runtime/game validation
or duplicate Native/vendor-body credit follows. The report separates Source,
one-object compilation and Root's later registration/common-build evidence.
