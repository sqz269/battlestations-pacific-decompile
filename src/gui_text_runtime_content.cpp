#include "bsp/gui_text_runtime_content.hpp"
#include "bsp/gui_text_glyph_child_runtime.hpp"
#include <stdexcept>
#include <utility>

namespace bsp {
GuiTextRuntimeContentContinuation::GuiTextRuntimeContentContinuation() = default;
GuiTextRuntimeContentContinuation::~GuiTextRuntimeContentContinuation() = default;
namespace {
class ChildOperation {
public:
    explicit ChildOperation(GuiTextRuntimeContentContinuation& frame) : frame_(&frame) {
        if (frame.child_operation_active)
            throw std::logic_error("Text child continuation is already executing");
        frame.child_operation_active = true;
    }
    ~ChildOperation() { clear(); }
    void clear() noexcept {
        if (frame_) frame_->child_operation_active = false;
        frame_ = nullptr;
    }
private:
    GuiTextRuntimeContentContinuation* frame_;
};
void require_services(GuiTextRuntimeContentServices& services) {
    auto& buffers = services.content.buffers;
    auto& single = services.single_line;
    auto& wrapped = services.wrapped;
    if (&services.nonempty.shader.buffers != &buffers ||
        &single.fonts != &services.nonempty.fonts || &wrapped.fonts != &single.fonts ||
        &single.actual_owners != &buffers.materials.retained_owners ||
        &wrapped.actual_owners != &single.actual_owners ||
        &single.layouts != &services.nonempty.layouts ||
        &single.mapping != &wrapped.mapping ||
        &single.vertical_scale_00e12fd4 != &wrapped.vertical_scale_00e12fd4 ||
        &wrapped.widgets != &buffers.widgets || &wrapped.parenting != &buffers.parenting ||
        &wrapped.children != &services.content.calls.glyph_child_calls())
        throw std::invalid_argument("Text content stages require the same live owner and service domains");
}
void finish(GuiTextRuntimeContentContinuation& frame) {
    finish_gui_text_content_after_geometry_00aba8d0_fragment(
        *frame.content.lifetime, std::move(frame.content), frame.services->nonempty);
}
template<class Builder>
GuiTextGlyphChildCallFrame saved_child_call(Builder& builder, std::uint32_t height) {
    if (!builder.lifetime || !builder.glyph || !builder.native_position)
        throw std::logic_error("Text child continuation lost its original caller arguments");
    return {*builder.lifetime, builder.glyph, builder.native_position.get(),
        builder.vertex_stream, static_cast<std::uint16_t*>(builder.indices),
        builder.native_position->data(), builder.native_position->data(),
        builder.quad_index, height, builder.first_vertex, builder.placement.code_unit};
}
bool complete_current_child(GuiTextRuntimeContentContinuation& frame) {
    if (frame.child_operation_active) return false;
    ChildOperation operation(frame);
    if (!frame.services) throw std::logic_error("Text child driver lost its outer services");
    if (!frame.child_tail) {
        if (!frame.services->glyph_children) return false;
        if (auto* single = std::get_if<GuiTextSingleLineContinuation>(&frame.builder)) {
            frame.child_tail = std::make_unique<GuiTextGlyphChildTailContinuation>(
                saved_child_call(*single, single->height), *frame.services->glyph_children);
        } else if (auto* wrapped = std::get_if<GuiTextWrappedContinuation>(&frame.builder)) {
            if (wrapped->pending != GuiTextWrappedPending::glyph_child) return false;
            frame.child_tail = std::make_unique<GuiTextGlyphChildTailContinuation>(
                saved_child_call(*wrapped, static_cast<std::uint32_t>(wrapped->signed_height)),
                *frame.services->glyph_children);
        } else {
            throw std::logic_error("Text has no suspended glyph builder to complete");
        }
    }
    auto& tail = *frame.child_tail;
    if (tail.status == GuiTextGlyphChildTailStatus::ready)
        begin_gui_text_glyph_child_tail_00ab98f0_fragment(tail);
    if (tail.status == GuiTextGlyphChildTailStatus::pending_content) {
        try {
            auto& implementation = tail.implementation();
            while (implementation.has_pending_operation()) {
                auto* nested = implementation.pending_content();
                if (!nested || !complete_current_child(*nested)) return false;
                try {
                    // Its current native child tail just completed. This owns
                    // the nested builder advance, final color and outer cleanup.
                    implementation.resume_after_glyph_child();
                } catch (const GuiTextRuntimePending&) {
                    // A later glyph suspended. Reload its SAME current frame.
                }
            }
            resume_gui_text_glyph_child_tail_after_content(tail);
        } catch (...) {
            tail.failure = std::current_exception();
            tail.status = GuiTextGlyphChildTailStatus::domain_required;
        }
    }
    return tail.status == GuiTextGlyphChildTailStatus::complete;
}
} // namespace

GuiTextRuntimeContentResult build_gui_text_content_00aba8d0(
    GuiTextLifetime& lifetime, std::u16string_view input,
    GuiTextRuntimeContentServices& services) {
    require_services(services);
    auto binding = lifetime.content_binding();
    auto prefix = prepare_gui_text_content_00aba8d0_fragment(binding, services.content, input);
    if (prefix.branch == GuiTextContentBranch::unchanged)
        return {GuiTextRuntimeContentStatus::unchanged, {}};
    if (prefix.branch == GuiTextContentBranch::cleared)
        return {GuiTextRuntimeContentStatus::cleared, {}};

    auto frame = std::make_unique<GuiTextRuntimeContentContinuation>();
    frame->services = &services;
    frame->content = prepare_gui_text_nonempty_00aba8d0_fragment(
        lifetime, std::move(prefix), services.nonempty);
    // The selection, main section/material and native temporary were captured
    // after the last preparation callback. Do not reread multiline here.
    if (frame->content.builder == GuiTextGeometryBuilder::wrapped_00aba270) {
        auto pending = build_gui_text_wrapped_00aba270(lifetime,
            frame->content.transformed_text, *frame->content.main_section, services.wrapped);
        if (pending) {
            frame->builder.emplace<GuiTextWrappedContinuation>(std::move(*pending));
            if (complete_gui_text_content_children_00aba8d0(frame) == GuiTextRuntimeContentStatus::complete)
                return {GuiTextRuntimeContentStatus::complete, {}};
            return {GuiTextRuntimeContentStatus::pending_builder, std::move(frame)};
        }
    } else {
        auto pending = build_gui_text_single_line_00ab9fd0(lifetime,
            frame->content.transformed_text, *frame->content.main_section, services.single_line);
        if (pending) {
            frame->builder.emplace<GuiTextSingleLineContinuation>(std::move(*pending));
            if (complete_gui_text_content_children_00aba8d0(frame) == GuiTextRuntimeContentStatus::complete)
                return {GuiTextRuntimeContentStatus::complete, {}};
            return {GuiTextRuntimeContentStatus::pending_builder, std::move(frame)};
        }
    }
    finish(*frame);
    return {GuiTextRuntimeContentStatus::complete, {}};
}

GuiTextRuntimeContentStatus resume_gui_text_content_after_child_00aba8d0(
    std::unique_ptr<GuiTextRuntimeContentContinuation>& pending) {
    if (!pending || !pending->services)
        throw std::invalid_argument("Text content requires its pending outer frame");
    auto& frame = *pending;
    ChildOperation operation(frame);
    require_services(*frame.services);
    if (frame.child_tail) {
        if (frame.child_tail->status != GuiTextGlyphChildTailStatus::complete)
            throw std::logic_error("Text cannot advance before its retained actual child tail completes");
        frame.child_tail.reset();
    }
    if (auto* single = std::get_if<GuiTextSingleLineContinuation>(&frame.builder)) {
        auto next = resume_gui_text_single_line_after_child_00ab9fd0(std::move(*single));
        if (next) {
            frame.builder.emplace<GuiTextSingleLineContinuation>(std::move(*next));
            return GuiTextRuntimeContentStatus::pending_builder;
        }
    } else if (auto* wrapped = std::get_if<GuiTextWrappedContinuation>(&frame.builder)) {
        if (wrapped->pending != GuiTextWrappedPending::glyph_child)
            throw std::logic_error("Text content is pending at a different native boundary");
        auto next = resume_gui_text_wrapped_after_child_00aba270(std::move(*wrapped));
        if (next) {
            frame.builder.emplace<GuiTextWrappedContinuation>(std::move(*next));
            return GuiTextRuntimeContentStatus::pending_builder;
        }
    } else {
        throw std::logic_error("Text content has no pending native builder frame");
    }
    frame.builder.emplace<std::monostate>();
    finish(frame);
    operation.clear();
    pending.reset();
    return GuiTextRuntimeContentStatus::complete;
}
GuiTextRuntimeContentStatus complete_gui_text_content_children_00aba8d0(
    std::unique_ptr<GuiTextRuntimeContentContinuation>& pending) {
    if (!pending || !pending->services)
        throw std::invalid_argument("Text child driver requires its pending outer content frame");
    require_services(*pending->services);
    while (complete_current_child(*pending)) {
        if (resume_gui_text_content_after_child_00aba8d0(pending) == GuiTextRuntimeContentStatus::complete)
            return GuiTextRuntimeContentStatus::complete;
    }
    return GuiTextRuntimeContentStatus::pending_builder;
}
} // namespace bsp
