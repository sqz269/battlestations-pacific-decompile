# Ship avoidance settings from Lua

Addresses: 0083B5E0; inspected dependencies00424A10,00424C40,00B66270,
00B66330; inspected consumers009EB660,009EF910.

Packet `orch6_avoid_tuning_j`. Ghidra queries verified
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`; all were read
only. Existing names and key metadata are reused. This packet adds a compiled
five-field reader, not another76Ch settings object or native private Lua owner.

## Contract and exact fields

`bsp::game::read_ship_avoidance_tuning_lua(lua_State&, std::array<float,5>&,
std::string&)` reads already loaded tables from a borrowed Lua state. Output
order follows the five native getter/store sites, as below. The caller should
capture these values **once after the represented settings-script load** and
retain them in its settings owner. Geometry callbacks borrow those stored
floats. Calling this reader again observes current Lua tables and is an explicit
host reload; it is not equivalent to the normal native singleton read path.

Every path is rooted at `ShipGlobals`. The path, one-based index, getter and
fallback come from the existing `ship_ai_settings_keys` metadata. Its
`installed_value` column is evidence only and is never used as a runtime value
or fallback by this reader.

| Output | Native offset | Lua path | Getter call / store | Native fallback | Installed value |
| --- | --- | --- | --- | --- | --- |
|0|+194|ShipAvoidance.CollectTimer[2]|0083B810 /0083B815|none, Number|2|
|1|+1D4|ShipAvoidance.NearbyShip_WayClearCheckTime|0083BC27 /0083BC2C|0.5f, FloatOrDefault|0.25|
|2|+1D8|ShipAvoidance.HitDetector_LastHitDistAddOn|0083BC6D /0083BC72|30.0f, FloatOrDefault|50|
|3|+214|LandAvoidance.YTurnDirDiff[1]|0083C393 /0083C398|1.8f, FloatOrDefault|1.8|
|4|+218|LandAvoidance.YTurnDirDiff[2]|0083C40C /0083C411|2.1f, FloatOrDefault|2.1|

The fallbacks are loaded as singles and pushed withFSTP[ESP]. Their bytes are
`0000003f` at00CE3800, `0000f041` at00CE38C8, `6666e63f` at00CF4848, and
`66660640` at00D0B3C8. The last two values are exactly
1.7999999523162841796875 and2.099999904632568359375. The source uses the existing
float metadata, not decimal double conversions or an installed-value default.
These constants and the selected key strings were checked against live Ghidra
and installed PE bytes; the manifest retains that evidence.

`native_lua_number_00b66270` is already compiled in `native_lua_objects.cpp`.
Native00B66270 calls00A67770 `lua_tonumber` at00B6627B, spills the result to
float32 at00B66280 and reloads it at00B66283; it returnsST0 withRET. Numeric
strings are accepted. Nil, booleans and nonnumeric values reach the bare
tonumber result, normally0; there is no fallback argument or exact-type gate.

`native_lua_number_or_00b66330` is also already compiled. The native body is
00B66330-00B66371: it requires reference kind2, calls `lua_type` at00B66347,
requires exact type3 (`LUA_TNUMBER`), then calls `lua_tonumber` at00B66359 and
spills/reloads float32 at00B6635E/00B66362. Otherwise it loads the caller's
float fallback at00B6636A. Both paths returnST0 withRET4. Numeric strings
therefore take the fallback; using `lua_isnumber` or a bare Number reader would
change behavior. Actual numeric NaN, infinities and signed zero remain numbers;
there is no finite/range check or clamp.

Name and index lookup use the existing native object wrappers, which call
`lua_gettable`. Metamethods participate. A missing leaf may reach a default, but
indexing a missing/non-table parent can raise an error before the getter.
For example, `YTurnDirDiff={}` yields the two fallbacks, while
`YTurnDirDiff=nil` raises on the first indexed lookup. No preflight table test,
raw lookup, invented parent table or fallback after an indexing error is added.

## Loader sequence, lifetime and consumers

The sole direct0083B5E0 caller is00424BFC in the constructor00424A10-00424C15.
The singleton00424C40-00424CFF reads global00F8753C at00424C55. When non-null,
00424C5F branches to00424CF1 and returns that pointer without Lua work. Its
allocation branch allocates76Ch at00424C9F, calls the constructor at00424CB6,
then publishes the returned pointer at00424CC4. There is no automatic refresh
on ordinary singleton access. This establishes the normal path; it is not a
claim that all hypothetical indirect calls or external writes were disproved.

The loader0083B5E0-00842951 establishes ESI=ECX at0083B604. It constructs its
own Lua owner at0083B60E, opens it with41h at0083B625, runs
`Scripts\global\luaMW_init.lua` at0083B672 and
`Scripts\datatables\ShipGlobals.lua` at0083B6E6, selects globals at0083B721 and
`ShipGlobals` at0083B73D, then reads/stores settings. The existing loader report
records closing the private state at00842937. Mission Lua table changes do not
by themselves alter the already stored native settings.

Selected table acquisition is `ShipAvoidance` at0083B76A and `LandAvoidance`
at0083BFEC. CollectTimer is fetched for[2] at0083B7EB, indexed at0083B801 and
converted/stored at0083B810/0083B815. YTurnDirDiff is fetched separately for
each element:0083C361 then0083C37A for[1], and0083C3DA then0083C3F3 for[2].
The new reader preserves these selected lookup and conversion orders. It does
not execute the intervening unrelated settings reads or retain every temporary
object until the whole native loader ends.

Native stores occur immediately after each getter. The new reader writes a
private five-float temporary in that order and publishes the output only after
all reads succeed. Output unchanged on error is the explicit source interface's
contract; it is not a claim that native partial settings stores are rolled back.
The protected C frame starts with an empty argument stack and uses only trivial
automatic objects across possible Lua longjmp. It borrows the actual state,
restores the caller's original stack top on either outcome, and supplies an
error string on failure. It does not run scripts or own/close the state.

| Consumer | Native singleton call | Read | Meaning in existing consumer |
| --- | --- | --- | --- |
|009EB660|009EB681|009EB686,+1D8|additional distance when the sector's earlier blocked state is set|
|009EB660|009EBEDE|009EBEE3,+194|neighbor hit-memory extension base; the consumer adds its own native constant and only increases lifetime|
|009EF910|009EF943|009EF948,+1D4|refresh timer written at brain+374 by009EF956|
|009EF910|009F0076|009F007F,+214 or009F0089,+218|heading-error gate selected by the existing BL state|

The reader returns the **raw stored values**. It does not apply the sector's
extra arithmetic, timer policy or gate choice. The existing metadata also
records+194 consumers009EACA0,009F0D20 and009F1160; those routines are not newly
reconstructed here.

`load_gameplay_tuning_settings(GameplayTuningRowView&, GameplayTuningSettings&)`
already compiles a broader abstract row-view projection, but a live mission
RowView adapter was not found. It is not a private-state native loader. Its
ShipAvoidance source statements also put+1D0 before+1D4/+1D8, whereas native
getter/store order is+1D4, +1D8, then+1D0 at0083BCB3/0083BCB8. The existing
`ship_ai_settings_keys` table has the correct ordering and is reused here.
No unrelated loader source is changed. The six-field
`read_ship_layer_timing_input_lua` provides the existing protected-reader pattern,
but covers+1F4..+208, not these five fields.

## Evidence and limits

Coverage is a **partial projection of0083B5E0**: exactly these five source
lookups/getters, with the existing compiled helpers. The full private
interpreter, script override runner, other76Ch fields, constructor/SEH lifetime,
native incremental error publication and runtime cache binding remain outside
this module. There is no original ABI or gameplay-parity claim.

The ignored probe links the compiled new reader from the Win32Release core
library against this repository's Lua5.1.1. It loads the installed
ShipGlobals.lua and its ScriptOptions dependency, actual unlocks/fundamentals,
and an exact excerpt of bootstrap unit-conversion functions beginning at line434.
It sets the established PC platform flag. This is installed Lua/source evidence,
not original-byte execution or the complete native bootstrap.

The six focused cases cover installed values, bare-number numeric strings with
the float32 spill, exact-type fallback versus numeric strings, missing leaves,
numeric special values, a late missing-parent error, and metamethod lookup order.
The caller's three existing stack values are retained. A second explicit reader
invocation sees modified tables while a previously saved snapshot stays intact.
No new tracked tests or broad runtime host edits are introduced.

Validation passed: Win32Release build, both existing CTest targets,38/38 reported
direct-call rows and6/6 focused reader cases. The installed output bits are
`40000000 3E800000 42480000 3FE66666 40066666`, corresponding to the five table
rows above. The late error preserved the entire output and all three caller
stack entries. This result covers the compiled reader, not native runtime use.

Reproduce with `python local/prepare_avoid_tuning_probe.py`,
`./scripts/build.ps1`, `cmd /c local\build_avoid_tuning_probe.cmd`, and
`local\avoid_tuning_probe.exe`. The preparation requires `pefile`, the configured
installed binary/scripts and the verified Ghidra server. The probe requires the
existing MSVCWin32 toolchain and the freshly built core/Lua/zlib libraries;
its manifest is embedded. Logs, exact input/source hashes and direct-call checks
are recorded in `reports/ship_ai_avoidance_tuning.json`.
