# MPKG provider adapter and enumeration implementation

`mpkg_provider.hpp/.cpp` connects an owning `MpkgArchive` to the existing VFS
callback representation. It implements the MPKG suffix factory gate, initial
opening through a supplied current-VFS callback, later logical-source reopens,
and mounted open/exists/resolve/enumerate operations. `resource_enumeration`
implements the exact shared predicate at `00bee340`; it is not a generic
replacement for physical-directory or FileStore filtering.

The static contract and full native byte hashes are in
[MPKG_MOUNT_INTEGRATION.md](MPKG_MOUNT_INTEGRATION.md) and
[mpkg_mount_integration_audit.json](../reports/mpkg_mount_integration_audit.json).
No new live analysis was necessary for this implementation. The native object,
intrusive references, allocator, registration and virtual-table ABI are not
reproduced by these C++ interfaces.

## Enumeration

`resource_enumeration_match_00bee340_fragment(directory, extension, flags,
candidate)` returns `ResourceEnumerationMatch::match`, `no_match`, or
`unsupported`. The native ABI is directory in ECX, extension in EDX, flag and
candidate on the stack, AL result and RET 8. The host uses bounded string views.

Within its supported domain, it requires a case-sensitive directory prefix
and extension suffix. No dot or directory-separator boundary is added. When
the flag's low byte is zero, the candidate's final `/` index must be at most
the directory length. A missing slash is the native unsigned `FFFFFFFF` result
and rejects. Other flag bits do not change the predicate. Backslash receives
no special treatment, and no name is normalized or case-folded.

The host rejects empty directories, embedded NUL in any argument, and lengths
above INT32_MAX as `unsupported`. Empty extensions and candidates are supported;
an empty candidate is an ordinary no-match against the required nonempty
directory. Non-ASCII bytes are compared exactly. These guards avoid native
pre-buffer access in the archive enumeration wrapper and ambiguous C-string
termination behavior. They do not redefine those failures as ordinary no-match.

`MpkgArchive::enumerate_00bb97b0_fragment` corresponds to ECX archive and
directory/extension/flag/output-vector stack arguments, RET 10h. It passes
the original directory to the predicate. The native temporary with an appended
slash is deliberately omitted because the filter never receives it. Accepted
whole stored names append in central-directory order; existing output remains
first, and duplicates are preserved. No local header is resolved or payload
read during enumeration. Query validation runs even for an empty archive.

The host stages matching names before appending, so an unsupported stored name,
query or output count leaves output unchanged. It supports output counts through
INT32_MAX and propagates allocation exceptions. Manager-level first-spelling
deduplication remains separate. Physical and FileStore provider policies use
different native routines and must not be inferred from this predicate.

## Factory and source reentry

`create_mpkg_archive_00bb9d90_fragment(system_path, opener, output, error)`
returns `MpkgCreateStatus::declined`, `loaded`, or `failed`. Decline and guarded
failure preserve the existing output owner. The native factory has ECX factory,
system/virtual paths on the stack, EAX provider and RET 8. Native virtual-path
and incoming factory are unused in this body, so the host signature omits them.

The path must have stored length greater than five and end in `.mpkg` under
CRT `_stricmp`; the five-byte string `.mpkg` declines. There is no filesystem
query, normalization, basename extraction or header check in this gate. An
empty/nonmatching path declines; embedded NUL or length above INT32_MAX is an
explicit host failure. Allocation and caller callback exceptions propagate.

`MpkgLogicalOpen` is:

```cpp
std::function<VfsMemoryOpen(const std::string& logical_path,
                          std::uint32_t flags)>
```

On an accepted gate the factory calls this opener with the unchanged supplied
logical path and flags 2. The returned memory must have fully initialized,
positive backing through INT32_MAX. Archive construction uses the entire
backing without moving the returned stream's cursor. A failed load is never
published as a provider. Factory construction does not itself register a mount,
set its priority/device metadata, or implement the startup factory list.

