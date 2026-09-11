#include "bsp/gui_text_runtime_submit.hpp"
#include "bsp/gui_text_ellipsis.hpp"
#include "bsp/native_physical_file_date.hpp"
#include <limits>
#include <stdexcept>
#include <utility>

namespace bsp {
namespace {
template<class String>
void require_string(const String& value) {
    if (value.size() > static_cast<std::size_t>((std::numeric_limits<std::int32_t>::max)()) ||
        value.find(typename String::value_type{}) != String::npos)
        throw std::invalid_argument("Text submission requires a valid native terminated string domain");
}
bool same_source(const std::string& current, const std::string& source) {
    require_string(current);
    require_string(source);
    struct Header { std::uint32_t length; const char* data; };
    static_assert(sizeof(Header) == 8, "Text source headers require Win32");
    const Header left{static_cast<std::uint32_t>(current.size()), current.c_str()};
    const Header right{static_cast<std::uint32_t>(source.size()), source.c_str()};
    return equal_native_string_headers_00435c40(&left, &right);
}
std::u16string convert_source(const std::string& source, bool localize,
    GuiTextSubmitServices& services) {
    auto& locale = services.content.content;
    if (localize)
        return LocaleTextResolver(locale.locale, locale.locale_runtime).resolve(source);
    // 004C5E60 ordinary byte widening; native header/pool ABI is excluded.
    std::u16string converted;
    converted.reserve(source.size());
    for (unsigned char byte : source) converted.push_back(static_cast<char16_t>(byte));
    return converted;
}
std::unique_ptr<GuiTextSubmitContinuation> frame_for(GuiTextLifetime& lifetime,
    GuiTextSubmitServices& services, bool final_color) {
    auto frame = std::make_unique<GuiTextSubmitContinuation>();
    frame->lifetime = &lifetime;
    frame->services = &services;
    frame->final_color = final_color;
    return frame;
}
void finish(GuiTextSubmitContinuation& frame) {
    // Distinct native temporaries, in native cleanup order before color reload.
    std::u16string{}.swap(frame.clipped);
    std::u16string{}.swap(frame.converted);
    if (frame.final_color) {
        auto binding = frame.lifetime->style_binding(frame.services->content.nonempty.style);
        set_gui_text_color50_00ab6b50(binding, binding.widget.layout().color);
    }
}
GuiTextSubmitResult submit(std::unique_ptr<GuiTextSubmitContinuation> frame,
    std::u16string_view text) {
    auto result = build_gui_text_content_00aba8d0(*frame->lifetime, text, frame->services->content);
    if (result.status == GuiTextRuntimeContentStatus::pending_builder) {
        frame->content = std::move(result.pending);
        return {GuiTextSubmitStatus::pending_content, std::move(frame)};
    }
    finish(*frame);
    return {GuiTextSubmitStatus::complete, {}};
}
bool default_width(float width, const volatile float& sentinel) noexcept {
    const volatile float* address = &sentinel;
    bool equal;
    __asm {
        mov edx, address
        movss xmm0, width
        ucomiss xmm0, dword ptr [edx]
        lahf
        test ah, 44h
        setnp al
        mov equal, al
    }
    return equal;
}
void copy_size_pair(GuiWidgetSize& destination, const GuiWidgetSize& source) noexcept {
    auto* to = &destination;
    const auto* from = &source;
    __asm {
        mov ecx, to
        mov eax, from
        fld dword ptr [eax]
        fstp dword ptr [ecx]
        fld dword ptr [eax + 4]
        fstp dword ptr [ecx + 4]
    }
}
} // namespace

GuiTextSubmitResult submit_gui_text_utf16_00ab6ab0(GuiTextLifetime& lifetime,
    std::u16string_view text, GuiTextSubmitServices& services) {
    return submit(frame_for(lifetime, services, true), text);
}

GuiTextSubmitResult submit_gui_text_source_00abaed0(GuiTextLifetime& lifetime,
    const std::string& source, bool localize, GuiTextSubmitServices& services) {
    auto& text = lifetime.text();
    if (same_source(text.source, source))
        return {GuiTextSubmitStatus::unchanged_source, {}};
    text.source = source;
    auto frame = frame_for(lifetime, services, true);
    frame->converted = convert_source(source, localize, services);
    const auto view = std::u16string_view(frame->converted);
    return submit(std::move(frame), view);
}

GuiTextSubmitResult submit_gui_text_ellipsis_00abb000(GuiTextLifetime& lifetime,
    const std::string& source, float width, bool localize, GuiTextSubmitServices& services) {
    auto& text = lifetime.text();
    if (same_source(text.source, source))
        return {GuiTextSubmitStatus::unchanged_source, {}};
    text.source = source;
    if (default_width(width, services.default_width_00d7a260))
        width = lifetime.content_binding().widget.layout().transform.size.width;
    auto frame = frame_for(lifetime, services, true);
    frame->converted = convert_source(source, localize, services);
    auto& font = services.content.nonempty.fonts.resolve(text.font);
    auto& locale = services.content.content;
    frame->clipped = ellipsize_gui_text_00ab8f00(text, font.data(),
        locale.locale, locale.locale_runtime, frame->converted, width);
    const auto view = std::u16string_view(frame->clipped);
    return submit(std::move(frame), view);
}

GuiTextSubmitResult rebuild_gui_text_content_00abb1d0(GuiTextLifetime& lifetime,
    GuiTextSubmitServices& services) {
    require_string(lifetime.text().text);
    auto frame = frame_for(lifetime, services, false);
    frame->converted = lifetime.text().text;
    std::u16string{}.swap(lifetime.text().text);
    const auto view = std::u16string_view(frame->converted);
    return submit(std::move(frame), view);
}

GuiTextSubmitResult resize_gui_text_00abbf30(GuiTextLifetime& lifetime,
    const GuiWidgetSize& size, GuiTextSubmitServices& services) {
    auto binding = lifetime.content_binding();
    copy_size_pair(binding.widget.layout().transform.size, size);
    // Refresh the preexisting legacy view of these same base fields.
    binding.text.size = binding.widget.layout().transform.size;
    binding.widget.recompose_00aa7220(); // AA797F; no bounds refresh.
    return rebuild_gui_text_content_00abb1d0(lifetime, services);
}

GuiTextSubmitStatus resume_gui_text_submit_after_child(
    std::unique_ptr<GuiTextSubmitContinuation>& pending) {
    if (!pending || !pending->content)
        throw std::invalid_argument("Text submission requires its pending caller and content frames");
    if (resume_gui_text_content_after_child_00aba8d0(pending->content) ==
        GuiTextRuntimeContentStatus::pending_builder)
        return GuiTextSubmitStatus::pending_content;
    finish(*pending);
    pending.reset();
    return GuiTextSubmitStatus::complete;
}
} // namespace bsp
