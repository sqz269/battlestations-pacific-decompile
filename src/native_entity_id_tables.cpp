#include "bsp/native_entity_id_tables.hpp"
#include "bsp/singleton_lifetime.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native entity ID tables require MSVC Win32.
#endif

namespace bsp {
namespace {
using U = std::uint32_t;
using W = std::uint16_t;
static_assert(sizeof(void*) == 4 && sizeof(NativeEntityIdTableStorage) == 0x54);
static_assert(sizeof(NativeEntityIdSlot) == 0x10);
static_assert(offsetof(NativeEntityIdSlot, payload_0c) == 0xc);
template<class T> volatile T& field(void* base, U offset) noexcept {
    return *reinterpret_cast<volatile T*>(reinterpret_cast<U>(base) + offset);
}
void* offset(void* base, U bytes) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<U>(base) + bytes);
}
const NativeEntityIdTableProfile profile{delete_native_entity_id_table_00951790};
} // namespace

const NativeEntityIdTableProfile& native_entity_id_table_profile_00d19b88() noexcept {
    return profile;
}

void reset_native_entity_id_table_00951560(void* owner) noexcept {
    field<W>(field<void*>(owner, 0x4c), 8) = 0;
    field<void*>(field<void*>(owner, 0x4c), 0xc) = nullptr;
    void* previous = offset(owner, 0xc);
    U bytes = 0x10;
    for (U i = 1; static_cast<std::int32_t>(i) <
            static_cast<std::int32_t>(field<U>(owner, 8)); ++i, bytes += 0x10) {
        field<void*>(field<void*>(owner, 0x4c), bytes) = previous;
        const W id = static_cast<W>(field<W>(owner, 4) + static_cast<W>(i));
        field<W>(field<void*>(owner, 0x4c), bytes + 8) = id;
        field<void*>(field<void*>(owner, 0x4c), bytes + 0xc) = nullptr;
        previous = offset(field<void*>(owner, 0x4c), bytes);
    }
    U i = field<U>(owner, 8) - 1u;
    void* next = offset(owner, 0x1c);
    if (static_cast<std::int32_t>(i) >= 1) {
        bytes = i << 4;
        do {
            field<void*>(field<void*>(owner, 0x4c), bytes + 4) = next;
            next = offset(field<void*>(owner, 0x4c), bytes);
            bytes -= 0x10;
            --i;
        } while (i != 0);
    }
    void* const slots = field<void*>(owner, 0x4c);
    field<void*>(owner, 0x10) = offset(slots, 0x10);
    const U last = (field<U>(owner, 8) << 4) - 0x10u;
    field<void*>(owner, 0x1c) = offset(slots, last);
    field<U>(owner, 0xc) = 0;
    field<U>(owner, 0x20) = 0;
    field<U>(owner, 0x2c) = 0;
    field<U>(owner, 0x40) = 0;
    field<void*>(owner, 0x30) = offset(owner, 0x3c);
    field<void*>(owner, 0x3c) = offset(owner, 0x2c);
}

void* construct_native_entity_id_table_00951660(void* owner, U first_id, U count) {
    field<U>(owner, 4) = first_id;
    field<U>(owner, 0x50) = count - 1u;
    field<U>(owner, 8) = count;
    const U bytes = count > 0x0fffffffu ? 0xffffffffu : count << 4;
    field<const NativeEntityIdTableProfile*>(owner, 0) = &profile;
    void* const slots = singleton_lifetime_allocate({
        SingletonAllocationKind::object, bytes, bytes});
    field<void*>(owner, 0x4c) = slots;
    reset_native_entity_id_table_00951560(owner);
    return owner;
}

void release_native_entity_id_009516d0(void* owner, U id) noexcept {
    void* const slot = offset(field<void*>(owner, 0x4c),
        (static_cast<U>(static_cast<W>(id)) - field<U>(owner, 4)) << 4);
    void* const next = field<void*>(slot, 0);
    void* const previous = field<void*>(slot, 4);
    field<void*>(slot, 0xc) = nullptr;
    field<void*>(next, 4) = previous;
    void* const current_previous = field<void*>(slot, 4);
    void* const current_next = field<void*>(slot, 0);
    field<void*>(current_previous, 0) = current_next;
    void* const head = offset(owner, 0x1c);
    field<void*>(slot, 4) = head;
    void* const old = field<void*>(head, 0);
    field<void*>(slot, 0) = old;
    field<void*>(old, 4) = slot;
    field<void*>(field<void*>(slot, 4), 0) = slot;
    field<U>(owner, 0x50) = field<U>(owner, 0x50) + 1u;
}

