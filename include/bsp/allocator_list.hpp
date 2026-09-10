#pragma once
#include <cstdint>
#include <unordered_map>

namespace bsp {

// Canonical allocator element used by the shared native 00E188B4 list. These
// are native data words, not a C++ vptr. The element is 12 bytes on Win32.
struct AllocatorListElement {
    std::uint32_t native_vtable_00;
    AllocatorListElement* previous_04;
    AllocatorListElement* next_08;
};

// An explicit binding for a concrete recovered virtual-zero implementation.
// Foreign allocator profiles require their own real owner/invoke binding.
struct AllocatorListVirtual0Binding {
    std::uint32_t native_vtable;
    std::uint32_t native_virtual0;
    void* context;
    void (*invoke)(void* context);
};

class AllocatorListDomain {
public:
    static constexpr std::uint32_t base_vtable = 0x00d7a0c0;
    explicit AllocatorListDomain(AllocatorListElement*& shared_head_00e188b4) noexcept;
    AllocatorListDomain(const AllocatorListDomain&) = delete;
    AllocatorListDomain& operator=(const AllocatorListDomain&) = delete;

    void bind_virtual0(AllocatorListElement&, AllocatorListVirtual0Binding);
    void unbind_virtual0(AllocatorListElement&) noexcept;
    // Constructor order in 00B7B940: base vtable, previous, next, old previous,
    // then shared head. Unlink leaves this element's old link fields intact.
    void prepend_base_element(AllocatorListElement&) noexcept;
    void unlink_base_element_00403970(AllocatorListElement&) noexcept;
    void trim_all_004b46b0();
    AllocatorListElement* head() const noexcept { return shared_head_; }

private:
    AllocatorListElement*& shared_head_;
    // This table contains no list links and is never the allocator-list head.
    std::unordered_map<AllocatorListElement*, AllocatorListVirtual0Binding> virtual0_;
};

} // namespace bsp
