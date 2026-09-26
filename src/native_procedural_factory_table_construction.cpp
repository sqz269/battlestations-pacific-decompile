#include "bsp/native_procedural_factory_table_construction.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <new>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw procedural table construction requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
Word bits(const void* p) noexcept { return reinterpret_cast<Word>(p); }
void* pointer(Word p) noexcept { return reinterpret_cast<void*>(p); }
void store(void* p, unsigned byte_offset, Word value) noexcept {
    __asm {
        mov eax, p
        add eax, byte_offset
        mov ecx, value
        mov [eax], ecx
    }
}
void full_state(volatile Word& cell, int value, int& diagnostic) noexcept {
    cell = static_cast<Word>(value);
    diagnostic = value;
}
void byte_state(volatile Word& cell, unsigned value, int& diagnostic) noexcept {
    *reinterpret_cast<volatile unsigned char*>(&cell) = static_cast<unsigned char>(value);
    diagnostic = static_cast<std::int32_t>(cell);
}
void* header(const NativeProceduralFactoryTableConstructionFrame& f) noexcept {
    return const_cast<Word*>(&f.locals[3]);
}
bool fresh(const NativeResourceRegistryLinkAcquired& a) noexcept {
    return !a.started && !a.node.started;
}
bool fresh(const NativeProceduralFactoryStorageAcquired& a) noexcept {
    const auto& v = a.registration.value;
    return !a.started && !a.registration.started && !v.started &&
        !v.hint.started && fresh(v.hint.direct) && !v.hint.fallback.started &&
        fresh(v.hint.fallback.link);
}
void require_fresh(const NativeProceduralFactoryTableConstructionAcquired& a) {
    if (a.started) throw std::logic_error("table construction acquisition is not fresh");
    for (unsigned i = 0; i != 4; ++i)
        if (!fresh(a.factory[i]) || a.cleanup[i].started)
            throw std::logic_error("nested table construction acquisition is not fresh");
}

// DFEA64: previous state, then genuine action. The otherwise unarmed
// 6/9/12/15 entries are retained; no normal stores are invented for them.
constexpr int previous[16] = {-1,0,1,2,3,4,3,3,7,3,3,10,3,3,13,3};
void unwind(const NativeProceduralFactoryTableConstructionFrame& f,
    NativeProceduralFactoryTableConstructionContext& c,
    NativeProceduralFactoryTableConstructionAcquired& a) {
    a.cleanup_started = true;
    try {
        for (;;) {
            const int state = static_cast<std::int32_t>(f.eh_state);
            if (state == -1) break;
            if (state < 0 || state >= 16)
                throw std::invalid_argument("unsupported current table EH state");
            a.cleanup_action = state;
            full_state(f.eh_state, previous[state], a.native_eh_state);
            if (state <= 3) {
                void* const current_self = pointer(f.locals[1]);
                a.cleanup_receiver[state] = current_self;
                void* const slot = pointer(bits(current_self) + 4u * state);
                if (state == 0)
                    destroy_native_procedural_factory_slot_00bbc4e0(slot,
                        f.cleanup[state], c.profiles, a.cleanup[state]);
                else
                    destroy_native_procedural_factory_slot_00bbc500(slot,
                        f.cleanup[state], c.profiles, a.cleanup[state]);
            } else if (state == 4 || state == 7 || state == 10 || state == 13) {
                const unsigned stage = static_cast<unsigned>((state - 4) / 3);
                void* const current = pointer(f.locals[2]);
                a.current_allocation_freed[stage] = current;
                singleton_lifetime_free(current);
            } else {
                const unsigned stage = static_cast<unsigned>((state - 5) / 3);
                const Word bit = 1u << stage;
                const Word current_mask = f.locals[0];
                if ((current_mask & bit) != 0) {
                    f.locals[0] = f.locals[0] & ~bit; // native memory AND rereads CURRENT mask
                    a.mask_cleanup_entered[stage] = true;
                    destroy_native_string_header_0041dd20(header(f), c.factories.insertion.strings);
                    a.mask_cleanup_returned[stage] = true;
                    a.name_credit_outstanding[stage] = false;
                }
            }
        }
        a.cleanup_completed = true;
    } catch (...) {
        a.cleanup_failed = true;
        throw; // no retry or remaining source actions
    }
}
} // namespace