void sweep_native_entity_id_table_00951720(void* owner) noexcept {
    U i = field<U>(owner, 8) - 1u;
    if (static_cast<std::int32_t>(i) < 1) return;
    U tested_bytes = i << 4;
    do {
        void* const slots = field<void*>(owner, 0x4c);
        if (field<void*>(slots, tested_bytes + 0xc) == nullptr) {
            void* const slot = offset(slots,
                (static_cast<U>(static_cast<W>(i)) - field<U>(owner, 4)) << 4);
            void* const next = field<void*>(slot, 0);
            void* const previous = field<void*>(slot, 4);
            field<void*>(slot, 0xc) = nullptr;
            field<void*>(next, 4) = previous;
            void* const current_previous = field<void*>(slot, 4);
            void* const current_next = field<void*>(slot, 0);
            field<void*>(current_previous, 0) = current_next;
            void* const head = offset(owner, 0x1c);
            field<void*>(slot, 4) = head;
            void* const old = field<void*>(head, 0);
            field<void*>(slot, 0) = old;
            field<void*>(old, 4) = slot;
            field<void*>(field<void*>(slot, 4), 0) = slot;
            field<U>(owner, 0x50) = field<U>(owner, 0x50) + 1u;
        }
        --i;
        tested_bytes -= 0x10;
    } while (static_cast<std::int32_t>(i) >= 1);
}

void* delete_native_entity_id_table_00951790(void* owner, U flags) noexcept {
    void* const slots = field<void*>(owner, 0x4c);
    field<const NativeEntityIdTableProfile*>(owner, 0) = &profile;
    singleton_lifetime_free(slots);
    if ((flags & 1u) != 0) singleton_lifetime_free(owner);
    return owner;
}

U allocate_native_entity_id_009517c0(void* owner, U requested_id, void* payload) noexcept {
    if (field<U>(owner, 0x50) == 0) sweep_native_entity_id_table_00951720(owner);
    void* slot;
    if (static_cast<W>(requested_id) == 0) {
        slot = field<void*>(owner, 0x10);
        requested_id = field<W>(slot, 8);
    } else {
        slot = offset(field<void*>(owner, 0x4c),
            (static_cast<U>(static_cast<W>(requested_id)) - field<U>(owner, 4)) << 4);
    }
    void* const previous = field<void*>(slot, 4);
    field<void*>(slot, 0xc) = payload;
    void* const next = field<void*>(slot, 0);
    field<void*>(next, 4) = previous;
    void* const current_previous = field<void*>(slot, 4);
    void* const current_next = field<void*>(slot, 0);
    field<void*>(current_previous, 0) = current_next;
    void* const head = offset(owner, 0x3c);
    field<void*>(slot, 4) = head;
    void* const old = field<void*>(head, 0);
    field<void*>(slot, 0) = old;
    field<void*>(old, 4) = slot;
    field<void*>(field<void*>(slot, 4), 0) = slot;
    field<U>(owner, 0x50) = field<U>(owner, 0x50) - 1u;
    return requested_id;
}

void* resolve_native_entity_id_00521e30(U input, const ObjectHandleTables& tables) noexcept {
    const U id = static_cast<W>(input);
    U index;
    const void* entries;
    if (static_cast<std::int32_t>(id) < tables.first_end_00f89a10) {
        index = id - static_cast<U>(tables.first_begin_00f89a0c);
        entries = tables.first_entries_00f89a54;
    } else {
        index = id - static_cast<U>(tables.second_begin_00f89a60);
        entries = tables.second_entries_00f89aa8;
    }
    return field<void*>(const_cast<void*>(entries), (index << 4) + 0xc);
}
} // namespace bsp
