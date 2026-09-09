# Physical-directory and FileStore enumeration
Addresses: `00bf47e0`, `00be6480`, `00bee520`, `00bee390`, `00bf3970`, `00be4c30`, `00be54d0`, `00443d00`, `00bdd990`, `00bdd0a0`.

Physical provider `00bf47e0`, FileStore `00be6480`, and MPKG `00bb97b0` use
different matching and output-name rules. Neither physical nor FileStore
enumeration calls the shared `00bee340` predicate. The audited contracts now drive bounded C++ implementations in
`src/physical_directory.cpp` and `src/file_store.cpp`.

## Native ABI and dispatch

Both enumeration methods have **ECX provider**, then four native stack
arguments in order: directory string pointer, extension string pointer,
flags DWORD, output string-vector pointer. Both end with `RET 10h` and have
no semantic success/error return. A host bool/error interface is a new API.
The physical decompiler's `unaff_retaddr` and misleading parameter count
are contradicted by the stack loads and recursive call sequence.

Physical table `00d69168` has enumeration `+14=00bf47e0` and path builder
`+1C=00bf3970`. FileStore table `00d689e8` has enumeration `+14=00be6480`.
Both tables were reread in full through slot `+28` and matched installed PE.

## Physical access path versus returned logical name

At `00bf4800..00bf4815`, physical enumeration calls provider virtual `+1C`
with output and the supplied directory. `00bf3970` concatenates the provider's
unchanged root with the directory, inserting **no separator**, then converts
`/` to `\` only in the appended directory portion. Its ABI is ECX provider,
output/suffix stack arguments, EAX output and RET 8.

Enumeration then examines the last byte of that physical path. If it is not
`\`, it appends one through `0054aa70`; it copies this path and appends the
one-character literal `*` from `00cfe9c4`. That full pattern is supplied to
`FindFirstFileA`. It does not append `*.*` or add the extension to the pattern.
Root bytes are not independently canonicalized here. A trailing `/` in a root
with empty suffix remains `/` and causes another `\` to be appended.

There is **no empty-path check** before the last-byte access at `00bf4821`.
An empty directory is usable when root construction produces a nonempty path;
an empty root plus empty directory reaches a pre-buffer read. A host adapter
should reject an empty constructed path explicitly. Treating every empty
directory as native failure would unnecessarily exclude the nonempty-root case.
The routine does not read the physical provider's existence-cache fields,
constructor flag `+28`, or indexed-name count `+34`; enumeration follows this
OS route even when existence uses its persistent-data shortcut.

Returned names and recursive directory arguments take another path:
`00bee520` has **ECX destination string, EDX directory string, child string
on the stack**, EAX destination, RET 4. Its rule is:

```text
if directory.stored_length == 0: canonicalize(child)
else if child.stored_length == 0: canonicalize(directory)
else: canonicalize(directory + "/" + child)
```

Both concatenations use `004261a0`; canonicalization is `00bee390`. This
lowercases through native CRT `tolower`, changes both separator spellings to
`/`, collapses separators, removes dot segments, and applies the previously
audited parent-segment cursor algorithm. It does not trim spaces or reject
unresolved leading parents. It does **not** call the separate trailing-slash
trimmer `00584110` used by mount registration. Reusing the full mount-prefix
preparation wrapper would therefore add a transformation absent from this helper.

For example, physical query `.` finding `Patch2.MPKG` returns `patch2.mpkg`;
query `Foo\Bar` finding `NAME.MPKG` returns `foo/bar/name.mpkg`. These are
assembly deductions, not executed fixture cases. The physical root is never
included in the returned logical name. Unlike MPKG enumeration, returned
physical names are not raw concatenations retaining the leading `./`.

## Physical matching, recursion and order

Each `WIN32_FIND_DATAA.cFileName` is processed immediately in the OS's
`FindFirstFileA` / `FindNextFileA` order:

1. If its first byte is `.`, skip it completely, whether file or directory.
   This excludes all dot-prefixed names, not only `.` and `..`.
2. If `FILE_ATTRIBUTE_DIRECTORY` is clear, require filename C-string length
   **strictly greater than** the extension's stored length. Compare the
   final extension-length bytes through `00425850`, using `_stricmp` for
   nonnull strings. No dot boundary is required. Next reject a filename
   case-insensitively equal to literal `MidwayDL_Content` (`00d69154`).
   Append the canonical joined logical name through `004cdc20` on success.
3. If the directory attribute is set, recurse only when the flags DWORD's
   **low byte is nonzero**. Join/canonicalize the directory and child name,
   then call provider virtual `+14` with the same extension, full flags DWORD
   and output vector. The child completes before the parent calls FindNext.

The `MidwayDL_Content` check exists only in the non-directory branch
(`00bf49ad..00bf49c1`). A directory with that name is still recursively visited.
Directory traversal does not depend on the extension, and directories are not
themselves emitted. There is no hidden/system-attribute exclusion and no
reparse-point exclusion in this body. Reparse cycles/depth limits, if added by
a host implementation, are explicit guards rather than recovered native policy.

The result is depth-first order interleaved with each directory's OS encounter
order, not files-first order and not a sorted vector. Existing output entries
are retained; results append. There is no provider-level deduplication.

An empty extension matches every nonempty ordinary filename through the suffix
comparison at its terminating NUL, subject to the dot-prefix and special-name
exclusions. A filename exactly equal in length to the extension never matches.
Case-insensitive suffix matching here must not be replaced with FileStore's
first-substring rule or MPKG's case-sensitive rule.

## Physical completion and failures

The installed PE import descriptors independently identify:

| IAT slot | Native call |
|---|---|
| `00ce2128` | `KERNEL32.dll!FindFirstFileA` |
| `00ce212c` | `KERNEL32.dll!FindNextFileA` |
| `00ce2130` | `KERNEL32.dll!FindClose` |

An invalid FindFirst handle causes cleanup and return without additions. Any
false FindNext result ends the loop, regardless of reason; existing/appended
names survive, and FindClose is called once. Its return value is ignored.
There is no `GetLastError` call, no distinction between no files and other
enumeration failures, no rollback, and no native success bool. A recursive
call's return register is not inspected: a child producing no names or stopping
early does not prevent the parent from continuing.

Host path/domain errors, allocation exceptions, recursion protection and any
new diagnostic distinction must be identified as host behavior. In particular,
returning false and discarding all output for a FindNext error changes the
native partial-result behavior. FindClose on exceptional host exits is sensible
resource ownership, but no native C++ exception guarantee is inferred from the
straight-line Win32 calls.

## FileStore filtering and tree order

`00be6480` enumerates only the **primary** name tree at provider `+14`, whose
sentinel is stored at `+18`. It starts at sentinel's leftmost node and advances
through `00be4c30`, the ordinary in-order successor: descend to the right
subtree's leftmost node, otherwise climb parents. Existing tree ordering is
empty key first, then CRT `_stricmp` for nonempty keys, with no length
tie-break. Insertion already retains the first case-insensitive duplicate.
Enumeration order is that tree order, not insertion order, archive order or
byte-exact case-sensitive lexical order.

The record's stored name length/data are node `+C/+10`; the stream pointer
at `+14` is not read. No payload is opened, cloned or retained by enumeration.
The **flags argument is never read**, so it does not control recursion or
directory depth. There is no secondary-tree traversal, directory separator
boundary, dot-prefix exclusion, `MidwayDL_Content` exclusion, normalization,
basename extraction or join operation.

For each primary-tree name the actual pointer/length contract is:

```text
if name.data == null or directory.data == null: skip
p = strstr(name.data, directory.data)
if p == null or uint32(p - name.data) != 0: skip

