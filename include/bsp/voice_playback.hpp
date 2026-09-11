#pragma once

#include "bsp/native_string.hpp"
#include "bsp/scheduled_voice_types.hpp"
#include "bsp/panel_sequence_types.hpp"
#include "bsp/panel_publication_types.hpp"
#include "bsp/panel_owner_types.hpp"

#include <array>
#include <cstdint>
#include <optional>
#include <vector>

namespace bsp {

struct GuiWidgetTransform;
struct GuiTextWidget;
struct VoiceSubtitleContext;
struct VoiceFadeBindings;
struct VoiceAttachedNode;

// Typed projections, not native binary layouts. Clip12 is {vtable, record*,
// resource index}; record begins {NativeString, signed sound id}. Records and
// resource array references are borrowed. Native lifetime stays with the owner.
struct VoiceClipRecord {
    NativeString text_00;
    std::int32_t sound_id_08{};
    NativeString auxiliary_0c; // initialized/copied/destroyed by00703FF0/42B0/4060
    std::uint8_t alternate_14{};
    NativeString alternate_name_18;
    std::vector<void*> resources_20; // resource wrappers, not SoundOwnedResource
    VoiceTimedKeys timed_keys_34; // canonical record+30 vector, native begin+34
};
struct VoiceClip {
    std::uint32_t native_vtable_00{0x00cf0dd0};
    const VoiceClipRecord* record_04{};
    std::uint32_t word_08{}; // index into record+20 on the ordinary branch
};
using VoiceClips = std::vector<VoiceClip>;

// 18h native slot at manager+8. The loop limit is ONE, despite the stride.
struct VoicePlaybackSlot {
    std::int32_t state_00{};
    void* sound_04{}; // intrusive reference; virtual +8 stop, +C completion
    void* auxiliary_08{}; // intrusive reference; virtual +8(0), then reset
    NativeString alternate_name_0c; // owned; explicit 005B7FC0 teardown
    float started_at_14{};
};

class VoiceSlotHost {
public:
    virtual ~VoiceSlotHost() = default;
    virtual float mission_clock_00f876a4() = 0;
    virtual bool sound_completed_vslot_0c(void*) = 0;
    virtual float sound_duration_00a81860(void*) = 0; // sound+4C -> returned+2C
    virtual void stop_sound_vslot_08(void*, std::uint32_t flag) = 0;
    virtual void release_reference(void*) noexcept = 0; // InterlockedDecrement/+0
    virtual void stop_auxiliary_vslot_08(void*, std::uint32_t flag) = 0;
    virtual void reset_auxiliary_0054d510(void*& field, void* replacement) = 0;
    virtual void set_sound_flag_00a7d120(std::uint32_t flag) = 0;
    virtual bool alternate_playing_00a77730() = 0; // this = [00F8BBCC]
};

struct VoiceLine {
    std::uint32_t native_vtable_00{0x00cf0ed4};
    VoiceClips clips_04;
    void* widget_14{};
    std::int32_t slot_index_18{-1};
    std::optional<std::uint32_t> clip_index_1c;
    std::array<float, 3> layout_20{};
    std::uint32_t target_2c{};
    void* shortcut_widget_34{};
};
struct VoiceLineNode {
    VoiceLineNode* previous_00{};
    VoiceLineNode* next_04{};
    VoiceLine* line_08{};
};
struct VoiceLineQueue {
    std::uint32_t count_00{};
    VoiceLineNode* first_04{};
    VoiceLineNode* last_08{};
};
struct VoicePlaybackManager {
    VoicePlaybackSlot slot_08;
    GuiWidgetTransform* panel_28{};
    GuiWidgetTransform* template_2c{};
    GuiWidgetTransform* group_30{};
    GuiTextWidget* text_34{};
    GuiTextWidget* text_38{};
    GuiWidgetTransform* background_3c{};
    GuiWidgetTransform* decoration_40{};
    std::array<float, 4> base_color_44{};
    VoiceLineQueue lines_54;
    std::uint8_t dirty_60{};
    VoiceAttachedNode* attached_sentinel_68{};
    std::uint32_t attached_count_6c{};
    std::int32_t slot_index_70{-1};
    const VoiceClipRecord* pending_record_74{};
    float initial_78{};
    float per_character_7c{};
    float base_80{};
    std::uint32_t selected_row_84{0xffffffffu};
    float delay_88{};
    float hold_8c{};
    VoiceScheduledRows rows_94;
    std::array<void*, 5> speaker_table_a0{}; // borrowed intrusive references
    float fade_value_d4{};
    float fade_target_d8{};
    float fade_rate_dc{};
    NativeString fade_callback_e0;
};
struct VoicePanelState {
    // Initialized by the owner allocation preparation/construction bodies.
    // Its palette header is real Win32 storage and must keep a stable address.
    PanelOwnerNativeStorage owner_native;
    PanelCharacterMap characters_04;
    void* palette_10{}; // actual initialized Win32 palette tree; owner-managed
    PanelSequenceQueue queued_1c;
    std::uint32_t field_24{};
    NativeString current_28;
    float default_pause_30{};
    std::uint32_t field_34{};
};
struct VoiceSlotStartContext;

// Required game services at the named native callsites. They must implement
// the stated effects; no fallback, fake FMOD or implicit success exists here.
// Small native intrusive-reference boundary shared by tracked sound arrays.
// Full voice services retain this same interface; no second reference store.
class VoiceReferenceHost {
public:
    virtual ~VoiceReferenceHost() = default;
    virtual void retain_reference(void*) = 0;
    virtual void release_reference(void*) noexcept = 0;
};
class VoiceLineHost : public VoiceSlotHost, public VoiceReferenceHost {
public:
    virtual void release_reference(void*) noexcept override = 0;
    virtual std::int32_t local_side_18cc_18ec() = 0;
    virtual std::int32_t speaker_side_54(void* speaker) = 0;
    virtual bool speaker_kind_vslot_5c(void* speaker, std::uint32_t kind) = 0;
    virtual void notify_005a2650(std::uint32_t target, void* speaker,
        std::uint32_t flag) = 0;
    virtual VoiceLine* allocate_line_00bf681b(std::uint32_t native_bytes) = 0;
    // Native dereferences a null node allocation. This contract requires a
    // valid fresh node (or throws) and leaves lifetime with the queue owner.
    virtual VoiceLineNode& allocate_node_00bf681b(std::uint32_t native_bytes) = 0;
    virtual void retain_reference(void*) = 0; // InterlockedIncrement at +4
    virtual std::uint32_t invalid_target_00e188d8() = 0;
    virtual bool target_valid_00645160(std::uint32_t target, bool flag) = 0;
    // Side-effect-free accessors expose live real owners and services. Resolve
    // the global manager separately at every recovered native load site.
    virtual VoicePlaybackManager& current_voice_manager_00e198c4_a4() = 0;
    virtual VoiceSubtitleContext& subtitle_context() = 0;
    virtual VoiceSlotStartContext& slot_start_context() = 0;
    virtual void log_004254b0(const char* format, const char* text) = 0;
    // These perform the native dirty-bit tests and refresh calls, returning
    // float camera+120/124/128 and entity+FC/100/104 coordinates respectively.
    virtual std::array<float, 3> camera_position_00b6db70() = 0;
    virtual std::array<float, 3> entity_position_00414db0(void* entity) = 0;
    virtual double vector_length_0042b2f0(const std::array<float, 3>&) = 0;
    virtual float global_sound_level_6c() = 0;
    virtual VoiceFadeBindings& fade_bindings() = 0;
};

// Native ECX=slot, RET; polls AND may stop/release/reset the slot.
std::int32_t poll_voice_slot_007027b0(VoicePlaybackSlot&, VoiceSlotHost&);
// Native ECX=manager, vector* stack, RET4. Each clip polls slot+8 afresh;
// empty input bypasses polling, but still observes manager+6C.
bool voice_can_play_005b71d0(VoicePlaybackManager&, const VoiceClips&,
    const VoicePanelState&, VoiceSlotHost&);
std::uint32_t classify_voice_speaker_005bbc10(void* speaker, VoiceLineHost&);
// Native ECX=queue(+54), line* stack, RET4. Nullable line is appended as-is.
void append_voice_line_005b7790(VoiceLineQueue&, VoiceLine*, VoiceLineHost&);
// Native ECX=fresh 38h line, vector*/target/ref-value stack, RET0C/EAX=this.
// bank owns ONE already-retained argument reference, consumed on return.
VoiceLine& construct_voice_line_005babb0(VoiceLine&,
    const VoiceClips&, std::uint32_t target, void* bank, VoiceLineHost&,
    NativeStringStorage&);
// Native ECX=manager, vector*/target/speaker stack, RET0C.
VoiceLine* play_voice_line_005bbc10(VoicePlaybackManager&, const VoiceClips&,
    std::uint32_t target, void* speaker, VoiceLineHost&, NativeStringStorage&);
// Native ECX=manager, Clip12 BY VALUE then entity*, RET10. Double host distance
// is a numerical projection of ST0, not bit-identical x87 extended precision.
bool play_positional_voice_line_005bbdc0(VoicePlaybackManager&, VoiceClip,
    void* entity, VoiceLineHost&, NativeStringStorage&);
// Native ECX=manager, target float/duration float/NativeString* stack, RET0C.
void begin_voice_sound_fade_005b9760(VoicePlaybackManager&, float target,
    float duration, const NativeString& callback, VoiceLineHost&,
    NativeStringStorage&);

} // namespace bsp
