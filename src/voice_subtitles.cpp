#include "bsp/voice_subtitles.hpp"

#include <cstring>
#include <string>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Voice subtitle layout requires the recovered MSVC Win32 x87 operations.
#endif

namespace bsp {
namespace {
// Original binary64 operands, including the non-float-exact padding values.
constexpr double width_units_00cec380 = 960.0;
constexpr double width_padding_00cef290 = 0.0625;
constexpr double height_padding_00cf0de8 = 0x1.1111111111111p-7;
constexpr double decoration_padding_00cf0de0 = 0x1.ddddddddddddep-6;
constexpr double half_00d7a280 = 0.5;
constexpr double zero_00d7a258 = 0.0;
constexpr double one_00d7a210 = 1.0;

float normalized_width(float value) noexcept {
    float result;
    __asm {
        fld dword ptr [value]
        fdiv qword ptr [width_units_00cec380]
        fstp dword ptr [result]
    }
    return result;
}

float add_double(float value, double bias) noexcept {
    float result;
    __asm {
        fld dword ptr [value]
        fadd qword ptr [bias]
        fstp dword ptr [result]
    }
    return result;
}

float decoration_offset(float width) noexcept {
    float result;
    __asm {
        fld dword ptr [width]
        fmul qword ptr [half_00d7a280]
        fadd qword ptr [decoration_padding_00cf0de0]
        fstp dword ptr [result]
    }
    return result;
}

GuiWidgetPoint decoration_position(const GuiWidgetPoint& origin, float x_offset) noexcept {
    const float* from = &origin.x;
    GuiWidgetPoint result;
    float* to = &result.x;
    // Native computes/spills Y, Z, X in that order, then copies Y and Z.
    __asm {
        mov eax, from
        mov edx, to
        fld dword ptr [eax + 4]
        fsub qword ptr [zero_00d7a258]
        fstp dword ptr [edx + 4]
        fld dword ptr [eax + 8]
        fsub qword ptr [one_00d7a210]
        fstp dword ptr [edx + 8]
        fld dword ptr [eax]
        fsub dword ptr [x_offset]
        fstp dword ptr [edx]
    }
    return result;
}

float sum_float(float first, float second) noexcept {
    float result;
    __asm {
        fld dword ptr [first]
        fadd dword ptr [second]
        fstp dword ptr [result]
    }
    return result;
}

float copy_x87(float value) noexcept {
    float result;
    __asm {
        fld dword ptr [value]
        fstp dword ptr [result]
    }
    return result;
}

float character_term(std::int32_t character_count, float factor, float base) noexcept {
    float result;
    __asm {
        fild dword ptr [character_count]
        fmul dword ptr [factor]
        fadd dword ptr [base]
        fstp dword ptr [result]
    }
    return result;
}

GuiWidgetTransform& line_widget(VoiceLine& line) noexcept {
    return *static_cast<GuiWidgetTransform*>(line.widget_14);
}

void set_source(GuiTextWidget& widget, const NativeString& text,
    VoiceSubtitleHost& calls) {
    // The existing GUI API owns a std::string projection; preserve embedded
    // NUL bytes and rebuild this argument for each native source submission.
    const std::string source(text.data(), text.length());
    set_localised_source_00abaed0(widget, calls.text_host(widget), source, true);
}

struct ShortcutName {
    NativeString value;
    NativeStringStorage& storage;
    ~ShortcutName() { value.release_to(storage); }
};
} // namespace

void display_voice_subtitles_005b8510(VoiceLine& line, const NativeString& text,
    VoiceSubtitleContext& context, NativeStringStorage& storage) {
    auto& calls = context.calls;
    // 005B8528: only this template lookup uses the first manager identity.
    auto clone_manager = calls.manager_00e198c4_a4();
    line.widget_14 = clone_subtree(*clone_manager.template_2c, nullptr, context.scene);
    calls.set_visible_vslot_34(line_widget(line), true);
    {
        ShortcutName shortcut{{}, storage};
        shortcut.value.resize_0041dd40(storage, 13, true);
        if (shortcut.value.data()) {
            std::memcpy(shortcut.value.data(), "Shortcut_Icon", 14);
        }
        line.shortcut_widget_34 = calls.find_child_00aa7e00(
            line_widget(line), shortcut.value, true);
    } // The temporary releases before the second manager lookup.

    // 005B85D0 compares the unsigned string length before loading manager.
    const bool has_text = text.length() != 0;
    auto text_manager = calls.manager_00e198c4_a4();
    if (has_text && calls.subtitles_enabled_00f88989()) {
        calls.set_visible_vslot_34(*text_manager.group_30, true);
        calls.set_visible_vslot_34(*text_manager.background_3c, true);
        calls.set_visible_vslot_34(*text_manager.decoration_40, true);
        set_source(*text_manager.text_34, text, calls);
        set_source(*text_manager.text_38, text, calls);

        GuiTextWidget& measured = *text_manager.text_34;
        const float width = normalized_width(measured.measured_width);
        const float height = add_double(calls.normalized_height_00ab6bd0(measured),
            height_padding_00cf0de8);
        const GuiWidgetSize background_size{
            add_double(width, width_padding_00cef290), height};
        calls.set_size_vslot_58(*text_manager.background_3c, background_size);

        // Size dispatch may replace +34/+3C; re-read both, as native does.
        const float offset = decoration_offset(
            normalized_width(text_manager.text_34->measured_width));
        const GuiWidgetPoint origin = resolved_position(*text_manager.background_3c);
        const GuiWidgetPoint destination = decoration_position(origin, offset);
        set_resolved_position(*text_manager.decoration_40, destination, context.transform);
    } else {
        calls.set_visible_vslot_34(*text_manager.group_30, false);
        calls.set_visible_vslot_34(*text_manager.background_3c, false);
        calls.set_visible_vslot_34(*text_manager.decoration_40, false);
        calls.set_literal_00abbe50(*text_manager.text_34, "", true);
        calls.set_literal_00abbe50(*text_manager.text_38, "", true);
    }

    auto queue_manager = calls.manager_00e198c4_a4();
    VoiceLineNode* node = queue_manager.last_line_5c;
    float stacking_y = 0.0f;
    while (node) {
        if (node->line_08->widget_14) {
            const float height = widget_size(line_widget(*node->line_08)).height;
            stacking_y = sum_float(height, node->line_08->layout_20[2]);
            break;
        }
        node = node->previous_00;
    }
    line.layout_20[2] = copy_x87(stacking_y);
    // 00AA67F0 returns &widget.position. 00AA7D00 preserves the *current*
    // widget's Z, writes XY, then publishes transform before bounds.
    const GuiWidgetPoint& local_position = line_widget(line).position;
    const float local_y = copy_x87(stacking_y);
    const float local_x = copy_x87(local_position.x);
    GuiWidgetTransform& widget = line_widget(line);
    const float local_z = widget.position.z;
    widget.position.x = local_x;
    widget.position.y = local_y;
    widget.position.z = local_z;
    recompose_local_transform(widget, context.transform);
    refresh_local_bounds(widget, context.transform);
    calls.set_visible_vslot_34(line_widget(line), true);

    auto duration_manager = calls.manager_00e198c4_a4();
    // +ECh is UTF-16 code-unit length, not +110h line count; FILD is signed.
    const auto length = static_cast<std::int32_t>(
        static_cast<std::uint32_t>(duration_manager.text_34->text.size()));
    line.layout_20[1] = character_term(length,
        duration_manager.per_character_7c, duration_manager.base_80);
    auto initial_manager = calls.manager_00e198c4_a4();
    line.layout_20[0] = copy_x87(initial_manager.initial_78);
    auto dirty_manager = calls.manager_00e198c4_a4();
    dirty_manager.dirty_60 = 1;
    // Native SETNZ AL leaves upper 24 argument bits from the manager address;
    // vslot +34 consumes the low bool byte only.
    calls.set_visible_vslot_34(
        *static_cast<GuiWidgetTransform*>(line.shortcut_widget_34), line.target_2c != 0);
}

} // namespace bsp
