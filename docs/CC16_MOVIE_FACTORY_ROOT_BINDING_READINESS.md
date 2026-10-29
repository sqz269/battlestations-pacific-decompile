# Movie factory root binding readiness (CC16)

The genuine source inputs now permit a **coupled default-OFF root binding
packet**. They do not permit an activation claim. The real heap pair, permanent
publication cell, raw manager services, factory owner and complete Bink import
owner exist. Startup invocation, both deletion-profile arms, explicit normal
retirement, terminal retention and the decoder consumer remain unwired.
No unused wrapper or invented provider is needed to address the root edges.

This read-only snapshot is the primary's clean local integration commit
`274590587a6320a73d4e9960b36e2f91f4ee10d9`. Origin was
`d8cb0f1621c767c8cc76694f26a38aad128b07be` at start; publication was pending.
It includes Bink `68ab882bbe7709822936c15b21f6e9804076ffb2`, staged owner
`6d4c288834c36f5ddc83da629336077389732da7`, and typed-cell producer
`0009115d8677b120169898f12ebd763f7cd8ba5d`. Bink/owner source bytes remain
unchanged from those candidates. The earlier `8f14bfd2` owner contract and
`1214cca8` caller-EH evidence still apply; their formerly missing cell is now
closed by the actual retained producer, not by a local substitute.

Only this document and its report are added. No source/CMake/ledger/Ghidra,
install/save/profile, guard or runtime state changed. No build, test, probe,
game, module load or imported Bink call was performed. All body/vendor/ABI/
activation credit is zero. Source names proposed below are hypotheses.

## Real suppliers

| Input | Exact available source / contract |
| --- | --- |
| Raw four-byte allocation | Root can implement mandatory `GameMovieFactoryAllocation::allocate_00bf681b(bytes)` with `singleton_lifetime_allocate({SingletonAllocationKind::object,bytes,bytes})`; the owner requests exactly4. This is the existing `malloc`/`_callnewh` retry/`bad_alloc` service, not a new heap or successful-null fallback. |
| Matching BF65AC role | `GameMovieFactoryAllocation::free_00bf65ac(captured)` must call existing `singleton_lifetime_free(captured)` (`std::free`, noexcept). Free the raw captured address, never the metadata object or shared decoder base. This is approved Source-domain pairing, not original CRT ABI/body proof. |
| Manager and sections | `GameSingletonHost::manager_publication_01090aa0()` -> same process manager cell. `SoundLifetimeAccess`/`CapturedSoundLifetimeSection` provide actual raw manager lookup, captured section and recursion/guard behavior. No semantic lifetime domain. |
| Factory publication | `GameSingletonHost::movie_factory_publication_00f8aefc()` -> `game_native_string_process().movie_factory_00f8aefc()`. One `void* volatile&` authority, loader-zero provenance, permanently retained with the manager through CRT/atexit. Not a fixed-address F8AEFC mapping; no constructor/drain reset or copied pointer authority. |
| Profile data | SAME `GameNativeReadOnlyData&`: validate CE3818/4, CFEA60/4, CFEA78/8 words `{412440}`, `{737030}`, `{737950,737730}`. These are evidence tokens, not callable host pointers. |
| Module and tables | One stable `BinkRuntimeLibrary`, all thirteen exports bound before owner publication; factory borrows its exact `BinkFactoryImports` and `BinkImports` storage. Root must retain the module AND tables AND every dependent owner/context. |
| Selected path | Required nonempty absolute `GameExecutableOptions::game_root`, append `binkw32.dll` and retain the selected path. `selected_library_path` currently falls back to CWD and is not this contract. There is no existing Bink option/member; do not invent `options_.bink_dll` as an available supplier. |

`GameMovieFactoryOwner` is stable C++ metadata around a separate raw
`MovieFactoryPrefix`, exactly4 bytes. Its constructor performs metadata/token
validation only, not native registration or sound setup. No application
instantiates it or `BinkRuntimeLibrary`; no movie arms exist in
`NativeSingletonDeletionBindings` or the finite dispatcher.

## Actual caller and identity epochs

