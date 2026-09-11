#pragma once
// bsp_game.exe milestone 2d: the GUI Text widget's text path, as process bindings.
//
// Addresses: 00abb630 (the Text property reader, vtable +18h), 00ab8c30 (font by name
// through the registry lookup 00ac3570), 00abaed0 (set localised source), 00a9fad0 /
// 00a9f4b0 (the localisation resolver), 004c5e60 (the non-localised widening),
// 00aba8d0 (the geometry update that selects a builder and uppercases the text),
// 00a9ec30 (the in-place uppercase pass 00aba8d0 runs for an uppercase-only font),
// 00a9eba0 (the per-code-unit uppercase), 00ab9fd0 (the single-line layout builder),
// 00aba270 (the wrapped layout builder), 00aba860 (the wrapped vertical offset),
// 00ab98f0 (the glyph quad writer) and 00ad4480 (glyph selection).
// Recorded but not implemented: 00ab8ce0 (the font shader), 00ab8400 (the glyph vertex
// and index buffers) and 00ab8530 (the draw sections), which are the native material and
// submission half; the sprite bridge draws the quads instead.
//
// Nothing in this file is a reconstruction of native code. GameTextHost is an integration
// binding: it satisfies bsp::GuiTextHost with the already reconstructed font, layout,
// geometry and localisation routines, and records every native call site it reaches, as
// concrete or as the explicit unimplemented policy in GameHostLog.
//
// Evidence: docs/GUI_TEXT_WIDGET.md, docs/FONT_SINGLE_LINE_IMPLEMENTATION.md,
// docs/FONT_WRAPPED_LAYOUT.md, docs/FONT_GEOMETRY.md, docs/LOCALE_TEXT_LOOKUP.md,
// docs/FONT_MATERIAL_DRAW.md, docs/GAME_EXECUTABLE.md.

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <d3d9.h>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace bsp {
struct GuiLayoutWidget;
class LocaleTables;
}  // namespace bsp

namespace bsp::game {

class GameHostLog;
class GameFontHost;

// One glyph quad, in the same normalised 0..1 GUI space the sprite bridge already draws
// widget rectangles in. `left`/`top`/`right`/`bottom` are what 00ab98f0 wrote (its own
// /960 and /720 normalisation, plus the wrapped vertical offset of 00aba860) with the
// widget's resolved origin added; the four UVs are the glyph's own atlas edges.
struct GameTextQuad {
    float left{0.0f};
    float top{0.0f};
    float right{0.0f};
    float bottom{0.0f};
    float u1{0.0f};
    float v1{0.0f};
    float u2{0.0f};
    float v2{0.0f};
};

// What the text path produced for one Text widget.
struct GameTextRun {
    std::string page;
    std::string key;
    std::string font_name;      // the authored "Font"
    std::string source;         // the authored "DefaultText", the localisation key
    std::string resolved_ascii; // the resolved text, ASCII-folded for the log only
    bool font_resolved{false};
    bool localised{false};      // the source went through 00a9fad0 rather than 004c5e60
    bool locale_hit{false};     // the key was in the loaded table, not the fallback
    bool uppercased{false};     // the font is uppercase_only, so 00a9ec30 ran
    bool wrapped{false};        // 00aba8d0 chose 00aba270 rather than 00ab9fd0
    std::uint32_t container_width{0};
    std::size_t glyphs{0};
    std::size_t lines{0};
    float measured_width{0.0f};
    float measured_height{0.0f};
    float vertical_offset{0.0f};
    float color[4]{1.0f, 1.0f, 1.0f, 1.0f};
    IDirect3DTexture9* sheet{nullptr};  // the font's own glyph sheet, borrowed
    std::vector<GameTextQuad> quads;
    std::string error;  // set when the run produced nothing
};

struct GameTextSummary {
    std::size_t text_widgets{0};    // Text widgets the bridge offered
    std::size_t runs_built{0};      // runs that produced at least one quad
    std::size_t runs_empty{0};      // no authored DefaultText, so no native text either
    std::size_t runs_failed{0};     // a font, a layout or a quad write refused
    std::size_t glyph_quads{0};
    std::size_t locale_hits{0};
    std::size_t locale_misses{0};   // 00a9f4b0's stripped-key fallback
    // The image value of the mutable global 00e12fd4 the quad writer and the wrapped
    // builder multiply every y by. Reported so a run states which value it used.
    float vertical_scale{0.0f};
};

// The text half of the sprite bridge. One instance for the run; it borrows the font
// registry (for the glyph sheets and the decoded DAT metrics) and the locale tables.
class GameTextHost {
public:
    GameTextHost(GameHostLog& log, GameFontHost& fonts, LocaleTables& locale);
    ~GameTextHost();
    GameTextHost(const GameTextHost&) = delete;
    GameTextHost& operator=(const GameTextHost&) = delete;

    // Runs the whole native text path for one Text widget of a loaded page:
    // 00abb630 over the authored table, which resolves the font (00ab8c30), the string
    // (00abaed0 through 00a9fad0), the uppercase pass (00a9ec30) and the layout builder
    // 00aba8d0 selects, then writes one quad per placement with 00ab98f0.
    //
    // `origin_x` / `origin_y` is the widget's resolved position with its own pivot
    // subtracted, in normalised GUI units: the same corner the bridge draws an Icon from.
    // Returns false when nothing is drawable; `run.error` says why.
    bool build_run(const std::string& page, const GuiLayoutWidget& widget,
        float origin_x, float origin_y, GameTextRun& run);

    const GameTextSummary& summary() const noexcept;

    // Defined in src/game_hosts_text.cpp. Public only so the GuiTextHost binding in
    // that file can name it; nothing outside it uses the type.
    struct Impl;

private:
    std::unique_ptr<Impl> impl_;
};

}  // namespace bsp::game
