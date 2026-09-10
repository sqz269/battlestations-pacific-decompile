# Actual pooled-string cleanup and ASCII lowercase

`destroy_native_string_header_0041dd20` reconstructs the full 29-byte native
body against an existing eight-byte header. It captures data at +4 first.
Null data returns without reading length or calling storage. Otherwise it
captures length+1 with DWORD wrap and returns the captured pointer through
the supplied storage. It does not reset either field, end the header's C++
lifetime, or undo changes made during release. The original ECX/RET ABI and
singleton lookup are replaced by the explicit existing storage interface.

`lowercase_native_string_header_004bcc00` reconstructs the full 51-byte native
body against the same actual header. It captures data once and returns for
null data or zero length. Each iteration reads one byte, converts only ASCII
A..Z, and stores the resulting byte even if unchanged. Bytes after embedded
NUL remain part of the counted string. Its unsigned loop index is compared
with the current header length after each store; the implementation does not
cache that bound. Cursor and index additions retain Win32 wrapping arithmetic.
High-bit bytes are unchanged, matching the native signed-byte comparisons.

The earlier `lowercase_resource_name` interface remains available in
`resource_path.cpp` for semantic string consumers. Its prior ledger record is
preserved under the actual helper's record; the same native address is counted
once. No new allocator, temporary string owner, global lifetime policy or
normalization algorithm is introduced by these two entries.

Both complete native spans are compared with the installed PE and live Ghidra
program. Full assembly review and the strict Win32 build with the existing
tests provide the scoped validation; no new routine-helper fixture is added.
The actual resource-path normalization chain and complete resource-container
removal remain separate work. Evidence, saved prior annotations and current
source hashes are in `reports/native_pooled_string_cleanup_lower_audit.json`.
