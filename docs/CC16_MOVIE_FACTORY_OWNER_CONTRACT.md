# Production movie factory ownership contract

The smallest useful next packet couples a real retained `F8AEFC` publication, its raw lifetime/deletion binding and the actual Startup constructor/retirement hooks. A new unused bootstrap wrapper alone would not close this producer. The normal-path source contract is pinned; actual caller failure cleanup remains a primary max-effort prerequisite. No code, address/ledger state, Ghidra state, module call, game, probe, build or test was changed/run in this packet.

Reviewed main: `61c13da1218bb93a3f952d4aa0fc831c86af3f33`. Read-only dependencies: readiness `857d4234e4a65ce623f166aca3894eeb9194a7d4` and module owner `68ab882bbe7709822936c15b21f6e9804076ffb2`. The latter is not in this main snapshot; this contract does not independently review my own module implementation or claim its admission.

## Native producer, identity and normal terminal

- `0073DF9B` pushes **4**, `0073DF9D` calls the existing `00BF681B` allocation boundary, and `0073DFAD` arms caller state `0x21` for the captured receiver. The null branch at `0073DFB8` skips constructor effects. This establishes four bytes for this concrete producer; the prefix declaration alone was not size proof.
- The successful path calls `007366B0` at `0073DFBC`, writes `CFEA78` at `0073DFC7`, then calls the Bink sound-system thunk at `0073DFCD`. Existing `construct_movie_factory_00737710` already expresses that same-owner constructor sequence. Reuse it; do not reconstruct/credit the body again.
- Native order is after the terrain `F8C218` virtual `+0Ch` call at `0073DF99`, before the following `010909B0/CFEAC8` producer at `0073DFD2..0073DFFF`, particle allocation/construction at `0073E005/0073E02B` and locale construction at `0073E057`. The current concrete Startup source has input followed by particle/locale; the terrain/adjacent producer gaps remain explicit dependencies, not returning stubs.
- Normal explicit movie retirement reads `F8AEFC` at `0073804F`, pushes flag 1 at `0073805D` and calls slot zero at **`0073805F`**, after `010909B0` and before `F8C218`. `00737950` captures incoming `ECX`, calls `00736750`, frees that captured address iff bit 0 is set, returns the captured address in `EAX`, and uses `RET4`.

Static installed PE and saved analysis agree: `F8AEFC` is a loader-zero DWORD in the zero-fill portion of `.data`; `CFEA78` contains `{00737950,00737730}` and `CFEA60` begins with `00737030`. These words are **numeric native profile/code identities**, not callable host pointers or a C++ vptr. The recorded image bytes establish initial storage provenance, not a current process publication.

## Real input suppliers and gaps

| Required input | Exact type / supplier | State at reviewed main |
|---|---|---|
| Canonical manager publication | `void* volatile&`; `GameSingletonHost::manager_publication_01090aa0()`, backed by `game_native_string_process().manager_01090aa0()` | Available; borrow this same cell |
| Captured section and register/remove operations | `SoundLifetimeAccess` from `GameSingletonHost::sound_lifetime()`, `CapturedSoundLifetimeSection`, resolved `SoundLifetimeManagerView` | Available actual-storage path; no private semantic domain |
| Movie publication | One retained `void* volatile& actual_factory_publication_00f8aefc` | **Missing** cell/accessor/context in current application singleton owner; root must admit it from the original zero/publish/clear contract |
| Module/import storage | `BinkRuntimeLibrary&`, `const BinkImports&`, `const BinkFactoryImports&` from the same owner | Implemented in pending `68ab882`; root admission/instance is missing |
| DLL path | Explicit audited absolute `std::wstring`; root composition can use nonempty absolute `GameExecutableOptions::game_root` plus `binkw32.dll` | Concrete hook missing; reject empty/relative root and avoid `selected_library_path` cwd fallback |
| Four-byte receiver | `MovieFactoryPrefix*`; existing `singleton_lifetime_allocate({object,4,sizeof(MovieFactoryPrefix)})` | Allocator exists; actual movie Startup allocation/caller missing |
| Matching free | Existing `singleton_lifetime_free(void*)` | Available; use captured primary address, never default-delete a metadata wrapper |
| Constructor and two bootstrap callbacks | Existing `construct_movie_factory_00737710(MovieFactoryPrefix&,MovieFactoryBootstrapHost&)` | Body available; concrete actual-publication/Bink bootstrap host missing |
| Native deletion routing | `GameSingletonHost::native_deletion_bindings()` | Dispatch exists; `CFEA78` and transient `CFEA60` movie arms/context are **missing** |
| Explicit retirement caller | Current publication, null guard, typed deleting call with flags 1 at `0073805F` | **Missing**; raw-manager drain does not establish original explicit shutdown order |

