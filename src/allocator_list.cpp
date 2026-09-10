#include "bsp/allocator_list.hpp"
#include <cstddef>
#include <stdexcept>

namespace bsp {
static_assert(sizeof(void*) != 4 || sizeof(AllocatorListElement) == 12);
static_assert(sizeof(void*) != 4 || offsetof(AllocatorListElement, previous_04) == 4);
static_assert(sizeof(void*) != 4 || offsetof(AllocatorListElement, next_08) == 8);

AllocatorListDomain::AllocatorListDomain(AllocatorListElement*& head) noexcept
    : shared_head_(head) {}

void AllocatorListDomain::bind_virtual0(AllocatorListElement& element,
    AllocatorListVirtual0Binding binding) {
    if (!binding.native_vtable || !binding.native_virtual0 || !binding.context || !binding.invoke)
        throw std::invalid_argument("allocator virtual-zero binding requires a concrete native profile");
    const auto found = virtual0_.find(&element);
    if (found != virtual0_.end()) {
        const auto& old = found->second;
        if (old.native_vtable != binding.native_vtable || old.native_virtual0 != binding.native_virtual0
            || old.context != binding.context || old.invoke != binding.invoke)
            throw std::logic_error("allocator element already has a different concrete binding");
        return;
    }
    virtual0_.emplace(&element, binding);
}

void AllocatorListDomain::unbind_virtual0(AllocatorListElement& element) noexcept {
    virtual0_.erase(&element);
}

void AllocatorListDomain::prepend_base_element(AllocatorListElement& element) noexcept {
    element.native_vtable_00 = base_vtable;
    element.previous_04 = nullptr;
    element.next_08 = shared_head_;
    if (shared_head_) shared_head_->previous_04 = &element;
    shared_head_ = &element;
}

void AllocatorListDomain::unlink_base_element_00403970(AllocatorListElement& element) noexcept {
    auto* previous = element.previous_04;
    element.native_vtable_00 = base_vtable;
    if (previous) previous->next_08 = element.next_08;
    else shared_head_ = element.next_08;
    if (element.next_08) element.next_08->previous_04 = element.previous_04;
}

void AllocatorListDomain::trim_all_004b46b0() {
    auto* current = shared_head_;
    while (current) {
        const auto found = virtual0_.find(current);
        if (found == virtual0_.end() || found->second.native_vtable != current->native_vtable_00)
            throw std::logic_error("allocator element has no matching concrete virtual-zero binding");
        // Copy the binding before invocation: the callback can change bindings
        // and list links. Native 004B46C8 reloads this same element's next after it.
        const auto binding = found->second;
        binding.invoke(binding.context);
        current = current->next_08;
    }
}
} // namespace bsp
