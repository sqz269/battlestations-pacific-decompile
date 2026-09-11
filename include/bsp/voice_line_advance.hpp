#pragma once

#include "bsp/voice_playback.hpp"

namespace bsp {

// 005B91E0..005B92F3, ECX=line, RET, AL=still active. Typed projection.
// An active line must have an initialized clip_index_1c and a valid native-size
// clip vector. Polling can reenter and mutate line or replace the global manager;
// every loaded manager/line/record must remain valid for its observed use.
bool advance_voice_line_005b91e0(VoiceLine&, VoiceLineHost&);

} // namespace bsp
