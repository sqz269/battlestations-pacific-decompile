# Native Lua packed-color reader

Addresses: 00B67F10, 00B67720, 00B66DE0, 00A67770, 0087D7B0

## Reconstructed behavior

`read_native_lua_color_00b67f10` covers the complete 386-byte body at
00B67F10..00B68091. The original takes its Lua object in ECX, a four-byte
destination on the stack, returns that destination in EAX, and uses RET4.
`BSP_LuaObject_ReadPackedBgra` is a descriptive hypothesis, not a recovered symbol.

The helper reads Lua indices 1, 2, 3, 4 in order and writes destination bytes
2, 1, 0, 3. Each lookup uses the existing actual 14h Lua object and 4C8h state
implementation. It calls `lua_tonumber` directly, so there is no intermediate
float32 rounding. After each byte write it releases that lookup's temporary
before starting the next lookup. Non-kind2 inputs retain B67720's stack-relative
index behavior. No default alpha, numeric-kind gate, clamping, or nil fallback
is added to the reader; Lua's own numeric conversion supplies zero for values
that cannot be converted.

The assembly explicitly saves the x87 control word, sets its rounding bits to
truncate, performs FISTP to a signed DWORD, writes the low byte, and restores
the saved word. Source retains these operations with Win32 inline assembly.
With masked exceptions, invalid/overflow conversions produce the integer
indefinite value 80000000h and therefore a zero channel; values inside the
signed32 range retain their low byte. A float32 intermediate would incorrectly
turn 255.99999999999997 into a zero channel, whereas this body produces 255.

The original establishes no exception handler. Source adds no protected Lua
boundary or automatic cleanup on failure. The copied native and source bodies
both retain already-written channels when a later `__index` raises an error.

## Validation

- Verified ten live-Ghidra spans against the original PE: 1,224 bytes total.
- Copied B67F10, B67720 and B66DE0 into the focused probe: 910 bytes and all
  19 direct CALL relocations. All seven external boundaries call actual linked
  stock Lua 5.1.1 through original-register-ABI adapters. No Lua values or
  tracking behavior are mocked. Library code is reused, not reconstructed.
- 444 original/source comparisons passed: 300 observation bytes per case,
  133,200 bytes per result stream. Inputs include finite fractions, negative
  values, byte wrapping, signed32 boundaries, NaN/infinity, numeric/non-numeric
  strings, nil and booleans; table and non-kind2 indexing; and real `__index`
  callbacks and nonlocal Lua errors at the first and third lookups.
- All four rounding modes at x87 24-, 53- and 64-bit precision passed with
  exceptions masked. Checks cover byte guards, returned pointer, publication
  observed by subsequent callbacks, control-word restoration, x87 exception
  status/TOP, stack heights, live reference counts and the surviving input
  reference. Unused/stale tracking pointer bytes are not compared.
- Strict MSVC Win32 build and all three existing CTests passed. No permanent
  test framework or CTest cases were added.

Exact inputs, native bytes/listings, hashes, call-site rows, toolchain, logs,
probe source and immutable archive receipts are recorded in
`reports/native_lua_color_r115.json`. The focused probe is ignored local evidence.

## Integration boundary and follow-up

This closes one concrete dependency of Globals.lua loading. The full
0087D7B0 loader and its binding to the captured 2E8h configuration owner remain
open. R115 does not replace that loader with the old Minimap-only projection.
Its other prerequisites include the checked native string/float vectors and
008DBE90 objective-sound reference assignment. No application path reaches
this new helper yet, and no runtime or gameplay test is claimed.

The interface is rebuilt Win32 C++, not a drop-in binary replacement. Original
stack placement, unmasked floating-point traps, Lua allocation failure and
whole-loader native SEH behavior remain unvalidated. Existing Lua object
dependency source is unchanged; it is compared here against its original
copied bodies in the reader's exercised paths.
