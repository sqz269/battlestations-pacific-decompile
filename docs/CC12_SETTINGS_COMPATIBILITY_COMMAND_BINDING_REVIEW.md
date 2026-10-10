# Settings compatibility command binding review

The accepted Source at `48d432db3` has no composed production route from the
settings archive reader to compatibility publication and Lua command delivery.
The reader delegates to a pure virtual callback with no concrete Source
override. Its only forwarding caller, `PcProfileIoHost`, also has no production
construction in the retained Source. This is an unbound route, not evidence
that an executing archive callback dropped or duplicated a command.

The current raw startup loader is `008D8190`, a different path from archive
reader `008D6DC0`. Adding compatibility publication to that loader would create
a new call at an unproved time. No C++ change or activation is made here.

## Evidence and scope

Only function `008D44C0` was newly queried in Ghidra. Typed reads verify
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, x86 Windows, and the
unchanged modification number `47` during the typed capture. Its complete body
is `008D44C0..008D4518`, **89 bytes / 24 instructions**. The end-minus-start
delta is `58h`; this is not a 58-byte body. The finite bytes agree with the
complete retained 12,223,752-byte PE, SHA-256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

Source/Git is frozen at `48d432db3863ea5b0254dc05a8bdbd405dbb74c6`.
All 4,206 tracked files under `src/` and `include/` are retained as complete
Git and working-byte archives. The 24 reviewed Source roots close through
546 project files and four Lua API headers from the CMake-hash-checked Lua
archive. System/C++/Windows/Direct3D headers remain explicit external compile
dependencies; no SDK state or compiler closure is claimed. The Lua archive is
the configured input, not a newly built interpreter or an asserted current
build preimage; `cmake/lua.cmake` records its later `LUA_COMPAT_LSTR` transform.

Nineteen further complete Source/Git inputs include the build declarations,
query tools and earlier reports. Whole saved exports for `008D44C0` and
`008D6DC0` are retained. The whole post-audio reader report and document are
retained from worker `83d73fe97`, with their own `7af8a0ac6` Source epoch and
older admitted evidence preserved. That worker report was pending Root
acceptance when assigned; it is not relabeled as this packet's fresh Native
capture. No fresh `006B8AD0`, `006B89F0`, other provider body or data-range query
was made. Their contracts below are reused from retained prior evidence.

## Exact Native operation

The original ABI is `__thiscall`: ECX is the whole settings receiver, the
low byte of `[ESP+4]` is the input, and each exit uses `RET 4`. The stored
Ghidra signature is still `undefined(void)`; the assembly and prior ledger
supply the actual register/stack contract. This review did not repair it.

| Native site | Established local behavior |
| --- | --- |
| `008D44C0/44C4` | Read the stack byte into AL; retain the settings receiver in EDX. |
| `008D44C6` | Store that byte at settings `+B0h`. |
| `008D44CC` | Store the same byte at `0108FF20`. |
| `008D44D1..44EE` | After both stores, check game `[00E188A8]`, game `+1A08h`, then that host's `+4h` machine pointer. Any null exits at `008D4516`. |
| `008D44F0..44FD` | Reload settings `+B0h`, compare with zero, and push mode `2`, null error output, null result sink. |
| `008D44FF/4504` | Nonzero: push literal `00CF7F38` (`X360COMP=true`) and call `006B8AD0` once. |
| `008D450C/4511` | Zero: push literal `00CF7F28` (`X360COMP=false`) and call `006B8AD0` once. |

ECX at either command call is the **LuaMachine pointer** from host `+4h`.
The setter does not additionally read or guard the machine's `+4h` Lua state.
Prior `MISSION_LUA_HOST` evidence distinguishes those two objects. It also
establishes `006B8AD0(text, results, error, mode)`, `RET 10h`, forwarding
`(text, strlen(text), text, results, error, mode)` to `006B89F0`.
The latter performs Lua load/pcall and stack/result/error handling. Thus the
setter can have delegated interpreter effects when the machine is present.

The reader's retained prior evidence supplies `ECX=settings` and the byte
read from settings `+B0h` at `008D7253`. This packet closes the setter's two
local data stores and optional call schedule. It does not establish absence
of delegated, Lua, alias or global effects, whole-reader preservation of
`+24h`, archive timing, or the original sound-output selector.

## Actual Source composition

