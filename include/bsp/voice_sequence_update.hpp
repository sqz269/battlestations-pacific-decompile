#pragma once

#include "bsp/scheduled_voice.hpp"
#include "bsp/voice_subtitles.hpp"

namespace bsp {

class VoiceManagerUpdateHost;
struct MissionLuaHostServices;

class VoiceSequenceHost {
public:
    virtual ~VoiceSequenceHost() = default;
    virtual bool input_action_pressed_004c43c0(std::uint32_t action) = 0;
    virtual bool widget_visible_vslot_38(GuiWidgetTransform&) = 0;
    // Resolve current [00E188A8]+21E4, then its signed-int -> Float4 map+10.
    // The real 0044EC00 performs lookup-or-insert. It is not an ARGB decoder;
    // missing keys must use its real insertion/default contract, not a fallback.
    virtual const std::array<float, 4>& palette_value_0044ec00(std::int32_t key) = 0;
    virtual void decoration_vslot_88(GuiWidgetTransform&, std::int32_t key,
        std::uint32_t zero, float one) = 0;
    virtual MissionLuaHostServices& current_mission_lua_1a08() = 0;
    // ECX=current [00E188A8]+21E4. Advances the actual panel state machine.
    virtual bool advance_panel_state_004527f0() = 0;
    virtual void log_004254b0(const char* message) = 0;
};

struct VoiceSequenceContext {
    ScheduledVoiceContext& scheduled;
    VoiceSubtitleContext& subtitles;
    VoiceManagerUpdateHost& attached;
    VoiceSequenceHost& host;
    NativeStringStorage& strings;
};

// ECX=voice manager; NativeString*, byte force at stack+4/+8; RET8.
// Same manager identity throughout; fields are reread after GUI callbacks.
void set_voice_panel_text_005b6710(VoicePlaybackManager&, const NativeString&,
    bool force, VoiceSubtitleContext&);

// ECX=voice manager; float delta stack; RET4; AL=active/panel advancement.
// Complete bounded timed-sequence update. Rows, records and widgets retained
// across callbacks must remain valid; no corrupt-vector or native ABI claim.
// Uses MSVC Win32 x87 float spills; signaling-NaN, unmasked FP traps/status and
// native allocator/SEH equivalence remain outside this typed projection.
bool update_voice_sequence_005bbf10(VoiceSequenceContext&, float delta);

} // namespace bsp
