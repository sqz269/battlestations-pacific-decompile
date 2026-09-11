#include "bsp/voice_line_advance.hpp"
#include "bsp/voice_slot_start.hpp"

namespace bsp {

bool advance_voice_line_005b91e0(VoiceLine& line, VoiceLineHost& host)
{
    const auto initial_slot = line.slot_index_18;
    if (initial_slot == -1) return false;
    auto& manager = host.current_voice_manager_00e198c4_a4(); // 005B91F2..F8
    auto& slot = initial_slot == 0 ? manager.slot_08
        : host.slot_start_context().host.resolve_nonzero_slot(manager, initial_slot);
    if (poll_voice_slot_007027b0(slot, host) != 0) return true;

    // Native ADD dword wraps, then compares the unsigned index against the
    // freshly read vector count. The active-line precondition supplies +1C.
    do {
        line.clip_index_1c.value() += 1u;
        const auto index = line.clip_index_1c.value();
        if (index >= line.clips_04.size()) break;
        if (line.clips_04[index].record_04->sound_id_08 >= 0) break;
    } while (true);

    const auto index = line.clip_index_1c.value();
    if (index >= line.clips_04.size()) {
        line.slot_index_18 = -1;
        return false;
    }
    const auto& clip = line.clips_04[index];
    const VoiceClip next{0x00cf0dd0, clip.record_04, clip.word_08};
    const auto selected_slot = line.slot_index_18; // re-read after poll
    auto& current_manager = host.current_voice_manager_00e198c4_a4(); // 005B92C9..CF
    // Subsequent clips use a null bank, even if the constructor used a speaker
    // bank. The native returns true without testing the newly started slot.
    start_voice_clip_005b9050(current_manager, selected_slot, next, nullptr,
        host, host.slot_start_context());
    return true;
}

} // namespace bsp
