#pragma once

#include "bsp/voice_playback.hpp"
#include <cstdint>

namespace bsp {

// Semantic projection of the two bases proved by 005B7290/005BC87C:
// slot at native+0; callback-owner base at+18, attached entity at+2C.
// Other callback-owner storage remains with its required native host adapter.
struct VoiceAttachedEntry {
    VoicePlaybackSlot slot_00;
    std::uint32_t lifetime_vtable_18{};
    void* attached_entity_2c{};
};
struct VoiceAttachedNode {
    VoiceAttachedNode* next_00{};
    VoiceAttachedNode* previous_04{};
    VoiceAttachedEntry* entry_08{};
};
// Manager+64 is the stateless allocator, +68 sentinel, +6C count. Bind to
// canonical storage, including the existing count formerly named blocked_6c.
struct VoiceAttachedQueueView {
    VoiceAttachedNode*& sentinel_68;
    std::uint32_t& count_6c;
};
struct VoiceSoundVolumeView {
    float& volume_24;
    std::uint8_t& dirty_14; // canonical SoundLevelEntry dirty byte
};
struct VoiceAlternateChannelsView {
    void*& channel_08;
    void*& channel_0c;
};

class VoiceManagerUpdateHost {
public:
    virtual ~VoiceManagerUpdateHost() = default;
    // Pure aliases into real storage, with no game callbacks of their own.
    virtual VoiceSoundVolumeView sound_volume_view(void* sound) = 0;
    virtual VoiceAlternateChannelsView alternate_channels_00f8bbcc() = 0;
    // Native engine primitives remain required; no fake audio/observer objects.
    virtual void set_alternate_channel_volume_00a78330(void* channel,
        const NativeString& name, float volume) = 0;
    virtual void stop_alternate_00a78620() = 0; // reload CURRENT00F8BBCC
    virtual float relayout_step_00432650() = 0; // getter, then returned owner+80
    // Actual virtual dispatch at005BC70E. Parent binds the sibling's concrete
    // scalar_delete_voice_line_005b9b80 for canonical vtable00CF0ED4 only.
    virtual void scalar_delete_line_005bc70e(VoiceLine*, std::uint8_t flag) = 0;
    virtual void free_line_node_005bc711(VoiceLineNode*) = 0;
    virtual void free_attached_node_005bc868(VoiceAttachedNode*) = 0;
    // ECX=entity and EDX=entry's actual callback-owner base at native+18.
    virtual void unregister_attached_006952a0(void* entity, VoiceAttachedEntry&) = 0;
    // ECX=that same actual+18 base; includes its remaining unprojected storage.
    virtual void destroy_attachment_00695870(VoiceAttachedEntry&) = 0;
    virtual void free_attached_entry_005bc8c0(VoiceAttachedEntry*) = 0;
    virtual void invalid_iterator_00bf6713() = 0; // native CRT policy, no fallback
    virtual std::uint32_t external_activity_00f8a0c4_e8() = 0;
};

struct VoiceManagerUpdateContext {
    VoiceLineHost& lines;
    VoiceManagerUpdateHost& calls;
    NativeStringStorage& strings;
    VoiceAttachedQueueView attached;
};

// ECX=alternate engine, name*/float stack, RET8; two channel loads/calls.
void set_named_voice_volume_00701870(VoiceAlternateChannelsView,
    const NativeString&, float volume, VoiceManagerUpdateHost&);
// ECX=slot, float stack, RET4. Aux first; main if state1, otherwise named route.
void set_voice_slot_volume_00702130(VoicePlaybackSlot&, float volume,
    VoiceManagerUpdateHost&);
// ECX=slot, RET. Reload references after stop callbacks; state after aux release.
void stop_voice_slot_007026f0(VoicePlaybackSlot&, VoiceSlotHost&,
    VoiceManagerUpdateHost&);
// ECX=entry, RET, bool in AL. Poll then spatial attenuation/admission or stop.
bool update_attached_voice_005b7290(VoiceAttachedEntry&, VoiceLineHost&,
    VoiceManagerUpdateHost&);
// ECX=manager, float delta stack, RET4, activity DWORD in EAX. Complete loops,
// including bytes hidden by incorrect _free no-return overrides. Current and
// captured successor nodes/entries must survive until their next native use.
// No native ABI/SEH or signaling-NaN/x87-status compatibility is claimed.
bool update_voice_manager_005bc640(VoicePlaybackManager&, float delta,
    VoiceManagerUpdateContext&);

} // namespace bsp
