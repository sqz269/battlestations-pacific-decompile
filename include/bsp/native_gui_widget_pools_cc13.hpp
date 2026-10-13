#pragma once
// The fourteen fixed-slot GUI widget pools of segments 76-79 (packet cc13-gui1,
// docs/CC13_GUI_1.md). Every one is an instantiation of the same Mit slab pool
// template the GUI Group (00AC7410, include/bsp/native_gui_group_storage.hpp) and
// GUI layer pools use: a 38h header (allocator-list element, CRITICAL_SECTION,
// recursion count, slab pointer table, count, capacity, first slab with a free
// slot), slabs of N slots each followed by a uint16 free stack and a uint16
// free count, and the owning slab index in the dword that follows each payload.
//
// Evidence: tools-free listing comparison (local scripts, see the doc) of every
// member against the Widget pool after replacing immediates: Allocate, Construct,
// Destroy and FreeThunk are instruction-for-instruction identical; Free differs
// only in the division-by-stride sequence (IMUL magic, or SAR for 100h), InitBlock
// and Trim only in loop strength reduction and constants. The constants below are
// read from each Allocate (slab size, free stack, free count), Construct (vtable)
// and FreeThunk / AllocatePoolBlock stub (global).
//
// Status: reconstructed over the shared pool engine in src/native_material_pools.cpp
// and build-tested; the payloads are never constructed or destroyed here (the
// native pools do not either). Not game-validated.
#include "bsp/allocator_list.hpp"
#include "bsp/native_material_pools.hpp"

#include <cstddef>
#include <cstdint>
#include <mutex>

namespace bsp {

using NativeGuiWidgetPoolStorage = NativeMaterialPoolStorage;

// One row per pool. object_bytes is the widget size the type-table stub
// allocates (the slot minus the trailing slab-index dword).
#define BSP_GUI_WIDGET_POOL_TRAITS(Name, Global, Vtable, Slot, Slots, Construct, Allocate, \
                                   Free, Trim, Destroy, InitBlock, FreeThunk, Stub)         \
    struct Name {                                                                            \
        static constexpr std::uint32_t native_global = Global;                               \
        static constexpr std::uint32_t native_vtable = Vtable;                               \
        static constexpr std::uint32_t native_virtual0 = Trim;                               \
        static constexpr std::size_t slot_bytes = Slot;                                      \
        static constexpr std::size_t object_bytes = Slot - 4;                                \
        static constexpr std::size_t slot_slab_index_offset = Slot - 4;                      \
        static constexpr std::uint32_t slots_per_slab = Slots;                               \
        static constexpr std::size_t free_indices_offset = Slot * Slots;                     \
        static constexpr std::size_t free_count_offset = Slot * Slots + 2 * Slots;           \
        static constexpr std::size_t slab_bytes = Slot * Slots + 2 * Slots + 4;              \
        static constexpr std::uint32_t construct_address = Construct;                        \
        static constexpr std::uint32_t allocate_address = Allocate;                          \
        static constexpr std::uint32_t free_address = Free;                                  \
        static constexpr std::uint32_t trim_address = Trim;                                  \
        static constexpr std::uint32_t destroy_address = Destroy;                            \
        static constexpr std::uint32_t init_block_address = InitBlock;                       \
        static constexpr std::uint32_t free_thunk_address = FreeThunk;                       \
        static constexpr std::uint32_t allocate_stub_address = Stub;                         \
    }

