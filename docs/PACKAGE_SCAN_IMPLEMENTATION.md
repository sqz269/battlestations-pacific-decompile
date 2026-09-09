# Package scan implementation
Addresses: `0073cb10`, `0073d881`, `0073d888`, `00bdd990`, `00bdb120`, `00be1890`.

[package_scan.hpp](../include/bsp/package_scan.hpp) and
[package_scan.cpp](../src/package_scan.cpp) implement one ordered package scan
and the two fresh calls made during startup. The interface takes explicit
current-manager callbacks for enumeration, mounted system-name lookup, and
factory construction plus registration. It does not select a physical source,
normalize a package name, or reconstruct application initialization.

## Observed scan and lookup contracts

Native `0073cb10` has no stack arguments, does not consume incoming ECX, and
ends with plain `RET` at `0073ce15`. It accesses the current global manager
`0109ceec` when querying and when processing names. The host callbacks must
therefore refer to current manager state; capturing an immutable registration
snapshot changes the contract.

Each call invokes the `00bdd990` enumeration contract once with directory `.`,
extension `mpkg`, and flags zero. That manager preserves mount/provider order,
deduplicates by equal length plus `_stricmp`, and keeps the first spelling.
The scanner owns the returned list and processes it from front to back. No
query, sort, normalization, basename extraction or extra deduplication occurs
between mounts. New providers can affect current lookups, but cannot append
names to this call's already completed enumeration.

Priority is computed at `0073cc75..0073cd0e` **before** the mounted-name lookup
at `0073cd19`. The existing `package_mount_priority_0073cb10_fragment` supplies
the audited whole-name rule: ordinary names use 1000; a case-insensitive leading
`patch` adds the Win32 decimal `strtol` result of the entire suffix, with DWORD
addition wrapping. Thus `patch2.mpkg` gets 1002, while `./patch2.mpkg` gets 1000.
Names are passed unchanged to both subsequent callbacks.

`00bdb120` receives manager in ECX and one native system-name string pointer
on the stack, returns the first matching provider or null in EAX, and ends in
`RET4`. It traverses current mounted records and compares provider `+8/+C`
with the requested name: first stored lengths, then `_stricmp` for nonempty
equal lengths. Equal zero lengths match without calling the CRT. Virtual mount
prefixes, basenames, aliases and physical paths are not substitute identities.
The supplied `already_mounted` callback implements this read-only lookup.

For a new name, `0073cd55..0073cd6e` calls `00be1890` with five arguments:
`(unchanged name, ".", priority, 0, -1)`. The last two values are directly
observed ownership-byte and device-id inputs. No teardown policy or device
identity is inferred from them. The manager adapter owns actual factory order,
provider construction, system-name/device metadata and registration; the
scanner does not register partially constructed providers itself.

After the mount returns, the scan ignores its returned provider and processes
the remaining names. The null-provider route inside `00be1890` calls the
manager's error callback before returning null if that callback returns. The
host distinguishes a declined factory chain from a construction/registration
failure, records each outcome, and continues on either returned status. It
does not claim to reproduce error callbacks that terminate or throw. The
primary integrator's concurrent startup audit identifies the callback installed
at `0073d642` as the one-byte `RET` at `00530620`; that configured startup
failure handler therefore returns without side effects. Its evidence belongs
to the primary's provider-manager packet.

Startup has two consecutive calls at `0073d881` and `0073d888`, before default
asset-search registration. `startup_scan_packages_0073d881_fragment` makes
exactly two calls, even when the first returns a host failure report. Each call
gets its own result list. The second can see providers mounted by the first;
it also repeats the current system-name check. There is no third pass or
iteration until discovery stops.

## Host API and failure boundaries

`PackageScanCallbacks` provides three explicit operations. The enumerator has
the existing manager signature `(directory, extension, flags, output, error)`.
It must complete before mount callbacks mutate registrations. `already_mounted`
receives the whole logical system name. `mount` receives all five observed
arguments and returns `mounted`, `declined`, or `failed` with an optional
diagnostic. These callbacks must use real supported operations; an unresolved
factory is not represented as a successful placeholder.

