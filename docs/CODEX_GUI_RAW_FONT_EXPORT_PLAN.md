# Genuine raw Font export for AB8CE0

Source plan at `aaf3941f5e21c370d34b94ff79636cdf18e455e9`; shader transport
`d13bd719591f923ef3aba0e9032fb6a6871cdf12` is compiled, uncalled preparation.
This refines `CODEX_GUI_RAW_TEXT_FONT_BORROWER_PLAN.md` for the actual image
export. No source/CMake/metadata/Ghidra mutation, test, build, game or activation.
Analysis target was checked with bounded `bsp.py ghidra proto 00AB8CE0`:
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`.

## What already exists

`GameFontHost` owns `GameNativeFontApplication`, its stable FontRegistryStartup
descriptors, load frames and ONE NativeFontResourceOwners collection. Existing
GUI_TEXT startup selects native images BEFORE loading (`game_hosts.cpp:3015`).
Without that arm, ready typed FontData is NOT an actual Font and cannot be exported
as one. Image-mode readiness includes complete resource loads and native registry.

Actual storage is `NativeFontResources::Image` (`native_font_resources.cpp:84`):

| Native location | Genuine current authority |
| --- | --- |
| Font AC bytes | actual AD55C0 construction; Image layout is statically checked |
| +04/+08 | native18h glyph-tree head/count; **NOT a reference count** |
| +0C/+10 | actual8B pooled Font-name header/data |
| +14, +18, +1C | DAT-populated height WORD, live scale FLOAT, alpha-scale FLOAT |
| +20/+28/+30/+38/+40 | pooled DAT/alpha/GFX/root/language names |
| +48 | actual uppercase BYTE, not a new type ID |
| +4C/+6C/+8C | actual20h special glyph payloads; ordinary glyphs have native tree nodes |
| +64/+68 | Font's OWNED GFX/alpha actual-image references |

Other glyph texture slots borrow those images; they do not create another Font
owner/retain. GFX/alpha can identify the same image only with both real credits.
They are canonical renderer actual owners, unlike the Font object itself.

Native registry is actual10h D5CA84 header (head08/count0C), circular0Ch nodes
(next00/previous04/Font08), SAME AC allocations. `find_native_font_00ac3570`
traverses actual list order and CURRENT Font-name header; `resolve_image(actual)`
maps exact identity to LIVE custody. Neither synthesizes a descriptor Font.
AD4C30 production already performs genuine renderer64/B319B0 GFX/alpha loads,
VFS DAT population, path/name/tree ownership and completion. No new native
Font constructor, parser, metric/glyph mirror, registry, ID or counter is needed.

## Exact shader requirements; export does not publish Font108

AB8CE0 must receive that SAME real collection in ShaderContext even on cached,
override or height-mismatch paths. Its default arm reads actual configuration
0109CF04->28 via the real renderer publication CELL, ordered CVTSI2SS comparison
against D5C56C(720), and ONLY on match reads CURRENT Text108/Font18 against
live D7A24C. Never force height, scale, override, or use descriptor/copied metrics.
Use existing renderer.borrow_platform_publication_0109cf04(); the owner already
authenticates that cell through sampler_bindings. It is not the loader event adapter.

AB8CE0 does not read glyphs or GFX/alpha itself. Their real custody still survives
all Font users; this export authorizes no layout/glyph/material texture operation.
Font lookup must use actual ready registry/AC3570, with a genuine live raw8B
input header. A small typed native-image view may expose existing image/name0C
addresses after live SAME-owner admission; no manual header, metric copy, legacy
FontDescriptor/FontData cast, or extra Font credit. An accessor in the actual
resource owner can expose its genuine pooled-name header if needed; no fake view.

RawModelRef Text108 is still zero after DEFAULT construction. Existing AB8C30
field storage accepts ONLY NativeGuiTextReference (legacy Model transport).
Its real schedule compares/copies cached1C8 name, AC3570 lookup, publishes borrowed
Font108, stores actual Font1C/default one to1D4, then CURRENT cached1EC release/clear.
A raw overload with independent retained acquisition/header publication evidence
is a SEPARATE dependency. Export must not assign108 manually, cast the reference,
use an unrelated empty legacy factory, or call shader before this admission closes.

## Smallest truthful producer/borrower linkage

Current Host native_fonts()/bind_text_factory and producer retire_fonts() permit
only ONE exact GuiTextRuntimeFactory borrower (`game_hosts_fonts.cpp:127,145`,
`game_native_font_application.cpp:221,226,239`). Do not bypass their sticky export
gap or certify raw use with an empty factory whose allocations are unrelated.

Propose a separate exclusive strong raw-owner arm at BOTH layers, e.g.
`GameFontHost::bind_raw_text_owner(GameNativeRawDefaultTextOwner&)`. Caller first
stores the fresh stable owner, then this combined metadata-only attempt marks
bind-entered BEFORE irreversible producer export. It installs the SAME collection,
ready registry image, producer identity, renderer/VFS domains into that owner
before preparation/Text allocation, then stores the exact owner borrow. No
Font lookup/retain, Native getter, shader or pool acquisition occurs in binding.
Require the producer's SAME already-selected native image lookup association.
Legacy arm remains exact; no simultaneous raw/legacy borrowers or replacement.
Failure latches Host/App/owner retention; no cancellation, rebinding or replay.

Owner's real shader context/nonmoving explicitly prepared Frame must use this
link BEFORE raw122, whose existing immutable font_shader_frame reservation then
covers it. Real raw/typed THREE-cell strings, cache/renderer/program/VFS/compiler/
Lua/sampler graph and real GuiFont/GuiFontBilinear name constructors remain required.
Export/link state alone grants no shader, native field or disposal permission.

Before Font retirement the producer calls a direct method on the SAME retained
typed owner, checking exact resources/image/domain plus terminal retired phase,
no failure/active operation and ALL actual Font-using Text/Model/material/header/
shader invocation custody settled (including scalar/lateflags/pool/disposal).
requires_process_retention()==false while bound is NOT quiescence. No expired
bool callback, old DEFAULT-only settlement, cleared108 or post-dead payload read.
Native AB8250 does not clear/release108; Font is borrowed, so no Font retain/release.

Host already retires raw Text before fonts (`game_hosts.cpp:2064`, GUI app797),
then verifies renderer BEFORE facade closure. Add explicit post-parent-drain
FontHost/App raw-borrow detach AFTER owner.after_native_renderer_drain but BEFORE
owner.reset (GUI app819); keep Font collection/descriptors/VFS/services alive.
Native Font retirement destroys payloads, GFX/current alpha, SIX pooled strings,
nodes/sentinel and AC storage. Do not chase captured Font/name pointers afterward.
Existing frontend/other-borrower retirement guards remain mandatory.

## Remaining ownership blockers and bounded implementation

Text1EC is an owned EFFECT credit from B318B0, not a Font or material object.
Later material binding has its OWN retain; export must not conflate these credits.
Current raw retirement still rejects1EC. A separate packet must admit the exact
d13 reserved Frame/context/destination, completed noncached attempt, known B318B0,
nonnull result matching CURRENT1EC and real cache complete/result/caller_acquired,
with branch-specific name cleanup. Cached fast return and attempted-null grant
no new credit. Replacement/Font-setter release needs explicit retained consumption
evidence; no speculative release, count/profile-only proof or double cleanup.

Smallest disjoint packets: raw Font link/view metadata in Font Host/App and actual
resource HPP/CPP (only if name view needed); exact raw-owner attachment/quiescence/
detach HPP/CPP; separately reviewed raw AB8C30 and cached1EC retirement/reservation.
Root alone coordinates host call sites/CMake. Preserve current DEFAULT, prefix,
geometry/parenting/content gates. Font export can be prepared independently;
actual shader invocation remains BLOCKED by the raw field/credit/lifetime links,
then combined source review/build and a separately preregistered/gated fixture
within the active reconstruction goal.

Root and independent Model/owner Source review accepted this plan. The actual
Font storage and native image providers exist; the link is metadata work rather
than another Font constructor. Raw AB8C30 publication and cached1EC effect-credit
consumption/retirement remain separate execution gates. No raw Font export or
shader caller has been implemented or activated by this document.
