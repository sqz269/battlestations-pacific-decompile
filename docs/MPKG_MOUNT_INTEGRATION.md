# MPKG provider, enumeration and VFS reentry

Addresses: `00bb9d90`, `00bb9cb0`, `00bb5590`, `00bb8e70`, `00bb8e80`, `00bb98f0`, `00bb97b0`, `00bee340`, `0043e9a0`, `004bcb80`, `004cdc20`, `00425850`, `00bb9c10`, `00bb9e70`, `00bb9ee0`, `00bb5380`, `00bd30e0`, `00bb5540`, `00bf0fb0`, `00be8120`, `00be80b0`, `00be7fa0`, `00be7760`, `00bdd990`, `00bdbe20`, `00bdbeb0`, `00be1130`, `00be0fc0`, `00bdbfd0`, `00bdd0a0`, `00be0660`, `00bdb040`, `00bdb120`, `00be1890`, `00be1740`, `00bdce40`, `00bdeec0`, `00bdf310`, `00bda690`, `00bb9920`, `00bb8be0`, `0073cb10`

The existing `MpkgArchive` can be bound to mounted opening without inventing a
physical-file source. Its provider factory opens the supplied **logical path**
through the current VFS before registration; large stored entries later reopen
that same logical path through the current VFS again. Enumeration is now a
separately recovered, implementable operation. Its filter differs from normal
filesystem extension and directory matching in several material ways.

This packet is read-only analysis. It adds no C++, Ghidra annotations, tests or
startup integration. Earlier archive-entry and package-startup notes are prior
art; the decisive spans below were reread and compared with the installed PE.
The archive parser/materializer itself is documented in
[MPKG_ENTRY_LOADING.md](MPKG_ENTRY_LOADING.md).

## Factory and provider identity

`00bb9d90` is the MPKG factory virtual `+4`: factory in ECX, system-path and
virtual-path native-string pointers on the stack, provider in EAX, `RET 8`.
The incoming factory pointer and virtual path are not consumed by this body.
It accepts a system-path stored length **greater than 5**, takes its last five
characters, and calls `00425850` with `.mpkg` at `00d1d818`. That helper uses
CRT `_stricmp` for nonnull strings. Thus `x.MPKG` passes this gate, while the
five-character string `.mpkg` does not. There is no directory existence check,
header check, path normalization or extension stripping in the gate.

On acceptance it allocates `0x18` bytes and calls `00bb9cb0`. This constructor
has object in ECX, one system-path stack argument, EAX object and `RET 4`.
Base constructor `00bb5590` initializes intrusive count `+4=1`, copies the
supplied name into provider `+8/+C`, and initializes device id `+10=-1`.
The derived constructor installs table `00d64390`, allocates `0x34` archive
state bytes, constructs that state with the same path through `00bb9920`,
and stores it at provider `+14`. No caller-supplied virtual prefix is copied
into the archive's entry names.

The complete 44-byte provider table is:

| Slot | Target | Observed operation |
|---|---|---|
| `+0` | `00bd30e0` | Calls virtual `+4` with deleting flag 1; plain RET |
| `+4` | `00bb9ee0` | Scalar deleting destructor, ECX provider, flag stack, RET 4 |
| `+8` | `00bb8e70` | Replace ECX with `[ECX+14]`, tail-jump to entry open `00bb8d60` |
| `+C` | `00bb9d30` | AL=false, RET 10h; operation meaning remains unassigned |
| `+10` | `00bb8e80` | Replace ECX with `[ECX+14]`, tail-jump to exists `00bb8e00` |
| `+14` | `00bb98f0` | Replace ECX with `[ECX+14]`, tail-jump to enumeration `00bb97b0` |
| `+18` | `00bb8640` | AL=false, RET 4; operation meaning remains unassigned |
| `+1C` | `00bb5540` | Copy second string argument to first, EAX first, RET 8 |
| `+20` | `00bb9d50` | Zero five DWORDs at first output argument, RET 8 |
| `+24` | `00bf0fb0` | Exists then copy requested logical name on success, AL result, RET 8 |
| `+28` | `00bb9d40` | Plain RET; operation meaning remains unassigned |

`00bb8e60` is **not** an open thunk: it is a `RET 4` within the preceding
exists function. Open begins at `00bb8e70`. The open/exists thunks inherit
their target ABIs: ECX state, name/flags and RET 8 for open; name and RET 4
for exists. Open rejects flags bit 0, otherwise the flags do not change
`00bb8be0`'s three entry routes. The host mounted API currently limits read
flags to exactly 2 and `0x32`; retaining that limit is a host domain choice.