| Source component | Observed binding and limit |
| --- | --- |
| `set_xbox_compatibility_008d44c0` | Updates the semantic `GameSettingsBlock` Boolean and returns optional text from a supplied presence Boolean. It does not write raw `NativeGameSettingsStorage` or execute Lua. |
| `publish_lua_xbox_compatibility_008d44c0` | Calls that helper, writes the supplied `LuaRuntimeGlobals.x360comp`, and returns the text. The header explicitly delegates delivery. The complete Source search finds its declaration and definition, with no caller. |
| `read_settings_archive_008d6dc0` | Reads `XboxCompatibilityMode`, then calls `SettingsArchiveReadHost::publish_xbox_compatibility_008d44c0` once. The callback is pure virtual; no override exists in the retained corpus. |
| `PcProfileIoHost` | Forwards its supplied `services_.archive_read` to the reader. All mentions are confined to its implementation/header; no production construction was found. This is a dependency seam, not a binding witness. |
| Raw startup | `GameNativeSettingsApplication::load` calls `load_native_game_settings_008d8190` on its retained raw process settings. It subsequently copies raw `+B0h` into a semantic read view. It neither calls the archive reader nor publishes this setter. |
| `GameScriptOrdersHost::lua_run_string_006b8ad0` | Discards `chunk` and `mode`. It is not connected to this setter and is not a delivery implementation. Its name/ledger row cannot prove successful execution. |
| `GameMissionLuaHost` | Supplies real `luaL_loadbuffer`/`lua_pcall` operations to `run_lua_chunk`. This usable semantic command machinery exists, but no compatibility-publication adapter connects it to the archive reader. |
| Press-start adapter | `refresh_input_bindings` still logs `008D44C0` as unimplemented; this is another explicit unbound call site, not a publisher. |

The Boolean helpers cover normalized Source values. The Native setter stores
the original byte unchanged and tests its current value for zero. Arbitrary
noncanonical input bytes, the raw receiver layout and a post-store reload of
the actual three-pointer chain are outside the helper signatures. A supplied
`command_sink_present` is a precomputed predicate, not proof of those Native
loads. The helpers are not drop-in Native ABI replacements.

## Readiness and ordering

`game_main.cpp` explicitly initializes the retained Lua globals process before
creating `GameStartupHost`. Access to its globals requires successful explicit
static startup. `GameNativeLuaServices` borrows those same fields; its bootstrap
binding retains the compatibility-byte reference. `GameScriptHost` also borrows
those globals. In ordinary Source initialization, Lua services and input tables
are established before the raw settings loader and read-view copy.

This is positive evidence for canonical global ownership, not evidence for the
setter's mission Lua machine. Native-owner bootstrap reads the shared byte;
semantic owner construction snapshots it into an environment and executes its
own compatibility chunk. Neither establishes a live update to existing states
after a later setter call. Fundamentals availability, an input Lua owner and a
renderer Lua owner are not substitutes for the selected mission machine.

The actual `GameMissionLuaHost` is separately created and started in
`game_hosts_mission.cpp`'s mission-load path. `start_machine_00884be0` checks the
resulting state before marking the machine started; its real Lua execution
services return failure if state is absent. Source explicitly comments that
this construction time differs from the original OnInitOnce schedule. There
is no bound archive callback from which to prove its relative delivery timing.
No startup, selector or live-machine readiness credit is earned here.

## Smallest follow-up contract

The next bounded work should implement and review **one publication adapter**
only after assigning its actual settings receiver and mission-machine owner.
It must preserve these operations:

1. Write settings `+B0h` and the canonical process compatibility byte before
   inspecting command availability. For a raw path, use the same retained raw
   owner the reader changes; writing only `settings_view_` is insufficient.
2. Resolve the actual optional mission machine after those stores. Preserve
   skip-on-absence; do not construct an early interpreter, queue a replay,
   broadcast to all Lua states, or infer readiness from fundamentals/input Lua.
3. When the represented machine exists, deliver exactly one selected command
   with the recovered null result/error sinks and mode `2`. The existing
   semantic chunk service would require `(command, strlen(command), command,
   false, false, 2)` on the correct live `GameMissionLuaHost`. Its representation
   of Native machine presence, including partial initialization, needs explicit
   acceptance; `state != nullptr` is not the setter's original third guard.
4. Bind the archive callback to that adapter and prove its owner/lifetime and
   caller order. The current callback takes only a Boolean, so any retained
   receiver must demonstrably be the reader's same object. Preserve the
   distinction between a semantic `GameSettingsBlock` route and a raw BCh route.

A one-line call to the text-returning helper does not complete this contract.
Connecting it to the argument-discarding method would drop the command. Adding
it to raw startup would change the call schedule. The missing archive/raw-owner
composition is therefore a prerequisite for a complete production fix, not
permission to invent one in this review. No C++ GO is requested or assumed.

## Verification

`reports/cc12_settings_compatibility_command_binding_review.json` pins the
complete evidence directory, exact excerpts, Source queries, Native captures
and replay scripts under `local/cc12_settings_compatibility_command_binding_review`.
The portable replay verifies all retained hashes, full Source archives and Git
blob IDs, include closure, ten complete-corpus searches, 32 exact excerpts,
31 Source anchors, statement order, PE placement and all 89 function bytes.
Its audit hook forbids external reads, writes, subprocesses, network access
and dynamic-library loading: installation opens are zero.
The same replay also passed from a separately copied evidence directory.

Separate Git mode verifies 4,227 baseline/prior blobs; its optional working
check verifies 4,225 files. It is explicitly not the artifact-only mode.
No build, compiler, fixture, game, interpreter or Native code execution was
performed. No C++, SDK, operating-system, installed-game or GPR state was changed.
