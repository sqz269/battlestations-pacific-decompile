# Startup provider manager

Addresses: `00bdb040`, `00be1890`, `00bdb120`, `00be80b0`, `00be8120`, `00be7fa0`, `00bb5590`, `00bf4d30`, `00be1dc0`, `00530620`, `0073d410`

The concrete startup manager now constructs the three known providers and keeps
their ordered registrations in one live VFS context. The asset probe uses this
manager for its initial mounts, both package scans, subsequent search registration
and existing preloads. The installation root remains a supplied host input;
this does not construct the full native application or its global singleton.

## Factory selection and provider identity

Native `00bdb040` receives ECX manager and system/virtual string pointers,
returns EAX provider and RET8. It visits factories in registration order and
returns the first nonnull result. The initial order is physical directory,
FileStore, MPKG. VfsProviderFactories implements those actual gates and providers.
It distinguishes a declined path from a guarded construction failure. Native
malformed-data/allocator behavior and arbitrary additional factories are outside
that typed domain.

The physical factory accepts a nonempty system path ending in backslash, then
copies that exact root through base constructor `00bb5590`. The MPKG provider
copies its unchanged logical source name. Neither name is the virtual prefix.

FileStore factory `00be8120` matches the case-insensitive word filestore, but
provider constructor `00be7fa0` passes the empty string at `00ce3a0c` to its
base. Its native system name is therefore empty. Factory getter `00be80b0`
caches the provider at factory+8. Repeated mounts use the same provider; separate
managers using the same explicit VfsProviderFactories object share it too.
The host factory object represents application-scoped ownership, not a hidden
process-global C++ singleton or the original intrusive allocator.

Shared VfsProviderIdentity represents kind, system name and device ID. The
base starts device+10 at -1. Mount `00be1890` receives five stack arguments,
in order: system path, virtual path, signed priority, ownership byte, device ID.
Both returns are RET14h; the decompiler's shorter signature is wrong. Success
constructs the provider, assigns the device ID, then registers it. Repeated
FileStore mounts overwrite the shared ID, which all aliases observe.

Lookup `00bdb120` receives ECX manager and one system-name argument, RET4.
It visits mounted records in native iteration order and returns the first
provider whose stored length and case-insensitive name match. Empty names match
without dereferencing data, so an empty query can find FileStore. The host
lookup preserves this rule instead of treating the factory word as its name.

## Registration and live source ownership

Existing registration implements signed descending priority, stable insertion
after equal priorities and lexical prefix preparation. The ownership byte is
stored without inferring a deleter. Host callbacks retain providers.

The manager constructs an MPKG before inserting it. Its source opener captures
a weak reference to the stable current VFS context, so later mount/alias changes
affect each original-logical-path reopen. Publishing new registrations replaces
only the current mount vector; aliases and context identity remain live.
A retained archive outliving its manager gets an explicit expired-context
diagnostic, not a dangling reference or captured old source.

Copies/validation for the host registration transaction finish before publishing
new vectors/device metadata. Guarded failure preserves registered state and the
output argument. That exception/guard behavior is additional host behavior,
not an assertion about native SEH mutations. Allocation exceptions propagate.

## The configured error callback is a real no-op

Startup writes address `00530620` into manager+90 at `0073d642`.
A fresh read shows `00530620` is exactly one byte, C3 (RET), followed by INT3
padding. It was absent from the saved function inventory. Manager mount calls
that callback if every factory declines, then returns null if it returns.

Consequently the startup manager's failure path has no callback side effects.
Omitting a call to this proved RET does not invent a successful provider or
replace an unknown error handler. The typed result still reports decline/failure.
Other possible application-installed callbacks are not modeled.

## Pending operations

Physical providers own real PhysicalPendingReads queues and bind the recovered
submit/pump through the manager's same mount view. FileStore and MPKG expose
their byte-verified immediate-false pending submit and RET pump routines.
These are recovered constant methods, not placeholders for asynchronous work.

All access remains serialized. No mount mutation or recursive pump is allowed
during callbacks. The explicit host stop_pending_submissions operation stops
acceptance without cancelling or dispatching; callers must pump until drained
before destroying physical owners. Native manager destruction, cancellation,
frame/stop-loop integration and global singleton replacement are not claimed.

## Evidence and validation boundary

The primary's fresh audit is in reports/startup_provider_manager_audit.json.
Fourteen selected complete bodies, call/literal spans and the one-byte callback
matched both saved Ghidra bytes and the installed PE after target verification.
The original source remains unchanged. The provider-specific enum/materializer
contracts remain in their referenced subsystem documents.

The scanner and actual nested-archive/manager scenario are integrated into the
existing probe. The Win32 build, both existing CTests and full installed-asset
probe passed. The nested scenario observes three first-pass and four second-pass
entries, shared FileStore identity, valid mounts surviving failed/declined
requests, and retained marker bytes. The installed root yielded no packages in
either scan. Runtime outcomes and limits are recorded in
reports/startup_manager_validation.json; real installed archives remain untested.