`00be8120/00be80b0/00be7fa0/00be7760` are a separate FileStore path. The
factory matches `filestore`, lazily returns one provider from factory `+8`,
and constructs two empty trees. `00be7760` inserts a normalized key into
the primary tree, retains the supplied stream only on new insertion, and
keeps the first duplicate. These calls do not register MPKG factories or
mount archives. Existing FileStore implementation details remain in
[ARCHIVE_PROVIDER_ENTRY.md](ARCHIVE_PROVIDER_ENTRY.md).

## Exact archive enumeration

`00bb97b0` has ECX archive state and **four** stack arguments: directory
string, extension string, flag DWORD (only its low byte matters), and output
string vector. It ends with `RET 10h`; the saved decompiler omitted an
argument. It scans state `+28` entries from index zero through count `+2C-1`,
using the existing 36-byte central-directory order. Each accepted entry
appends a copied **whole stored name** through `004cdc20`. It does not clear
the supplied vector, strip a directory, add a virtual mount prefix, sort,
deduplicate, consult entry method/size, or resolve a local header.

There is a misleading directory temporary: `00bb97cb..00bb9875` copies the
directory and appends `/` if its last byte is not `/`. The subsequent filter
call does **not** use that copy. At `00bb9886..00bb9896`, after accounting for
the two pushes, ECX is loaded from the original first stack argument. The
temporary is only destroyed afterward. An empty directory can cause the
temporary's last-byte check to read before its buffer (`00bb9814`); a bounded
host wrapper should reject it explicitly rather than reproduce that access.

The shared filter `00bee340` has directory string in **ECX**, extension string
in **EDX**, then flag and candidate-name pointers on the stack; AL result,
`RET 8`. For ordinary valid NUL-free native strings its exact rule is:

```text
require case-sensitive candidate prefix equals directory
if uint8(flag) == 0:
    slash = last index of '/' in candidate, or UINT32_MAX when absent
    require slash <= directory.stored_length  // unsigned comparison
start = uint32(candidate.stored_length - extension.stored_length)
require case-sensitive candidate substring at start begins with extension
```

`0043e9a0` performs the case-sensitive byte comparison, rejects an unsigned
start greater than the candidate's stored length, and succeeds when the
pattern reaches its terminating NUL. Thus a longer extension underflows the
subtraction and fails the offset check. It adds no dot or separator rule.
`004bcb80` scans backward for the final `/` and returns `FFFFFFFF` when absent;
the filter uses unsigned `JA` at `00bee37b`, not a signed comparison.
Backslash is not treated as `/` by these functions.

Assembly-derived examples, not newly executed tests:

| Directory / extension / flag | Candidate | Result and reason |
|---|---|---|
| `.` / `mpkg` / 0 | `./patch2.mpkg` | Accepted |
| `.` / `mpkg` / 0 | `patch2.mpkg` | Rejected by directory prefix |
| `.` / `mpkg` / 0 | `./patch2.MPKG` | Rejected by case-sensitive suffix |
| `foo` / `mpkg` / 0 | `foo/barmpkg` | Accepted; no dot boundary is required |
| `foo` / `mpkg` / 0 | `foo.mpkg` | Rejected; absent slash is UINT32_MAX |
| `foo` / `mpkg` / 1 | `foobar/sub.mpkg` | Accepted; no directory boundary is required |
| `foo` / `mpkg` / `0x100` | `foobar/sub.mpkg` | Rejected; low flag byte is zero |

These rules must not be silently replaced with case-insensitive extension
matching, basename extraction, a normalized trailing slash, or a host
filesystem search. Host bounds, NUL/ASCII domain limits and allocation errors
should be labeled separately from native successful behavior.

## Query aggregation, scan order and registration

`00bdd990` receives ECX manager plus directory, extension, flag and output
list, `RET 10h`. It copies the extension and low flag byte into callback
`00bdbe20` and calls mount traversal `00bdd0a0` on the supplied directory.
Neither function normalizes that query or applies open aliases. Traversal
checks mount prefixes and passes a provider-relative suffix. At an empty
mount prefix, startup directory `.` therefore remains `.`.

