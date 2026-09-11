#pragma once

#include "bsp/native_string.hpp"

#include <array>
#include <cstdint>
#include <vector>

namespace bsp {

struct GuiWidgetTransform;
struct VoiceClipRecord;

// Native1Ch key at record+34 begin pointer. Strings retain NativeString's
// explicit owner/storage teardown. This projection adds no implicit cleanup.
// Timing/flag names describe consumer behavior, not recovered symbols.
struct VoiceTimedKey {
    NativeString text_00;
    NativeString callback_08;
    float start_10{};
    float end_14{};
    std::uint8_t flag_18{};
};
using VoiceTimedKeys = std::vector<VoiceTimedKey>;

// Native34h row in manager+94/+98. Unknown +4/+8 and padding are omitted.
// Record+24 is the SAME borrowed VoiceClipRecord prefix used by playback;
// key storage is resolved from that identity, never a copied clip record.
// Default initializers are host convenience, not a recovered constructor.
struct VoiceScheduledRow {
    GuiWidgetTransform* widget_00{};
    float fade_remaining_0c{};
    std::array<float, 4> color_10{};
    std::uint8_t keys_active_20{};
    const VoiceClipRecord* record_24{};
    std::uint32_t key_index_28{};
    float started_at_2c{};
    std::int32_t slot_index_30{};
};
using VoiceScheduledRows = std::vector<VoiceScheduledRow>;

} // namespace bsp
