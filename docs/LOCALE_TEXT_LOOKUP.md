# Locale text resolution and live Lua substitutions

The reconstruction now appends actual localized UTF-16 text through
`LocaleTextResolver`, using `LocaleTables::find_00a9ec70`. `LocaleLuaContext`
binds substitutions to a borrowed, live Lua 5.1.1 state. The caller must select
the current owner corresponding to native global `00e1ae90`, virtual `+14h`.
This packet does not create or guess that application context.

```cpp
bsp::LocaleLuaContext context(*current_lua_state);
bsp::LocaleTextResolver text(tables, context);
std::u16string caption = text.resolve("^menu.caption|.!");
```

The tables, context and Lua state must remain alive during resolution. Names
are descriptive hypotheses, not recovered symbols. This is a new C++ interface,
not a drop-in native object or allocator ABI.

## Native control flow

| Address | Original ABI | Reconstructed behavior |
| --- | --- | --- |
| `00a9fad0` | ECX manager; stack output wide string, key string; RET 8 | Split on `|` and append every piece in order, including the final empty piece. Empty whole input returns immediately. |
| `00a9f4b0` | ECX manager; stack output wide string, key string; RET 8 | Optional `^`, then optional `.`, lookup/fallback, substitutions, then uppercase only the newly appended range. |
| `00a9eba0` | ECX manager; stack 16-bit character; RET 4, AX result | First matching character in manager `+4024h` maps through `+4034h` at the same index; otherwise CRT `towupper`. |
| `00b68d70` | ECX root LuaObject; stack output object, path, success byte pointer; RET 0Ch | Dot-path traversal, string lookup before numeric fallback. The adapter implements the globals-root use reached by the locale resolver. |
| `00b692c0` | ECX root LuaObject; stack output NativeString, path; RET 8 | `invalid` for unsuccessful traversal, otherwise the converted final value. |
| `00b69130` | ECX LuaObject; stack output NativeString; RET 4 | Convert the final value without Lua `tostring` or `__tostring`. |
| `00b68550` | ECX LuaObject; stack output NativeString; RET 4 | Type name: unbound kind 0 becomes `none`; tracked kind 2 uses `lua_type`; other owner pseudo-objects become `table`. |
| `00b68460` | ECX output NativeString, EDX type; RET | Fixed type-name switch, including `none` and `unknown`. |
| `00b66a60` | ECX LuaObject; RET, AL Boolean | Require Lua number; spill to float32, truncate with CVTTSS2SI, ordered x87 comparison against that signed int32. |

`00a9f4f3..00a9f52f` proves prefix order. `.` bypasses both localization and
substitutions. Missing localized keys append the stripped key widened byte by
byte, rather than an empty string: `00a9f599..00a9f5d0`. Existing output remains
untouched by scanning and uppercase conversion; its length is captured at
`00a9f4e8`, passed to the first marker search at `00a9f5f0`, and restored as the
uppercase start at `00a9fa5e`.

For localized text and missing-key fallback, paired `#...#` markers mean:

- `##` inserts a newline (literal `00ce4390`, call at `00a9f7fd`).
- `#:#` inserts one literal `#` (comparison `00a9f713`, literal `00cf828c`).
- Other contents become a Lua path; its resulting string is recursively
  resolved as another key list (`00a9f766..00a9f7f6`).

Replacement constructs prefix + resolved replacement + suffix. The next scan
starts after the replacement (`00a9f973..00a9f977`), so a literal `#` inserted by
`#:#` or a nested dot-literal is not scanned again. Uppercase conversion happens
after this entire pass.

The native string conversion helpers are deliberately not Unicode codecs.
`004c5e60` zero-extends each unsigned byte into a UTF-16 code unit. `00436630`
copies each low byte back; an early zero low byte terminates copying while the
allocated native header can still describe a longer string. The port rejects
that ambiguous/uninitialized-tail case rather than inventing trailing bytes.

## Lua path and formatting details

`00b68f25..00b6902e` first looks up the segment as a string. If that result is
nil, `_atol` is used; the binary's `00bf8417` delegates to `_strtol(...,10)`.
Numeric fallback occurs when the result is nonzero or the segment is exactly
`"0"`. Consequently `t.12suffix` may index `t[12]`, while `t.00` does not index
`t[0]`. Both accesses use actual `lua_gettable`, preserving `__index` behavior.