`singleton_lifetime_allocate` is the existing Win32 host CRT boundary: malloc/new-handler retry, then `bad_alloc` when the handler declines. Do not invent a successful null-return provider or catch failure as an empty factory. Preserve the original caller null guard without making it a fallback policy. The native receiver is the four-byte `MovieFactoryPrefix` allocation, not a larger adapter object or a stack substitute.

## Required registration and deletion ordering

`007366B0` establishes the `CFEA60` registration profile, obtains the canonical manager and captures its `+10h` section, enters/increments that section at `+18h` when nonnull, publishes the incoming owner, **resolves the manager again, then rereads the current publication** for registration. The key listing is `00736706` store, `0073670C` second getter, `00736711` publication load, `0073671A` register. Unlock/decrement uses the originally captured section, even if a callback changed the manager publication.

The existing generic `cc13_seg41::singleton_register/unregister` accepts `void*&` and passes a loaded `global` into `SingletonLifetimeHost`. A host method that obtains the second manager internally would receive its publication argument too early; casting away `volatile` would not fix the ordering. A production adapter must explicitly resolve the second manager in a completed statement before evaluating the real volatile publication, using the already genuine raw lifetime views.

`00736750` writes `CFEA60`, captures/enters the first section, resolves the manager again, unregisters the **current** `F8AEFC`, clears that cell unconditionally, leaves the captured section, then writes `CE3818` to incoming storage. It does not require incoming owner == current publication. On an interrupted operation preserve reached stores/registration residue and the established guard unwind; do not invent clear/unregister/retry success.

`00737950` frees and returns the captured incoming address after normal unregister, independent of any publication replacement. The transient `CFEA60` slot-zero terminal `00737030` has the same unregister/free/return contract. Bind both profiles before first registration; do not let the finite raw dispatcher encounter an unadmitted transient profile. The current dispatcher deliberately throws on unknown profiles or missing bindings.

Future factory slot `+4` must map to existing `create_movie_decoder_00737730`: it allocates the real `38h` decoder object, constructs it and returns shared state at allocation `+10h`. The existing GUI host method name `movie_player_00f8aefc_4` is provisional; its returned decoder state is not the `34h` frontend `MoviePlayer` object. This does not provide a fake Bink handle or a usable clock. Actual GUI/decoder ownership and open/update/close callers remain required.

## Bink custody and Startup retention

Use the same module owner from `68ab882` for all eleven decoder imports and the two factory exports. Its required table is complete before factory publication. The actual sound call is `factory_imports.set_sound_system(factory_imports.open_direct_sound, 0)`; pass the real callback and ignore the signed result as native does. Do not invoke `OpenDirectSound` manually, create a default callback, or add a new return-value failure policy. The installed DLL remains the exact PE32/x86 178,440-byte artifact, SHA-256 `7f22055e8f62e228eb90f09846f9b668e0a0b1d29057af6e3b7bff1e08c0d724`; the module report retains the older public-header typing qualification and all thirteen export identities.

Normal lifetime must keep the module/Impl/table storage, actual publication/context, raw manager and all real decoder/widget/renderer borrowers alive through handle close, explicit factory retirement and the final shared drain. Factory deletion does not itself close unrelated decoder handles or unload Bink. Release the module only after those obligations finish. Root must add its services owner before potential borrowers in destruction order or explicitly close it after the complete successful drain; accidental member unwinding is not custody.

