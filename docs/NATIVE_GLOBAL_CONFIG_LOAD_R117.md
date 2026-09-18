# Full native global configuration loader

Addresses: `0087D7B0`, `00432650`, `004DDB90`.

## Result

The complete normal `0087D7B0..0087F96F` body (8,640 bytes) is reconstructed in
`src/native_global_config_load.cpp`. `NativeGameConstructionCalls` now supplies
concrete `00432650` and `0087D7B0` defaults. The parent captures the getter's
actual 2E8h configuration pointer at `004DDFBD` and passes that same pointer to
the loader at `004DDFC4`, with explicit shared lifetime, string, Lua, VFS and
sound contexts. Parser registration `00717E80` and grid construction `0070BD70`
remain required external game-constructor services.

This closes the population/binding frontier recorded in
`NATIVE_GLOBAL_CONFIG_FIELDS_R116.md`. The earlier minimap projection in
`game_hosts_lua.cpp` remains separate historical application code; this change
does not wire the full raw game constructor into the application.

## Native behavior

Native ABI: ECX is the actual configuration owner; no stack arguments; plain
RET. The new C++ API exposes dependencies and retained operation state and is
not a drop-in binary replacement. Descriptive names remain hypotheses.

The loader constructs an actual 4C8h Lua owner, opens library mask 1, executes
`Scripts/datatables/Globals.lua` including script overrides, and performs all
72 named lookups. Register/stack analysis resolves each receiver and temporary;
the `Globals` literal is at `00CE490C` (correcting the old provisional ledger
literal description). The report records every direct call site and all field
descriptors. Existing concrete Lua/string/container/color/sound implementations
are dependencies; this packet does not recreate Lua or general STL code.

Important ordering and conversion details:

- Extra-effect names and all four difficulty vectors append on repeated loads.
  The HP table alone controls the difficulty loop, starting at index 1. HP and
  player-cheat numbers are inverted through x87; score and lock-radius values
  are stored directly. Spare capacity uses the native direct store; growth
  uses the existing float insertion storage contract.
- Eight numeric defaults require an exact Lua NUMBER. Ordinary numeric reads
  retain Lua conversion. Seven message strings use the existing nullable
  C-string assignment and preserve native resize/publication behavior.
- Marker radius is squared through x87. Ship and Plane FOV conversions use
  double constants at `00CE3D28` and `00CE3D20`, then independently read the live
  float divisor at `00F889B4`. Its PE/BSS zero is not a runtime default.
- Ten named map colors, six marker colors and eleven chat colors use the
  existing packed-byte color reader, then x87 division by double 255. Marker
  and chat outer indices start at zero. The inner color reader uses indices
  1 through 4 and preserves its BGRA packing.
- Objective sounds iterate table values in Lua's order and assign sequential
  slots through the existing sound cache/lifetime implementation. Native
  malformed-index behavior is not replaced by clamping.
- Persistent Lua references are destroyed in native order before the state
  closes. The initial script path and objective names release their current
  header; extra-effect append releases its captured temporary allocation.

The operation is one-shot. C++ failure retains the partial configuration and
Lua locals. Explicit diagnostic abandonment releases retained locals and
closes Lua without rolling back configuration writes or permitting replay.
Parent diagnostic retirement refuses a still-retained loader child. This
diagnostic mechanism is not the original FH3 unwind schedule. Raw Lua nonlocal
errors are not converted into a new protected boundary.

## Validation

- Ghidra project/program checked against `C:/Users/sqz269/bsp.gpr` and
  `/battlestationspacific.exe`; original PE SHA256 is recorded in the report.
  9,827 live/PE bytes match: loader/getter bodies, eight constant cells,
  73 literals, and both parent call sites.
- A copied original loader executes all 8,640 bytes with 325 direct CALL sites
  bridged to the same concrete source dependencies used by the reconstruction.
  It uses linked Lua 5.1.1, real bootstrap/file/DoFile/override execution over
  controlled in-memory fixture files, and the concrete cache with controlled
  sample factory/lifetime callbacks. These are explicit fixture services, not
  proof of the installed game's authored scripts, disk VFS or FMOD playback.
- Twelve comparisons cover all four rounding modes at x87 precisions 24, 53
  and 64 bits, with three successive loads in each. The source side invokes
  the concrete game-call getter and loader defaults on the same owner.
  7,560 observations match 13,668,480 bytes: complete configuration storage,
  normalized owned-pointer fields, string/vector contents and capacities,
  sample reference counts, string allocation totals and Lua/file events.
  Lookup metamethods observe intermediate writes and change the FOV divisor
  between Ship and Plane. Numeric-string/false defaults, nullable messages,
  empty/populated effect and sound tables, and repeated vector growth are
  included. Unconsumed allocation tails and Lua private pointers are excluded.
- One source-only VFS exception check verifies retained failure and explicit
  diagnostic cleanup. It does not exercise copied-original FH3 handling.
- The adapted existing parent fixture passes all 33 cases: 1,657 observations
  and 121,405,968 matching bytes. It verifies context/child-operation routing;
  its global-config boundary remains controlled. The separate full-loader
  comparison above supplies the loader evidence.
- The existing genuine Dyn-world fixture passes: four worlds, 280 paired
  buffers, 204 explicit handle closes and real CRT atexit cleanup. It remains
  separate from the controlled parent fixture.
- Strict MSVC Win32 build and all three existing CTests pass. No permanent
  tests were added. Detailed source, dependency, executable and archive hashes
  are in `reports/native_global_config_load_r117.json`.

## Remaining limits and follow-up

Original FH3/SEH, private stack aliasing, malformed tables/indices, allocation
failures, unmasked floating-point traps and whole binary ABI remain open.
The raw game constructor is still outside the application's reached path.
No application or gameplay run is claimed for this packet; the earlier
device-creation/runtime limitation has not been retested here.

Ready follow-up investigation is the concrete parser-registration closure at
`00717E80`, or the independent grid constructor `0070BD70` and its geometry and
renderer dependencies. Claim their addresses and files before starting work.
