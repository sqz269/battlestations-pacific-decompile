// Process-static GUI widget pools and their stubs (cc13-gui1, docs/CC13_GUI_1.md).
// The pool members themselves are instantiated in src/native_material_pools.cpp
// over the shared native engine.
#include "bsp/native_gui_widget_pools_cc13.hpp"
#include "bsp/cc13_seg87.hpp"
#include "bsp/native_gui_curve_pool_table.hpp"
#include "bsp/native_gui_scrollbar_pool_table.hpp"
#include "bsp/game_native_physical_pool.hpp"

#include <cstdlib>
#include <new>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native GUI widget pools require MSVC Win32.
#endif

namespace bsp::game {
namespace {
template <class T>
void construct_process_pool(NativeGuiWidgetPool<T>& pool) {
    pool.initialize();
}

void construct_process_pool(NativeGuiListboxPool& pool) {
    // bsp.py show 00A9B4A0: construct the retained F8BC10 pool on the
    // E188B4 allocator list, including its real CS and reserve(32).
    // CD6FF0 calls this entry before registering the CE0A00 exit stub.
    construct_native_listbox_pool_00a9b4a0(pool);
}

template <class T>
void destroy_process_pool(NativeGuiWidgetPool<T>& pool) {
    pool.destroy();
}

void destroy_process_pool(NativeGuiListboxPool& pool) {
    // bsp.py show 00CE0A00 / 00A9AFD0: the exit stub destroys this SAME
    // pool, releasing slabs/table/CS before unlinking the allocator node.
    destroy_native_listbox_pool_00a9afd0(pool);
}
} // namespace

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
    construct_process_pool(pool_);
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
    destroy_process_pool(instance().pool_);
}
} // namespace bsp::game

namespace bsp {
__declspec(noinline) void native_gui_scrollbar_pool_table_set_capacity_00ac7f40(
    NativeGuiPoolSlabTable& table, std::uint32_t capacity) {
    // AC7F40 is identical to AA6FA0: publish capacity before new(capacity*4),
    // copy current entries with null-tested destinations, free, then adopt.
    native_gui_pool_table_set_capacity_00aa6fa0(table, capacity);
}

__declspec(noinline) void native_gui_scrollbar_pool_table_grow_00ac7f90(
    NativeGuiPoolSlabTable& table) {
    // AC7F90 inlines that same schedule with modulo32 capacity*2+2.
    native_gui_scrollbar_pool_table_set_capacity_00ac7f40(
        table, table.capacity * 2u + 2u);
}

__declspec(noinline) void native_gui_scrollbar_pool_table_push_back_00ac8110(
    NativeGuiPoolSlabTable& table, std::byte* const& slab) {
    // AC8110 and AC83C0's inline append use the actual pool+28h header,
    // after first-free publication and slab initialization, under its lock.
    if (table.count == table.capacity)
        native_gui_scrollbar_pool_table_grow_00ac7f90(table);
    const auto destination = reinterpret_cast<std::uintptr_t>(table.data) + table.count * 4u;
    if (destination != 0)
        ::new (reinterpret_cast<void*>(destination)) std::byte*(slab);
    ++table.count;
}

__declspec(noinline) void native_gui_scrollbar_pool_table_reserve_00ac81a0(
    NativeGuiPoolSlabTable& table, std::uint32_t capacity) {
    // AC81A0: unsigned comparison; AC8240 inlines reserve(32) in EH state2.
    if (table.capacity < capacity)
        native_gui_scrollbar_pool_table_set_capacity_00ac7f40(table, capacity);
}

__declspec(noinline) void native_gui_curve_pool_table_set_capacity_00abc1e0(
    NativeGuiPoolSlabTable& table, std::uint32_t capacity) {
    // Evidence: bsp.py show 00ABC1E0. Reuse the existing identical AA6FA0
    // capacity-before-allocation, null-tested copy, free, adopt schedule.
    native_gui_pool_table_set_capacity_00aa6fa0(table, capacity);
}

__declspec(noinline) void native_gui_curve_pool_table_grow_00abc230(
    NativeGuiPoolSlabTable& table) {
    // 00ABC230 inlines the same body with modulo32 capacity*2+2.
    native_gui_curve_pool_table_set_capacity_00abc1e0(table, table.capacity * 2u + 2u);
}

__declspec(noinline) void native_gui_curve_pool_table_push_back_00abc480(
    NativeGuiPoolSlabTable& table, std::byte* const& slab) {
    // Evidence: bsp.py show 00ABC480 and 00ABC650. Called with the actual
    // newly initialized slab, after first-free publication, under its lock.
    if (table.count == table.capacity) native_gui_curve_pool_table_grow_00abc230(table);
    const auto destination = reinterpret_cast<std::uintptr_t>(table.data) + table.count * 4u;
    if (destination != 0)
        ::new (reinterpret_cast<void*>(destination)) std::byte*(slab);
    ++table.count;
}

__declspec(noinline) void native_gui_curve_pool_table_reserve_00abc500(
    NativeGuiPoolSlabTable& table, std::uint32_t capacity) {
    // 00ABC570's constructor inlines this unsigned reserve in unwind state 2.
    if (table.capacity < capacity)
        native_gui_curve_pool_table_set_capacity_00abc1e0(table, capacity);
}

__declspec(noinline) void reserve_native_gui_icon_pool_table_00ab2420(
    NativeGuiPoolSlabTable& table, std::uint32_t capacity) {
    // Evidence: bsp.py show 00AB2420 and 00AB2850. The Icon constructor's
    // reserve(32) is an inline copy, not an original CALL to this entry.
    // AA6FA0 supplies the same allocate/copy/free body, including DWORD size
    // wrapping and the null-tested destination; pass the retained actual table.
    if (table.capacity < capacity)
        native_gui_pool_table_set_capacity_00aa6fa0(table, capacity);
}

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
BSP_GUI_WIDGET_POOL_PROCESS(GuiListboxPoolTraits)
#undef BSP_GUI_WIDGET_POOL_PROCESS
} // namespace bsp
