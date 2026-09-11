#include "bsp/voice_sequence_update.hpp"

#include "bsp/mission_lua_host.hpp"
#include "bsp/native_pooled_string_substring.hpp"
#include "bsp/voice_manager_update.hpp"
#include "bsp/voice_slot_start.hpp"

#include <cstdlib>
#include <cstring>
#include <string>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Voice sequence reconstruction requires MSVC Win32 x87 operations.
#endif

namespace bsp {
namespace {
constexpr double width_units = 960.0;
constexpr double width_padding = 0.0625;
constexpr double height_padding = 0x1.1111111111111p-7;
constexpr double decoration_padding = 0x1.ddddddddddddep-6;
constexpr double half = 0.5;
constexpr double key_delay = static_cast<double>(0.1f); // 00D7A3A0
constexpr double zero = 0.0;
constexpr double one = 1.0;

float subtract(float first, float second) noexcept {
    float value;
    __asm {
        fld first
        fsub second
        fstp value
    }
    return value;
}
float divide(float first, float second) noexcept {
    float value;
    __asm {
        fld first
        fdiv second
        fstp value
    }
    return value;
}
float width_normalized(float first) noexcept {
    float value;
    __asm {
        fld first
        fdiv qword ptr [width_units]
        fstp value
    }
    return value;
}
float add_double(float first, double second) noexcept {
    float value;
    __asm {
        fld first
        fadd second
        fstp value
    }
    return value;
}
float decoration_offset(float width) noexcept {
    float value;
    __asm {
        fld width
        fmul qword ptr [half]
        fadd qword ptr [decoration_padding]
        fstp value
    }
    return value;
}
GuiWidgetPoint decoration_position(const GuiWidgetPoint& origin, float x_offset) noexcept {
    const float* from = &origin.x;
    GuiWidgetPoint result;
    float* to = &result.x;
    __asm {
        mov eax, from
        mov edx, to
        fld dword ptr [eax + 4]
        fsub qword ptr [zero]
        fstp dword ptr [edx + 4]
        fld dword ptr [eax + 8]
        fsub qword ptr [one]
        fstp dword ptr [edx + 8]
        fld dword ptr [eax]
        fsub x_offset
        fstp dword ptr [edx]
    }
    return result;
}

struct ScopedString {
    NativeString value;
    NativeStringStorage& storage;
    explicit ScopedString(NativeStringStorage& strings) : storage(strings) {}
    ScopedString(const NativeString& source, NativeStringStorage& strings) : storage(strings) {
        copy_construct_native_string_header_00426060(&value, &source, strings);
    }
    ScopedString(const NativeString& source, std::uint32_t start, std::uint32_t count,
        NativeStringStorage& strings) : storage(strings) {
        construct_native_string_substring_00469840(&source, &value, start, count, strings);
    }
    ~ScopedString() { destroy_native_string_header_0041dd20(&value, storage); }
};

VoicePlaybackSlot& resolve_slot(VoicePlaybackManager& manager, std::int32_t index,
    VoiceLineHost& playback) {
    return index == 0 ? manager.slot_08
        : playback.slot_start_context().host.resolve_nonzero_slot(manager, index);
}

void set_source(GuiTextWidget& widget, const NativeString& source, VoiceSubtitleHost& calls) {
    const std::string text = source.length() ? std::string(source.data(), source.length()) : std::string{};
    set_localised_source_00abaed0(widget, calls.text_host(widget), text, true);
}
} // namespace

void set_voice_panel_text_005b6710(VoicePlaybackManager& manager, const NativeString& text,
    bool force, VoiceSubtitleContext& context)
{
    auto& calls = context.calls;
    const bool show = (text.length() != 0 && calls.subtitles_enabled_00f88989()) || force;
    calls.set_visible_vslot_34(*manager.group_30, show);
    calls.set_visible_vslot_34(*manager.background_3c, show);
    calls.set_visible_vslot_34(*manager.decoration_40, show);
    if (!show) {
        calls.set_literal_00abbe50(*manager.text_34, "", true);
        calls.set_literal_00abbe50(*manager.text_38, "", true);
        return;
    }
    set_source(*manager.text_34, text, calls);
    set_source(*manager.text_38, text, calls);
    auto& measured = *manager.text_34;
    const float width = width_normalized(measured.measured_width);
    const float height = add_double(calls.normalized_height_00ab6bd0(measured), height_padding);
    const GuiWidgetSize size{add_double(width, width_padding), height};
    calls.set_size_vslot_58(*manager.background_3c, size);
    const float offset = decoration_offset(width_normalized(manager.text_34->measured_width));
    const auto origin = resolved_position(*manager.background_3c);
    const auto destination = decoration_position(origin, offset);
    set_resolved_position(*manager.decoration_40, destination, context.transform);
}

bool update_voice_sequence_005bbf10(VoiceSequenceContext& context, float delta)
{
    auto& scheduled = context.scheduled;
    auto& manager = scheduled.manager; // native ESI stays this owner throughout
    auto& fields = scheduled.fields;
    auto& playback = scheduled.playback;
    if (context.host.input_action_pressed_004c43c0(0xe6)) {
        const auto index = fields.slot_index_70; // read before the two stores
        manager.delay_88 = 0.0f;
        manager.hold_8c = 0.0f;
        if (index >= 0) {
            auto& slot = resolve_slot(manager, index, playback);
            if (poll_voice_slot_007027b0(slot, playback) != 0)
                stop_voice_slot_007026f0(slot, playback, context.attached);
        }
    }
    for (std::uint32_t index = 0; index < fields.rows_94.size(); ++index) {
        auto& row = fields.rows_94[index]; // native row address retained across calls
        if (index != fields.selected_row_84) {
            if (context.host.widget_visible_vslot_38(*row.widget_00)) {
                row.fade_remaining_0c = subtract(row.fade_remaining_0c, delta);
                float remaining = row.fade_remaining_0c;
                if (remaining < 0.0f) remaining = 0.0f;
                else if (remaining > manager.initial_78) remaining = manager.initial_78;
                row.fade_remaining_0c = remaining;
                // Native COMISS/JBE takes the clearing path for unordered too.
                if (!(remaining > 0.0f)) {
                    ScopedString empty(context.strings);
                    // Native inlines the same empty/disabled panel sequence here.
                    set_voice_panel_text_005b6710(manager, empty.value, false, context.subtitles);
                } else row.color_10[3] = divide(remaining, manager.initial_78);
            }
            if (index != fields.selected_row_84) continue; // reload after GUI calls
        }
        if (fields.pending_record_74 || !row.keys_active_20) continue;
        const auto& keys = scheduled.records.timed_keys_34(*row.record_24); // EBX captured
        if (row.key_index_28 >= keys.size()) continue;
        const float elapsed = subtract(playback.mission_clock_00f876a4(), row.started_at_2c);
        bool advance = false;
        if (keys[row.key_index_28].end_14 != 0.0f)
            advance = elapsed > keys[row.key_index_28].end_14;
        if (!advance && keys[row.key_index_28].end_14 == 0.0f) {
            auto& polled_slot = resolve_slot(manager, row.slot_index_30, playback);
            if (poll_voice_slot_007027b0(polled_slot, playback) != 0) {
                auto& current_slot = resolve_slot(manager, row.slot_index_30, playback);
                advance = playback.sound_completed_vslot_0c(current_slot.sound_04);
            }
        }
        if (advance) {
            {
                ScopedString empty(context.strings);
                set_voice_panel_text_005b6710(manager, empty.value, false, context.subtitles);
            }
            if (keys[row.key_index_28].callback_08.length()) {
                ScopedString callback(keys[row.key_index_28].callback_08, context.strings);
                auto& lua = context.host.current_mission_lua_1a08();
                const std::string name = callback.value.length()
                    ? std::string(callback.value.data(), callback.value.length()) : std::string{};
                (void)call_named_entry_point_threadsafe(lua, name, {});
            }
            ++row.key_index_28; // after callback and temporary cleanup; dword wrap
        }
        if (row.key_index_28 >= keys.size()) continue;
        const float threshold = add_double(keys[row.key_index_28].start_10, key_delay);
        if (!(elapsed > threshold)) continue; // equality/unordered does not show
        const auto& key = keys[row.key_index_28]; // pointer survives string callbacks
        ScopedString text(key.text_00, context.strings);
        if (text.value.data() && text.value.length()) {
            const auto prefix_length = static_cast<std::uint32_t>(std::strcspn(text.value.data(), "^"));
            // A stored string with embedded NUL still follows CRT strcspn's
            // terminator result; native does not separately check for '^'.
            if (prefix_length != text.value.length() && static_cast<std::int32_t>(prefix_length) > 0) {
                std::int32_t palette_key;
                {
                    ScopedString prefix(text.value, 0, prefix_length, context.strings);
                    palette_key = static_cast<std::int32_t>(std::atol(prefix.value.data() ? prefix.value.data() : ""));
                }
                const auto& color = context.host.palette_value_0044ec00(palette_key);
                row.color_10 = color;
                {
                    ScopedString suffix(text.value, prefix_length + 1, 0x7fffffffu, context.strings);
                    text.value.copy_from_00be0a30_fragment(context.strings, suffix.value); // 00425F40
                }
                if (palette_key)
                    context.host.decoration_vslot_88(*manager.decoration_40, palette_key, 0, 1.0f);
            }
        }
        set_voice_panel_text_005b6710(manager, text.value, key.flag_18 != 0, context.subtitles);
    }
    manager.hold_8c = subtract(manager.hold_8c, delta);
    if (poll_scheduled_voice_005b9420(scheduled)) return true;
    // COMISS/JA and FCOMIP/JA permit unordered to advance both terminal gates.
    if (manager.hold_8c > 0.0f) return true;
    fields.selected_row_84 = 0xffffffffu;
    manager.delay_88 = subtract(manager.delay_88, delta);
    if (manager.delay_88 > 0.0f) return true;
    const bool active = context.host.advance_panel_state_004527f0();
    if (active) context.host.log_004254b0("vanmeg");
    context.subtitles.calls.set_visible_vslot_34(*manager.panel_28, active);
    return active;
}

} // namespace bsp
