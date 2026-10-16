# Native GUI Source CRT provider contract51

An honest production **Source UCRT** comparison bridge is available in the
same sense as the existing host's `_stricmp`/`_wcsicmp` substitutions. It does
not require reconstruction of the whole vendor CRT. It must be named as that
Source contract, rather than silently treating image preimages or a constant
zero as live Original CRT state. The remaining concrete seam is refreshing or
removing the content gate: current public application admission alone does not
cover all actual Text calls. Root owns the implementation decision.

This is a bounded read-only source/API/import analysis. No source, settings,
locale, game/audio, Ghidra, ledger or installation state was changed. Main
`638f686c80ea7ed4248b9187e3ecfdafc138010f` was merged without resetting the
committed49 work; analysis HEAD is `3b82d3013ee18063e647589fae4d7680114a4cf6`.

## What the data owner really supplies

`GameNativeMutableCrtData` commits exactly `00E15000`, `00E16000` and
`0109E000` (`src/game_native_mutable_crt_data.cpp:29`). It copies the first two
complete pages from the hash-verified Original PE and zero-fills the third
(`:81`). Its public accesses cover security cookie/complement, NLG/failure
state, feature word and debugger hook (`include/bsp/game_native_mutable_crt_data.hpp:37`).
The header explicitly distinguishes the remaining image bytes from initialized
host services (`:14`). The joint process owner initializes the recovered cookie,
publishes after checked bootstrap adoption and intentionally survives teardown
(`src/game_native_mutable_crt_data.cpp:145`).

| Address | What is admitted | What is not established |
|---|---|---|
| `00E159E8` | Writable initial PE bytes in the first page. | Current owning-thread errno. The Original getter may choose PTD+8; forcing every error here would invent the getter's decision. |
| `00E161D0` | Initial/default locale bytes in the second page. | A current locale/PTD owner, live category/refcount/lock updates, or a modern UCRT locale object. |
| `0109DE1C` | Its containing64KB allocation may be reserved by the handoff. | Its page is `0109D000`, **not** one of the three committed/admitted pages. There is no current-locale flag accessor or initializer here. Reservation identity is not cell admission. |
| `0109EEA4` | A mutable DWORD accessor and checked zero preimage. | The frontend's already queried Source conversion bool, or proof that Original CPU-feature initialization ran. |

The RO accessor is restricted to its verified RO section and owned bands
(`src/game_native_readonly_data.cpp:137`); it is not an accessor for these CRT
globals. Neither owner supplies current PTD/TLS, errno lookup, locale update,
invalid-parameter dispatch or runtime locale mutation. The current Main
`reports/goal_oct15_crt_locale_provider_readiness.json` also explicitly leaves
general locale/TLS/errno parity unproved. This owner therefore cannot close the
real Original46/48/49 provider route merely because adjacent default bytes exist.

## Existing host and installed CRT evidence

`NativeSettingsChoiceCalls::compare_language_00bf7fbf` already calls
`::_stricmp` (`src/native_settings_choices.cpp:35`). Many `game_*.cpp` paths use
that Source CRT directly; `GameLocaleTextRuntime::crt_uppercase_00c0391c`
uses `std::towupper` (`src/game_hosts_text.cpp:169`). These are existing Source
host substitutions, not proof of Original callee ABI or state identity.

The built Win32 `bsp_game.exe` import table contains `_stricmp`, `_wcsicmp` and
`setlocale` from `api-ms-win-crt-*` DLL contracts. Installed SDK
`10.0.26100.0/ucrt/locale.h:97,106,114,120` declares query/setlocale,
`_get_current_locale`, `_free_locale` and `_wsetlocale`; string headers declare
the ordinary and `_l` comparisons. The locally installed UCRT source shows:

- `string/wcsicmp.cpp:133` delegates to UCRT's own locale handling after its
  own changed-state decision. `_wcsicmp_l` selects the ASCII helper when the
  current locale's `locale_name[LC_CTYPE]` is null (`:50`). The Win32 helper
  reads unsigned16-bit units. `Include/.../ucrt/ctype.h:149` folds only A..Z.
- `string/stricmp.cpp` similarly uses its owning UCRT state and unsigned bytes.
- `locale/setlocale.cpp:33` locks and queries this thread's UCRT locale. Query
  `LC_CTYPE`, **not LC_NUMERIC**, for this comparison decision. Copy/compare
  the returned spelling immediately; do not retain the CRT's mutable string.
- `locale/wsetlocale.cpp:201` shows `_get_current_locale` allocates a retained
  current-thread locale snapshot. It needs owning-UCRT `_free_locale`; it is
  not a borrowed Original locale object or permission to cast private layouts.
  A snapshot is optional for a distinct pinned-locale design, not needed for
  the smallest stateless current-UCRT callbacks.

These installed sources are API/behavior evidence, not a byte/hash match to
the OS's runtime UCRT DLL. Modern code validates before `_LocaleUpdate`,
optimizes mapping when raw units already match, and uses its modern locale,
PTD, errno, invalid-handler and OS NLS behavior. Original48 establishes an
update **before** null validation and only an untyped DWORD test at
old locinfo+14h, not a recovered modern `LC_CTYPE` field. General non-C mapping,
error/private-state effects and ABI interchangeability are not proved equal.
For valid C-locale strings the relevant ASCII-fold equality is supported; this
does not require a permanent C-only application restriction.

## Writers, admission and the exact missing refresh seam