// The Widget pool's two stubs (00AA7880 free thunk, 00AA7CF0 allocate stub) are
// real code in INT3 padding that Ghidra has not made functions (open item).
//                         name                     global      vtable      slot   n   construct   allocate    free        trim        destroy     init block  free thunk  alloc stub
BSP_GUI_WIDGET_POOL_TRAITS(GuiWidgetPoolTraits,     0x00F8BC94, 0x00D5C120, 0x0F0, 64, 0x00AA79A0, 0x00AA7A80, 0x00AA75F0, 0x00AA7550, 0x00AA7470, 0x00AA6CA0, 0x00AA7880, 0x00AA7CF0);
BSP_GUI_WIDGET_POOL_TRAITS(GuiButtonPoolTraits,     0x00F8BD00, 0x00D5C238, 0x100, 64, 0x00AABE00, 0x00AABF80, 0x00AABAB0, 0x00AABEE0, 0x00AAB9F0, 0x00AAB680, 0x00AABD60, 0x00AAC260);
BSP_GUI_WIDGET_POOL_TRAITS(GuiMoviePoolTraits,      0x00F8BD4C, 0x00D5C24C, 0x134,  4, 0x00AAD5F0, 0x00AAD6D0, 0x00AAD1C0, 0x00AAD120, 0x00AAD040, 0x00AACA00, 0x00AAD550, 0x00AAD990);
BSP_GUI_WIDGET_POOL_TRAITS(GuiIconPoolTraits,       0x00F8BDA0, 0x00D5C3B0, 0x13C, 64, 0x00AB2850, 0x00AB2930, 0x00AB2150, 0x00AB20B0, 0x00AB1FD0, 0x00AB1340, 0x00AB24A0, 0x00AB2B60);
BSP_GUI_WIDGET_POOL_TRAITS(GuiCurvePoolTraits,      0x00F8BE5C, 0x00D5C7CC, 0x144, 64, 0x00ABC570, 0x00ABC650, 0x00ABC410, 0x00ABC370, 0x00ABC290, 0x00ABC0A0, 0x00ABC560, 0x00ABC790);
BSP_GUI_WIDGET_POOL_TRAITS(GuiSectionPoolTraits,    0x00F8BEAC, 0x00D5C880, 0x130,  8, 0x00ABEE60, 0x00ABEF40, 0x00ABEC40, 0x00ABEBA0, 0x00ABEAA0, 0x00ABE890, 0x00ABED90, 0x00ABF080);
BSP_GUI_WIDGET_POOL_TRAITS(GuiLinePoolTraits,       0x00F8BEFC, 0x00D5C9D8, 0x108, 32, 0x00AC2160, 0x00AC2240, 0x00AC1F30, 0x00AC1E90, 0x00AC1DB0, 0x00AC1BE0, 0x00AC2080, 0x00AC2380);
BSP_GUI_WIDGET_POOL_TRAITS(GuiScrollbarPoolTraits,  0x00F8C004, 0x00D5CD78, 0x100, 16, 0x00AC8240, 0x00AC83C0, 0x00AC80A0, 0x00AC8320, 0x00AC7FE0, 0x00AC7E40, 0x00AC8200, 0x00AC84F0);
BSP_GUI_WIDGET_POOL_TRAITS(GuiGridPoolTraits,       0x00F8C050, 0x00D5CE08, 0x128, 16, 0x00AC9B30, 0x00AC9C10, 0x00AC9640, 0x00AC95A0, 0x00AC94C0, 0x00AC90F0, 0x00AC98F0, 0x00AC9EA0);
BSP_GUI_WIDGET_POOL_TRAITS(GuiModelPoolTraits,      0x00F8C09C, 0x00D5CEF0, 0x120,  8, 0x00ACC1D0, 0x00ACC350, 0x00ACC050, 0x00ACC2B0, 0x00ACBF70, 0x00ACBCF0, 0x00ACC1C0, 0x00ACC490);
BSP_GUI_WIDGET_POOL_TRAITS(GuiSoundPoolTraits,      0x00F8C0EC, 0x00D5CFBC, 0x10C, 16, 0x00ACD530, 0x00ACD6B0, 0x00ACD3B0, 0x00ACD610, 0x00ACD2F0, 0x00ACD0F0, 0x00ACD520, 0x00ACD7F0);
BSP_GUI_WIDGET_POOL_TRAITS(GuiClipBoxPoolTraits,    0x00F8C13C, 0x00D5D0D4, 0x110, 16, 0x00ACE750, 0x00ACE8D0, 0x00ACE4A0, 0x00ACE830, 0x00ACE3E0, 0x00ACE230, 0x00ACE610, 0x00ACEA10);
BSP_GUI_WIDGET_POOL_TRAITS(GuiFrameBoxPoolTraits,   0x00F8C188, 0x00D5D0FC, 0x120, 16, 0x00ACFF70, 0x00AD0050, 0x00ACF610, 0x00ACF570, 0x00ACF490, 0x00ACED50, 0x00ACF8E0, 0x00AD0280);
BSP_GUI_WIDGET_POOL_TRAITS(GuiAnimIconPoolTraits,   0x00F8C1D4, 0x00D5D1DC, 0x148,  4, 0x00AD36A0, 0x00AD3820, 0x00AD32B0, 0x00AD3780, 0x00AD31F0, 0x00AD2E40, 0x00AD3420, 0x00AD3960);
#undef BSP_GUI_WIDGET_POOL_TRAITS

