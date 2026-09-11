# GUI Lua reader String and Handle fidelity

This packet corrects two cases of the existing 00BD63B0 reconstruction. The
current Ghidra name `BSP_LuaReader_StoreValue` is retained. Original ABI is
`__fastcall`: ECX points to the looked-up LuaObject, EDX points to the
eight-byte `{field_type,destination}` pair, and the function ends with `RET`.
The C++ projections are not native ABI entry points.

## String conversion

The native String case obtains a char pointer through 00B662B0 at 00BD63D7.
The helper passes a null length-output argument to Lua's 00A67810. Then
00BD63E5..00BD63EE explicitly scan for the first NUL, 00BD63F9 resizes the
destination NativeString to that count, and 00BD640E copies that count.
A null conversion produces length zero. This is a NUL-terminated conversion,
not a length-preserving copy of a Lua byte string.

The live-reader `gui_lua_store_ref_00bd63b0` already used `std::string(char*)`
and matched this rule. Its behavior is unchanged. The evaluated-value
`gui_lua_store_value_00bd63b0` previously copied the whole materialized
`std::string`, preserving bytes beyond embedded NUL. Its String case now
assigns from the converted text's `c_str()`, matching the native scan. The
incorrect source comment claiming use of Lua's reported length is corrected.

## Live table identity

The Handle case first checks whether the Lua object is an integral number;
that path still calls the existing numeric resolver. If it is a table,
00BD64E0 loads ECX from ESI, the original looked-up LuaObject, before calling
the callback at 0109CED8. 00BD64E9 stores that callback's integer result.
There is no conversion to a materialized `GuiTable` and no null argument.

The old live path called `resolve_by_table(nullptr)`, losing the object that
the callback must inspect. The new required method is:

```cpp
std::int32_t resolve_by_live_table(GuiLuaHost& host, GuiLuaRef object);
```

The token is copied by value and belongs to the supplied host. A resolver can
inspect its table using the existing `type_of`, `get_by_name`, `get_by_index`
and conversion operations. The token is borrowed for this call: the reader
releases the looked-up object on its normal leave, so the resolver must not
release it or retain it after the call. The separate evaluated-table
`resolve_by_table(const GuiTable*)` contract remains unchanged. No default
implementation bridges the two methods or manufactures a null table.

00BD6830 performs the lookup at 00BD6891, calls 00BD63B0 at 00BD68A6 and
destroys the looked-up LuaObject at 00BD68B7. This establishes the lifetime
of the forwarded object. No new Lua-host or concrete runtime API is needed.
The actual gameplay callback behind 0109CED8 remains an external service;
preserving its argument does not reconstruct that callback's behavior.

## Validation and boundaries

`reports/gui_lua_reader_identity.json` records the Win32 build, existing
CTest and one ignored real-Lua fixture. The fixture uses `GuiLua51Host`,
the repository's Lua 5.1.1, and `GuiLuaReader`. It reads two distinct tables
with markers 17 and 29 through the new resolver, checks normal token release,
and compares both String paths for `ab` + NUL + `cd`, expecting `ab`.
The evaluated-table resolver must not be called by these live reads.
Release Win32 `scripts/build.ps1`, the existing `reconstructed_math` CTest
(1/1), and the ignored fixture all passed for this packet.

Fixture recipe from this worktree after `scripts/build.ps1`:

```powershell
cmd /c local\run_gui_lua_reader_identity_fixture.cmd
```

The script compiles `local/gui_lua_reader_identity_fixture.cpp` with MSVC
Win32 `/W4 /WX /EHsc /std:c++17 /MD` and links the already-built canonical
core and Lua libraries. Neither fixture file is tracked; no repository test
or CMake registration is added. The existing test only covers the unrelated
ParsedFloat/default asymmetry in `tests/math_tests.cpp`.

This packet does not change reader exception cleanup, callback registration,
gameplay handle resolution or native SEH. Normal-path token lifetime is
checked; throwing/reentrant resolver behavior and game/archive validation
remain outside its scope. Ghidra was only read; no project mutation or save
was performed.