`00be1130` receives callback ECX, mount record and suffix on the stack,
`RET 8`. It calls provider `+14` with suffix/extension/flag/output-vector,
then visits that temporary vector in ascending order. It forwards each name
unchanged to `00be0fc0`; there is no reattachment of the virtual mount prefix.
The latter appends to the output list only if `00bdbfd0` finds no equal stored
length plus `_stricmp` match. First spelling and encounter order survive.
Callback `+8=00bdbeb0` returns AL=false, so a provider producing results does
not stop traversal. This is mount order followed by provider order, with
case-insensitive deduplication at the manager layer only.

Startup `0073cb10` asks for `.` / `mpkg` / flag 0, consumes the front of the
result list, skips names already mounted according to `00bdb120` (provider
system name, equal length plus `_stricmp`), computes the existing priority
helper, and calls `00be1890(name, ".", priority, 0, -1)`. For an MPKG provider,
an enumerated `./patch2.mpkg` retains that spelling; the priority helper sees
the whole string and consequently assigns 1000, since `patch` does not start
at index zero. This is a consequence of the verified routines, not evidence
that installed packages use these particular stored names.

The startup call pair at `0073d881/0073d888` performs two fresh scans. The
first scan's query result is fixed before it begins mounting those names;
the second query can see providers registered by the first. There is no
sort between query and mounting. The reason for exactly two calls remains
an inference. No installed archive was available in the earlier read-only
archive search; this packet makes no additional installed-data claim.

`00be0660` appends factories in registration order; `00bdb040` tries virtual
`+4` until the first nonnull provider. Existing startup evidence establishes
physical directory, FileStore, then MPKG. `00be1890` constructs the provider
**before** calling `00be1740`, sets provider device id from argument five,
then registers the virtual prefix and priority. Existing registration code
implements signed descending priorities and insertion after equal priorities.
The native registration copies a raw provider pointer and a byte; no inferred
reference acquisition or teardown meaning should be added to that byte.

## Lifetime and original-source reopening

Provider `00bb9e70` destroys and frees its archive state at `+14`, then calls
base destructor `00bb5380` to release the copied system-name string.
`00bb9ee0` optionally frees the provider object when deleting flag bit 0 is
set. Archive-state destructor `00bb9c10` decrements/releases its decoded stream
at `+C`, destroys entry-name records and the entry array, and destroys its
separate path string at `+4/+8`. The full tail after `_free` was read: it
continues through `00bb9ca0`; the decompiler's earlier truncation is not a
no-return contract. The manager's complete unmount/shutdown policy and the
registration byte's role in it remain outside this packet.

Construction copies the supplied logical name into archive state, then
`00bb998d..00bb99a0` reads **the current global** `0109ceec`, dispatches
manager virtual `+4`, and supplies that path plus flags 2. It converts the
returned stream to memory and releases the temporary source. This occurs
before the new provider is inserted, so that provider cannot select itself
during its own ordinary construction.

For method-zero entries larger than `0x40000`, `00bb8ce7..00bb8cf8` again
reads current global manager `0109ceec`, supplies state `+4` and flags 2, and
then passes the returned source to the adopting range reader `00bf1130`.
There is no saved physical handle, fixed original provider, self-provider
exclusion, or recursion guard. Manager open `00bdf310` normalizes a copy,
applies one current alias through `00bdca80`, and traverses the current mount
order; `00bda690` dispatches each provider open and records the first nonnull
stream. Source identity can therefore differ from the source chosen during
construction if mounts or aliases change. “Original source” means reopening
the original **logical path**, not asserting immutable physical bytes.

The existing host materializer intentionally buffers the declared large-entry
range; native `00bf1000` remains an unclamped forwarding reader. A host VFS
callback must preserve logical reentry and actual-count/error reporting. It
must not silently reopen a disk file or substitute decoded archive backing.
An explicit failure on a recursive reopen cycle is an added host guard. A
weak/current-context capture avoids a host ownership cycle of context ->
mount callbacks -> archive -> reopen callback -> context; capturing a frozen
context snapshot would change native provider selection after later mounts.

## Smallest coherent next API

First add archive enumeration and the mounted adapter; no general provider
framework is needed. Suggested concrete interfaces for the primary integrator:

