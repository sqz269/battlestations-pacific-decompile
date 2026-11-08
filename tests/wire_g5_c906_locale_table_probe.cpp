#include "bsp/native_locale_pool_table.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>

namespace {
struct Table {
    std::uint32_t* data;
    std::uint32_t count;
    std::uint32_t capacity;
};
static_assert(sizeof(Table) == 12 && sizeof(void*) == 4);
Table* current;
std::uint32_t* callback_table;
bool allocated;
bool released;
}

// Fixture boundaries deliberately mutate the borrowed native triple. This
// catches both descriptor copies and an early read of the append source cell.
namespace bsp {
void* singleton_lifetime_allocate(const SingletonAllocationRequest& request) {
    assert(!allocated && !released && current->capacity == 6u);
    assert(request.kind == SingletonAllocationKind::object);
    assert(request.native_bytes == 24u && request.host_bytes == 24u);
    allocated = true;
    current->data = callback_table;
    current->count = 1u;
    void* result = std::calloc(6u, sizeof(std::uint32_t));
    assert(result);
    return result;
}
void singleton_lifetime_free(void* allocation) noexcept {
    assert(allocated && !released && allocation == callback_table);
    released = true;
    current->capacity = 8u; // also the source cell, read after this callback
    std::free(allocation);
}
}

int main() {
    auto* original = static_cast<std::uint32_t*>(std::calloc(2u, sizeof(std::uint32_t)));
    callback_table = static_cast<std::uint32_t*>(std::calloc(2u, sizeof(std::uint32_t)));
    assert(original && callback_table);
    original[0] = 0x1111u;
    callback_table[0] = 0x3333u;
    Table table{original, 2u, 2u};
    current = &table;
    bsp::push_native_string_map_pool_slab_00a9ee40({&table}, &table.capacity);
    assert(allocated && released && table.count == 2u && table.capacity == 8u);
    assert(table.data[0] == 0x3333u && table.data[1] == 8u);
    std::free(table.data);
    std::free(original); // callback changed the current table; native did not free this one
    std::puts("A9EA60/A9EE40 actual-triple callback/aliased-source probe passed");
}