A whole `src`/`include/bsp` search for locale setters/configuration found only
`std::setlocale(LC_NUMERIC,nullptr)` **queries** at `src/scene_file.cpp:203,250`.
This is not a complete writer proof. The linked Lua5.1.1 `loslib.c:193,200,219`
implements/exposes `os.setlocale`, including `all` and `ctype`. The real host
loads `luaopen_os_guarded` (`src/game_hosts_lua.cpp:106`), but that guard replaces
only `execute` (`include/bsp/lua_exec_guard.hpp:48`). No reached script changing
locale was demonstrated here; the mutation capability is real and was not
disabled or reclassified as a policy violation by this packet.

`GameNativeTextApplicationBindings` currently has a plain `const int32_t&`
gate, two-argument wide callback, name callback and `const bool&` conversion
selector (`include/bsp/game_native_text_application.hpp:32`). The gate cannot
intercept a read or refresh itself. The actual content branch is
`src/gui_text_content.cpp:29..35`; empty strings bypass comparison, and all
nonempty comparison decisions converge here. Preparation invokes section
providers at `:191` and uppercase callbacks at `:201` **before** this branch
at `:205`. There is no established invariant that all possible retained
providers or other threads leave the Source CRT untouched in that interval.

`GameNativeTextEntry` (`src/game_hosts_text.cpp:661`) currently only checks
admission and marks active. It guards application `make_type`, `submit_source`,
`submit_utf16` and completed-child resume. It has no CRT refresh callback. More
importantly, the following genuine routes bypass that guard:

- Loader `GuiWidgetOwnerRuntime::properties_bound/loaded78`
  (`src/gui_widget_owner.cpp:691,695`) directly calls the concrete Text type;
  `GuiTextRuntimeImplementation::properties_bound` can submit content
  (`src/gui_text_runtime_factory.cpp:521`).
- `GameNativeTextApplication::factory()` returns the bare factory (`:770`).
  Concrete implementation submit/ellipsis/rebuild/resume and factory copy
  continuations are callable outside the application wrapper (`runtime_factory.cpp:598..622`).

Refreshing only in `GameNativeTextEntry`, once at startup, or once per frame
therefore does not close the current reference's stale-gate seam. App-entry
preflight is useful, but is not a substitute for covering the actual branch.

## Smallest concrete production choices

For a Source UCRT composition, use stateless two-argument adapters to
`::_wcsicmp` and `::_stricmp`, with Win32 `wchar_t`/`char16_t` width assertions
and the existing valid, terminated input contracts. They use the already
initialized, currently owning UCRT; no new CRT global, default locale, private
PTD/errno adapter or string pool is necessary. Keep `LocaleTextRuntimeHost`'s
CRT uppercase service in that same Source domain.

Root can choose either of these small Source interface changes:

1. **Query at the gate.** Add an explicit Source query/refresh seam immediately
   before `equal_content` reads its selector. Query actual `LC_CTYPE` on the
   calling GUI thread, reject/query-failure as an error, and derive a clearly
   named Source selector (C -> intrinsic ASCII arm; other -> current UCRT wide
   callback). This is sampled current Source category state, not the Original
   `0109DE1C` value or UCRT's own historical changed-data word. It must not be
   described as either. A callback returning the decision avoids a cache;
   if retaining the existing reference, its one stable metadata cell must be
   updated there and live through all Text/factory/continuation uses. This
   central seam covers the bare routes as well as the application wrappers.
2. **Explicit current-Source comparator mode.** For that declared Source
   composition, always call the supplied current-UCRT comparator for nonempty
   content; let UCRT make its own actual locale decision. Preserve the borrowed
   Original-flag mode as a separate supported interface. This removes a
   redundant mirror/query and must be an explicit mode, **not** a fake constant
   nonzero Original flag. It naturally follows locale changes between calls.

Both allow non-C locales under the Source contract. Sampling at the actual
branch avoids staleness caused by preceding callbacks. If Root instead needs
one frozen locale across an entire operation's uppercase/name/content calls,
that is an additional pinned-locale contract using one owning-UCRT snapshot
and `_l` calls, with `_free_locale` after every pending user has retired; the
current context-free callback/ref fields do not carry such a snapshot. This
packet does not impose that stronger contract or a new restriction on scripts.
Existing external GUI operation serialization still applies. Refresh on each
resumed operation rather than assuming a value survived a pending interval;
any entered failure must keep existing nonreplay/retention behavior.

For the shared SSE2 bool, expose the **same** queried frontend owner
(`src/game_hosts_frontend.cpp:162`, loader getter `:365`) to native Text, or
deliberately lift that queried capability into a longer-lived common owner
and retarget both consumers. Preserve `IsProcessorFeaturePresent` provenance
and one stable reference. Retire all native Text/factory/pending uses before
its owner dies. Do not add a constant, take a bool view of the image DWORD, or
quietly add another independent selector. Other existing subsystems have their
own queried values; unifying those is outside this packet.

The alternative actual Original route remains valid but not closed:46/49 need
the genuine live flag and owning errno/invalid services;48 additionally needs
real BF7E24 locale/PTD update and BFD39F mapping with correct cleanup. Modern
UCRT `_locale_t`/PTD casts cannot supply those private providers. Choosing the
explicit Source route is an implementation choice within the existing host's
substitution practice, not a claim that the Original providers now exist.

Only the two documentation/report files are committed. JSON/diff checks apply;
no C++ build, executable probe or native GUI runtime claim is added by51.
