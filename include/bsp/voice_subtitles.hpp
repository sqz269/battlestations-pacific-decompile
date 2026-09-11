#pragma once

#include "bsp/gui_text.hpp"
#include "bsp/gui_widget_scene.hpp"
#include "bsp/voice_playback.hpp"

namespace bsp {

// Aliases into the CURRENT voice manager at [[00E198C4]+A4]. The GUI
// projections are the existing canonical types, not new widget objects. Bind
// last_line_5c to VoicePlaybackManager::lines_54.last_08. A returned view keeps
// its manager identity, but observes pointer/field changes made by GUI calls.
struct VoiceSubtitleManagerView {
    GuiWidgetTransform*& template_2c;
    GuiWidgetTransform*& group_30;
    GuiTextWidget*& text_34;
    GuiTextWidget*& text_38;
    GuiWidgetTransform*& background_3c;
    GuiWidgetTransform*& decoration_40;
    VoiceLineNode*& last_line_5c;
    std::uint8_t& dirty_60;
    float& initial_78;
    float& per_character_7c;
    float& base_80;
};

// Bind one current canonical manager without duplicating its queue or GUI
// state. The host resolves the global manager at each native reload site.
inline VoiceSubtitleManagerView voice_subtitle_manager_view(
    VoicePlaybackManager& manager) noexcept {
    return {manager.template_2c, manager.group_30, manager.text_34,
        manager.text_38, manager.background_3c, manager.decoration_40,
        manager.lines_54.last_08, manager.dirty_60, manager.initial_78,
        manager.per_character_7c, manager.base_80};
}

class VoiceSubtitleHost {
public:
    virtual ~VoiceSubtitleHost() = default;
    // Data/projection accessors, not native calls. They must only expose live
    // storage; text_host must bind the actual supplied widget's GUI services.
    virtual VoiceSubtitleManagerView manager_00e198c4_a4() = 0;
    virtual bool subtitles_enabled_00f88989() = 0;
    virtual GuiTextHost& text_host(GuiTextWidget&) = 0;

    // One boundary per remaining dispatched/native call. No default behavior.
    virtual void set_visible_vslot_34(GuiWidgetTransform&, bool) = 0;
    virtual void set_size_vslot_58(GuiWidgetTransform&, const GuiWidgetSize&) = 0;
    virtual GuiWidgetTransform* find_child_00aa7e00(GuiWidgetTransform&,
        const NativeString& name, bool recursive) = 0;
    // ECX=text, char* literal/flag stack, RET8. Owned by the locale packet.
    virtual void set_literal_00abbe50(GuiTextWidget&, const char*, bool) = 0;
    // ECX=text, RET, ST0; native callee spills/reloads a float before return.
    virtual float normalized_height_00ab6bd0(GuiTextWidget&) = 0;
};

struct VoiceSubtitleContext {
    VoiceSubtitleHost& calls;
    GuiWidgetSceneHost& scene;
    GuiWidgetTransformHost& transform;
};

// 005B8510..005B880B. Native __thiscall(38h line, NativeString*), RET4.
// Reconstructs the full call sequence using canonical GUI clone, localised
// text and transform implementations. Line widget void* fields must refer to
// GuiWidgetTransform projections. Required pointers are not null-guarded.
// MSVC Win32 x87 arithmetic retains double operands and float spill sites;
// the surrounding typed projections are not native binary layouts.
void display_voice_subtitles_005b8510(VoiceLine&, const NativeString&,
    VoiceSubtitleContext&, NativeStringStorage&);

} // namespace bsp
