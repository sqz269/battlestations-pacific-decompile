#pragma once

#include "bsp/scheduled_voice.hpp"
#include "bsp/panel_palette.hpp"
#include "bsp/panel_sequence.hpp"
#include "bsp/voice_subtitles.hpp"

namespace bsp {

class VoiceManagerUpdateHost;
struct MissionLuaHostServices;
struct PanelSequenceContext;

inline PanelSequenceView voice_panel_sequence(VoicePanelState& panel) noexcept {
    return {panel.queued_1c, panel.field_24, panel.current_28, panel.field_34};
}

class VoiceSequenceHost {
public:
    virtual ~VoiceSequenceHost() = default;
    virtual bool input_action_pressed_004c43c0(std::uint32_t action) = 0;
    virtual bool widget_visible_vslot_38(GuiWidgetTransform&) = 0;
    // Pure alias: current [00E188A8]+21E4, then actual palette tree+10.
    virtual void* current_panel_palette_00e188a8_21e4_10() = 0;
    virtual void decoration_vslot_88(GuiWidgetTransform&, std::int32_t key,
        std::uint32_t zero, float one) = 0;
    virtual MissionLuaHostServices& current_mission_lua_1a08() = 0;
    // Pure alias of current [00E188A8]+21E4 and its required service bindings.
    virtual PanelSequenceContext& current_panel_sequence_00e188a8_21e4() = 0;
    virtual void log_004254b0(const char* message) = 0;
};

struct VoiceSequenceContext {
    ScheduledVoiceContext& scheduled;
    VoiceSubtitleContext& subtitles;
    VoiceManagerUpdateHost& attached;
    VoiceSequenceHost& host;
    NativeStringStorage& strings;
    // Native0044EC00 miss payload is residual stack storage, not a default
    // color. Explicit incoming words are used only on an actual missing key.
    const PanelPaletteValueWords& missing_palette_words;
    const SingletonLifetimeCallbacks& palette_callbacks;
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