// Host companion over one native pool header, like NativeGuiGroupPool. It adds
// no state of its own; initialize/destroy reproduce the native lifecycle and the
// C++ destructor only unbinds the trim callback.
template <class Traits>
class NativeGuiWidgetPool final {
public:
    using traits = Traits;
    NativeGuiWidgetPool(AllocatorListDomain&, NativeGuiWidgetPoolStorage&);
    ~NativeGuiWidgetPool();
    NativeGuiWidgetPool(const NativeGuiWidgetPool&) = delete;
    NativeGuiWidgetPool& operator=(const NativeGuiWidgetPool&) = delete;

    // Traits::construct_address. __fastcall(ECX = pool), returns the pool. Links
    // into the allocator list 00E188B4, writes the vtable, initializes the
    // critical section and reserves the slab table to 20h entries (80h bytes).
    void initialize();
    // Traits::allocate_address. __fastcall(ECX = pool), EAX = raw slot. Appends a
    // slab when no slab has a free slot, pops the free stack, then advances the
    // first-free index past slabs that became full. The slot is not constructed.
    void* allocate();
    // Traits::free_address. __thiscall(pool, slot), RET 4. Pushes the slot index
    // back onto its slab's free stack (index = signed (slot - slab) / slot_bytes)
    // and lowers the first-free index to that slab when smaller.
    void free(void* slot) noexcept;
    // Traits::trim_address, vtable +0 (the allocator-list callback). No lock.
    // Frees every fully free slab, moving the last table entry into its place and
    // renumbering the moved slab's slots, then recomputes the first-free index.
    void trim();
    // Traits::destroy_address. __fastcall(ECX = pool). Frees every slab and the
    // table, deletes the critical section and unlinks from the allocator list.
    // Payload destructors are never run.
    void destroy();
    // Traits::init_block_address. __thiscall(slab, index), RET 4, returns the slab:
    // free count = N, free stack = N-1..0, slab index in every slot trailer.
    static void* initialize_slab(void* raw, std::uint32_t index) noexcept;

    NativeGuiWidgetPoolStorage& storage() noexcept { return storage_; }

private:
    AllocatorListDomain& allocator_list_;
    NativeGuiWidgetPoolStorage& storage_;
    static void invoke_trim(void*);
};

// The slab table at pool+28h viewed as the Mit pointer vector its out-of-line
// members take in ECX (fields slabs_28, slab_count_2c, table_capacity_30).
struct NativeGuiPoolSlabTable {
    std::byte** data;
    std::uint32_t count;
    std::uint32_t capacity;
};
static_assert(sizeof(NativeGuiPoolSlabTable) == 12);
inline NativeGuiPoolSlabTable& native_gui_pool_slab_table(NativeGuiWidgetPoolStorage& storage) noexcept {
    static_assert(offsetof(NativeGuiWidgetPoolStorage, slabs_28) == 0x28);
    return *reinterpret_cast<NativeGuiPoolSlabTable*>(&storage.slabs_28);
}
// 00AA6FA0 / 00AAB8E0 / 00AACDF0, __thiscall(table, capacity), RET 4: allocate
// capacity*4 bytes, copy the count entries, free the old block, adopt the new.
void native_gui_pool_table_set_capacity(NativeGuiPoolSlabTable& table, std::uint32_t capacity);
// 00AA6FF0 / 00AAB930, __fastcall(ECX = table), RET: capacity = capacity*2+2.
void native_gui_pool_table_grow(NativeGuiPoolSlabTable& table);
// 00AA76C0 / 00AABC70, __thiscall(table, const slab*&), RET 4: grow when full
// (capacity*2+2), then append.
void native_gui_pool_table_push_back(NativeGuiPoolSlabTable& table, std::byte* const& slab);
// 00AA7740 / 00AABD00, __thiscall(table, capacity), RET 4: set capacity only
// when it grows.
void native_gui_pool_table_reserve(NativeGuiPoolSlabTable& table, std::uint32_t capacity);