```cpp
// New MpkgArchive member. Append whole names in central-record order.
bool enumerate_00bb97b0_fragment(std::string_view directory,
    std::string_view extension, std::uint32_t flags,
    std::vector<std::string>& output, std::string& error) const;

VfsMount bind_mpkg_archive_fragment(std::string prefix,
    const std::shared_ptr<MpkgArchive>& archive);

using MpkgLogicalOpen = std::function<VfsMemoryOpen(
    const std::string& logical_path, std::uint32_t flags)>;
enum class MpkgCreateStatus { declined, loaded, failed };
MpkgCreateStatus create_mpkg_archive_00bb9d90_fragment(
    const std::string& system_path, MpkgLogicalOpen open_current_vfs,
    std::shared_ptr<MpkgArchive>& output, std::string& error);
```

The factory helper applies only the recovered suffix gate, invokes the supplied
current-VFS opener with flags 2 for initial bytes, and captures the unchanged
system path plus that opener for large-entry reopening. A small owned
MemoryStream-to-InflateSource adapter supplies independent cursor and actual
counts. The caller must provide a live, lifetime-safe context binding; the
helper must not capture the current source or a context snapshot as a shortcut.
Factory decline, host load failure and success remain distinguishable. Failed
construction must not register a partially loaded provider.

The mount adapter captures archive ownership, uses `contains_00bb8e00`,
implements shared `00bf0fb0` resolution, and calls entry materialization for
flags 2/0x32. A matched entry that fails host bounds/decompression/buffering
must produce a terminal diagnostic rather than fall through and silently load
a lower-priority resource. That policy is a host failure boundary; malformed
native-provider behavior is not established. Missing entries still permit
normal traversal fallback.

Full package scan integration additionally needs a provider-enumeration
callback in the shared mount representation, manager-order aggregation and
deduplication, and registration metadata retaining the original **system
name** for `00bdb120`. The existing `VfsMount` has neither enumeration nor
system-name identity; the virtual prefix cannot substitute for it. Physical
and FileStore enumeration order/filter contracts must be available before
claiming a complete two-pass startup scan. They were not reconstructed here.
No broad test suite is proposed; an existing-probe scenario can later compare
two logical reopen providers and the enumeration quirks once integration is
authorized and reviewable.

## Evidence and remaining boundaries

Every live export batch verified project `bsp`, program
`/battlestationspacific.exe`, x86 LE and base `00400000`, using `Client` with
`config/target.json`. Installed PE SHA-256 is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Full ranges and all exploratory windows matched its disk bytes. The complete
range list, exact ABIs, target snapshots and proposed names are indexed in
[mpkg_mount_integration_audit.json](../reports/mpkg_mount_integration_audit.json);
raw exports and Capstone disassembly are ignored under
`exports/bsp/parallel_mpkg_mount/`.

Decisive complete-body SHA-256 identities:

| Start | Bytes | SHA-256 |
|---|---:|---|
| `00bb97b0` enumeration | 319 | `0164b11c0dbcc07f24e5d53d65d5502ae5ef9d909000476e824f2e646fca9b90` |
| `00bee340` filter | 80 | `9344ea2e15d5bbdad619ff5284c46806f3c3a5f1f6e059f83e9bfec3fdc83a3b` |
| `0043e9a0` case-sensitive match | 88 | `f5235f47d7eb4a353170b67564fe0e481f1d0dbeb1ca00c54ec8427723e8a200` |
| `004bcb80` reverse byte-set search | 120 | `67a509a06f321bdd3896ea560fb586f490904711b06c890f219bf30f80b098c4` |
| `00be1130` query callback including tail | 229 | `8ff18707b0dc279b87eaab60a5a6505a20bf87664bc7a684d1d05fe04c451909` |
| `00bb9d90` factory | 211 | `f29b9e0da7b9117c1397ac017446b00b782a94cb9de0a799d32e2d4ad2c0121b` |
| `00bb9c10` state destructor including tail | 145 | `27e72a6707206cee9bb5612a6d27462c8347867bd6ddf3e5994182d3c0e53901` |
| `00bb9e70` provider destructor | 99 | `b26519a41333a39f609d7235f4171335c6a70ddef2a25d48850a9b640df4973d` |
| `00bdf310` manager open | 421 | `e354c379eeae3b582f71333f00c627d34b1d19e8c2c835c861922bda2cd7e748` |

Compiler/library identities were preserved, and no project save or annotation
was attempted. This establishes static contracts, not native differential
execution, real installed archive loading, allocator/ABI compatibility,
unmount policy, malformed-input equivalence, or game startup validation.
