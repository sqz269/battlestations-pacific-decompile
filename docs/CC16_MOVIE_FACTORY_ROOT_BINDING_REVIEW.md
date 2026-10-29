# Coupled movie factory root binding review

Independent qualified pass of `38e16c60111b0f0914f0a2f529e3324cd8845a1b`,
based on `e64249b004a989f162a1b7b9cda3493606535e88`. No blocking source or
static-object discrepancy was found. This admits a dormant, finite Source
composition for review; `kMovieFactoryRootCc16Bound` remains **false**. It does
not admit activation, a complete native parent or an original binary ABI.

## Actual suppliers and ownership

All four source/header diffs, the full candidate document/report, the actual
owner/library/lifetime/cell suppliers and the pinned ownership/EH contracts
were inspected. All 21 source pins, their candidate/baseline Git blobs and all
21 retained artifact pins match. The unchanged owner source is the identical
blob at `6d4c2888`, published `5ec9904f` and the candidate.

The native receiver is the separately allocated four-byte `MovieFactoryPrefix`.
The real heap adapter uses `singleton_lifetime_allocate({object,bytes,bytes})`
and `singleton_lifetime_free`; the owner requests four bytes. The implementation
is the existing malloc/new-handler retry/free pair, with no default heap,
successful-null fallback or metadata-wrapper allocation substituted for it.
This is source-domain matching, rather than original CRT ABI proof.

Both canonical references come from the same retained `GameNativeStringProcess`
authority: the established `01090AA0` manager cell and its sole appended logical
`F8AEFC` cell. Construction, unregister and explicit shutdown borrow that same
mutable cell. There is no copied publication, private manager, reset or writable
cast of `GameNativeReadOnlyData::data_at`. The source cell is not a literal
fixed-address mapping. Read-only native table words remain identity tokens.

Services contain their heap, actual Bink library/Impl/tables, owner, publication
reference and deletion record. The metadata constructor can load/resolve the
library and validate original table identities, but performs no native factory
construction, registration or imported sound call. The heap-held `unique_ptr`
keeper owns the complete aggregate, the Host observer is set, the complete
deletion record is installed, and only then does factory construction run.
Unescaped preparation failure may unwind its library; escaped factory effects
never rely on member unwinding for custody.

The keeper is intentionally never destroyed or reset. Its three emitted users
have no atexit-registration relocation. It retains the complete graph even
after normal flags1 prefix retirement. The Host owns only an observer. Neither
the aggregate nor the owner stores a Host/log/renderer/NativeData reference;
the remaining cell references are permanent process authorities. This prevents
early `FreeLibrary`, without proving vendor quiescence or unload timing.

## Routing, epochs and failure limits

The core borrows a 16-byte record with actual context, observational live check
and real `00737950`/`00737030` owner callbacks. Its pointer is appended at 216;
the binding record grows 216 to 220 with existing asserted offsets retained.
The native singleton core object has no application-owner or Bink symbol
dependency. Its non-inlined shared bridge
checks all bindings, then live captured-owner metadata, before reading the raw
prefix. It admits current `CFEA78`/`CFEA60` only, forwarding the complete DWORD
flags to the real corresponding owner method. It never calls a numeric PE word,
assumes a foreign replacement belongs to this owner or installs a foreign shim.

The actual manager arms and explicit shutdown use that same bridge. The generic
manager body still reads the popped receiver's profile **before** entering the
narrow bridge. The bridge therefore does not make a dangling registry safe;
the immediate root failure guards are essential. Genuine cells/registry entries,
stable real bindings and one serialized graph remain preconditions; arbitrary
reentrant mutation, replay, reload or concurrent teardown is not established.

The unchanged owner preserves completed-second-getter then volatile-publication
reread ordering, captured-section exit, current-publication unregister/clear,
incoming profile stamps and captured-allocation free identity. `737730` remains
decoder creation. Allocation exceptions precede acquired cleanup; constructor
registration/sound C++ exceptions perform captured raw free only and can leave
dangling publication/registry effects, as pinned by caller state 0x21's
`00DB61C8 -> 00C86BEC -> BF65AC` action. Root catch marks failure and immediately
uses metadata-only log/`_Exit` before prefix lookup, cleanup or drain. It adds no
unregister, clear, sound reset, free, retry or rollback. Deletion callbacks latch
failure and rethrow; explicit retirement/drain catches prevent normal continuation.

Shutdown reloads the current cell, preserves the null guard, and performs flags1
dispatch after current Source menu/widget/Font retirement and before device/raw
drain. Null current publication with a Ready owner is terminal; it does not cause
cached-owner retirement. Failed/interrupted/flags0-retained states and unretired
Ready owners also block ordinary destructor/drain. Checks cover construction
catch, shutdown, Host destructor and raw-drain entry. This is a host terminal
custody policy, not native FH3/SEH/foreign-error or cleanup-failure equivalence.

## Independent static validation

Fresh MSVC Win32 core, tracked OFF and private one-guard ON objects passed
`/c /std:c++17 /O2 /Ob2 /DNDEBUG /MD /EHsc /W4 /WX /fp:strict`. The private
header differs only in the new false-to-true guard. It was never linked or run.

- All 46 relevant ON function emissions match primary code bytes and relocation
  tuples exactly. All 25 core emissions match with only worktree-derived
  anonymous-namespace symbol hashes normalized. Whole-object equality is not claimed.
- OFF has zero movie graph symbols; its constructor/retirement helpers emit
  `RET 0`, its bool-argument guard `RET 4`, with no relocations. Retained common OFF
  object/map also have zero graph references. The native singleton core object
  has zero owner/Bink symbols; this is not a claim about every member of `bsp_core`.
- Core bridge emits live call at +28, false rejection at +2F, first prefix read at +31,
  actual callback arms at +49/+5E and manager shared-bridge call at +411. The 35-byte
  actual live predicate reads metadata only, never the captured receiver.
- Explicit retirement emits current-cell load at +48, null guard at +4A/+4C,
  `PUSH 1` at +55, shared bridge call at +58, then Ready rejection at +75/+79 even when null.
  Actual deletion thunks forward receiver/full flags and latch failure on catch.

Primary final common build/check evidence is hash-verified: 2/2 checks,
11.64 seconds. This review did not repeat a full build or run any tests. Its
rerunnable verifier, three objects and narrow machine windows are retained in
`local/movie-factory-root-binding-review/`; `object_audit.json` SHA-256 is
`d471db1cc8f05142843db49e5f174bdc5e9a032dc4dd30d6d8e27de3d9c8837c`.
Full artifact and input hashes are in the review report.

Independent callback review `722fda951` corroborates installed callback machine
stack/return shapes only. The DLL and source-header hashes still match that
input. Setter status may reflect an older saved callback; ordinary sound return
remains ignored and is not fresh-installation or audio-success proof.

Terrain/`010909B0` neighbors, the complete ordinary parents, actual decoder and
renderer/widget/Lua borrowers, installed semantic/runtime ABI, FH3/SEH/foreign
transport, callback completion and vendor quiescence remain open. No DLL load or
call, game, probe, Ghidra analysis/mutation, source/CMake/shared-metadata change or guard
flip occurred in this review. Native/vendor body, original ABI and activation
credit remain zero. Keep the source guard OFF.
