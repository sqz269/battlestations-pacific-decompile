# Raw GUI property prefix, CC10

Addresses: `00AAA710`, boundary-only handler `00CB7571`.

| Routine | Coverage | Native ABI |
|---|---|---|
| `read_native_gui_widget_property_prefix_00aaa710` | **partial projection**: `[AAA710,AAABD6)`, 1,222 bytes | ECX actual widget; stack actual visitor; whole function RET4 |
| `CB7571` | Unimplemented FH3 boundary only, `[CB7571,CB757B)` | EAX=DEDDEC, JMP BF6B43 |

The last included instruction is `AAABD3 FSTP [EBP]`, length3. The whole
Ghidra body ends at AAAECE inclusive; `[AAABD6,AAAECE]` is excluded. That tail
contains widget virtual+5C/+34, later properties, children and the native
epilogue. This source does not compose transforms or invoke the tail. Its
names describe observed behavior, not recovered symbols. The logical
GuiWidgetOwner/GuiLuaReader implementations are not used.

| Property / key address | Call | Tag | Actual destination | Fallback |
|---|---|---|---|---|
| Pos / CF168C | AAA795 | 5 | widget+C | local three +0 floats |
| Size / CFF278 | AAA7EE | 6 | widget+20 | local two +0 floats |
| Pivot / D5C228 | AAA847 | 6 | widget+18 | same two locals reset to +0 |
| Scale / CE60AC | AAA8A5 | 6 | widget+28 | two copies of one CURRENT D7A24C load |
| Rotate / D5C220 | AAA8F0 | 2 | widget+48 | inline +0 bits |
| Color / CE93F8 | AAA973 | 8 | widget+50 | actual F8BCE0 white storage |
| LowColor / D5C214 | AAA9FB | 8 | widget+A4 | actual F8BCCC black storage |
| HighColor / D5C208 | AAAA7A | 8 | widget+B4 | captured F8BCE0 pointer |
| BlendFactor / D5C1FC | AAAAC4 | 2 | widget+C4 | inline +0 bits |
| WideScreenAlign / D5C1EC | AAAB12 | 0 | actual local eight-byte header | empty C string CE3A0C |

The old logical descriptor mapping for the last color/alignment keys was
incorrect for this body. Literal addresses and raw tags above come from the
listing and current PE bytes. Each key is tag0. This packet does not rename
unread producer types.

Each call freshly loads `*visitor` then `profile+0C`, after constructing the
three actual by-value pairs. The final metadata store follows the target
load. `NativeGuiWidgetPropertyDispatch` is a **required** external binding:
it receives that exact target, current actual visitor and the actual pair
addresses. No target is hardcoded or silently replaced. For BD68D0, bind
`read_native_lua_field_or_default_00bd68d0` using the same actual reader,
NativeLuaStateStorage/ref slots, raw pool and initialized reader/value scratch.
Other current targets require real implementations from the caller. The
normal CE44FC profile has BD68D0 at +0C, but Lua callbacks may replace it.

Binding addresses/literal pointers stay fixed during a call; the borrowed
cells and actual pointed-to storage remain mutable. White uses a low-byte
bit0 test of F8BCF0, one current D7A24C load, DWORD OR1 preserving other bits,
then four same-bit stores. Black uses F8BCDC, OR1, three +0 stores, **then**
loads current D7A24C for alpha. After LowColor returns, the white guard is
tested again and can initialize again. HighColor uses the earlier captured
white pointer. No atomic initialization or caching is introduced.

Let S be native ESP after the four register saves. Caller-owned initialized
`NativeGuiWidgetPropertyScratch::bytes[820h]` represents `[S-18h,S+808h)`.

| Offset in explicit scratch | Native storage |
|---|---|
| 0 / 8 / 10h | key / field / fallback pairs, eight bytes each |
| 18h..27h | native saved-register gap, excluded from source bookkeeping |
| 28h | first of 7F8h native local bytes; transient pair pointer metadata |
| 2Ch / 30h | reused vec2 defaults, then actual string length/data header |
| 34h | late position-X float spill |
| 40h / 44h / 48h | Pos default vector |

