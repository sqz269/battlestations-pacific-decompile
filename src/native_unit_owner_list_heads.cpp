#include "bsp/native_unit_owner_list_heads.hpp"

#include "bsp/detail/native_blank_container_storage.hpp"
#include "bsp/singleton_lifetime.hpp"

namespace bsp {
namespace {
void* allocate_head(std::uint32_t bytes) {
    return detail::allocate_self_linked_list_storage(bytes, [](std::uint32_t count) {
        return singleton_lifetime_allocate({SingletonAllocationKind::object, count, count});
    });
}
} // namespace

void* buy_native_unit_owner_list_head_0077b940() {
    return allocate_head(0x18);
}

void* buy_native_unit_owner_list_head_0077b960() {
    return allocate_head(0x0c);
}
} // namespace bsp
