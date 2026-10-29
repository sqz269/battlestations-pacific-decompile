#include "bsp/cc16_dyn_storage.hpp"

namespace bsp::cc16_dyn_storage {
namespace {
using U32 = std::uint32_t;
static_assert(sizeof(void*) == 4, "The recovered raw storage uses Win32 pointers");

U32 address(const void* pointer) noexcept { return reinterpret_cast<U32>(pointer); }
void* pointer(U32 value) noexcept { return reinterpret_cast<void*>(value); }
volatile U32& word(U32 location) noexcept {
    return *reinterpret_cast<volatile U32*>(location);
}
volatile U32& field(const void* object, U32 offset) noexcept {
    return word(address(object) + offset);
}
}  // namespace

void** append_pair_reference_00c37390(void* endpoint, void* pair,
                                    const cc15_dyn_live::DynLiveCrt& crt) {
    const U32 capacity = field(endpoint, 0x38);
    if (field(endpoint, 0x34) == capacity) {
        const U32 grown_capacity = capacity * 2u + 2u;
        field(endpoint, 0x38) = grown_capacity;
        const U32 grown = address(crt.operator_new_00bf55be(grown_capacity * 4u));
        U32 index = 0;
        U32 output = grown;
        while (index < field(endpoint, 0x34)) {
            if (output != 0) {
                word(output) = word(field(endpoint, 0x30) + index * 4u);
            }
            ++index;
            output += 4u;
        }
        const U32 old = field(endpoint, 0x30);
        if (old != 0) {
            crt.free_00bf6989(pointer(old));
        }
        field(endpoint, 0x30) = grown;
    }
    const U32 index = field(endpoint, 0x34);
    const U32 data = field(endpoint, 0x30);
    const U32 slot = data + index * 4u;
    if (slot != 0) {
        word(slot) = address(pair);
    }
    field(endpoint, 0x34) = field(endpoint, 0x34) + 1u;
    return reinterpret_cast<void**>(slot);
}

void** add_endpoint_pair_00c40a30(void* storage, void* first_endpoint,
                                void* second_endpoint,
                                const cc15_dyn_live::DynLiveCrt& crt) {
    const bool need_page = field(storage, 0x34) == 0;
    const U32 second = address(second_endpoint);  // EBP captures argument two before malloc.
    if (need_page) {
        const U32 page = address(crt.malloc_00bf9f1a(0x3e80u));
        U32 output = page + 0x0cu;
        U32 remaining = 0x3e7u;
        do {
            word(output) = output + 4u;
            output += 0x10u;
            --remaining;
        } while (remaining != 0);
        word(page + 0x3e7cu) = remaining;

        // The equality result and old capacity precede publication of the
        // free head. A later allocation exception leaves this state visible.
        const U32 capacity = field(storage, 0x30);
        const bool grow = field(storage, 0x2c) == capacity;
        field(storage, 0x34) = page;
        if (grow) {
            const U32 grown_capacity = capacity * 2u + 2u;
            field(storage, 0x30) = grown_capacity;
            const U32 grown = address(crt.operator_new_00bf55be(grown_capacity * 4u));
            U32 index = 0;
            U32 destination = grown;
            while (index < field(storage, 0x2c)) {
                if (destination != 0) {
                    word(destination) = word(field(storage, 0x28) + index * 4u);
                }
                ++index;
                destination += 4u;
            }
            const U32 old = field(storage, 0x28);
            if (old != 0) {
                crt.free_00bf6989(pointer(old));
            }
            field(storage, 0x28) = grown;
        }
        const U32 index = field(storage, 0x2c);
        const U32 data = field(storage, 0x28);
        const U32 slot = data + index * 4u;
        if (slot != 0) {
            word(slot) = page;
        }
        field(storage, 0x2c) = field(storage, 0x2c) + 1u;
    }

    // Reload free head after every page-vector callback, as the image does.
    const U32 node = field(storage, 0x34);
    const U32 next_free = word(node + 0x0cu);
    field(storage, 0x58) = field(storage, 0x58) + 1u;
    const U32 first = address(first_endpoint);
    const bool first_below_second = first < second;  // CMP/JNC is unsigned.
    field(storage, 0x34) = next_free;
    word(node + 0x0cu) = address(storage) + 0x48u;
    word(node + 0x08u) = field(storage, 0x50);
    const U32 tail = field(storage, 0x50);  // Reload after writing node.previous.
    word(tail + 0x0cu) = node;
    field(storage, 0x50) = node;
    if (first_below_second) {
        word(node) = first;
        word(node + 4u) = second;
    } else {
        word(node + 4u) = first;
        word(node) = second;
    }
    append_pair_reference_00c37390(pointer(first), pointer(node), crt);
    return append_pair_reference_00c37390(pointer(second), pointer(node), crt);
}

}  // namespace bsp::cc16_dyn_storage