Each local and argument-pair address is stable through callbacks. The source
preserves each staging store order, including metadata-before-field-tag for
the three vec2 reads and pointer-before-tag for HighColor. Unwritten local
bytes retain their preimage. Native later CRT pushes reuse argument space,
and native CALL/save/EH bookkeeping occupies other bytes; the new explicit
C++ interface excludes those ABI writes. It preserves the call-time pairs,
not native stack return addresses. At the middle frontier EBP still denotes
widget+C, EDI the widget, ESI the visitor and EBX is zero. The local header is
stale after release; local+C is the captured current X. The middle fragment
must restage its transient pairs and must not claim whole-stack identity.

After the tenth read, compare current nonnull header.data to `Left` with the
linked CRT `_stricmp`. Failure/null selects raw `equal_native_string_header_00425850`
on the current actual header and `Right`. Write mode1/2/0 at widget+E0.
Normal cleanup captures current header.data before current length+1 and
resolves the current raw pool. It leaves the header unchanged. Mode is
reloaded after cleanup; then CURRENT widget+C is copied to the local spill
and widget+8. Mode0 skips the platform publication completely. Other modes
load CURRENT `[109CF04]` and test actual platform+D. Modes1/2 use native x87
FLD, FSUB/FADD of the current double D5C118, then FSTP widget+C. Its PE bits
are `3FC2222240000000`; no rounded decimal or SSE arithmetic substitutes.

| Call | Native provider | Exact boundary |
|---|---|---|
| ten rows above | current visitor virtual+0C | ECX visitor, six stack DWORDs, observed BD68D0 RET18h |
| AAAB22 | BF7FBF `__stricmp` | cdecl(data,Left), caller ADD ESP8; linked current CRT remains external |
| AAAB48 | 425850 actual-header insensitive equality | ECX header, stack Right, AL boolean, RET4 |
| AAAB81 | 419CC0 raw pool getter | no args, RET; previously pushed release args remain |
| AAAB88 | BD1510 raw sized return | ECX current pool, data/length+1/unused1, RET0Ch |
| CB7576 | BF6B43 FH3 | raw undefined handler tail jump, no worker mutation |

`destroy_native_string_header_0041dd20(header, raw_context)` supplies the
same getter/return sequence. Header semantics and all callees were inspected;
no Lua, CRT or STL body is ported. Native EH state becomes0 before AAAB12 and
-1 before cleanup. CB7571 has no Ghidra function: inclusive endpoint CB757A;
last instruction CB7576 JMP is five bytes. Descriptor DEDDEC has magic
19930522, maxstate3, map DEDDD4: `(-1,CB7550),(-1,CB755B),(1,CB7566)`.
Prefix state0's cleanup funclet CB7550..CB755A computes `[EBP-800h]` and
jumps 41DD20. The source does not supply FH3/SEH or exceptional cleanup.

Validation: strict `scripts/build.ps1` MSVC Win32 Release and all three CTests
pass. The 1,222 prefix bytes, ten handler bytes and eight constant bytes
match live Ghidra and PE; `reports/native_gui_widget_property_prefix_cc10.json`
records hashes, exact direct and indirect call rows and limits. Workers did
not alter Ghidra. One ignored original/source probe runs eight pairs against
the copied original prefix, with only data relocations, four call bridges
and an explicit normal epilogue replacing the excluded continuation. It uses
genuine Lua/actual reader/ref slots and the canonical raw string pool.

The probe checks mixed present/default fields, a Pos lookup profile change,
LowColor resetting white's guard/changing one, alignment case folding,
Left/Right/unknown/empty strings, a platform pointer published by the final
callback, null platform skipped for mode0, two x87 rounding modes, late X,
all widget/color bytes, call-time pair relationships, callback order and all
7F8 allocated local bytes (two pointer words normalized). Actual Lua refs and
the raw pool manager drain. It does not validate callees independently,
all possible aliases, native ABI/FH3 or in-game behavior. No application or
installed game integration is made. A helper launcher argument-count defect
was corrected before the successful run; all of that helper's processes were
stopped before rebuilding. Prior packet archives remain byte-identical.
