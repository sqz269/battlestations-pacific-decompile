# Retained settings initialization and language selection

Addresses: 00CD2D80, 008D7710, 008D56C0. Read-only dependencies: 0041E870,
008D41C0, 008D41F0, 008D4520, 008D4820, 008D45D0, 00A3F530, 00A3F5D0,
00A40DF0, 0073D410. Names are reconstruction hypotheses, not recovered symbols.

## Initialization before the options reader

The CRT initializer pointer at `00ce3054` targets `00cd2d80..00cd2d95` (22 bytes,
six instructions). It loads ECX with `00f88980`, calls constructor `008d7710`,
then registers destructor thunk `00cdeec0` through `00bf6ff5` (`atexit`). The
thunk calls `008d78d0`; destruction and native registration are not reimplemented
by this packet. The parent defined the previously missing initializer function
under the write lock; its audit is `reports/settings_initial_state_function_definitions.json`.
The worker made no Ghidra mutations.

`008d7710..008d78cd` is ECX=this, EAX=this, plain RET, with a temporary SEH frame.
Its inline reset stores establish these represented native values:

| Fields | Initial values | Store evidence |
| --- | --- | --- |
| Language index +04 | 0 | 008d7733 |
| Gameplay +08/+09/+0c/+10/+4a/+7c/+80/+b2 | true, false, 2, true, true, true, 0.0, true | 008d77e0..008d77f9; 008d7845 |
| Audio +20/+24/+28/+2c/+30 | 0.5, true, 0.5, 0.5, 0.5 | 008d7736; 008d77b9..008d77d3 |
| Controls +40/+41/+42/+43/+44 | false, true, false, false, true | 008d787b..008d7888 |
| Resolution +14/+18/+78 | 640, 480, 0 | 008d7801..008d780b |
| +1c/+1d/+1e: NoLOD, HiResShadow, fullscreen | false, false, true | 008d7824..008d7827; 008d7861 |
| +54/+58/+5c/+60/+64/+68: object detail, AA, AA index, VSync, gamma, texture detail | 2, 0, 0, true, 0.0, 2 | 008d7812..008d782a |
| +6c/+70/+74: reflection, unknown, clouds | true, 0, true | 008d783a..008d7841 |
| +84/+85/+86/+88/+8c/+8d/+90: shadows, foliage, shader, shader flag, blur, old film | true, false, true, 1, byte 1, true, 1 | 008d782d..008d7871 |
| +34 unknown float | bits 3f32b8c3 = 0.6981317400932312f | 00ce7d20; 008d7865 |
| +94/+95/+b0/+b1: firewall, hardware reported, compatibility, safe area | false, false, false, false | 008d7759..008d775f; 008d77a1..008d77a7 |
| Downloaded-content +98/+9c/+a0; ClanText +b4/+b8 | empty; empty | 008d7769..008d7775; 008d7791..008d77b4 |

This corrects two existing initial/default facts: ShowSafeArea is explicitly
false, and the previous `0.698161f` literal encoded `3f32baae`, not `3f32b8c3`.
The +8c projection now uses `uint8_t`, matching its byte store and avoiding the
incorrect implication that the adjacent +8d byte belongs to that integer.

Native storage outside the current `GameSettingsBlock` projection is explicit:
vtable +00=`00d15d58`; +38 byte=0; +3c dword=1; +49/+4b/+4c bytes=0;
+50 dword=2; a second vector header +a4/+a8/+ac is zero. This packet does not
invent element types for that vector or meanings for those scalars. All other
native bytes are unwritten by the constructor. The static object's original
image storage is zero before construction; generic construction on existing
memory does not justify clearing its unspecified bytes.

The callable `initialize_game_settings_008d7710` updates the real retained
`GameSettingsBlock`, reusing the four reset store projections. It leaves the
non-native `options_file.language` and `unknown_tokens` metadata alone. Owned
string/vector members clear normally instead of leaking a previous allocation
as native constructor misuse would. This is a C++ state projection, not native
storage layout or a drop-in constructor.

## Why the cold initializer skips the selected-user tail

At 008d7897, a null global `00f8abe8` skips the whole Xenon branch. Otherwise
the branch requires manager state 2 and a selected user before calling
`008d45d0` at 008d78b5. That routine reads four XUser profile settings and can
change control bytes and input state; its platform operation is not simulated.
The generic initializer takes both conditions explicitly and returns true if
the import still needs to occur.

For static startup the null pointer is supported by current binary evidence:

- `00f8abe8` contains four zero image bytes.
- Its reference list has only two writes: `00a3f586` installs the manager in
  `00a3f530`; `00a3f639` clears it in the corresponding destructor.
- The sole known caller of `00a3f530` is `00a40df0`. Its application call site
  is `0073dc7c`, after settings loading at `0073daa5`.
- A raw scan for the `00f8abe8` operand across the static initializer region
  `00cc893b..00ce2000` returned no matches.

`initialize_static_game_settings_00cd2d80` therefore invokes the generic
initializer with explicit false/false. This is justified for the cold retained
object; callers constructing later settings objects must use actual conditions.

## Exact language selection

`008d56c0..008d5796` is native `void __thiscall(const char*)`, RET 4 at 008d578b.
The input becomes a temporary NativeString through `0041e870`, whose constructor
uses C-string length. The loop scans `00f88974` in stride 0x20, bounded by
`00f88978`. It first compares the input length with entry+0 at 008d570b.
Equal zero lengths match directly; equal nonzero lengths call `__stricmp`
at 008d572a on input text and entry+4. The first match stores its index at
008d5792 and exits. No match, including an empty table, writes zero at 008d5753.

The previous exact `std::string` equality and unchanged-index failure behavior
were incorrect. The repaired helper models the C-string input length, stored
catalog length, empty-string case, current CRT case-insensitive comparison,
first duplicate, and index-zero fallback. Its bool result is a projection-only
match indicator; native returns void. It does not update the projected language
token: consumers must derive that token from the actual selected table entry.
Locale behavior outside the installed ASCII language names follows the host CRT;
cross-CRT locale equivalence has not been established.

## Validation scope

Win32 Release build and the existing two CTests passed. A single ignored
fixture passed checks of dirty retained-state initialization, preservation of non-native
metadata, the exact float bits, the explicit profile-tail result, and the
language case/duplicate/miss/C-string boundaries. This is build and fixture
evidence; no original-constructor differential execution, native ABI substitution,
or game-runtime validation is claimed. Machine-readable results and uncertainty
are in `reports/settings_initial_state.json`.
