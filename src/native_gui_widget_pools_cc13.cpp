// Process-static GUI widget pools and their stubs (cc13-gui1, docs/CC13_GUI_1.md).
// The pool members themselves are instantiated in src/native_material_pools.cpp
// over the shared native engine.
#include "bsp/native_gui_widget_pools_cc13.hpp"
#include "bsp/game_native_physical_pool.hpp"

#include <cstdlib>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native GUI widget pools require MSVC Win32.
#endif

namespace bsp::game {
template <class T>
GameNativeGuiWidgetPoolProcess<T>::GameNativeGuiWidgetPoolProcess()
    : pool_(game_native_physical_pool_process().allocator_list_domain_00e188b4(), storage_) {}

template <class T>
GameNativeGuiWidgetPoolProcess<T>& GameNativeGuiWidgetPoolProcess<T>::instance() {
    static GameNativeGuiWidgetPoolProcess process;
    return process;
}

// The CRT initializer: MOV ECX, global; CALL construct; PUSH exit stub; CALL atexit.
template <class T>
int GameNativeGuiWidgetPoolProcess<T>::initialize_once() {
    std::lock_guard lock(startup_mutex_);
    if (state_ == State::returned) return registration_status_;
    if (state_ == State::threw) throw std::logic_error("GUI widget pool startup previously threw");
    state_ = State::threw;
    pool_.initialize();
    registration_status_ = std::atexit(&GameNativeGuiWidgetPoolProcess::destroy_at_exit);
    state_ = State::returned;
    return registration_status_;
}

template <class T>
NativeGuiWidgetPool<T>& GameNativeGuiWidgetPoolProcess<T>::pool() {
    std::lock_guard lock(startup_mutex_);
    if (state_ != State::returned)
        throw std::logic_error("GUI widget pool requires completed explicit startup");
    return pool_;
}

// The exit stub: MOV ECX, global; JMP destroy.
template <class T>
void GameNativeGuiWidgetPoolProcess<T>::destroy_at_exit() noexcept {
    instance().pool_.destroy();
}
} // namespace bsp::game

namespace bsp {
template <class T>
void* allocate_gui_widget_pool_slot() {
    return game::GameNativeGuiWidgetPoolProcess<T>::instance().pool().allocate();
}

template <class T>
void free_gui_widget_pool_slot(void* slot) {
    game::GameNativeGuiWidgetPoolProcess<T>::instance().pool().free(slot);
}

#define BSP_GUI_WIDGET_POOL_PROCESS(T)                                 \
    namespace game { template class GameNativeGuiWidgetPoolProcess<T>; } \
    template void* allocate_gui_widget_pool_slot<T>();                 \
    template void free_gui_widget_pool_slot<T>(void*);
BSP_GUI_WIDGET_POOL_PROCESS(GuiWidgetPoolTraits)
BSP_GUI_WIDGET_POOL_PROCESS(GuiButtonPoolTraits)
BSP_GUI_WIDGET_POOL_PROCESS(GuiMoviePoolTraits)
BSP_GUI_WIDGET_POOL_PROCESS(GuiIconPoolTraits)
BSP_GUI_WIDGET_POOL_PROCESS(GuiCurvePoolTraits)
BSP_GUI_WIDGET_POOL_PROCESS(GuiSectionPoolTraits)
BSP_GUI_WIDGET_POOL_PROCESS(GuiLinePoolTraits)
BSP_GUI_WIDGET_POOL_PROCESS(GuiScrollbarPoolTraits)
BSP_GUI_WIDGET_POOL_PROCESS(GuiGridPoolTraits)
BSP_GUI_WIDGET_POOL_PROCESS(GuiModelPoolTraits)
BSP_GUI_WIDGET_POOL_PROCESS(GuiSoundPoolTraits)
BSP_GUI_WIDGET_POOL_PROCESS(GuiClipBoxPoolTraits)
BSP_GUI_WIDGET_POOL_PROCESS(GuiFrameBoxPoolTraits)
BSP_GUI_WIDGET_POOL_PROCESS(GuiAnimIconPoolTraits)
#undef BSP_GUI_WIDGET_POOL_PROCESS
} // namespace bsp
