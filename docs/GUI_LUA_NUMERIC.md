# GUI Lua numeric conversion

Packet `orch4_lua_reader_numeric_f`, 2026-09-11. The names are hypotheses,
not recovered symbols. This corrects numeric paths inside the existing reader;
it does not mark the complete reader or evaluated-table projection faithful.

## Native evidence

| Address | Native operation and ABI |
| --- | --- |
| `00BD63B0` | Fastcall: ECX looked-up LuaObject, EDX field pair; RET. Int calls `00B66290` at `00BD6432`. Handle calls the integer predicate at `00BD649E`, then a separate integer accessor at `00BD64A9`, then the required resolver at `0109CED4`. |
| `00B66290` | Thiscall LuaObject, integer in EAX. Lua `tonumber` at `00B6629B`; FSTP float32 at `00B662A0`; FLD at `00B662A3`; tail JMP `00BF7420` at `00B662A9`. |
| `00BF7420` | Existing CRT identity retained. Reads live `0109EEA4` after the float reload. Nonzero: FSTP double then CVTTSD2SI. Zero: existing `00BF7456` x87 conversion kernel, returning low32 of its signed64 result. No saturation or zero fallback. |
| `00B66270` | Thiscall LuaObject, result in ST0. Lua `tonumber` at `00B6627B`; FSTP float32 at `00B66280`; FLD at `00B66283`; RET. |
| `00B66A60` | Thiscall LuaObject, bool in AL. Kind must be 2 and Lua type number. `00B66A94..AE`: FSTP/FLD float32, CVTTSS2SI, FILD the integer, FXCH, FUCOMIP, pop; `LAHF; TEST AH,44h; JP` rejects unequal/unordered. A nonintegral double rounded to an integral float qualifies. This predicate is independent of the CRT mode. |
| `00BD5790` | Fastcall ECX output, EDX parent, stack key kind/value; RET8. FloatIndex uses CVTTSS2SI directly at `00BD5816`, then integer lookup at `00BD5824`. It never uses the CRT mode. |
| `00BD5F50` | Thiscall reader, stack output array; RET4. String test first, then integer predicate `00BD6037` and separate accessor `00BD6044`; otherwise number test and float accessor `00BD6067`. |

The predicate's existing Win32 instruction sequence already matched this
evidence and was retained. Its earlier shorthand description as an integral
double test was insufficient. The numeric Handle path and key enumeration must
perform both Lua reads; the first is not a cached value for the second.

## Reconstructed behavior and binding

`lua_number_float32_00b66270(double)` and
`lua_number_integer_00b66290(double, const bool&)` share the recovered tails
between live and evaluated values. The live wrappers still invoke the actual
host `to_number`. `lua_float_index_00bd5790(float)` contains the separate
CVTTSS2SI instruction. The Int path no longer tests the host type and fabricates
zero before calling Lua. Live Float and aggregate element access likewise call
the actual numeric accessor.

The required mode reference aliases the live decision corresponding to
`0109EEA4`; the helper reads it after the Lua callback and float32 spill/reload.
There is no default mode. Owners must keep the referenced bool alive for every
reader or retained service that uses it. Required bindings are:

- `GuiLuaReader(host, root, mode)`; its two store entry points and
  `load_gui_screen_table_00ac6600` also require the reference.
- `PcProfileIoContext::crt_sse2_conversion`, used by profile/settings readers.
- `GuiLayoutHost::crt_sse2_conversion() const`, returning `const bool&`; base
  property binding and page Priority forward that alias.
- `GuiIconRuntimeServices::crt_sse2_conversion` and the evaluated Icon authored
  page parser; the required reference is the first service member.
- Evaluated FrameBox property reading takes the reference as its final
  argument. The live-reader overload uses its reader's existing binding.

All evaluated-store calls require explicit binding, including float-only
callers. The sole existing math check was adapted to pass its explicit fixture
mode; no permanent test cases were added. Existing source registration covers
both modified implementation files, so no CMake change was needed.

## Validation

`./scripts/build.ps1` passed MSVC Win32 Release and CTest `reconstructed_math`
(1/1). One ignored fixture, `local/gui_lua_numeric_check.cpp`, linked the built
`bsp_core.lib` and stock `bsp_lua511.lib`, compiled with `/W4 /WX /fp:strict`, and
passed. Run `cmd /d /c local\run_gui_lua_numeric_check.cmd`; an optional first
argument chooses another checkout's headers and built libraries.

It uses real Lua values with a forwarding host that can change the supplied
mode during `to_number`. Checked cases include 16777217 becoming 16777216;
1.99999999 becoming 2; numeric strings; 2^32 yielding SSE2 INT_MIN versus x87
low32 zero; the late mode read; distinct Handle predicate/accessor calls;
evaluated numeric parity; integer/float key classification; and infinite
FloatIndex still selecting INT_MIN while CRT mode selects x87. It masks FP
exceptions, uses round-to-nearest, and restores its FP environment.

This is source/assembly and fixture validation, not an original-process
differential, original ABI compatibility, or game validation. The x87 kernel
is reused from `native_render_batch_keys.cpp`, not reimplemented.

## Remaining boundaries and Ghidra state

The evaluated `lua_tonumber_value` still uses `strtod` and accepts a parsed
prefix; it is not a complete replacement for the actual Lua lexer. Use the
live `GuiLua51Host` path for actual Lua conversion behavior. Materialized
tables, aggregate callback order, native LuaObject tracking, iteration cursor
storage, the host's 250-key cap, and exception cleanup retain their prior
projection limits. This packet does not claim to close those behaviors.

Verified target: `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe` through
the verified CLI. Ghidra was read-only. Fresh flow reports found zero listing
gaps for `00BD63B0` (319 instructions), `00BD5790` (62), `00BD5F50` (120), and
`00B66A60` (41). No missing starts or flow repairs are requested. Final inclusive
instructions: `00BD6826 RET` length1, `00BD5859 RET8` length3,
`00BD60FB RET4` length3, `00B66AC2 RET` length1. The parent may append the
corrected numeric evidence to the existing Ghidra names/comments under lock.
