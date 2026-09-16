#include "bsp/native_frame_clock_publication.hpp"
#include "bsp/native_renderer_control_worker.hpp"
#include <stdexcept>

namespace bsp {
namespace {
void* capture_for_slot(const NativeFrameClockPublicationContext& context,
                      std::size_t index, std::uint32_t target) {
    void* const clock = context.actual_clock_01090ab0;
    if (!clock || !context.methods.profile_d68d50)
        throw std::logic_error("native frame-clock publication or profile is null");
    if (*static_cast<const volatile std::uint32_t*>(clock) != 0x00d68d50u)
        throw std::logic_error("unsupported native frame-clock object profile");
    if (context.methods.profile_d68d50[index] != target)
        throw std::logic_error("unsupported native frame-clock method target");
    return clock;
}
} // namespace

void advance_published_native_frame_clock(const NativeFrameClockPublicationContext& context) {
    update_native_frame_clock_00bedc30(capture_for_slot(context, 2, 0x00bedc30u));
}
const void* current_published_native_frame_clock(const NativeFrameClockPublicationContext& context) {
    return get_raw_timer_current_00bee050(capture_for_slot(context, 5, 0x00bee050u));
}
const void* interval_published_native_frame_clock(const NativeFrameClockPublicationContext& context) {
    return get_raw_timer_interval_00bee070(capture_for_slot(context, 7, 0x00bee070u));
}
ClockTimestamp* sample_published_native_frame_clock(
    const NativeFrameClockPublicationContext& context, ClockTimestamp* output) {
    void* const clock = capture_for_slot(context, 8, 0x00bee080u);
    if (!output)
        throw std::logic_error("native frame-clock sample output is null");
    return sample_native_frame_clock_00bee080(clock, nullptr, output);
}
void enable_fixed_published_native_frame_clock(
    const NativeFrameClockPublicationContext& context, std::int32_t milliseconds) {
    enable_fixed_native_frame_clock_00bedb20(
        capture_for_slot(context, 9, 0x00bedb20u), nullptr, milliseconds);
}
} // namespace bsp