An empty path retains the globals pseudo-object and stringifies as `table`.
Leading or consecutive empty segments fail. A trailing dot is accepted because
the remaining path becomes empty. Missing final values stringify as `nil`;
trying to traverse through a non-table produces `invalid`.

Native globals have LuaObject kind 1. Therefore numeric fallback at the root
uses `00b67720`'s pseudoindex addition, unlike its kind-2 table-index path:

- Root `0` refers to `LUA_GLOBALSINDEX` (`-10002`).
- Root `2` refers to `LUA_REGISTRYINDEX` (`-10000`).
- Root `1` refers to `LUA_ENVIRONINDEX` (`-10001`). The adapter captures the
  outer active function environment before entering its protected trampoline.
  Vendored Lua 5.1.1 `lapi.c:index2adr` dereferences the current closure for this
  index; outside an active call, no closure exists. That case throws here.
- Other offsets depend on native temporary-stack/upvalue object layout and are
  explicitly rejected in this adapter. They are not reinterpreted as globals
  table keys.

Value conversion preserves nil, booleans, strings (native `strlen`, so an
embedded NUL truncates), table/function/thread/full-userdata type names, and the
special light-userdata spelling `ptr`. Numbers are narrowed through x87 to
float32. An integral float in signed-int32 range uses `%d`; other numbers use
`%f`, with the float promoted to double (`00b691c1`, `00b691f9`, `0043bdf0`).
Current Win32 CRT formatting and `towupper` are bound directly. Historical
VS2005 decimal-tie rounding, nonfinite spelling, locale tables and CRT internal
state have not been proven byte-identical to the current CRT.

The previous shared `gui_lua_is_integer_00b66a60` implementation compared a
double against `floor(double)`. The assembly `00b66a94..00b66ab4` instead executes
FSTP float, CVTTSS2SI, FILD, FUCOMIP, LAHF and the parity test. The shared host and
materialized-reader paths now use one `gui_lua_is_integer_number_00b66a60`
implementation. Win32 retains that instruction sequence and caller FP state.
For example, `1.0000000001` rounds to float `1` with ordinary nearest rounding
and qualifies, while positive `2147483648` and NaN do not. A portable fallback
preserves numerical results for its default float environment, without claiming
native FP flags or traps.

## Evidence, validation and limits

Analysis used `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`,
through verified BSP CLI queries and exports. Resolver assembly was checked
through RET 8 at `00a9facc` and `00a9fc1f`; Lua helpers through their actual
returns. Flow inspection found only two non-call alignment gaps in `00a9f4b0`
and no gaps in `00b68d70` or `00b692c0`. No Ghidra mutation was made by the worker;
name/comment updates and forced export refresh belong to the integrator.

`scripts/build.ps1` passed MSVC Win32 Release after seed verification. All eight
existing native seeds matched the installed executable, and both existing CTest
tests passed. The single ignored fixture `local/locale_text_lookup_fixture.cpp`
also passed using real Lua 5.1.1 and `LocaleTables` loaded from synthetic `.lan`
and `.lanx` bytes. It covers recursive replacements, separator/prefix behavior,
first sidecar match, byte narrowing, live Lua type/path/metatable behavior,
registry and outer environment pseudoindices, stack restoration on errors,
float32/int32 limits, and the caller's upward x87 rounding mode.

The fixture is not native differential or game validation. Current-context
selection, GUI presentation, locale file mounting/loading and sidecar lifetime
are external integration responsibilities. The sibling file-loader packet owns
`locale_tables.hpp/.cpp`.

Host guards accept null-free strings up to INT32_MAX code units. A missing paired
`#` is rejected: natively close `-1` wraps the suffix start to zero and repeatedly
reinserts the entire original output (`00a9f849..00a9f859`), so no terminating
result exists. Cyclic recursive substitutions retain their native lack of a
cycle limit. Lua lookup errors are caught with `lua_pcall` and reported as C++
exceptions, rather than invoking the native panic path. Prior successful output
appends are retained if a later resolution fails.
