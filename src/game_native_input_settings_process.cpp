#include "bsp/game_native_input_settings_process.hpp"
#include "bsp/game_native_string_process.hpp"
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
