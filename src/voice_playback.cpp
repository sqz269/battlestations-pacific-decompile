#include "bsp/voice_playback.hpp"
#include "bsp/mission_events.hpp"
#include "bsp/voice_fade_update.hpp"
#include "bsp/voice_slot_start.hpp"
#include "bsp/voice_subtitles.hpp"

#include <cstring>

namespace bsp {
namespace {
class ConsumedReference {
public:
    ConsumedReference(void* value, VoiceSlotHost& host) : value_(value), host_(host) {}
    ~ConsumedReference() { if (value_) host_.release_reference(value_); }
    ConsumedReference(const ConsumedReference&) = delete;
    ConsumedReference& operator=(const ConsumedReference&) = delete;
private:
    void* value_;
    VoiceSlotHost& host_;
};
class TemporaryText {
public:
    explicit TemporaryText(NativeStringStorage& storage) : storage_(storage) {}
    ~TemporaryText() { text.release_to(storage_); }
    void append(const char* data, std::uint32_t length) {
        if (!length) return;
        const auto old = text.length();
        text.resize_0041dd40(storage_, old + length, true);
        std::memcpy(text.data() + old, data, length);
    }
    NativeString text;
private:
    NativeStringStorage& storage_;
};
} // namespace

std::int32_t poll_voice_slot_007027b0(VoicePlaybackSlot& slot, VoiceSlotHost& host)
{
    if (slot.state_00 == 1) {
        const float now = host.mission_clock_00f876a4(); // capture before +C call
        bool retire = host.sound_completed_vslot_0c(slot.sound_04);
        if (!retire) {
            const float duration = host.sound_duration_00a81860(slot.sound_04);
            const float deadline = static_cast<float>(static_cast<double>(duration) + 2.0);
            // Native now-start remains in x87; duration+2 is spilled to float.
            // JBE keeps the slot alive on equality and unordered comparisons.
            retire = static_cast<double>(now) - slot.started_at_14 > deadline;
        }
        if (retire) {
            host.stop_sound_vslot_08(slot.sound_04, 1);
            if (slot.sound_04) {
                host.release_reference(slot.sound_04);
                slot.sound_04 = nullptr;
            }
            slot.sound_04 = nullptr;
            slot.state_00 = 0;
            if (slot.auxiliary_08) {
                host.stop_auxiliary_vslot_08(slot.auxiliary_08, 0);
                host.reset_auxiliary_0054d510(slot.auxiliary_08, nullptr);
            }
            host.set_sound_flag_00a7d120(0);
        }
    } else if (slot.state_00 == 2 && !host.alternate_playing_00a77730()) {
        slot.state_00 = 0;
    }
    return slot.state_00;
}

bool voice_can_play_005b71d0(VoicePlaybackManager& manager, const VoiceClips& clips,
    const VoicePanelState& panels, VoiceSlotHost& host)
{
    if (panels.field_34 || panels.field_24 || manager.pending_record_74) return false;
    // Count is reread each iteration just as 005B5DF0 is called in the native.
    for (std::size_t index = 0; index < clips.size(); ++index) {
        if (poll_voice_slot_007027b0(manager.slot_08, host) != 0) return false;
    }
    return manager.attached_count_6c == 0;
}

std::uint32_t classify_voice_speaker_005bbc10(void* speaker, VoiceLineHost& host)
{
    if (!speaker) return 0;
    const auto local_side = host.local_side_18cc_18ec();
    if (host.speaker_side_54(speaker) != local_side && local_side != 2) return 4;
    if (host.speaker_kind_vslot_5c(speaker, 8)) return 1;
    if (host.speaker_kind_vslot_5c(speaker, 0x1a)
        || host.speaker_kind_vslot_5c(speaker, 0x1b)
        || host.speaker_kind_vslot_5c(speaker, 0x19)) return 2;
    return host.speaker_kind_vslot_5c(speaker, 6) ? 3u : 0u;
}

void append_voice_line_005b7790(VoiceLineQueue& queue, VoiceLine* line, VoiceLineHost& host)
{
    auto& node = host.allocate_node_00bf681b(0x0c);
    node = {};
    node.line_08 = line;
    node.previous_00 = queue.last_08;
    if (queue.count_00) queue.last_08->next_04 = &node;
    else queue.first_04 = &node;
    queue.last_08 = &node;
    node.next_04 = nullptr;
    ++queue.count_00;
}

VoiceLine& construct_voice_line_005babb0(VoiceLine& line,
    const VoiceClips& clips, std::uint32_t target, void* bank, VoiceLineHost& host,
    NativeStringStorage& strings)
{
    ConsumedReference argument(bank, host);
    line.native_vtable_00 = 0x00cf0ed4;
    line.clips_04 = clips; // fresh destination branch of 005BA4B0
    if (target && (target == host.invalid_target_00e188d8()
        || !host.target_valid_00645160(target, true))) target = 0;
    line.target_2c = target;
    TemporaryText joined(strings);
    bool has_sound = false;
    for (std::size_t index = 0; index < clips.size(); ++index) {
        const auto& record = *clips[index].record_04;
        if (joined.text.length()) joined.append("|", 1); // 00CF100C
        joined.append(record.text_00.data(), record.text_00.length());
        if (record.sound_id_08 >= 0) has_sound = true;
    }
    if (joined.text.length())
        display_voice_subtitles_005b8510(line, joined.text, host.subtitle_context(), strings);
    else {
        line.layout_20 = {};
        line.widget_14 = nullptr;
    }
    if (has_sound) {
        std::uint32_t index = 0;
        while (clips[index].record_04->sound_id_08 < 0) ++index;
        line.clip_index_1c = index;
        const VoiceClip selected{0x00cf0dd0, clips[index].record_04, clips[index].word_08};
        auto& poll_manager = host.current_voice_manager_00e198c4_a4(); // 005BAE1C..21
        line.slot_index_18 = poll_voice_slot_007027b0(poll_manager.slot_08, host) == 0 ? 0 : -1;
        if (bank) host.retain_reference(bank);
        // 005B9050 consumes this retained by-value argument itself.
        const auto selected_slot = line.slot_index_18;
        auto& start_manager = host.current_voice_manager_00e198c4_a4(); // 005BAE85..8B
        start_voice_clip_005b9050(start_manager, selected_slot, selected, bank,
            host, host.slot_start_context());
    } else {
        line.slot_index_18 = -1; // +1C remains untouched on this branch
    }
    host.log_004254b0("Message displayed: %s", joined.text.data() ? joined.text.data() : "");
    return line;
}

VoiceLine* play_voice_line_005bbc10(VoicePlaybackManager& manager, const VoiceClips& clips,
    std::uint32_t target, void* speaker, VoiceLineHost& host, NativeStringStorage& strings)
{
    const auto category = classify_voice_speaker_005bbc10(speaker, host);
    if (target || speaker) host.notify_005a2650(target, speaker, 1);
    auto* line = host.allocate_line_00bf681b(0x38);
    if (line) {
        void* bank = speaker ? manager.speaker_table_a0[category] : nullptr;
        if (bank) host.retain_reference(bank);
        construct_voice_line_005babb0(*line, clips, target, bank, host, strings);
    }
    append_voice_line_005b7790(manager.lines_54, line, host);
    return line;
}

bool play_positional_voice_line_005bbdc0(VoicePlaybackManager& manager, VoiceClip clip,
    void* entity, VoiceLineHost& host, NativeStringStorage& strings)
{
    if (clip.record_04->sound_id_08 < 0) return false;
    const auto camera = host.camera_position_00b6db70();
    const auto position = host.entity_position_00414db0(entity);
    std::array<float, 3> separation{};
    for (std::size_t axis = 0; axis < separation.size(); ++axis)
        separation[axis] = static_cast<float>(static_cast<double>(position[axis]) - camera[axis]);
    const double distance = host.vector_length_0042b2f0(separation);
    const float attenuation = static_cast<float>((static_cast<double>(kVoiceAudibleRange) - distance)
        / static_cast<double>(kVoiceAudibleRange));
    // FCOMIP(.25,a)/JA and COMISS(0,a)/JNC accept unordered (masked NaN).
    if (attenuation < kVoiceMinimumAttenuation || attenuation <= 0.0f) return false;
    VoiceClips one{clip}; // 005BB020 on an empty vector, original Clip12 copied
    play_voice_line_005bbc10(manager, one, 0, nullptr, host, strings);
    return true; // attenuation is admission only; it is NOT a volume argument
}

void begin_voice_sound_fade_005b9760(VoicePlaybackManager& manager, float target,
    float duration, const NativeString& callback, VoiceLineHost& host,
    NativeStringStorage& strings)
{
    manager.fade_value_d4 = host.global_sound_level_6c();
    manager.fade_target_d8 = target;
    constexpr float minimum = 0.0001f; // 00CE3C68; 00D7A268 is this float widened
    const float divisor = duration > static_cast<double>(minimum) ? duration : minimum;
    manager.fade_rate_dc = static_cast<float>(1.0 / static_cast<double>(divisor));
    manager.fade_callback_e0.copy_from_00be0a30_fragment(strings, callback);
    update_voice_sound_fade_005b8c30(manager, minimum, host.fade_bindings());
}

} // namespace bsp
