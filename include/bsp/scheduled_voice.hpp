#pragma once

#include "bsp/scheduled_voice_types.hpp"
#include "bsp/voice_slot_start.hpp"

namespace bsp {

// Bind these references to the actual manager fields. No independent pending
// queue, record copy, mirrored index or duplicate row vector is permitted.
// This view allows integration without editing the shared manager header here.
struct ScheduledVoiceBindings {
    std::int32_t& slot_index_70;
    const VoiceClipRecord*& pending_record_74;
    std::uint32_t& selected_row_84;
    VoiceScheduledRows& rows_94;
};

class ScheduledVoiceHost {
public:
    virtual ~ScheduledVoiceHost() = default;
    // Side-effect-free projection of this SAME native record's vector at
    // +30 (begin+34/end+38); records and vector storage remain owner-borrowed.
    // Never look up by text/id or return data from a copied record identity.
    virtual const VoiceTimedKeys& timed_keys_34(const VoiceClipRecord&) = 0;
};
struct ScheduledVoiceContext {
    VoicePlaybackManager& manager;
    ScheduledVoiceBindings fields;
    ScheduledVoiceHost& records;
    VoiceLineHost& playback;
};

// Full005B9300. ECX=manager, borrowed record* stack, RET4. Sound ID>=0
// starts Clip12{CF0DD0,&record,0} with null bank, using current manager+70.
// Only afterward resolve current selected row; write slot and key scheduling.
// Nonempty keys reset record/index/time; empty keys only clear active byte.
void start_scheduled_voice_record_005b9300(ScheduledVoiceContext&,
    const VoiceClipRecord&);

// Full005B93D0. ECX=manager, borrowed record* stack, RET4. Playable records
// poll ONLY slot0, write+70=0/-1, and defer into+74 if busy. Negative-ID
// records go directly to start without changing+70 or clearing existing+74.
void admit_scheduled_voice_record_005b93d0(ScheduledVoiceContext&,
    const VoiceClipRecord&);

// Full005B9420. ECX=manager, RET; result in AL only. Pending entry returns
// true even after successful start; pending is reloaded after poll and cleared
// after start callbacks. No-pending branch polls the captured nonnegative+70
// slot, clears+70 only when that poll finishes, and returns its busy status.
bool poll_scheduled_voice_005b9420(ScheduledVoiceContext&);

} // namespace bsp
