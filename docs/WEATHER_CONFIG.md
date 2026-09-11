# Daylight, weather and camera configuration

Packet `orch4_weather_config_f`, branch `agent/orch4-weather-config-20260911f`,
2026-09-11. The verified live target is `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. Worker access was read-only. Names are descriptive
hypotheses, not recovered symbols. Source: `src/weather_config.cpp`.

## Owner and initialization

`004DC6A0` allocates7Ch, conditionally constructs `00445B10`, publishes the
result at game+21C4, then unconditionally calls `00444D20`. That outer sequence
belongs to the parent packet. This packet provides the canonical `WeatherConfig`
projection and its concrete constructor/loader, without a whole-loader callback.

The owner combines daylight visibility/reconnaissance settings and parameters
for camera oscillation. The strings name the latter `CameraHimbilimbi`,
`Frequencies`, `BaseAmplitudes`, `ViewModeAmpMultipliers` and
`WeatherAmpMultipliers`. This packet does not reconstruct the oscillation
consumer or infer physical units for the three amplitude/frequency lanes.

| Native offset | Proven content |
| --- | --- |
|00|vtable00CE4934|
|04|DayTime.StartDayTimeDefault, normalized after all reads|
|08,0C|MorningStart, MorningEnd|
|10,14|EveningStart, EveningEnd|
|18|NightVisibility|
|1C,20|FireMaxModifier, SmokeMaxModifier|
|24|FireSmokeModifierDuration|
|28|HighNoonMaxVisionRange|
|2C,30|owned NativeString, assigned `default` by loader|
|34|current weather modifier, loader writes float1|
|38..40|WeatherReconModifiers map owner/head/count|
|44,48,4C|BaseAmplitudes lanes1,2,3|
|50,54,58|Frequencies lanes1,2,3|
|5C..64|ViewModeAmpMultipliers map|
|68..70|WeatherAmpMultipliers map|
|74,78|loader writes float1; current amplitude-multiplier names provisional|

`00445B10` writes the vtable, clears the2C NativeString, and initializes maps
in38,5C,68 order. It does not initialize any of the19 scalar words. The C++
shell therefore requires `WeatherConfigAllocationWords`: ten words for04..28,
then34, three for44..4C, three for50..58, then74 and78. The shell copies these
bits without floating-point evaluation. Optional standard maps defer their
initialization until the recovered constructor reaches them. The constructor
is valid only for a fresh shell; it is not a reset/destructor operation.

Each native map initializes a1Ch node via `00443E20`, changes its sentinel
flag19 to1, sets all three links to itself, and clears the map count. The node
allocator itself sets links0/4/8 to zero, color18=1 and sentinel19=0; it leaves
key/value/padding untouched. Successful allocation is required: its apparent
null guards do not make later null-derived stores safe. These are native
library node contracts, represented by engaged standard maps rather than a
new implementation of STL tree allocation.

## Complete loader ordering

`00444D20` is ECX=owner, RET, complete reachable assembly through
`0044578C` (one-byte RET). Its local Lua owner is constructed, opened with
mask1 (base only), and runs `Scripts\datatables\Globals.lua`, followed by the
real override sequence. The native path string is released before table reads.
The implementation reuses `PcStorageLuaOwner`, `LuaScriptRuntime`,
`GuiLua51Host` and `lua_object_number_00b66270`; no script values are fabricated.

The loader obtains globals, resolves `Globals`, releases the pseudo-object,
then obtains `DayTime`. It reads the ten named values in the table's offset
order above. Each value is converted through the actual Lua numeric accessor,
stored to its destination float, and released before the next lookup. Missing
numeric fields consequently get Lua's actual `lua_tonumber` behavior; there
is no added fallback or type filter.

Next it reads **Globals.WeatherReconModifiers**, iterating actual Lua entries.
For each string key it constructs a NativeString, gets/inserts the actual
destination map value, converts the Lua value, stores it, frees the key string,
and advances the iterator. Iterator advance releases value then key first.
The temporary key pointer is captured before map insertion; recon also captures
its length there, whereas both later amplitude loops reread length for release.

It then assigns the literal `default` to2C without preserving old bytes and
writes34=1 from00D7A24C. Equal string length skips allocation and terminator
rewriting, including the native stale/null-pointer case; the final copy uses
the current header length and pointer.

It reacquires globals and `Globals.CameraHimbilimbi`, releasing the two lookup
temporaries before reading `Frequencies`. Assignment replaces the current Lua
table reference. For each vector it performs **lookups3,2,1**, conversions1,2,3,
stores all three float lanes, then releases references1,2,3. Frequencies are
stored first at50..58, then BaseAmplitudes at44..4C. This ordering remains
observable through Lua `__index`; it is not replaced with an ascending loop.

`ViewModeAmpMultipliers` and `WeatherAmpMultipliers` replace the current Lua
table in that order and fill maps5C and68. All three maps are updated without
clearing them, so omitted prior entries survive a repeated load. Finally the
loader writes74=1 and78=1, normalizes day time, releases camera, iteration
value/key, current table, DayTime and Globals, and closes the Lua owner.

## Map values and numerical boundaries

`00444BE0` is string-to-float lookup-or-insert, returning a borrowed float at
native node+14. `00443D60` finds lower-bound and the reverse comparison uses
the existing `00443D00` length-gated CRT `_stricmp`. Existing entries retain
identity and value. A miss copies a temporary key and explicitly writes
**+0.0f** with XORPS/MOVSS before hinted insertion. Node construction00444150
deep-copies that key and copies the float with FLD/FSTP. Thus two key copies
and post-publication temporary cleanup are preserved. This map has a proven
zero initial value, unlike the panel palette's uninitialized scratch payload.

The implementation uses `std::map<NativeString,float,PanelSequenceNameLess>`.
Native1Ch node links0/4/8, key+C, value+14, color18 and sentinel19 are documented
for evidence; no binary-layout equivalence is claimed. Generic hinted insertion,
node allocation/linking and rebalance helpers remain analyzed library
dependencies. `004442A0` performs linking/rebalance after a length-limit check;
its old `STL_xlen_throw` tag describes only one branch.

Day normalization preserves the native COMISS/x87 loops: repeatedly add24.0
while day<0, then repeatedly subtract24.0 while day>24. Each iteration spills
to float. Zero and24 are both accepted; negative zero and quiet NaN are kept.
It is not `fmod`: positive multiples of24 normalize to24, negative multiples
can normalize to0. Infinities and sufficiently large floats whose value cannot
change by24 never terminate natively and are not given a new stopping rule.
No integer accessor or CRT SSE2 conversion-mode input is needed in this loader.

## Integration, ownership and limits

Allocate storage for the host shell, placement-construct
`WeatherConfig(allocation_words)`, call `construct_weather_config_00445b10`,
publish that same owner, then call:

```cpp
load_weather_config_00444d20(config, environment, scripts, strings);
```

`release_weather_config_storage` is an explicit host convenience that releases
all owned map-key strings and the current-weather string before the shell is
discarded. It is not claimed as the native destructor. Standard containers
own generic library storage; no synthetic map or Lua lookup results are used.

Validated behavior assumes valid fresh/engaged containers and successful
allocations. Native node/proxy/allocator layouts, checked-iterator diagnostics,
allocator failure and MSVC SEH unwinding, and callbacks invalidating active
map nodes or strings are outside the semantic projection. Native scalar bits
are preserved from supplied allocation inputs, but opaque allocator/padding
bytes have no C++ projection. Signaling-NaN trap delivery and floating-point
status equivalence are not claimed.

For native iteration equivalence these three naming tables use string keys.
The existing registry-backed host preserves an independent Lua cursor: after
numeric-key `to_string`, it retains the original numeric iteration key, while
the native stack-object next can observe the converted string. This existing
Lua-host boundary is documented rather than silently changed. Embedded-NUL
string keys are truncated by the native strlen/copy path, and CRT comparison
keeps its locale/C-string behavior. Missing intermediate tables are not
fabricated; real Lua indexing/iteration errors remain errors.

## Evidence and validation

The loader has665 listed instructions and666 instructions in a raw contiguous
decode. The difference is unreachable alignment bytes004450DD..004450DF,
`8D 49 00` (LEA ECX,[ECX]), skipped by JMP004450DB->004450E0. This is not a
CALL_RETURN truncation. All leased helper listings have zero gaps. No function
definition or flow repair is required, and no Ghidra write was made.

Standalone MSVC Win32 Release build passed; existing reconstructed_math test
passed1/1. One ignored fixture uses the actual Lua interpreter, VFS/runtime
contract and map/string implementation. It passed script/override and table
lookup order,3/2/1 vector lookup order, scratch preservation, zero default and
borrowed identity, retained entries on reload, numeric-string float narrowing,
negative/positive wrapping, signed-zero/quiet-NaN boundaries, close order and
balanced storage. An initial negative-zero test literal was interned with Lua's
earlier positive zero; using numeric string `'-0.0'` supplied the intended input.
No source correction was needed for that fixture-input issue.

Fixture/recipe: `local/weather_config_fixture.cpp` and
`local/run_weather_config_fixture.ps1 [-Repository <tree>]`. Detailed ABI,
scope, addresses and gaps are in `reports/weather_config.json`. There is no
native differential, drop-in binary ABI or game-runtime validation claim.
