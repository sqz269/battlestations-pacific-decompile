# Ship navigation tuning and avoidance timing inputs

Addresses: `0083B5E0` (partial settings producer), `00837DE0` (selected tuning
block), `006FE6A0` (surface copy witness), `00854230` (Submarine copy witness),
`00B66270`/`00B66290` (existing native Number/Integer conversions).

The mission Lua host now reads the full selected `ShipLeafTuning` and the six
LandAvoidance timing floats from its current interpreter. The existing scalar
depth reader keeps its public API and behavior. All three readers leave output
unchanged on failure; the borrowed readers restore the caller's Lua stack.

## API and ownership

`GameShipNavigationInput` contains the existing `ShipLeafTuning tuning`,
`settings_block_offset`, and static `class_key`. The member entry is
`GameMissionLuaHost::read_ship_navigation_input(type_id, session_mode, out, error)`.
Its borrowed-state counterpart, `read_ship_navigation_input_lua`, also accepts
the existing `const bool& crt_sse2_conversion` input. The member uses the same
represented CRT capability selection as the scalar reader.

`read_ship_layer_timing_input(std::array<float,6>& out, std::string& error)` and
`read_ship_layer_timing_input_lua(lua_State&, out, error)` return, in order:
move minimum, move maximum, ship minimum, ship maximum, travel minimum, travel
maximum. The member rejects an unavailable mission state. Neither reader runs
scripts or creates a replacement settings singleton.

## Native source and coverage

| Native routine/span | Coverage | Established behavior |
| --- | --- | --- |
| `0083B5E0..00842951` | partial projection; only the following two spans | The complete loader, its interpreter construction, other settings, and unselected class/session values remain outside this binding |
| `0083BFD8..0083C26E` | complete six-value timing fragment analyzed | `ShipGlobals.LandAvoidance` pairs, six bare Number calls, stores `settings+1F4h..208h` |
| `00841B7B..008425DD` | producer loop analyzed; selected values projected | EBP selects the two native 70h records; EDI is `settings+80h` or `settings+F0h`; integer conversions write the class-key values |
| `00837DE0..00837DFA` | complete existing selector reused | Session dword at `game+1FE4h` equal to zero selects `80h`; every nonzero value selects `F0h`; ECX=settings, EDX unused, EAX=selected pointer, no stack arguments, plain `RET` |
| `006FE6A0..006FE6EC` | complete listing checked; existing surface leaf contract reused | One selected block `+0Ch` dword copied into all four class `+560h..56Ch` slots; block `+08h` copied to `+570h`; `RET 4` |
| `008545BC..00854628` in `00854230` | partial copy tail; other Submarine fields remain external | Four distinct array sources `60h/64h/68h/6Ch`; scalar source `5Ch` |

The Type and variant mapping is shared with the scalar parser: existing
`vehicle_class_kind_row`, `kShipLeafClasses`, `kShipLeafTuningSources`, and the
existing scalar-offset-to-key map. Cruiser's `HeavyCruiser` and LandingShip's
`BigLandingShip` use the existing exact Boolean-or-false conversion. Numeric
Lua row ids are not class ids. Unsupported types remain errors.

The full reader converts element 1 through `native_lua_integer_00b66290` into
`tuning.scalar`. For surface leaves it reads element 2 once and replicates that
converted dword four times. Submarine uses `kSubmarineTuningArraySources`; the
index formula `1 + (source_offset - scalar_source)/4` selects elements 2 through
5. `array_source` and `scalar_source` retain the existing source-table offsets.
`MiniSub` remains an unselected native settings key; no new vehicle class or
variant is inferred from it.

The scalar API shares this parser with array reading disabled. An erroring or
missing array element therefore cannot change a scalar-only read. The protected
frame's automatic objects are trivially destructible; `lua_pcall` catches lookup
and metamethod errors. The wrapper restores its saved stack top and assigns the
completed result only after success. Native object slots stay within the
existing owner's capacities, with at most six live tuning slots or five timing
slots and one reference each. The state is borrowed, so it is not closed.

## Timing fields and conversion

The timing reader selects the six rows from `ship_ai_settings_keys`; it uses
their existing path, index, and getter kind. No second key mapping or installed
value fallback is introduced.

| Settings offset | LandAvoidance key/index | Number call | Float store | Installed value |
| --- | --- | --- | --- | --- |
| `1F4h` | `CheckMovePosZoneTime[1]` | `0083C032` | `0083C037` | 2.5 |
| `1F8h` | `CheckMovePosZoneTime[2]` | `0083C09B` | `0083C0A0` | 3 |
| `1FCh` | `CheckShipPosZoneTime[1]` | `0083C104` | `0083C109` | 2.5 |
| `200h` | `CheckShipPosZoneTime[2]` | `0083C16D` | `0083C172` | 3 |
| `204h` | `CheckTravelZoneTime[1]` | `0083C1D6` | `0083C1DB` | 3 |
| `208h` | `CheckTravelZoneTime[2]` | `0083C23F` | `0083C244` | 4 |

`00B66270` calls native `lua_tonumber` at `00B6627B`, spills to float at
`00B66280`, reloads at `00B66283`, and returns. Integer conversion has the same
float32 spill/reload at `00B662A0/A3`, followed by the existing CRT selector at
`00B662A9`. Numeric strings are accepted. A final nil/nonnumeric value converts
to zero; an invalid parent lookup raises the normal protected Lua error. No
clamping, timing sorting, alternate default, or signed-bit replacement is added.
Named and indexed lookups keep their native `RET 8` two-argument contracts;
the numeric getters take the object in ECX and no stack arguments.

ESI provenance was checked by filtering the whole loader listing: the receiver
is copied at `0083B604`, with no other assignment before the epilogue pop at
`0084294B`. The depth loop resets EBP at `00841B7B`, selects EDI at
`00841B81/8E`, increments at `008425D2`, and loops at `008425D8`. All native calls
in the reviewed producer spans and the reused selector/variant evidence are
listed in the report; `verify_report_calls` passed all 207 rows.

## Verification and limits

The existing ignored depth fixture was extended to compile an exact excerpt of
the shared implementation. It uses stock Lua 5.1.1 and the existing wrapper
core, loading the installed ShipGlobals/VehicleClass inputs into a fresh fixture
interpreter with the preserved installed conversion-function excerpts. It does
not execute the original machine-code loader. Production readers only access
the current mission interpreter and never execute those fixture scripts.

The fixture passed 320 full reads: all 160 installed ship rows and all ten
Type/variant keys in session modes 0 and -1. It rejected all 473 nonship rows
with unchanged output. The actual timing values are shown above. Submarine's
Single array is `[11,26,46,86]` with scalar 0, and Multi is `[11,46,46,86]` with
scalar 1. The source report retains all selected Type ids grouped by key/mode.

The compact boundary fixture passed stateful surface-index read count,
scalar-only independence from an erroring array, distinct Submarine slots,
positive and negative nonzero modes, both CRT conversion selections,
numeric-string float32 spilling, signed integer bits, final nil/nonnumeric
conversion, and late/non-string/parent lookup errors with output and stack
preservation. Win32 Release and both existing tests passed. No tracked test or
new test framework was added.

This is a partial C++ input binding, not a native ABI replacement, descriptor
allocator, full 76Ch singleton, or reproduction of the whole loader's lookup
schedule across every class and both session tables. Original SEH/GC timing and
new mission/gameplay behavior are not claimed. Runtime integration belongs to
the primary agent. Prior reported gaps outside these fragments remain listed
in the report; no new missing functions, false-free gaps, or Ghidra repairs were
introduced. All Ghidra queries used verified project/program wrappers, read-only.
