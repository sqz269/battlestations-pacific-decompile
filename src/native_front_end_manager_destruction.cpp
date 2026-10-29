#include "bsp/native_front_end_manager_destruction.hpp"
#include "bsp/native_game_lifetime.hpp"

namespace bsp {
namespace {
void delete_current(void* volatile& publication, NativeGameLifetimeCalls& calls) {
    void* const captured = publication;
    if (captured) {
        calls.virtual_scalar(captured, 0, 1);
        publication = nullptr;
    }
}
} // namespace

void destroy_native_front_end_managers_004da650(NativeGameStorage& game,
    NativeFrontEndManagerDestructionContext& context) {
    context.required_calls.call_004d95f0(game, 0);
    delete_current(context.actual_main_menu_00e198ac, context.lifetime_calls);
    delete_current(context.actual_multi_menu_00e198b4, context.lifetime_calls);
    delete_current(context.actual_options_00e198b8, context.lifetime_calls);
    delete_current(context.actual_in_mission_00e198c4, context.lifetime_calls);
}
} // namespace bsp