Read-only `bsp.py ghidra` queries verify `C:/Users/sqz269/bsp.gpr`,
`/battlestationspacific.exe`, x86 LE32/base00400000 before analysis. Selected
original caller, helper, slot and destructor assembly windows were checked.

* Startup: terrain F8C218 slot+0C returns at `73DF99`; `73DF9B/9D` requests4
  through BF681B, captures EAX, arms state21 at `73DFAD`, null-skips at
  `73DFB8`, calls registration `7366B0` at `73DFBC`, stamps CFEA78 at
  `73DFC7`, then calls `A4D356` at `73DFCD` with actual `A4D35C` callback and
  argument0. Result is ignored. State demotes at `73DFD4` before the next
  allocation; following owner is 010909B0/CFEAC8, then particle and locale.
* Registration publishes incoming storage at `736706`, resolves the manager
  again at `73670C`, **then** reloads volatile publication at `736711` for
  registration at `73671A`. Unregistration likewise resolves at `7367A6`,
  reloads current publication at `7367AB`, unregisters at `7367B4`, then clears
  at `7367B9`. Both leave the originally captured section. Replacing this with
  the generic `void*&` helper or pre-read argument changes the epoch.
* Explicit shutdown: load CURRENT publication at `73804F`, null-test, read
  CURRENT slot0, push flags1 at `73805D`, call at `73805F`, after 010909B0 and
  before F8C218. It must select the genuine current profile route; it cannot
  simply use the owner's cached allocation or assume CFEA78 forever.
* `737950` and transient-base `737030` capture incoming ECX, run `736750`,
  free that captured address iff flags bit0 is set, return captured bits and
  `RET4`. Incoming allocation and CURRENT publication are distinct identities.
  The owner explicitly supports each address via `delete_factory_00737950`
  and `delete_base_factory_00737030`, subject to its captured-owner contract.

**737730 is decoder creation, not scalar deletion.** Its ECX is unused, its
stack DWORD is ignored, and it returns with RET4. It allocates38h, calls
`A4C660(flag1)`, and returns the shared decoder base at allocation+10h. Real
consumer `AAE794..AAE7AD` reloads current F8AEFC, reads its current slot+4,
forwards zero-extended widget byte+F4 at `AAE7A6`, calls at `AAE7A7`, then stores
the returned shared base at widget+F0. Existing
`create_movie_decoder_00737730` implements this creation body with the same
canonical heap. The returned state is not the four-byte factory or 34h frontend
MoviePlayer. Neither recorded PE table may be invoked as host code.

The other F8AEFC read at `A4B2A1` only selects an argument to the verified RET
trace `4254B0`; it is not decoder creation, retirement or an activation witness.
Do not manufacture a separate query-name/string authority or use that unrelated
online path as movie reachability proof.

## Smallest coupled source packet

The four actual integration files were unleased when checked:
`include/bsp/game_hosts.hpp`, `src/game_hosts.cpp`,
`include/bsp/native_singleton_destruction.hpp`,
`src/native_singleton_destruction.cpp`. Refresh leases and the published base
before implementation. No new standalone wrapper or CMake source entry is
needed: Bink is already in `bsp_core`, staged owner in `bsp_game`.

That target split matters. Calling application-owned `GameMovieFactoryOwner`
methods directly from core deletion code would introduce app-symbol dependencies
into core probe/test links. Append a required borrowed callback record instead:
stable context plus real captured-receiver/flags callbacks for factory737950
and base737030. Root supplies exact thunks to the actual owner; no default
success/null callbacks. Keep existing binding offsets, append the new pointer,
and update the size assertion deliberately. The private current-profile helper
is not an existing public generic dispatch API; a narrow movie-profile bridge
used by BOTH actual explicit shutdown and the two manager-map arms can live in
these same files. It must reject unknown profiles/foreign owner provenance.

In the root CPP, use a retained nested services aggregate with the real heap
adapter, module, owner and deletion record. Publish that aggregate into the
host's stable owning member and install deletion bindings **before** calling
`construct_in_startup_0073df9b`. Do not call native construction inside an
aggregate constructor that could unwind and unload escaped tables before the
host owns it. Module/import preparation can fail before factory effects escape;
actual registration/sound entry must have an installed retention route.