Interrupted factory/native work must reach the existing whole-graph process-retention policy before ordinary destructors/fallback drain can release anything. Current `GameStartupHost` has terminal retention paths for VFS/renderer/fonts/native factories; the movie path needs its own real reached-state hook. Preserve the complete aggregate until OS exit, including table storage, rather than retaining only a copied HMODULE or attempting cleanup again.

Current `GameStartupHost::application_shutdown` does not bind the full 42-action `00737F30` driver. Its unimplemented log at `00737F80` names the system-array slice, **not** the movie row. The source table puts the movie row at action index 16 / `0073804F`. Raw drain at WinMain `008F8449` is later; it is necessary fallback custody, not evidence that original explicit retirement order already ran.

One additional xref, `00A4B2A1`, loads the same publication for a debug logging argument. It is not a factory method or a separate query-name producer. Do not fabricate a string global/provider or expand this packet into online activation.

## Smallest coupled source packet

1. Before actual caller activation, primary pins the failure boundary below. Normal-path lifetime implementation and module admission can proceed under their own leases; root owns the module CMake/path preparation.
2. Worker can own `include/bsp/native_movie_factory_lifetime.hpp` and `src/native_movie_factory_lifetime.cpp`, plus its evidence files. A minimal explicit host context borrows **only** `void* volatile& actual_manager_publication_01090aa0` and `void* volatile& actual_factory_publication_00f8aefc`. Provide raw register/unregister and captured-primary scalar-deletion bindings; reuse existing native factory prefix/constructor/bootstrap interface rather than inventing a larger native class. No new allocator or native body credit.
3. Root couples the existing `GameSingletonHost` header/source and `NativeSingletonDeletionBindings` header/source: one retained original publication cell/accessor, stable context, `CFEA60`/`CFEA78` dispatch arms installed before registration, all using the same canonical manager.
4. Root couples `game_hosts.hpp/.cpp` and `cmake/startup.cmake`: retained module + real bootstrap adapter, actual four-byte producer at the native order anchor, explicit `0073805F` retirement, failure/process-retention hooks and final module release. Existing terrain/adjacent/shutdown providers remain explicit gaps until their own real callers are bound.

New lifetime functions alone may be reviewable dormant source, but they are **not** a production provider closure without those root-owned publication, deletion and actual caller hooks. The detailed proposed signatures, field types, suppliers and files are in the report.

## Primary max-effort handoff

The original register ABI and captured-section behavior are already pinned; no new register/x87 recovery was performed. The new unresolved question is the **Application caller**: `0073D410` installs EH handler `00C86CBA`, stores the captured allocation at stack `+10h` and arms state `0x21` at `0073DFAD` before register/Bink setup. This packet does not recover that cleanup entry. Primary must pin its target/action, free identity and residual publication/registration before implementing a native-style failure rollback. Do not infer an unregister, publication clear or allocation free from the state number alone. A terminal host-retention policy may be reviewed separately; it is not original FH3/ABI equivalence.

## Validation boundary

Future source admission needs the normal Win32 build/existing checks, exact static ordering/profile review and root-scheduled fresh frozen production witnesses. A real normal witness should observe actual factory allocation/registration/Bink call return, explicit flags-1 deletion and module/context custody through drain. Old subtitle 409 OFF logs and the module object compile do not validate this new caller. No new test framework, fake handles, fabricated providers or new runtime witnesses are supplied here.

`kMovieSubtitlesBound` remains OFF. Real decoder handle/realtime triple and native selector, live locale/settings admission, renderer surfaces, widget/Text/localisation/extratitles and Lua/callback reachability remain separate dependencies. No native/duplicate/vendor-body, ABI, decoder-clock, playback or activation credit is added.

Exact paths/hashes, native listings, provider matrix and next packet: [contract report](../reports/cc16_movie_factory_owner_contract.json).
