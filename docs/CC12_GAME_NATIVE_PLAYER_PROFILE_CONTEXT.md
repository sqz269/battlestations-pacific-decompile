# Retained player-profile constructor context

`GameNativePlayerProfileContext` owns one existing concrete
`NativePlayerProfileCalls` service and one stable `NativePlayerProfileContext`.
It supplies the missing application-side context provider for the existing
raw game+`650h` profile constructor. This packet adds Source composition only;
the provider has no application caller or build registration in this packet.

The constructor borrows the exact `GameNativeVfsRawServices::strings` wrapper,
the address returned by `GameNativeSettingsProcess::settings()`, and the
reference-bearing `profile_context()`. Copying that small context retains its
references to the current manager `00F8ABE8`, current game `00E188A8` and SDK
call service. It does not snapshot either publication or create an SDK binding.
The settings getter preserves the existing initialized-owner check. The caller
must supply the same canonical VFS/settings/read-only owners used by the game.

The three literal pointers come from `GameNativeReadOnlyData::data_at`, which
checks section bounds and mapped bands of the supported SHA-256-verified image.
The existing R106 evidence records matching PE/live spans at `00CE3A0C[1]`
(empty), `00CEF794[18]` (`globals.newplayer` plus NUL), and `00CEF15C[5]`
(`RANK` plus NUL). The current reset copies 17 bytes of the new-player label
and 5 bytes of the rank string; the provider validates each full literal span.
No new live Ghidra or installed-image comparison is claimed by this packet.

Construction and `borrow_construction_context()` allocate no profile and invoke
no profile constructor, reset, settings reset or SDK call. They write no game
publication, clear no graph bytes and publish no global provider. Repeated
borrowing returns the same context and does not reset its consumers. Copy and
move are deleted because the context refers to the provider's owned calls.

The provider, VFS/string owner, settings process, mapped read-only image and
any subsequently bound SDK library must survive the profile graph and every
retained failed operation. The eventual game allocation still supplies the
profile's native preimage, including the valid or null progress pointer at
profile+`64h`; this provider does not prepare those bytes. The provider's
destruction does not drain a graph or synthesize native exception cleanup.

Profile teardown remains a separate application obligation. Its
`NativeGameProfileLifetimeContext` must use the same eventual
`NativeGameLifetimeCalls` object as the whole-game lifetime context and the
same actual raw string domain. The constructor's owned
`NativePlayerProfileCalls` is not a second game teardown dispatcher. No lifetime
context, virtual table, startup call, shared dispatcher or game allocation is
created here.

Native contracts remain those documented by the existing raw implementation:
`007FEE20` takes the actual `F8h` profile in ECX and returns it in EAX with plain
RET; `007FDB20` takes the same receiver in ECX with plain RET; `00436710`
constructs the actual empty-name header in ECX and returns it in EAX. This new
provider has an explicit C++ interface and makes no native ABI, FH3 exception,
application startup or gameplay claim. Descriptive names remain hypotheses.

Evidence: `native_player_profile_owner.hpp/.cpp`,
`game_native_settings_process.hpp/.cpp`, `game_native_vfs_runtime.hpp`,
`native_profile_settings.hpp`, `game_native_readonly_data.hpp/.cpp`, and the
recorded literal spans in `reports/native_player_profile_owner_r106.json`.
The dedicated report records source hashes and bounded static review. The
worker does not edit CMake/config/ledgers/Ghidra or run a build, test or probe;
the integrator owns registration, review and compilation.