index = UINT32_MAX
if name.data != null and extension.data != null:
    q = strstr(name.data, extension.data)
    if q != null: index = uint32(q - name.data)

if index == uint32(name.stored_length - extension.stored_length):
    append a copy of the whole stored name
```

Both calls to `00bf9440` are the existing CRT `_strstr`, so comparisons are
**case-sensitive** and use the **first occurrence**. There is no search from
the expected suffix offset and no attempt to find a later occurrence. For
example, directory `foo` accepts the prefix of `foobar/sub.mpkg`, but extension
`mpkg` rejects `mpkg/sub.mpkg` because its first occurrence is at zero even
though another occurrence is at the suffix. A suffix `.MPKG` does not match
lowercase extension `mpkg`. The input directory and extension are not normalized
by this method or by manager query `00bdd990`.

The failed-search sentinel and subtraction introduce a real edge case:
an extension exactly **one byte longer** than the stored name makes the DWORD
subtraction `FFFFFFFF`. A failed extension search then compares equal and the
name is appended, provided the directory-prefix test passed. For example,
name `abc`, directory `a`, extension `abcd` passes that final comparison.
The collision also occurs when extension data is null but its stored length
is name length plus one. A general `extension.size() <= name.size()` guard
would remove observed behavior and must not be added silently.

Empty strings require separate attention because a host `std::string` alone
does not retain the native data-pointer state:

| Native state | Behavior |
|---|---|
| Null candidate data | Always skipped |
| Null directory data | Always skipped, even if stored length is zero |
| Nonnull empty directory C-string | `strstr` finds offset zero; prefix passes |
| Null extension data | Index stays `FFFFFFFF`, including its length+1 collision |
| Nonnull empty extension C-string | First index is zero; final test normally fails for nonempty stored names |
| Empty candidate, directory and extension C-strings, all nonnull and stored lengths zero | Can append the empty name |

A bounded host API must state which empty/pointer states it excludes or
represents. It should not claim a single unconditional native rule for an
empty directory. Embedded NUL and inconsistent stored/C-string lengths are
additional representation boundaries. For ordinary nonempty, NUL-free names
and query strings, the first-occurrence/subtraction algorithm above is directly
implementable without an invented extension-length guard.

Accepted names append through `004cdc20` in tree order; no clear/sort/dedup
occurs here. The native routine has no error output. Invalid tree iterators
reach the existing CRT failure helper `00bf6713`; this packet does not assign
a recoverable host error meaning to malformed native tree state.

## Manager query boundary

`00bdd990` constructs the enumeration callback and calls `00bdd0a0` with the
directory query. Full `00bdd0a0` bytes confirm `MOV [EDI+18h],FFFFFFFF` at
**`00bdd0d2`**, before mount iteration. The host manager's
`context.error_code = -1` reset is supported by assembly.

That reset does not create per-provider OS diagnostics. The traversal makes
a copy of the supplied directory, performs mount-prefix matching and suffix
selection, but does not call path normalization or apply open aliases.
An empty-prefix mount receives startup query `.` unchanged. The query callback
deduplicates equal-length `_stricmp` names in provider encounter order;
those aggregation rules remain in
[MPKG_MOUNT_INTEGRATION.md](MPKG_MOUNT_INTEGRATION.md).

## Byte evidence and validation boundary

Every live batch verified `bsp`, `/battlestationspacific.exe`, x86 LE and
base `00400000` through `tools.ghidra_export.Client` configured from
`config/target.json`. Disk executable SHA-256 is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
The report stores full raw hex/Capstone disassembly for compared code spans,
target snapshots, PE import identities and precise ABI/contract records:
[provider_enumeration_audit.json](../reports/provider_enumeration_audit.json).
Previously cached exports were read from ignored
`exports/bsp/parallel_provider_enumeration/`; they were not changed.

| Complete body | Bytes | SHA-256 |
|---|---:|---|
| `00bf47e0..00bf4b6f` | 912 | `22bfdb84f7a28f1575d605e31fd73c6796d0c830f1f279570070e087ed5cef39` |
| `00be6480..00be654c` | 205 | `ebe69187dc5e615e57365b8e5037d35ce1c9340370fb2de2cedc4979782a9c36` |
| `00bee520..00bee683` | 356 | `80b2cba3a3c109ea684cedcf661eaf44846a867d9e700baac181f2eef36ca9d6` |
| `00bee390..00bee512` | 387 | `f3db474665b49451a2e7ec5b0662cbbac9c9531025ba92c19256c2f6eafc134a` |
| `00be4c30..00be4c92` | 99 | `5820cc905f639c22ac31c02a4c5f7a865941908b5f698b7c1909173051c478c8` |
| `00bf3970..00bf39b0` | 65 | `83322b019ce0e47cdec0a1fbce1d04a3d9593917cb2064655650fb18ef603a7b` |
| `00bdd0a0..00bdd33f` | 672 | `6688eee94e647c22020b87083766a158686a073c47ff9d89162e5440bd5d3397` |

Raw physical and FileStore bodies reach their stated RETs. Physical pseudocode
misidentifies stack arguments and string temporaries, but its current cached
assembly includes the whole body. The canonicalizer's heap-buffer path has a
misleading free-tail: raw bytes include `ADD ESP,4` at `00bee505`, then return
the destination in EAX and reach the common RET. Neither helper should inherit
a false no-return or allocator-return contract from truncated decompilation.

This is static evidence, not a host build/probe result or native differential
execution. No fixture, installed-directory enumeration, file mutation, tests,
Ghidra writes or broad provider-manager reconstruction was performed here.
OS ordering, reparse behavior, locale/high-byte handling, allocation failures
and native unsafe string states remain outside runtime validation.

## Integrated validation

`src/vfs_mounts.cpp` now aggregates all matching providers without query
normalization or aliases, forwarding only the native stored flag byte. It keeps
whole provider names in encounter order and deduplicates by equal length plus
`_stricmp`. A guarded host failure preserves the caller's output list; this
transactionality is additional host behavior. Physical and FileStore mounts
expose their distinct enumeration policies through that interface.

The existing installed startup-preload scenario enumerates34 `.lua`-suffixed
names: its five cached names appear first in tree order, followed by29 physical
additions, with cached names appearing once. The existing synthetic MPKG source
store checks FileStore's absent-extension/length+1 sentinel. Both CTests and the
full D3D9 probe pass under the required Win32 build. Recursive physical search,
OS-error partial results and ambiguous native null/empty string states remain
assembly/source-reviewed, not runtime-validated. The bounded interfaces do not
reproduce original provider/container ABI. See
`reports/parallel_provider_validation.json`.