A new default-OFF guard may prepare this complete source coupling, preserving
the existing cohort. OFF must instantiate/load no Bink graph. ON source uses the
real inputs above at the known factory slot between input-era work and particle;
it adds no returning terrain/010909B0 placeholders. Current startup goes directly
from input callbacks to particle/locale, and full shutdown is still partial.
Those surrounding original producers/order obligations remain explicit; isolated
factory insertion cannot claim the complete ordinary parent.

Normal caller hookup must reload the SAME cell and route captured current
receiver through CFEA78/CFEA60 flags1 deletion at the original explicit shutdown
position. Retain binding/module providers through the subsequent raw manager
drain. Clear/unregister only through the real owner operation, once. Flag0
retains raw storage and grants no second deletion/free/retry through this owner.
No new source/ABI/body credit follows from these reused bodies.

## Failure, consumer and module gates

Current owner reproduces the recovered C++ caller scope: allocation exceptions
precede acquired cleanup; constructor/register/sound exceptions free ONLY the
captured raw block through BF65AC and rethrow. Reached publication/manager
effects may therefore dangle. Root must enter whole-aggregate terminal retention
immediately, **before any raw profile read, explicit deletion or shared drain**.
This retains metadata/providers and dangling pointer bits, not an allocation
already freed. Never invent publication repair, unregister retry or sound reset.
This is not original FH3/SEH/foreign-error or cleanup-failure continuation proof.

Owner phase `ready` returns false from `requires_process_retention()` but still
requires explicit retirement; that query is not destructor permission. Failed,
overlapping, interrupted and flags0-retained operations require the entire graph
to survive until OS exit. Existing `GameStartupHost::drain_native_singletons`
already catches drain failure and exits without ordinary provider destruction,
but that is too late to protect an already dangling prefix: add the immediate
movie failure guard and repeat it at shutdown/destructor/drain entry. A later
unrelated startup failure also needs an explicit ready-owner retirement or
retention policy before member destruction/module release.

Activation remains closed by actual consumer lifetime. The only application
`MovieWidgetHost` is `GameMissionResultHost::MovieBridge`; its
`ensure_movie_resources` records AAE770, creates no decoder, and opens no Bink
handle. Existing `GuiMovieLoadedHookHost::movie_player_00f8aefc_4` is a required
provider, not an actual production binding. A real decoder consumer must retain
the shared state and its complete allocation correctly, real VFS/strings,
renderer/texture credits and exact Bink table. It must run genuine open/update/
close/destruction (`A4CA40`, `A4CD70`, `A4CD30`, `A4C860`/`A4CF30`) and resolve
widget/completion/Lua borrowers. Factory deletion closes none of those handles.

The source module resolves all thirteen decorated exports but installed runtime
ABI remains unproved. In particular, `BinkSoundSystemOpen` returns another
function pointer, `BinkSoundOpen`; an older public-header copy corroborates that
declaration, not a matching installed SDK. Decorations prove argument bytes,
not callback return/use. A SetSoundSystem return alone may not exercise the
eventual callback. Do not replace the real callback with a void/default provider.

`BinkRuntimeLibrary` unloads in its destructor. Prefix retirement and a drained
manager do not establish vendor/callback/decoder quiescence, nor original unload
timing. Retain the actual module, borrowed table storage and ALL dependent
aggregate/context through every decoder close, callback completion and drain;
only then may a separately qualified normal unload occur. Otherwise an explicit
reviewed process-retention policy is required. Keep this packet's proposed root
guard OFF until these actual consumer/module obligations are closed; it is not
an activation-ready packet. `kMovieSubtitlesBound` remains false and supplies no
factory/decoder proof. Camera/cinematic “movie” guards are unrelated.

Primary future validation must distinguish standard source build/link and
Default-OFF exclusion from actual loaded-module identity, registration/sound
return, real slot+4 decoder production, open/frame/close, callback completion,
captured flags1 retirement, shared drain and unload/retention witnesses. Native
EH/fault and installed callback ABI remain separate qualifications. No old
subtitle, generic startup or accuracy receipt is a witness for these new calls.
