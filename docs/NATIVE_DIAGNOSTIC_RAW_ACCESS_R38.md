# Native diagnostic raw access R38

## Result

The diagnostic singleton getter now accepts `SoundLifetimeAccess` by value, so
the same source body can borrow either a semantic `SingletonLifetimeDomain` or
the application's actual `01090AA0` manager-publication cell. The raw route
reads the already-resolved manager's native critical-section pointer at `+10`.
It does not construct another lifetime manager.

`NativePhysicalBufferLockContext` now retains that two-word access value. Its
Win32 source layout is 16 bytes: the `0109CF14` publication reference, the
eight-byte lifetime access, and the null-buffer sentinel. Index and vertex lock
diagnostics therefore reach the raw getter without changing their existing
lock behavior.

Resource support continues to use `NativeResourceSupportLifetime`. A narrow
cross-adapter predicate compares the semantic domain object or the address of
the raw `01090AA0` cell. It never compares current pointer values, so two
different loader-zero cells fail the identity check. The existing stream-clone
and mesh-reader guards already call this predicate; no guard-source change was
needed after the current-main support-context merge.

## Original evidence and schedule

Read-only Ghidra queries used `C:/Users/sqz269/bsp.gpr` and program
`/battlestationspacific.exe`. Fresh installed-PE reads matched every live byte:

- `004C14C0-004C1569`, 170 bytes, SHA-256
  `d1ed3a68f63885476351cadb135979cb5433c62f21ddc2778425c5b4a103b505`.
- `004BBCA0-004BBCC8`, 41 bytes, SHA-256
  `f462bb0027b9bfdceed4ade23ced0707acf58aa56d643492b1984d21f4da9c58`.
- derived profile `00CE752C` contains `A0 BC 4B 00`; base profile `00CE3818`
  contains `40 24 41 00`.

The getter retains the observed order: first publication read; first manager
lookup; capture of that manager's `+10` section; enter and `+18` depth update;
publication recheck; four-byte allocation/profile store/publication; second
manager lookup; registration of the then-current publication; captured-section
release; final publication reload. Registration failure leaves the published
owner in place and releases only the captured guard. A warm first read touches
neither manager nor section.

The `01090AA0` and `0109CF14` cells lie beyond `.data` physical raw data and
begin loader-zero. This packet adds access to those identities; it does not add
an application-owned `0109CF14` cell.

## Validation

The strict MSVC Win32 `/MD` build passed, as did `verify-seeds` and the three
existing CTests: `reconstructed_math`, `native_math_differential`, and
`tool_tests`. COFF inspection confirms x86 objects, `MSVCRT`, the value-form
getter signature, the native-section accessor, and the cross-adapter identity
symbol. The retained executable has an embedded manifest.

One ignored fixture used genuine reconstructed raw manager creation,
registration, unregistration, section entry, diagnostic scalar cleanup, and
index/vertex null-COM lock paths. It established:

- same raw cell and same semantic domain pass identity checks;
- distinct raw cells reject even while both contain null;
- cold index and warm vertex paths share one diagnostic publication;
- the registered owner has profile `00CE752C` and the captured section returns
  to zero depth;
- real unregister plus `004BBCA0` flag-one cleanup retires the fixture owner;
- a real registration-validation failure retains the publication, releases the
  section, and the following warm access does not retry registration.

The fixture directly unregisters and scalar-deletes the diagnostic owner, then
destroys an empty raw manager. It does not model the missing production drain
binding.

## Boundary

This phase does not change `GameSingletonHost`, `NativeSingletonDeletionBindings`,
startup, or shutdown. It adds no `CE752C` deletion case and does not reconstruct
the original `007363B0` shutdown wrapper. There is no eager diagnostic creation,
duplicate manager, fake dispatcher, renderer/device owner, or production
physical-lock context in this packet.

The C++ functions are typed source interfaces. The original cdecl/ECX ABI,
native FH3/SEH identity, asynchronous faults, game startup/shutdown, renderer
behavior, and gameplay remain unproved. The exact evidence and artifact hashes
are recorded in `reports/native_diagnostic_raw_access_r38.json` and the retained
archive manifest.
