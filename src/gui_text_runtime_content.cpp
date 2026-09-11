#include "bsp/gui_text_runtime_content.hpp"
#include <stdexcept>
#include <utility>

namespace bsp {
namespace {
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
        &wrapped.children != &services.content.calls)
        throw std::invalid_argument("Text content stages require the same live owner and service domains");
}
void finish(GuiTextRuntimeContentContinuation& frame) {
    finish_gui_text_content_after_geometry_00aba8d0_fragment(
        *frame.content.lifetime, std::move(frame.content), frame.services->nonempty);
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
            return {GuiTextRuntimeContentStatus::pending_builder, std::move(frame)};
        }
    } else {
        auto pending = build_gui_text_single_line_00ab9fd0(lifetime,
            frame->content.transformed_text, *frame->content.main_section, services.single_line);
        if (pending) {
            frame->builder.emplace<GuiTextSingleLineContinuation>(std::move(*pending));
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
    require_services(*frame.services);
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
    pending.reset();
    return GuiTextRuntimeContentStatus::complete;
}
} // namespace bsp
