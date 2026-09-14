# Real MPAK archive composition (BC)

The installed tree at `I:/SteamLibrary/steamapps/common/Battlestations Pacific`
contains no `.mpak`, `.mpkg` or `.pak` file in a recursive, hidden-inclusive
inventory on 2026-09-13. This confirms the earlier inventory in
`NATIVE_AX_INTEGRATION.md`. No installed MPAK member exists for a real-archive
fixture, byte comparison, or refcount/drain observation. No archive was made,
renamed, or copied to impersonate an installed input.

The source call path is already concrete downstream: `00BB83A0` accepts only a
`.mpak` suffix and calls `00BB8240`; that constructor obtains registry state,
opens the current manager at slot 4 and parses the stream with `00BB7C50`.
`NativeMpakRuntime` supplies manager-open, device selection and entry
materialization (`00BB5080`), while `NativeMpakStorageServices` supplies actual
file lookup, directory search, offset copy and vector lifetime interfaces.
Member opening routes through `00BB5BB0`. Provider teardown is `00BB7920`.
These source contracts do not demonstrate an installed archive or an executing
startup route.

There is also a distinct host integration boundary: `game_hosts_vfs.cpp` still
registers the MPAK factory as a token and explicitly says its Create is not
connected. Startup's package scans enumerate `mpkg`, not `mpak`, so they do not
automatically mount an MPAK path. The pending real VFS owner services are
borrowed dependencies for eventual composition; this packet does not duplicate
their manager, string pool, registry, streams or lock.

Next evidence gate: obtain an authentic, immutable `.mpak` archive with a known
member from a supported game installation, then trace the real owner/factory
dispatch into `00BB83A0` and compose existing services over one raw VFS owner.
Only then run a single Win32 member-read fixture and compare bytes with an
independent reader or known content, followed by stream/provider drain checks.
The host factory dispatch in `src/game_hosts_vfs.cpp:110` is the concrete source
integration packet; its native entry contract is `00BB83A0` in
`src/native_mpak_factory.cpp`. No new native-body implementation was identified
as necessary for this assessment, and runtime behavior remains unverified.