The archive retains the same callable state and a copy of that unchanged path.
Every large stored-entry reopen calls the opener again with flags 2. Each
successful result is adapted by a fresh `ReopenedMemorySource` retaining a
cursor-zero `MemoryStream::clone_reset_00bef6d0`. Thus source cursor movement
does not change the VFS-returned wrapper's cursor. The adapter reports actual
counts and uses the existing bounded seek/read behavior. It captures neither
the initial source nor a decoded-backing substitute.

The callback must consult the live VFS context on every invocation. The caller
should capture a weak owner of that context and report failure when it has
expired. Capturing a frozen copy changes native source selection after later
mounts or aliases. Capturing an owning context that itself retains this mount
would create a context -> mount -> archive -> callback -> context ownership
cycle; the factory cannot rewrite ownership hidden inside a caller's arbitrary
callable. The caller must not mutate the mount collection during traversal.

`MpkgReopenSource` now accepts a `std::string& error` output and returns an
owned `InflateSource` or null. This small host API change preserves a specific
current-VFS failure through `MpkgArchive`; a null result without text receives
the existing generic diagnostic. Success clears a stale callback diagnostic.

## Recursion, failure and ownership

`MpkgArchive` guards its entire large original-source materialization, covering
the reopen callback, seek and subsequent reads. A second large materialization
through the same archive while that operation is active fails with
`MPKG recursive original-source reopen was rejected.` The guard resets on
every return and exception. It also catches a chain from archive A through B
back to A. Small copied and raw-inflated entries remain callable during source
reentry; they do not repeat this archive's original-path opener.

This is an explicit host guard. Native large entries reread the global manager
and contain no observed provider exclusion or recursion limit. The guard does
not silently skip a provider or choose a physical fallback. Archives retain
their existing single-threaded domain: the lazy entry cache and the new
materialization state must not be concurrently accessed or moved/destroyed
during an operation.

`bind_mpkg_archive_fragment(prefix, shared_archive)` captures archive ownership
for all four callbacks. Membership uses the recovered ordered lookup. Resolve
implements shared `00bf0fb0`: existence followed by copying the requested
provider-relative logical name. Enumeration binds the operation above.

Mounted open accepts exactly flags 2 and `0x32`, the existing host VFS domain.
Missing entries permit traversal fallback. Once an entry matches, the adapter
marks the result terminal using `provider_opened`, even when a subsequent host
guard, source callback, decompression or copying fails. Its diagnostic remains
visible, and lower-priority bytes cannot silently replace the selected resource.
This use of the traversal stop flag is a host failure policy; malformed native
archive behavior is not established. Successful entries have independent copied
memory backing and survive archive/source destruction, as in the existing
materializer. A null archive produces an unbound mount with the supplied prefix.

Native provider/state destructors establish provider ownership of archive state
and archive ownership of decoded backing and copied names. Native manager
unmount/shutdown and the registration byte's teardown meaning remain outside
this implementation. Host captured shared owners implement their own lifetime;
they do not claim native reference-count or allocator compatibility.

## Integration and validation

The Win32 build, both existing CTests and full D3D9 probe pass. The existing
synthetic archive now also passes mounted-provider checks: construction opens
`Fixture.MpKg` unchanged with flags2; a later VFS alias selects changed source
bytes; a recursive source alias returns a diagnostic; clearing that alias
restores successful reopening; destroying the weakly bound current VFS reports
its expired-owner error. Each opened source uses an independent cursor.

The same fixture checks positive enumeration despite an active alias, manager
flag-byte truncation, MPKG case-sensitive filtering, the no-slash/nonrecursive
rule, and preservation of a preexisting differently cased spelling. This does
not independently establish multi-entry MPKG enumeration order, which is
source-reviewed. FileStore's absent-extension/length+1 sentinel is also checked
using the fixture's existing source store. No new test target was added.

See `reports/parallel_provider_validation.json`. Real installed archives,
native differential execution, complete startup scans, registration metadata,
unmount lifetime and original-game behavior remain unvalidated.
