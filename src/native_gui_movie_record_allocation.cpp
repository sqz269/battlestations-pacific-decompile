#include "bsp/native_gui_movie_record_allocation.hpp"

#include "bsp/singleton_lifetime.hpp"

#include <new>

namespace bsp {

static_assert(sizeof(void*) == 4 && sizeof(std::size_t) == 4);

void* __fastcall allocate_native_gui_movie_records_14h_00aacd30(
    std::uint32_t count, void*) {
    if (count > 0x0cccccccu)
        throw std::bad_alloc();
    const std::uint32_t bytes = count * 0x14u;
    return singleton_lifetime_allocate({SingletonAllocationKind::object, bytes, bytes});
}

void* __fastcall allocate_native_gui_movie_records_24h_00aacd90(
    std::uint32_t count, void*) {
    if (count > 0x071c71c7u)
        throw std::bad_alloc();
    const std::uint32_t bytes = count * 0x24u;
    return singleton_lifetime_allocate({SingletonAllocationKind::object, bytes, bytes});
}

} // namespace bsp
