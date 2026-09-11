#include "bsp/voice_manager_update.hpp"

#include "bsp/gui_widget.hpp"
#include "bsp/voice_line_advance.hpp"
#include "bsp/voice_line_lifetime.hpp"
#include "bsp/voice_slot_start.hpp"
#include "bsp/voice_subtitles.hpp"

namespace bsp {
namespace {
float subtract_float(float a, float b) noexcept {
    return static_cast<float>(static_cast<double>(a) - static_cast<double>(b));
}
float clamp_volume(float value) noexcept {
    // COMISS/JBE at00702159/63: unordered passes original; signed zero retained.
    if (value < 0.0f) return 0.0f;
    if (value > 1.0f) return 1.0f;
    return value;
}
GuiWidgetTransform& widget(void* value) {
    return *static_cast<GuiWidgetTransform*>(value); // canonical GUI projection
}
void store_volume(void* sound, float value, VoiceManagerUpdateHost& calls) {
    auto view = calls.sound_volume_view(sound);
    view.volume_24 = clamp_volume(value);
    view.dirty_14 = 1;
}
} // namespace

void set_named_voice_volume_00701870(VoiceAlternateChannelsView channels,
    const NativeString& name, float volume, VoiceManagerUpdateHost& calls)
{
    calls.set_alternate_channel_volume_00a78330(channels.channel_08, name, volume);
    // The first call may replace the second channel or change name storage.
    calls.set_alternate_channel_volume_00a78330(channels.channel_0c, name, volume);
}

void set_voice_slot_volume_00702130(VoicePlaybackSlot& slot, float volume,
    VoiceManagerUpdateHost& calls)
{
    void* auxiliary = slot.auxiliary_08;
    if (auxiliary) store_volume(auxiliary, volume, calls);
    if (slot.state_00 == 1) {
        store_volume(slot.sound_04, volume, calls); // native no null check
    } else {
        auto channels = calls.alternate_channels_00f8bbcc();
        set_named_voice_volume_00701870(channels, slot.alternate_name_0c, volume, calls);
    }
}

void stop_voice_slot_007026f0(VoicePlaybackSlot& slot, VoiceSlotHost& sounds,
    VoiceManagerUpdateHost& calls)
{
    if (slot.state_00 != 0) sounds.set_sound_flag_00a7d120(0);
    if (void* auxiliary = slot.auxiliary_08) {
        sounds.stop_auxiliary_vslot_08(auxiliary, 0);
        if (void* current = slot.auxiliary_08) {
            sounds.release_reference(current);
            slot.auxiliary_08 = nullptr;
        }
        slot.auxiliary_08 = nullptr;
    }
    const auto state = slot.state_00; // after auxiliary callbacks
    if (state == 1) {
        sounds.stop_sound_vslot_08(slot.sound_04, 0);
        if (void* current = slot.sound_04) {
            sounds.release_reference(current);
            slot.sound_04 = nullptr;
        }
        slot.sound_04 = nullptr;
        slot.state_00 = 0;
    } else if (state == 2) {
        if (sounds.alternate_playing_00a77730()) calls.stop_alternate_00a78620();
        slot.state_00 = 0;
    }
}

bool update_attached_voice_005b7290(VoiceAttachedEntry& entry, VoiceLineHost& lines,
    VoiceManagerUpdateHost& calls)
{
    if (poll_voice_slot_007027b0(entry.slot_00, lines) == 0) return false;
    if (entry.attached_entity_2c != nullptr) {
        const auto camera = lines.camera_position_00b6db70();
        // Reload attached entity AFTER camera refresh; no second null guard.
        const auto entity = lines.entity_position_00414db0(entry.attached_entity_2c);
        std::array<float, 3> difference{};
        for (std::size_t i = 0; i < 3; ++i)
            difference[i] = subtract_float(entity[i], camera[i]);
        const double distance = lines.vector_length_0042b2f0(difference);
        const float attenuation = static_cast<float>((2000.0 - distance) / 2000.0);
        // JA after lower-bound compare; JBE after positive test rejects NaN.
        if (attenuation >= 0.25f && attenuation > 0.0f) {
            set_voice_slot_volume_00702130(entry.slot_00, attenuation, calls);
            return true;
        }
    }
    stop_voice_slot_007026f0(entry.slot_00, lines, calls);
    return false;
}

bool update_voice_manager_005bc640(VoicePlaybackManager& manager, float delta,
    VoiceManagerUpdateContext& context)
{
    auto& calls = context.calls;
    auto& lines = context.lines;
    auto* node = manager.lines_54.first_04;
    while (node != nullptr) {
        VoiceLine* line = node->line_08; // cached EDI for this update branch
        const auto target = line->target_2c;
        if (target != 0 && (target == lines.invalid_target_00e188d8()
            || !lines.target_valid_00645160(target, true))) {
            void* shortcut = line->shortcut_widget_34; // loaded before target clear
            line->target_2c = 0;
            lines.subtitle_context().calls.set_visible_vslot_34(widget(shortcut), false);
        }
        // 005BC6A3 COMISS(0,timer)/JC: positive OR unordered skips advancement.
        if (!(line->layout_20[1] <= 0.0f) || advance_voice_line_005b91e0(*line, lines)) {
            line->layout_20[1] = subtract_float(line->layout_20[1], delta);
        } else {
            const float remaining = subtract_float(line->layout_20[0], delta);
            line->layout_20[0] = remaining;
            if (remaining <= 0.0f || line->widget_14 == nullptr) {
                if (node->previous_00) node->previous_00->next_04 = node->next_04;
                else manager.lines_54.first_04 = node->next_04;
                VoiceLineNode* next;
                if (node->next_04) {
                    next = node->next_04;
                    next->previous_00 = node->previous_00;
                } else {
                    next = nullptr;
                    manager.lines_54.last_08 = node->previous_00;
                }
                --manager.lines_54.count_00;
                // Reload payload after preceding calls and list edits.
                if (auto* deleted = node->line_08) {
                    if (deleted->native_vtable_00 == 0x00cf0ed4)
                        scalar_delete_voice_line_005b9b80(deleted, 1, context.lifetime);
                    else
                        calls.scalar_delete_line_005bc70e(deleted, 1);
                }
                calls.free_line_node_005bc711(node);
                node = next; // recovered005BC719; never dereference freed node
                manager.dirty_60 = 1; // recovered005BC71B, AFTER destruction/free
                continue;
            }
        }
        node = node->next_04; // native reload after advancement/visibility callbacks
    }

    if (manager.dirty_60 != 0) {
        node = manager.lines_54.first_04;
        manager.dirty_60 = 0;
        float baseline = 0.0f;
        while (node != nullptr) {
            if (node->line_08->widget_14 != nullptr) {
                if (node->line_08->layout_20[2] > baseline) {
                    const float step = calls.relayout_step_00432650();
                    auto* current = node->line_08; // reload after singleton/getter
                    current->layout_20[2] = subtract_float(current->layout_20[2], step);
                }
                auto* current = node->line_08;
                // 005BC794 compares baseline,y; JC includes unordered.
                if (!(baseline >= current->layout_20[2])) manager.dirty_60 = 1;
                else current->layout_20[2] = baseline;

                const float height = widget_size(widget(node->line_08->widget_14)).height;
                auto* position_line = node->line_08;
                baseline = static_cast<float>(static_cast<double>(height)
                    + static_cast<double>(position_line->layout_20[2]));
                const auto& position = widget(position_line->widget_14).position;
                const float y = position_line->layout_20[2];
                const float x = position.x;
                auto& destination = widget(node->line_08->widget_14);
                const float z = destination.position.z;
                destination.position.x = x;
                destination.position.y = y;
                destination.position.z = z;
                auto& transform = lines.subtitle_context().transform;
                recompose_local_transform(destination, transform);
                refresh_local_bounds(destination, transform);
            }
            node = node->next_04; // GUI callbacks can change the next link
        }
    }

    auto attached = context.attached; // aliases, not a snapshot of sentinel/count
    auto* attached_node = attached.sentinel_68->next_00;
    while (attached_node != attached.sentinel_68) {
        auto* next = attached_node->next_00; // saved BEFORE005B7290 callbacks
        auto* entry = attached_node->entry_08;
        if (!update_attached_voice_005b7290(*entry, lines, calls)) {
            if (attached_node == attached.sentinel_68) calls.invalid_iterator_00bf6713();
            if (attached_node != attached.sentinel_68) {
                attached_node->previous_04->next_00 = attached_node->next_00;
                attached_node->next_00->previous_04 = attached_node->previous_04;
                calls.free_attached_node_005bc868(attached_node);
                --attached.count_6c; // recovered005BC870: AFTER free callback
            }
            if (entry != nullptr) {
                entry->lifetime_vtable_18 = 0x00ce74fc;
                if (void* entity = entry->attached_entity_2c)
                    calls.unregister_attached_006952a0(entity, *entry);
                calls.destroy_attachment_00695870(*entry);
                destroy_voice_slot_005b7fc0(entry->slot_00, lines, context.strings);
                calls.free_attached_entry_005bc8c0(entry);
            }
        }
        attached_node = next; // recovered005BC8C8 ->005BC7FC
    }
    // This is the SAME storage as native manager+6C, not a new blocked flag.
    if (manager.lines_54.count_00 != 0 || attached.count_6c != 0) return true;
    return calls.external_activity_00f8a0c4_e8() != 0;
}

} // namespace bsp
