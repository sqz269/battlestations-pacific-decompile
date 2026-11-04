#include "bsp/game_native_input_settings_process.hpp"
#include "bsp/game_native_string_process.hpp"
#include "bsp/wire_goal4_chain389_switches.hpp"
#include <stdexcept>

namespace bsp::game {
GameNativeInputSettingsProcess::GameNativeInputSettingsProcess()
    :manager_(game_native_string_process().manager_01090aa0()){}
NativeGameSettingsInputReference GameNativeInputSettingsProcess::settings_reference() noexcept {
    return {publication_,manager_,context_};
}
void GameNativeInputSettingsProcess::bind_context(NativeInputSettingsLifetimeContext* context) {
    if(context) {
        if(&context->publication_00e198e8!=&publication_||
            &context->manager_publication_01090aa0!=&manager_)
            throw std::logic_error("input settings context uses different process publications");
        if(context_&&context_!=context)
            throw std::logic_error("input settings process already borrows another application context");
        if constexpr (kWireGoal4Chain389Bound) {
            auto& strings = game_native_string_process();
            // VFS has its own wrapper over the same process cells; compare
            // those cells, not the wrapper object's address. The tables, Lua
            // paths and destructor must borrow the SAME application pool.
            const auto* storage = dynamic_cast<const ActualNativeStringPoolStorage*>(
                &context->tables.scripts.strings);
            if (!storage || !storage->uses_raw_pool_cells(strings.raw_context()))
                throw std::logic_error("input settings cleanup uses a different string-pool domain");
            context->actual_raw_strings = &strings.raw_context();
        }
    }else if(publication_) {
        throw std::logic_error("input settings context cannot retire before native publication clears");
    }
    context_=context;
}
GameNativeInputSettingsProcess& game_native_input_settings_process() {
    static auto* const process=new GameNativeInputSettingsProcess;
    return *process;
}
} // namespace bsp::game
