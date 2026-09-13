# Native settings construction and script startup prefixes

Addresses: 006AB6B0, 006A7BE0, 0069D1F0, 0069D240, 0069D290, 0069D320

`native_input_settings_startup.cpp` supplies two raw-storage prefixes needed by
the settings producer. It does not replace either full original entry. The
constructor prefix stops before its loader call at 006AB7E3; the script prefix
stops before tree clearing and parsing at 006A7CE2. No singleton is published.
The remaining parser, complete constructor/getter/destructor, and application
wiring are required before this becomes a complete runtime settings path.

Existing `InputScriptStartup` remains a separate host composition. These new
fragments produce the actual allocation and Lua owners consumed by the raw
keyboard path, without converting that host model or inventing successful
implementations of missing calls.

## Construction storage

006AB6B0 receives fresh storage in ECX and eventually returns it in EAX with RET.
The allocation is 0x540 bytes (1344 decimal). The 307-byte reconstructed prefix
006AB6B0..006AB7E2 writes profile CF81CC, builds the following empty members,
constructs the persistent 4C8h Lua owner at +78, then clears bytes4 and5.

| Member offset | Original storage |
| --- | --- |
| +08 | 12-byte tree header; head allocation9Ch, color98/nil99 |
| +14 | Checked vector: opaque word, begin/end/capacity at+4/+8/+C |
| +24 | 12-byte tree header; head allocation2Ch, color28/nil29 |
| +30, +40 | Checked vectors, same header fields |
| +54 | 12-byte tree header; head allocation18h, color14/nil15 |
| +60 | 12-byte tree header; head allocation24h, color20/nil21 |
| +6C | 12-byte tree header; head allocation9Ch, color98/nil99 |
| +78 | Actual `NativeLuaStateStorage`, ending at+540 |

Each consumed allocation helper initializes the three links and color/nil bytes.
The constructor then makes its head nil, sets all three links to that same head,
and clears count. Head payloads/padding and header opaque words retain their
allocation preimages. The three vectors clear only begin/end/capacity. Byte50
DEVINPUTS is still untouched and cannot be treated as a loaded value.

These small empty-node allocation contracts use the existing CRT allocation
domain; this packet does not introduce a general STL implementation or rename
library bodies. Existing `construct_native_lua_state_00b66bd0` initializes the
embedded owner, including its 50 tracked-slot counts, without zeroing unrelated
reference cells or padding.

The original constructor has ten FH3 states at DACF70/DACF94. On supported source
allocation failure, only already-constructed empty members unwind in reverse
order. Its base cleanup69BA20 clears the supplied E198E8 publication and stamps
CE3818. Nonempty member destruction, ownership mutation from an allocator's
new-handler callback, and original FH3/hardware-fault behavior are not supplied.
Successful prefix storage remains caller-owned; this is not a complete owning
settings object or destructor.

## Script execution and continuation

006A7BE0 receives settings in ECX, has no native stack inputs and returns with
RET. The 258-byte prefix006A7BE0..006A7CE1 checks byte4 and takes the real early
return if already started. Otherwise it sets byte4 before opening the persistent
owner with mask1 and executes `Scripts/datatables/ControlPresets.lua` with flag0.
After releasing that path, it constructs and opens a separate temporary owner
with mask1, executes `Scripts\datatables\KeyboardSetup.lua` with flag0, and
releases its path. The exact path case and separators are preserved.

Both calls use existing native Lua bootstrap, pooled-string and file-loading
services. Nested DoFile uses the same application services. The persistent
presets state and temporary keyboard state remain distinct. Neither byte5 nor
DEVINPUTS is written. No tree clearing, table extraction, ControllerInputNames
execution or publication has occurred at the fragment boundary.

`NativeInputSettingsScriptFrame` retains the actual temporary owner at a stable
address for the parser continuation. `tables_pending` means that continuation
is required; `guard_return` means the original whole loader would return. A fresh
inactive frame is required. Closing the frame closes only the temporary owner;
the caller must retain the persistent owner for the settings lifetime.

The loader's FH3 descriptor DACB90 has 76 states; the prefix uses states0/1/2.
State0 releases the first path, state1 closes the temporary owner, and state2
releases the second path before unwinding state1. Source exceptions retain the
started flag and persistent interpreter. The frame disarms before closing, so
cleanup is not retried after a close failure. Original FH3 execution and private
native stack aliases remain outside these explicit source fragment interfaces.

## Evidence and validation

The report records six disk/live-equal native spans, two original epilogues and
21 audited CALL rows. Function identity is checked against the existing
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`. Existing names are retained
as behavioral hypotheses; comments/prototypes are recorded before annotation,
the project is saved and affected exports refreshed.

One ignored manifested Win32 fixture compares 482 words. It checks every
constructor-prefix allocation byte after normalizing the five head pointers,
plus defined head fields, Lua owner metadata and installed script results.
Uninitialized head payloads and native temporary-stack preimages are not compared.
The fixture executes copied constructor instructions through6AB7E2, followed by
the original epilogue. Its copied script prefix captures and closes the actual
stack temporary before the original epilogue; the original early guard bypasses
that capture. Those are explicit fragment returns, never parser success stubs.

Installed fundamentals, ControlPresets, KeyboardSetup and Inputs bytes are hashed.
The fixture sees five raw heads, two distinct interpreters, three script reads,
four KeyboardSetup categories, exact nested path/override order and a replayed
guard that performs no new work. Streams are backed by explicitly supplied
installed bytes with no overrides; this does not prove native VFS routing.
One source-only failure at the KeyboardSetup file open verifies temporary close,
persistent-state retention, string release and retention of the started flag.
Constructor allocation failure and original FH3 unwinding were not executed.

Strict MSVC Win32 compilation, both existing CTests and eight disk/live seed
checks pass. The report pins the final combined source and archive. No permanent
tests, subagents, game launches, input/device polling or gameplay claims are added.
