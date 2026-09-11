#pragma once

#include "bsp/scheduled_voice.hpp"
#include "bsp/voice_subtitles.hpp"

#include "bsp/panel_sequence_types.hpp"
#include "bsp/message_record_resolver.hpp"

namespace bsp {
struct MissionLuaHostServices;
struct PanelPublicationContext;
struct MessageRecordResolverContext;

// Alias the SAME native owner at game+21E4. In particular count and state
// must alias the VoicePanelState gates observed by voice_can_play_005B71D0.
// No implicit cleanup: queue owners must erase their entries through the
// recovered cleanup path before destroying the standard-container projection.
struct PanelSequenceView {
    PanelSequenceQueue& queued_1c;
    std::uint32_t& queued_count_24;
    NativeString& current_28;
    std::uint32_t& state_34;
};
struct PanelSequenceIterator {
    PanelSequenceQueue* owner;
    PanelSequenceQueue::iterator position;
};

class PanelSequenceHost {
public:
    virtual ~PanelSequenceHost() = default;
    // Pure projections: resolve native globals at each corresponding load.
    virtual VoicePlaybackManager& current_voice_manager_00e198c4_a4() = 0;
    virtual ScheduledVoiceContext scheduled_context(VoicePlaybackManager&) = 0;
    virtual MissionLuaHostServices& current_mission_lua_1a08() = 0;
    virtual std::uint8_t& callback_guard_00e17bfa() = 0;
    // Current native owner bindings only. The caller executes the recovered
    // publication and lookup/create bodies directly.
    virtual PanelPublicationContext& panel_publication_context() = 0;
    virtual MessageRecordStore& current_message_record_store_00e188a8_21dc() = 0;
    virtual MessageRecordResolverContext& message_record_resolver_context() = 0;
};
struct PanelSequenceContext {
    PanelSequenceView owner;
    PanelSequenceHost& host;
    NativeStringStorage& strings;
    VoiceSubtitleContext& subtitles;
    // Actual incoming bits of native stackfloat+18 at0044C390: native does
    // not initialize this until a candidate wins. Required external input,
    // read only when a -1e10 priority ties before the first winning candidate.
    const std::uint32_t& selection_scratch_18;
};

// Original ABI for all owner helpers: ECX=this; RET unless stated otherwise.
void destroy_panel_slot_0044b530(PanelSequenceSlot&, NativeStringStorage&);
void destroy_panel_entry_0044fff0(PanelSequenceEntry&, NativeStringStorage&);
void destroy_panel_pair_00450110(NativeString&, PanelSequenceEntry&, NativeStringStorage&);
PanelSequenceIterator find_panel_entry_0044ba00(PanelSequenceView, const NativeString&); // hidden iterator*,key*; RET8
PanelSequenceIterator erase_panel_entry_00451020(PanelSequenceView, PanelSequenceIterator,
    NativeStringStorage&); // hidden iterator*,by-value iterator; RET0C
PanelSequenceEntry& lookup_panel_entry_00451920(PanelSequenceView, const NativeString&,
    NativeStringStorage&); // key*; RET4
void select_panel_sequence_0044c390(PanelSequenceContext&);
void consume_panel_commands_00452360(PanelSequenceContext&);
void step_panel_sequence_00452740(PanelSequenceContext&);
bool advance_panel_sequence_004527f0(PanelSequenceContext&); // bool in AL

// ECX=voice manager. Reset005B8A00 index stack RET4, all005B9490 RET.
// Rechecks rows on every iteration; does not clear their scheduling fields.
void reset_voice_panel_row_005b8a00(VoicePlaybackManager&, std::uint32_t,
    VoiceSubtitleContext&, NativeStringStorage&);
void reset_voice_panel_rows_005b9490(VoicePlaybackManager&,
    VoiceSubtitleContext&, NativeStringStorage&);
// ECX=voice manager, row index/record*/delay float stack; RET0C.
void begin_panel_voice_record_005b94d0(ScheduledVoiceContext&, std::uint32_t,
    const VoiceClipRecord&, float delay, VoiceSubtitleContext&);
// ECX=panel owner, entry*/slot index/NativeString* stack; RET0C.
void request_panel_voice_004483f0(PanelSequenceContext&, PanelSequenceEntry&,
    std::uint32_t slot, const NativeString&);

} // namespace bsp
