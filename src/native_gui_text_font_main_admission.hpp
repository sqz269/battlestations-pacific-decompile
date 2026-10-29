#pragma once
#include "bsp/native_gui_text_primary_prefix_lifetime.hpp"
#include "bsp/native_gui_text_main_section_lifetime_storage.hpp"
#include "bsp/native_gui_text_font_lifetime_storage.hpp"
#include <stdexcept>

namespace bsp::gui_text_font_main_detail {
using Main = NativeGuiTextMainSectionLifetimeStorageFrame;
using Font = NativeGuiTextFontLifetimeStorageFrame;
using Prefix = NativeGuiTextPrimaryPrefixLifetimeContext;

inline bool selected(const Prefix& p) noexcept {
    return p.operation.kind() == NativeGuiTextPrimaryPrefixOperation::Kind::geometry_main_section_font;
}
struct Peers final { const Main& main; Font& font; };
// Active graph coordination only. Never use from a sibling's post predicate or
// destructor: either helper can already have ended during metadata teardown.
inline Peers peers(const Prefix& p) {
    const auto* const g = p.operation.geometry();
    const auto* const m = p.operation.main_section();
    auto* const f = p.operation.font_lifetime();
    const auto& c = p.text.factory_context;
    if (!selected(p) || !g || !m || !f || g->prefix != &p ||
        &m->prepared_context().geometry != g || &f->prepared_context().geometry != g ||
        &m->prepared_context().reference_frame != &c.reference_frame ||
        &f->prepared_context().reference_frame != &c.reference_frame ||
        c.publication.retirement.font_lifetime != f ||
        c.publication.font_shader_frame != &f->prepared_context().shader ||
        c.publication.retirement.font_shader_frame != &f->prepared_context().shader)
        throw std::logic_error("Font/Main admission needs its exact fixed peers, Geometry and canonical122/shader tuple");
    return {*m, *f};
}
inline Font::Stage font_stage(Main::Stage stage) {
    switch (stage) {
    case Main::Stage::bound: return Font::Stage::bound;
    case Main::Stage::before_scalar: return Font::Stage::before_scalar;
    case Main::Stage::scalar_preentry: return Font::Stage::scalar_preentry;
    case Main::Stage::raw118_preentry: return Font::Stage::raw118_preentry;
    case Main::Stage::raw118_body: return Font::Stage::raw118_body;
    case Main::Stage::base: return Font::Stage::base;
    }
    throw std::logic_error("Font/Main admission cannot infer an unknown lifetime stage");
}
inline void require_live(void* actual, const Prefix& p, Main::Stage stage,
    const NativeGuiTextRetirementAcquired* destruction = nullptr) {
    const auto q = peers(p);
    q.font.require_live(actual, font_stage(stage), destruction);
    q.main.require_live(actual, stage, destruction);
}
} // namespace bsp::gui_text_font_main_detail