namespace game {
// The process-static pool at Traits::native_global. The image constructs it from
// a CRT initializer (MOV ECX, global; CALL construct; PUSH atexit stub) and the
// atexit stub tail-jumps to destroy. Startup is explicit here, as for the Group
// pool (src/native_gui_group_storage.cpp).
template <class Traits>
class GameNativeGuiWidgetPoolProcess final {
public:
    static GameNativeGuiWidgetPoolProcess& instance();
    int initialize_once();
    NativeGuiWidgetPool<Traits>& pool();

private:
    GameNativeGuiWidgetPoolProcess();
    static void destroy_at_exit() noexcept;
    enum class State { unattempted, returned, threw };
    NativeGuiWidgetPoolStorage storage_{};
    NativeGuiWidgetPool<Traits> pool_;
    std::mutex startup_mutex_;
    State state_{State::unattempted};
    int registration_status_{};
};
} // namespace game

// Traits::allocate_stub_address: MOV ECX, global; JMP allocate. Called by the
// type-table factory stub of the widget class. Returns the raw slot.
template <class Traits>
void* allocate_gui_widget_pool_slot();

// Traits::free_thunk_address: __fastcall(ECX = slot); PUSH ECX; MOV ECX, global;
// CALL free; RET. The scalar deleting destructors reach the pool through it.
template <class Traits>
void free_gui_widget_pool_slot(void* slot);

#define BSP_GUI_WIDGET_POOL_EXTERN(T)                                     \
    extern template class NativeGuiWidgetPool<T>;                         \
    namespace game { extern template class GameNativeGuiWidgetPoolProcess<T>; } \
    extern template void* allocate_gui_widget_pool_slot<T>();             \
    extern template void free_gui_widget_pool_slot<T>(void*);
BSP_GUI_WIDGET_POOL_EXTERN(GuiWidgetPoolTraits)
BSP_GUI_WIDGET_POOL_EXTERN(GuiButtonPoolTraits)
BSP_GUI_WIDGET_POOL_EXTERN(GuiMoviePoolTraits)
BSP_GUI_WIDGET_POOL_EXTERN(GuiIconPoolTraits)
BSP_GUI_WIDGET_POOL_EXTERN(GuiCurvePoolTraits)
BSP_GUI_WIDGET_POOL_EXTERN(GuiSectionPoolTraits)
BSP_GUI_WIDGET_POOL_EXTERN(GuiLinePoolTraits)
BSP_GUI_WIDGET_POOL_EXTERN(GuiScrollbarPoolTraits)
BSP_GUI_WIDGET_POOL_EXTERN(GuiGridPoolTraits)
BSP_GUI_WIDGET_POOL_EXTERN(GuiModelPoolTraits)
BSP_GUI_WIDGET_POOL_EXTERN(GuiSoundPoolTraits)
BSP_GUI_WIDGET_POOL_EXTERN(GuiClipBoxPoolTraits)
BSP_GUI_WIDGET_POOL_EXTERN(GuiFrameBoxPoolTraits)
BSP_GUI_WIDGET_POOL_EXTERN(GuiAnimIconPoolTraits)
#undef BSP_GUI_WIDGET_POOL_EXTERN

} // namespace bsp
