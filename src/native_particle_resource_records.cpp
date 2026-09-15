#include "bsp/native_particle_resource_records.hpp"
#include "bsp/native_render_resource_record_construction.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native particle resource records require MSVC Win32.
#endif

namespace bsp {
// Defined by the disjoint particle cache lifetime packet; genuine provider.
void release_native_particle_resource_00871420(void* actual_resource);

namespace {
using U = std::uint32_t;
using I = std::int32_t;
static_assert(sizeof(void*) == 4);
template<class T> volatile T& field(const void* p, U offset) {
    return *reinterpret_cast<volatile T*>(reinterpret_cast<U>(p) + offset);
}
void* add(const void* p, U offset) { return reinterpret_cast<void*>(reinterpret_cast<U>(p) + offset); }
void* element(void* vector, U index) { return add(field<void*>(vector, 0), index * 0x2cu); }
void give_back(char* data, U size, NativeStringRawPoolContext& strings) {
    auto* const pool = native_string_pool_get_or_create_00419cc0(
        strings.actual_published_01090aa8, strings.actual_manager_publication_01090aa0);
    return_native_string_pool_00bd1510(pool, data, size, strings.actual_small_returns_disabled_01090aa4);
}
// True native unwind actions C95DF0/C95E30/C95FA9, not catch-handler cleanup.
// A second exception during this destructor terminates in the source EH domain.
struct NameUnwind {
    void* record;
    NativeStringRawPoolContext& strings;
    bool active = true;
    ~NameUnwind() noexcept {
        if (active) destroy_native_string_header_0041dd20(record, strings);
    }
};
}

void destroy_native_particle_resource_record_0086fcf0(void* record, NativeStringRawPoolContext& strings) {
    char* data;
    {
        NameUnwind unwind{record, strings};
        // Includes the returning-free tail86FD29: current sentinel is nulled.
        destroy_native_render_alias_list_004d0a10(add(record, 8), strings);
        data = field<char*>(record, 4); // Captured before state=-1.
        unwind.active = false;
    }
    if (data != nullptr) give_back(data, field<U>(record, 0) + 1u, strings);
}

void* copy_construct_native_particle_resource_record_0086ff50(
    void* destination, const void* source, NativeStringRawPoolContext& strings) {
    const bool same = destination == source; // Compare before either store.
    field<U>(destination, 0) = 0;
    field<void*>(destination, 4) = nullptr;
    if (!same) {
        resize_native_string_header_0041dd40(destination, strings, field<U>(source, 0), true);
        if (field<U>(source, 0) != 0) {
            const U copied = field<U>(destination, 0);
            auto* const data = field<void*>(destination, 4);
            auto* const from = field<void*>(source, 4);
            if (copied != 0) std::memmove(data, from, copied); // Native BF7680 overlap behavior.
        }
    }
    NameUnwind unwind{destination, strings}; // State0 only after name copy.
    copy_construct_native_render_alias_list_004d48a0(add(destination, 8), add(source, 8), strings);
    for (U offset = 0x14; offset != 0x2c; offset += 4)
        field<U>(destination, offset) = field<U>(source, offset);
    unwind.active = false;
    return destination;
}

void reserve_native_particle_resource_records_00870000(void* vector, I requested, NativeStringRawPoolContext& strings) {
    if (requested < 0x40) requested = 0x40;
    if (field<I>(vector, 8) >= requested) return;
    const U bytes = static_cast<U>(requested) * 0x2cu;
    void* const allocation = singleton_lifetime_allocate({SingletonAllocationKind::object, bytes, bytes});
    for (U index = 0; static_cast<I>(index) < field<I>(vector, 4); ++index) {
        void* const destination = add(allocation, index * 0x2cu);
        if (destination != nullptr)
            copy_construct_native_particle_resource_record_0086ff50(destination, element(vector, index), strings);
    }
    // Its only unwind action is RET-only placement delete401130. Do not free
    // allocation, destroy completed copies or restore old records on failure.
    for (U index = 0; static_cast<I>(index) < field<I>(vector, 4); ++index)
        destroy_native_particle_resource_record_0086fcf0(element(vector, index), strings);
    singleton_lifetime_free(field<void*>(vector, 0));
    field<void*>(vector, 0) = allocation; // Hidden8700C4/C6 publications.
    field<I>(vector, 8) = requested;
}

void append_native_particle_resource_record_00870ac0(void* vector, const void* source, NativeStringRawPoolContext& strings) {
    const U capacity = field<U>(vector, 8);
    if (field<U>(vector, 4) == capacity) {
        const I doubled = static_cast<I>(capacity + capacity);
        reserve_native_particle_resource_records_00870000(vector, doubled > 0x40 ? doubled : 0x40, strings);
    }
    void* const destination = element(vector, field<U>(vector, 4));
    if (destination != nullptr)
        copy_construct_native_particle_resource_record_0086ff50(destination, source, strings);
    field<U>(vector, 4) = field<U>(vector, 4) + 1u;
}

void resize_native_particle_resource_records_00870b30(void* vector, I requested, NativeStringRawPoolContext& strings) {
    if (requested > field<I>(vector, 8)) reserve_native_particle_resource_records_00870000(vector, requested, strings);
    for (U index = field<U>(vector, 4); static_cast<I>(index) < requested; ++index) {
        void* const record = element(vector, index);
        if (record != nullptr) {
            field<U>(record, 0) = 0;
            field<void*>(record, 4) = nullptr;
            NameUnwind unwind{record, strings};
            auto* const sentinel = allocate_native_render_alias_sentinel_004c3020();
            field<NativeRenderResourceAliasNode*>(record, 0xc) = sentinel;
            field<U>(record, 0x10) = 0;
            for (U offset = 0x24; offset != 0x10; offset -= 4) field<U>(record, offset) = 0;
            // Native constructor leaves the unknown word+8 and resource+28.
            unwind.active = false;
        }
    }
    while (requested < field<I>(vector, 4)) {
        field<U>(vector, 4) = field<U>(vector, 4) - 1u;
        destroy_native_particle_resource_record_0086fcf0(element(vector, field<U>(vector, 4)), strings);
    }
    field<I>(vector, 4) = requested;
}

void clear_native_particle_resource_cache_00871310(void* owner, NativeStringRawPoolContext& strings) {
    while (field<U>(owner, 8) != 0) {
        const U offset = field<U>(owner, 8) * 0x2cu - 4u;
        void* const resource = field<void*>(field<void*>(owner, 4), offset);
        const U table = field<U>(owner, 0);
        if (table == 0x00d0daf0 || table == 0x00d0db40) {
            release_native_particle_resource_00871420(resource);
        } else {
            using Release = void (__thiscall*)(void*, void*);
            auto target = field<Release>(reinterpret_cast<void*>(table), 0x10);
            target(owner, resource);
        }
        const U current_count = field<U>(owner, 8);
        if (current_count != 0) {
            destroy_native_particle_resource_record_0086fcf0(
                add(field<void*>(owner, 4), current_count * 0x2cu - 0x2cu), strings);
            field<U>(owner, 8) = field<U>(owner, 8) - 1u;
        }
    }
    resize_native_particle_resource_records_00870b30(add(owner, 4), 0, strings);
}
}
