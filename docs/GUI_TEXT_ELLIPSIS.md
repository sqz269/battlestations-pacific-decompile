# Text ellipsis and command-label preparation

Addresses: `00ab8f00`, `00abb000`. Descriptive names are hypotheses; the new
C++ interfaces do not reproduce the native Text allocation, string pools or ABI.

The command bar passes `-1.0f` to `00abb000` at both `0054bcc3` and `0054beac`.
That value selects the current label width at `widget+20h`. The earlier
[context ownership audit](FONT_CONTEXT_OWNERSHIP.md) incorrectly called the
sentinel zero: live bytes at `00d7a260` are `00 00 80 BF`. The ordered
`UCOMISS`/`LAHF`/`TEST AH,44h`/`JP` sequence at `00abb065..00abb070` makes
zero an explicit zero target and leaves a NaN unchanged. The new interface
rejects NaN at its finite arithmetic boundary after the conversion callback;
it does not silently replace NaN with the widget width.

| Routine | Original ABI and boundary | Coverage |
|---|---|---|
| `00ab8f00` | ECX Text; destination/source UTF16 wrapper pointers and float width; EAX destination; `RET 0Ch` at `00ab9226` and `00ab923d`; body ends `00ab923f` | Complete normal-path scalar/string behavior in the supported domain. Native pool/SEH representation excluded. |
| `00abb000` | ECX Text; narrow wrapper, float width and low-byte localization flag; `RET 0Ch` at `00abb1c2`, body ends `00abb1c4` | Partial: preparation through `00abb0c9` or `00abb148`; geometry/color/temporary cleanup at `00abb0ca..00abb115` and `00abb149..00abb1c4` remain external. Source-equal exit is represented by `nullopt`. |

`ellipsize_gui_text_00ab8f00` first converts `width * double(960)` to signed32
under temporary x87 truncation and retains the low16. It copies the source,
then, if the live font descriptor requests uppercase, applies the existing
`locale_uppercase_00a9eba0` for each code unit. This is the mapping used by
the consumed `00a9ec30` body; it is not the host locale's unrelated uppercase
operation. Actual assembly shows that helper consumes ECX locale manager and
one wrapper pointer, with `RET 4` at `00a9ec60`.

Each glyph comes from the existing `select_font_glyph_00ad4480` and contributes
its unsigned advance at glyph `+12h` multiplied by Text `+1d8h`. Each addition
is independently truncated to signed32 and narrowed to low16. If measured width
exceeds the unsigned target, the routine reserves three scaled dot advances,
using the same conversion/narrowing, removes suffix code units while too wide,
then appends `...`. Dot reservation may wrap to a large unsigned target and
retain the whole original string before appending dots. No clamping, line
breaking, Unicode shaping or kerning is introduced. Inline MSVC x86 assembly
retains the extended x87 value through `FISTP`; the calculation is not replaced
by float or double accumulation. The constants at `00cec380` and `00d7a2b0`
were read as doubles 960 and 3.

The caller supplies the decoded `FontData` belonging to `widget.font` and the
actual locale tables/runtime; those font identities must stay stable during
callbacks. These existing types and their producers own
the meanings of font `+48h`, glyph `+12h`, Text `+108h/+1d8h` and size `+20h`;
this module adds no record offsets or independent font/widget state. It reads
the font flag after copying the source and the scale for each arithmetic step.
Inputs and transformed output must be null-free and fit signed32 lengths.
Arithmetic scalars must be finite when consumed, and pre-truncation results
must lie in `[-2147483648,2147483648)`. Guards throw instead of emulating
malformed pointers, overflow/indefinite integers, allocation failure or traps.
The x87 control word is restored after every accepted integer conversion;
floating exception flags and trap timing are not equivalence claims.

`prepare_gui_text_ellipsis_00abb000_fragment` reuses `GuiTextWidget` and
`GuiTextHost`. It checks the existing source cache, stores a changed source,
captures the width fallback, calls localization or unsigned-byte widening,
then returns the concrete ellipsis output. Width is captured before the
conversion callback; a reentrant size change does not alter that captured
argument. Source cache remains changed if a later operation throws, as it is
stored before native conversion. The existing cache predicate retains its
documented ASCII case-fold projection for narrow localization keys.

An engaged empty result is a real update. The future Text owner must submit
every engaged result to actual `00aba8d0`, then call virtual50 with the current
color, even if geometry compares equal. The preparation function leaves
`widget.text`, measured width, geometry, color and owners untouched. Calling
the scalar builders directly here would omit shadow creation, glyph-child
release and native geometry equality behavior. No such substitute is added.
The geometry update repeats the native font uppercase pass; locale mappings
must not be assumed idempotent merely because ellipsis already transformed it.

All 58 live `00abb000` call sites in 29 containing functions were checked for
argument setup. The report records each site, containing function, source,
width and flag. Three sites pass localization false (`00561fc2`, `006349cd`,
`00634f26`); the remaining 55 pass true. Six sites use explicit widths:
`005dcc77/005e5a8c` load `00cf2318` (`89 88 08 3E`),
`005e8d3e/005e8efb` use their stack width, and `005ec14c/005ec1a5` use
screen `+dch`; the remaining 52 use `-1`. Every invocation has three arguments
by the callee's `RET 0Ch`. Both direct `00ab8f00` callers are the two conversion
branches in `00abb000`, also with three arguments and `RET 0Ch`.
Register-derived flags were checked against the whole containing listings:
`00561eb0` keeps EBX zero, `005e51d0` assigns EBP=1 before the relevant site,
and `005ebdd0` keeps EBX=1. The shared call at `00601c90` has four source
branches, all loading `-1` and pushing flag1.

| Native site(s) | Callee/operation | Concrete binding or remaining boundary |
|---|---|---|
| `00abb02a` | `00435c40`, cached narrow equality | Existing `source_text_changed_00abaed0` projection; width/flag unchanged on cache hit |
| `00abb0ac` | `00a9fad0`, localization key-list resolution | Existing `GuiTextHost::resolve_localised` requirement |
| `00abb127` | `004c5e60`, unsigned-byte widening | Existing `GuiTextHost::widen_source` requirement |
| `00abb0c5/00abb144` | `00ab8f00`, ellipsis | Concrete implementation in this module |
| `00ab8f8b` | `00a9ec30`, per-code-unit locale uppercase | Existing locale mapper applied in native order |
| `00ab8fb7/00ab9022/00ab90a2` | `00ad4480`, selected glyph / dot / removed suffix | Existing decoded `FontData` selector |
| `00abb0d2/00abb151` | `00aba8d0`, content geometry update | Excluded; actual Text content/shadow/child/resource owner required |
| `00abb1af` | Text virtual50 | Excluded; actual color/material owner required after geometry |

The canonical `GuiTypeDispatchFactory` in this checkout still lacks Text3.
Neither this module nor the existing command owner is a renderable command
bar. Binding Text state to the same `GuiWidgetOwner`, real font/material/scene
resources, content update, shadow attachment and child lifetime remains
required. Command lookup/visibility/background sizing can use existing GUI
owners when that Text composition is ready.

Verification: existing `bsp.gpr` and `/battlestationspacific.exe` were verified
by every live wrapper batch. Analysis was read-only; no Ghidra annotation,
prototype, name, saved project or installed game file changed. The new C++
translation unit compiled with MSVC Win32 `/O2 /W4 /fp:strict` without warnings.
The exact-site report verifier checked 70 call rows with zero failures; the
virtual50 row is explicitly indirect. The integrator owns the combined CMake
build. No executable in this worktree reaches the new
fragment yet, so no game-frame or visual result is claimed. No new tests added.
