# Native system constant registry

This packet reconstructs the actual `10h` owner published at `0108FE94`, its
single `0Ch` array at `+04`, and ten constructor, vector and lifetime bodies.
Entries are the existing `NativeCompiledShaderConstantStorage` (`20h`), in the
same `NativeCompiledShaderConstants` layout consumed by reflection. It does
not introduce a semantic registry, reference count, private string pool, or
second singleton lifetime domain.

The implementation uses the 52 literal rows in `shader_system_registry.inc`
only as data. Each row's name, dimensions, array count and semantic id was
rechecked against the immediate pushes and literal reference preceding its
original `B5BBC0` call in `B5BF70`. The whole constructor is additionally part
of the focused original/source fixture described in the report.

## Native order and layout

`B5B9E0` writes `D626F4`, gets the shared `01090AA0` lifetime manager, captures
manager `+10`, enters that section and increments its `+18` count, publishes
the actual owner at `0108FE94`, then gets the manager again and registers the
CURRENT global under the originally captured section. `B5BA80` repeats the
two-getter pattern, unregisters the CURRENT global, clears it, releases the
captured section, and writes `CE3818`. No identity comparison or count is
added to these native operations. The supplied live `void* volatile&` is the
publication itself.

`B5BF70` calls that base before writing `D62A3C` or zeroing the three array
words. It then creates, appends and releases 52 separate temporary names and
records, in the recovered order. `B5BBC0` zeros the record's actual string at
`+14/+18`, captures its second dimension, writes `+00=FFFF` and the wrapped
dimension product at `+04`, copies the current source header, and writes
first dimension `+0C`, captured second dimension `+08`, array count `+10`,
and semantic id `+1C` after that copy. Thus array count is excluded from the
dimension product. The constructor releases each current record header, then
releases the temporary name using its captured data pointer and CURRENT
length. Headers retain their native stale pointer/length preimages afterward.

`B5BED0` grows only when count equals capacity, using twice the current
capacity and minimum one. `B5BD10` also clamps its direct request to one,
allocates `request * 20h`, reloads current source rows/count while copying via
the existing `B38310`, releases old names FORWARD, frees the CURRENT old
buffer, and only then publishes replacement data and capacity. Append copies
the record and increments the CURRENT count. `B5BE10` uses existing
`B5BB40` for growth, preserving its unspecified record words; shrink decrements
count BEFORE releasing each name in reverse order, then writes the request.

`B5DF00` stamps `D62A3C`, shrinks the actual array to zero, frees its CURRENT
data buffer, and destroys the base. It leaves the native data/capacity words
stale. `B5DF70` and base-only `B5BB20` destroy unconditionally and free the raw
owner only for `flags & 1`, returning the original pointer. Neither touches a
reference count. Native ABI and byte pins for every body are in the report.

## Lifetime composition and failure boundary

Construct `NativeSystemConstantRegistryLifetimeBinding` from the application's
live `0108FE94` publication and the next singleton callback, construct the SAME
`SingletonLifetimeDomain` from `binding.callbacks()`, then
`binding.bind(domain, actual_strings)`. The actual string pool and all other
singleton users must share that domain. The binding dispatches the two actual
registry profiles and forwards other owners to the next callback. It does not
emulate a callable original vtable.

Every owner/vector entry takes a persistent `NativeSystemConstantRegistryOperation`.
Admission holds that actual owner in the binding until normal completion. It
retains the array child, unpublished replacement, copied/current records,
append source, temporary name/record headers, captured name pointer, phase and
literal index. A borrowed C++ allocation failure does not discard these,
unregister the published owner, roll back, or replay native work. Reusing a
failed operation is rejected before side effects; the canonical callback
terminates instead of retiring its guarded owner, and destroying a failed
operation or its guarded binding also terminates. Keep all retained inputs,
storage, operation and domains alive for explicit later recovery. No production
recovery algorithm is claimed. Standalone `B5BBC0` callers likewise retain
their actual destination header and source; the registry constructor keeps
them in its persistent frame before invoking it.

The binding admits only one unfinished registry operation. `begin` rejects a
second operation even on a different registry-profile owner while its guard
is held. Independent overlapping registry owners are therefore unsupported,
although `terminal_allowed(otherOwner)` can be true; it is an identity check,
not a promise that another registry operation can be admitted. Non-registry
owner callbacks still forward. This is a host validity limit, not proof of
native reentry behavior.

This is a normal valid-domain reconstruction with explicit host continuation
state. It does not emit the original private FH3 handlers. In particular,
borrowed failures do not pretend that original constructor unwind completed.
Readable nonnegative arrays and representable allocations are required; a
callback that grows the old count beyond the acquired replacement extent is
outside that domain and is retained as a failed operation. Required CRT and
pooled-string calls remain the established explicit host boundaries, including
the existing string bridge's `noexcept` release behavior.

## Evidence and limits

The Ghidra target is `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. Live bytes for all ten bodies plus shared record
copy/default dependencies matched the installed PE. Root repaired saved
post-free no-return gaps for `B5BD10`, `B5BB20`, `B5DF70`, and `B5DF00`; this
worker only refreshed/read the exports. The two deleting functions and
`B5DF00` were checked through their actual return instructions. Unreachable
alignment padding inside `B5BD10` is data in the saved listing, but is retained
in the complete byte pin.

Validation status, exact source hashes, original byte ranges, direct and
indirect call rows, assembly-table evidence, and fixture artifacts are recorded
in `reports/native_system_constant_registry.json`. The module is registered in
the default Win32 build; its build and existing CTest passed. The final focused
fixture links only the registered production library and passes the complete
52-row original/source construction/destruction comparison, shared-domain
canonical shutdown, and retained-failure terminal guard. The four original
critical-section IAT calls are recorded separately from 443 verified direct
call rows. New C++ interfaces are not
drop-in original ABI replacements. Shader compilation, rendering and game
execution are not established by this packet.
