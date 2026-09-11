# Dialog configuration loading

Addresses: `0044FA30`, `0044DA70`, `0044ADA0`, `00B66FA0`, `00B66270`, `00B66290`.

`load_dialog_config_0044fa30` now runs the dialog script through the existing
Lua owner, script runtime and VFS contracts, then reads its live Lua tables into
the same character map and actual palette tree consumed by panel publication
and the voice sequence. The panel owner remains supplied by its caller; this
packet does not construct that owner or its palette sentinel.

The temporary interpreter opens mask `1` (base only, with the existing owner
bootstrap), runs `Scripts/datatables/DialogGlobals.lua` and its overrides with
obfuscation disabled, and releases the pooled path before reading globals.
Character entries are visited with `lua_next`. Their key is converted through
`lua_tolstring` and the native NUL scan; `0044F220` supplies a writable mapped
integer. The reader's unconditional `picture` Int conversion writes zero for
nil through Lua's own conversion. Existing map entries are updated in place;
the loader does not clear either map.

After character iteration it reads `DialogDefaultPauseTime` into owner `+30`.
It then replaces the held table with `DialogColors`, converts each key with
`00B66290`, and captures the actual `0044EC00` value address once. Each of four
indexed Lua values is converted, divided by the double `255.0` at `00CE4B48`,
and stored before releasing that value or looking up the next. There is no
clamping. Missing palette entries retain the existing required input of four
native uninitialized stack words until these lane writes replace them. Finally
the routine writes state `+34 = 0`, releases iterator/table references, and
closes the temporary interpreter.

The listing disproved two premature returns in the original pseudocode. Calls
to `_free` at `0044FC1A` and `0044FD19` had `CALL_RETURN` overrides. The locked
repair removed only those overrides and decoded their three-byte `ADD ESP,4`
continuations. The saved body now contains 368 instructions through `0044FFE5`
`RET`, with zero call gaps. Original bytes and before/after evidence are in
[the repair report](../reports/dialog_config_flow_repair.json).

Both numeric accessors first spill Lua's double result to **float32**:

- `00B66270`: 10 instructions, ending
  `00B66287 RET`; the returned x87 value has already been rounded to float32.
- `00B66290`: 10 instructions, ending `00B662A9 JMP 00BF7420`. The same spill
  precedes the CRT conversion. Current `0109EEA4` chooses SSE2 signed32
  conversion or the low32 result of the existing complete x87 `00BF7456`
  kernel. The new API requires a live reference to this mode and rereads it
  after the Lua call. It adds no saturating cast or zero fallback.

The existing generic GUI reader's `narrow_to_int` still converts the double
directly and substitutes zero outside its interval. This loader uses the
recovered numeric accessor directly for its single Int field, so it does not
inherit that mismatch. Migrating all generic reader call sites to an explicit
CRT-mode binding remains follow-up work; no general reader parity is claimed.

`0044DA70` is a LuaObject vector copy constructor (77 instructions, `RET4` at
`0044DB44`); `0044ADA0` is its uninitialized-copy helper (42 instructions,
`RET10` at `0044AE2D`). Their library work is reused through standard ownership,
not newly ported. `00B66FA0` copies the native 14h reference/tracking fields
(29 instructions, `RET4` at `00B66FF2`). The concrete Lua adapter supplies an
independent registry reference to the same Lua value. The loader preserves the
seed/copy/release sequence with owned references, but does not reproduce the
native vector allocation layout or the 14h tracking arrays.

All descriptive names are hypotheses. Native `0044FA30` takes ECX=this and
plain RET; the new C++ views, explicit dependencies, registry handles and
standard character map are not binary replacements. Existing Lua owner/runtime
error transport raises C++ errors where the game could panic/longjmp; native
SEH, Lua allocation/GC timing and invalid native memory are not validated.

Validation results are recorded in `reports/dialog_config.json`. The focused
fixture uses actual Lua 5.1.1 and an actual palette node, checks override order,
NUL-terminated/case-insensitive character lookup, the float32 integer boundary,
both CRT modes, palette reference identity and unclamped values, metamethod
lane order, and state reset before Lua closes. It does not prove game startup,
screen rendering or gameplay.
