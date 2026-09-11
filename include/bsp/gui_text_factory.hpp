#pragma once

#include "bsp/allocator_list.hpp"
#include <cstddef>
#include <cstdint>

namespace bsp {

// The one physical owner corresponding to F8BDF0. No initializers substitute
// for AB77C0. Its allocation domain is singleton_lifetime_allocate/free; do
// not adopt slabs/table pointers allocated by the original executable's CRT.
struct NativeGuiTextPoolStorage {
    AllocatorListElement allocator_00;
    alignas(4) std::byte critical_section_0c[24];
    std::int32_t recursion_24;
    std::byte** slabs_28;
    std::uint32_t slab_count_2c;
    std::uint32_t table_capacity_30;
    std::uint32_t first_free_slab_34;
};

// Real pool ownership prerequisite, NOT a constructed GuiTextLifetime factory.
// This companion borrows the one storage and allocator-list domain. It stores
// no second slab table or free stack. Explicit initialize/destroy reproduce
// native lifetime; the C++ destructor only removes its host dispatch binding.
// The companion/domain/storage must outlive all slots and list membership.
class NativeGuiTextPool final {
public:
    static constexpr std::uint32_t native_vtable = 0x00d5c5d8;
    static constexpr std::uint32_t native_virtual0 = 0x00ab7500;
    static constexpr std::size_t payload_bytes = 0x1f4;
    static constexpr std::size_t slot_bytes = 0x1f8;
    static constexpr std::size_t slab_bytes = 0x7e84;
    static constexpr std::uint32_t slots_per_slab = 64;

    NativeGuiTextPool(AllocatorListDomain&, NativeGuiTextPoolStorage&);
    ~NativeGuiTextPool();
    NativeGuiTextPool(const NativeGuiTextPool&) = delete;
    NativeGuiTextPool& operator=(const NativeGuiTextPool&) = delete;

    void initialize_00ab77c0();
    void* allocate_raw_slot_00ab78a0();
    void return_raw_slot_00ab75a0(void* slot);
    void trim_empty_slabs_00ab7500(); // Native virtual0 takes no internal lock.
    void destroy_00ab7420();         // Does not destroy any Text payload.
    NativeGuiTextPoolStorage& storage() noexcept { return storage_; }

private:
    AllocatorListDomain& allocator_list_;
    NativeGuiTextPoolStorage& storage_;
    static void invoke_trim(void*);
    void destroy_critical_section_00402f70() noexcept;
};

// AB6F60: ECX raw7E84h slab, stack slab index, EAX same slab, RET4.
// Writes free-count64, reversed WORD indices, and each hidden slot+1F4 DWORD;
// preserves every1F4h payload and final two padding bytes.
void* initialize_gui_text_pool_slab_00ab6f60(void* actual_slab,
    std::uint32_t slab_index) noexcept;

// AB70C0: ECX address of pool+28 header; free nonnull *header; RET.
// No clearing. Used by the constructor's real unwind table after CS cleanup
// has become armed; it executes BEFORE that CS cleanup.
void free_gui_text_pool_table_00ab70c0(void* actual_table_header) noexcept;

// AB79E0 overwrites incoming ECX (size ignored) with F8BDF0 and tail-jumps
// AB78A0. AB76F0 takes the failed raw slot in ECX, selects that same pool and
// calls AB75A0, RET. It does not invoke AB8250 or destroy a C++ companion.
void* allocate_gui_text_raw_slot_00ab79e0(NativeGuiTextPool& actual_f8bdf0);
void return_gui_text_failed_slot_00ab76f0(void* slot,
    NativeGuiTextPool& actual_f8bdf0);

// Partial factory prerequisite only: raw slots are not GuiWidgetOwner,
// GuiLayoutWidget or GuiTextLifetime addresses and cannot be cast to them.
// No Text type registration, native pool startup registration, clone fallback,
// or implicit virtual74/78 dispatch is supplied. See docs/GUI_TEXT_FACTORY.md.
// New MSVC Win32 interfaces, not binary entry-point replacements.
} // namespace bsp