void* construct_native_procedural_factory_table_00bbc900(void* table,
    const NativeProceduralFactoryTableConstructionFrame& f,
    NativeProceduralFactoryTableConstructionContext& c,
    NativeProceduralFactoryTableConstructionAcquired& a) {
    require_fresh(a); // before native stores
    a.started = true;
    a.captured_table = table;
    full_state(f.eh_state, -1, a.native_eh_state); // native PUSH -1
    Word ebx = 0;
    f.locals[0] = ebx;
    f.locals[1] = bits(table);
    store(table, 0, 0);
    full_state(f.eh_state, 0, a.native_eh_state);
    store(table, 4, 0);
    store(table, 8, 0);
    store(table, 12, 0);
    byte_state(f.eh_state, 3, a.native_eh_state);
    try {
        for (unsigned i = 0; i != 4; ++i) {
            a.stage = i;
            a.allocation_entered[i] = true;
            void* const allocation = singleton_lifetime_allocate(
                {SingletonAllocationKind::object, 4, 4});
            a.allocation_returned[i] = true;
            a.captured_allocation[i] = allocation;
            f.locals[2] = bits(allocation);
            byte_state(f.eh_state, 4u + 3u * i, a.native_eh_state);
            if (allocation) {
                // Begin one trivial DWORD lifetime without changing its bytes.
                ::new (allocation) Word;
                a.name_entered[i] = true;
                construct_native_string_header_0041e870(header(f),
                    c.factories.insertion.strings, c.names[i]);
                a.name_returned[i] = true;
                a.name_credit_outstanding[i] = true;
                ebx = i == 0 ? 1u : ebx | (1u << i);
                a.captured_ebx_mask = ebx;
                f.factory.name_argument = bits(header(f));
                byte_state(f.eh_state, 5u + 3u * i, a.native_eh_state);
                f.locals[0] = ebx;
                if (i == 0)
                    construct_native_caustics_factory_00bbc5f0(allocation,
                        f.factory, c.factories, a.factory[i]);
                else
                    construct_native_shore_wave_factory_00bbc740(allocation,
                        f.factory, c.factories, a.factory[i]);
                store(allocation, 0, i == 0 ? 0x00d644e8u : 0x00d644f0u);
                a.final_profile_written[i] = true;
            }
            const Word bit = 1u << i;
            const bool return_name = (ebx & bit) != 0; // TEST before stores
            store(table, 4u * i, bits(allocation));
            a.slot_published[i] = true;
            full_state(f.eh_state, 3, a.native_eh_state);
            if (return_name) {
                const Word data = f.locals[4]; // capture BEFORE register clear
                a.normal_name_data[i] = data;
                if (i != 3) ebx &= ~bit; // does not clear CURRENT stack mask
                a.captured_ebx_mask = ebx;
                if (data != 0) {
                    const Word size = f.locals[3] + 1u;
                    a.normal_name_size[i] = size;
                    auto& strings = c.factories.insertion.strings;
                    a.normal_getter_entered[i] = true;
                    NativeStringPoolStorage* const pool = native_string_pool_get_or_create_00419cc0(
                        strings.actual_published_01090aa8, strings.actual_manager_publication_01090aa0);
                    a.normal_getter_returned[i] = true;
                    return_native_string_pool_00bd1510(pool, pointer(data), size,
                        strings.actual_small_returns_disabled_01090aa4);
                    a.normal_name_returned[i] = true;
                }
                a.name_credit_outstanding[i] = false;
            }
        }
        a.completed = true;
        return table;
    } catch (...) {
        a.source_failed = true;
        const unsigned i = a.stage;
        a.partial_name_unarmed = a.name_entered[i] && !a.name_returned[i];
        unwind(f, c, a);
        throw;
    }
}

void* publish_native_procedural_factory_table_00bbcb40(void* volatile& publication,
    const NativeProceduralFactoryTablePublicationFrame& f,
    NativeProceduralFactoryTableConstructionContext& c,
    NativeProceduralFactoryTablePublicationAcquired& a) {
    if (a.started) throw std::logic_error("table publication acquisition is not fresh");
    require_fresh(a.constructor);
    a.started = true;
    full_state(f.eh_state, -1, a.native_eh_state);
    f.cleanup_allocation = f.incoming_ecx; // native PUSH ECX before allocation
    try {
        a.allocation_entered = true;
        void* const allocation = singleton_lifetime_allocate(
            {SingletonAllocationKind::object, 0x10, 0x10});
        a.allocation_returned = true;
        a.captured_allocation = allocation;
        f.cleanup_allocation = bits(allocation);
        full_state(f.eh_state, 0, a.native_eh_state);
        void* result = nullptr;
        if (allocation) {
            // Trivial array placement construction starts live4-DWORD backing
            // without initialization stores or an owning host table overlay.
            ::new (allocation) Word[4];
            a.constructor_entered = true;
            result = construct_native_procedural_factory_table_00bbc900(allocation,
                f.constructor, c, a.constructor);
            a.constructor_returned = true;
        }
        a.captured_return = result;
        publication = result;
        a.publication_written = true;
        a.completed = true;
        return result;
    } catch (...) {
        a.source_failed = true;
        if (static_cast<std::int32_t>(f.eh_state) == 0) {
            full_state(f.eh_state, -1, a.native_eh_state);
            a.cleanup_started = true;
            void* const current = pointer(f.cleanup_allocation);
            a.current_allocation_freed = current;
            singleton_lifetime_free(current);
            a.cleanup_completed = true;
        }
        throw;
    }
}
} // namespace bsp