`PackageScanPass` records whether enumeration succeeded, one entry for each
processed name in order, and the first failure diagnostic. Entry dispositions
include `already_mounted`, `mounted`, `declined` and `failed`. A successful
empty scan returns true. A pass returns true only when enumeration succeeded
and all entries were already mounted or mounted successfully. The startup
wrapper returns the conjunction of both complete pass results without
short-circuiting the second call.

Reports are reset on entry. Missing callbacks reject the pass before querying.
Enumeration failure discards its local partial names and performs no mounts
in that pass. Priority's existing ASCII, embedded-NUL and signed-size guards
produce an explicit failed entry without invoking lookup or mount, then later
entries continue. These guards and reports are new host behavior; native
`0073cb10` has no semantic bool result. A callback's specific failure text is
preserved; a generic message is supplied only when it returns no diagnostic.

Returning failures never undo earlier successful mounts. Callback/allocation
exceptions propagate separately, can leave earlier mounts and a partial report,
and prevent later work from being promised. The wrapper does not add global
manager ownership, factory singleton lifetime, unmount behavior, shutdown,
thread safety, reentrant mount mutation during enumeration, or a native ABI.

## Evidence and validation

This implementation rechecked the complete raw scan and lookup assembly and
the startup call window after verifying project `bsp`, program
`/battlestationspacific.exe`, x86 LE base `00400000` with `Client` configured by
`config/target.json`. All ranges matched both their prior raw exports and the
original installed PE, whose SHA-256 is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

| Span | Bytes | SHA-256 |
|---|---:|---|
| `0073cb10..0073ce15` complete scan | 774 | `d6f12a9f69158bb91207a30f83b1c56f532ecd830274fc17c7684d3bad51f2fa` |
| `00bdb120..00bdb1dd` complete lookup | 190 | `331e9a5034d15f406819c3d027db02d1b69d68fdcf909ebcd7ee04f0caaeed8f` |
| `0073d87b..0073d898` startup call window | 30 | `e197032a1690a98ffc715b7d213f7ad092bd852743693905089ebac851c20f1b` |

The scan's full tail continues after `_free` at `0073cdfb`; its apparent
no-return behavior in an older decompilation is not the native contract.
Exact verified target data, range hashes and raw export references are in
[package_scan_audit.json](../reports/package_scan_audit.json). Dependencies are
documented in [PACKAGE_MOUNT_STARTUP.md](PACKAGE_MOUNT_STARTUP.md),
[MPKG_MOUNT_INTEGRATION.md](MPKG_MOUNT_INTEGRATION.md), and
[PROVIDER_ENUMERATION.md](PROVIDER_ENUMERATION.md).

The existing `probe_mpkg_archive()` now frames an encoded inner archive inside
an outer FileStore-backed package and runs this scanner through the concrete
`VfsProviderManager`. Its same-scenario malformed archive and declined filename
check that both scans finish and successful mounts survive reported failures.
The fixture expects two queries, seven lookups, six mount calls, unchanged
names, priority 1000 for `./patch2.mpkg`, and inner marker bytes after manager
destruction. Exact data and deliberately limited ordering claims are documented
in [MPKG_FIXTURE.md](MPKG_FIXTURE.md). No test target was added.

The coordinated MSVC Win32 build and both existing CTests passed, recorded in
[startup_manager_build.txt](../reports/startup_manager_build.txt). The full
[startup_manager_probe.txt](../reports/startup_manager_probe.txt) records all
seven expected entry outcomes, two queries, seven lookups, six mount requests,
and successful shared-identity/nested-discovery/failure-continuation checks.
These establish the synthetic host scenario. No live Ghidra mutation by this
worker, native differential execution of the scanner, real installed archive
scan or original-game startup validation is claimed.
