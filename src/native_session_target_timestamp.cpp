#include "bsp/native_session_target_timestamp.hpp"
#include "bsp/frame_clock.hpp"

namespace bsp {
void refresh_native_session_target_timestamp_007839a0(
    NativeSessionTargetStorage* target,
    const NativeFrameClockPublicationContext& clock,
    const ClockTimestamp& actual_sample_stack_preimage) {
    ClockTimestamp output = actual_sample_stack_preimage;
    const ClockTimestamp* const returned = sample_published_native_frame_clock(clock, &output);
    volatile float* const destination = &target->timestamp_d68;
    //007839B8..007839C5. No C++ floating expression or intermediate spill.
    __asm {
        mov eax, returned
        mov edx, destination
        fild qword ptr [eax]
        fild qword ptr [eax + 8]
        fdivp st(1), st(0)
        fstp dword ptr [edx]
    }
}
} // namespace bsp
