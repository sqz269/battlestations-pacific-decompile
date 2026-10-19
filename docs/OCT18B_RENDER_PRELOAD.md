# OCT18B native shader preload composition

`MissionShipShaderPreloadGraph` composes existing native producers for the
`BSP_NATIVE_MISSION_SHIP=1` diagnostic. It adds no complete native function or
unique byte credit. Instantiate it only in the enabled mission graph.

The primary's attempt05 identified the resource failure before this change:
the shared forward-only `B34890` cursor was 1064 after downscale depth, and the
next vertex key was `blendparticles0T3.vso`. The model worker independently
located that key at installed cache rows1026/1027 and downscale depth at
1062/1063. The installed preload script requests blendparticles before
downscale depth. Native startup calls full `73BF80` at `73DD12`; cold B107F0
material order cannot replace that preload. The composition calls the existing
full script implementation, including its native Lua table iteration; it adds
no manual name list, cursor reset, random lookup or source-compiler fallback.

The previous host has no native Application owner. The new graph establishes
live typed fields in an exact1Ch enclosing object without initializing their
preimages. Static size/offset assertions cover the profile, actual0Ch vector
at08, actual Game pointer at14 and first-update byte at18. Genuine
`737970 -> BEA970 -> BEA810` performs every native initialization and singleton
registration. Preserved bytes05..07 and1B are untouched. This mirrors the
enclosing1Ch storage role owned by original WinMain, without claiming its stack
location or private frame ABI.

The permanent string process now owns the sole loader-zero `E1AE90` publication
and a separate one-time Source admission flag. Its genuine manager authority
is unchanged. Cell borrowing starts no owner; BEA810 publishes it and BEA8B0
clears it. There is no second manager, semantic Application image or fabricated
profile store. The owner must be normally retired before singleton drain;
failure retains the entire graph rather than admitting an unbound profile to
the drain.

`create()` activates the renderer's existing Lua services and runs full
`preload_native_application_shaders_0073bf80` against the same pooled strings,
VFS, material effect cache/compiler, actual renderer and shader-cache process
cells. It requires a closed shader cache. Full73BF80 opens and closes its own
native bracket before the later fresh B107F0/model bracket. The real
Application08 vector retains every returned caller reference, including null.
Completed effects must resolve to their same actual04 canonical companions.
There is no extra retain or release on successful preload return.

`retire_before_native_drain()` follows the original inline
`737F80..737FB1` shutdown loop: ascending index, reload current vector data and
count, capture the row, decrement the actual effect04 once, invoke its canonical
current-profile terminal on zero, then clear the captured nonnull row. Each
release has retained observation metadata before native effects. Genuine
`7379A0` then resizes the vector to0, frees its current allocation and calls
BEA990/BEA8B0 to unregister and clear the Application publication. The actual
typed owner, operation and all providers remain alive through renderer/cache
drain. `after_native_drain()` closes metadata only after that drain and normal
Application retirement. Interrupted constructor/preload/retirement operations
cannot replay and require process retention.

Integration: include `game_native_mission_preload.inc` before
`game_native_mission_ship.inc`, with declarations from
`native_shader_preload.hpp`, `native_render_service_base.hpp`,
`native_material_effect_owner.hpp`, `game_native_string_process.hpp` and vector.
Keep the preload member after the model member so the model/provider domain
outlives it. Construct with log, canonical manager publication, model effect
cache, actual-owner registry, Lua services, shader-cache context and renderer
publication. Call create before opening the mission resource/model bracket;
include its retention predicate; retire its caller credits before scene/model
retirement and singleton drain; close its metadata before model metadata ends.
The graph is one-shot and cannot service a replacement device generation.
Primary owns device-generation admission, reset exclusion and all facade calls.

Original737970 uses ECX, returns the same receiver in EAX and plain RET.
73BF80 and7379A0 use ECX and plain RET. Inline737F80 uses ESI Application,
EBX unsigned index, EDI captured effect and EBP captured row; CE2220 receives
the address of effect04. The Source interfaces add explicit context arguments,
identity checks and retained operation metadata. Numeric profiles are identity
only, never callable host vtables. Original FH3/SEH, stack/register replacement
ABI, exceptional cleanup, concurrent access and game parity remain unproved.

The CLI verified `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, before live analysis queries. Complete737970
(42bytes) and inline737F80..737FB1 (50bytes) matched live Ghidra and the installed
PE. SHA256 values are respectively
`f1f9334e65e51c7d309f1f199cdd8e647b50e4ea0878b2d69a60913539c03f6d`
and `77f3abcd2c6dca082ad1b49acd114774f297be2656ced03788ed2a10f2d6a41c`.
Ignored receipt: `local/oct18b_a1_preload_byte_proof.json`.
`git diff --check` passed. The primary owns the combined build and live runs;
this worker performs neither and claims no new runtime or visual proof.

Primary integration subsequently built and ran ON attempt06. Its
`local/oct18b_render_on_attempt06.log:3537` records full preload completion,
179 entries and a closed cache at native site73C204. The same run completed
B107F0, loaded26 Fletcher models/445 sections and reached the ready mission
graph (lines3541..3544). First command execution then faulted separately at
host RVA4161FB reading28. This verifies the normal preload/cache-order path
in that integrated run; it supplies no completed draw, visual result or normal
preload-reference/Application retirement proof.
