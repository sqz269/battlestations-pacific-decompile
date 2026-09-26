#include "bsp/native_procedural_factory_table_storage.hpp"
#include "bsp/native_procedural_factory_storage.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw procedural factory table storage requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
Word read(const void* p) noexcept {
    Word result;
    __asm {
        mov eax, p
        mov ecx, [eax]
        mov result, ecx
    }
    return result;
}
void clear(void* p) noexcept {
    __asm {
        mov eax, p
        mov dword ptr [eax], 0
    }
}
Word bits(const void* p) noexcept { return reinterpret_cast<Word>(p); }
void* pointer(Word value) noexcept { return reinterpret_cast<void*>(value); }
void* add(void* p, Word offset) noexcept { return pointer(bits(p) + offset); }

struct Binding { const void* table; Word scalar; };
Binding binding(Word profile, const NativeResourceRegistryLookupContext& context,
    NativeProceduralFactorySlotAcquired& acquired) {
    switch (profile) {
    case 0x00d64470u: return {context.actual_factory_profile_00d64470, 0x00bbc650u};
    case 0x00d644acu: return {context.actual_factory_profile_00d644ac, 0x00bbc7a0u};
    case 0x00d644e8u: return {context.actual_factory_profile_00d644e8, 0x00bbc890u};
    case 0x00d644f0u: return {context.actual_factory_profile_00d644f0, 0x00bbc8e0u};
    default:
        acquired.unsupported_binding = true;
        throw std::invalid_argument("unsupported actual procedural factory deletion profile");
    }
}

void begin_slot(void* slot, NativeProceduralFactorySlotAcquired& acquired) {
    if (acquired.started)
        throw std::logic_error("procedural factory slot acquisition is not fresh");
    acquired.started = true;
    acquired.captured_slot = slot;
    acquired.captured_owner = pointer(read(slot));
}
void finish_slot(const NativeProceduralFactorySlotFrame& frame,
    const NativeResourceRegistryLookupContext& context,
    NativeProceduralFactorySlotAcquired& acquired) {
    void* const owner = acquired.captured_owner;
    void* const captured_slot = acquired.captured_slot;
    if (owner) {
        const Word profile = read(owner);
        acquired.captured_profile = profile;
        const Binding current = binding(profile, context, acquired);
        const Word target = read(current.table);
        acquired.captured_target = target;
        if (target != current.scalar) {
            acquired.unsupported_binding = true;
            throw std::invalid_argument("unsupported current procedural factory deletion target");
        }
        frame.flags_argument = 1; // native PUSH1, AFTER current target capture
        acquired.flags_written = true;
        acquired.terminal_entered = true;
        switch (target) {
        case 0x00bbc650u: delete_native_caustics_factory_00bbc650(owner, frame.flags_argument); break;
        case 0x00bbc7a0u: delete_native_shore_wave_factory_00bbc7a0(owner, frame.flags_argument); break;
        case 0x00bbc890u: delete_native_table_caustics_factory_00bbc890(owner, frame.flags_argument); break;
        case 0x00bbc8e0u: delete_native_table_shore_wave_factory_00bbc8e0(owner, frame.flags_argument); break;
        }
        acquired.terminal_returned = true;
        clear(captured_slot); // no owner reload; overwrite a replacement too
        acquired.slot_cleared = true;
    }
    acquired.completed = true;
}
void slot(void* actual_slot, const NativeProceduralFactorySlotFrame& frame,
    const NativeResourceRegistryLookupContext& context,
    NativeProceduralFactorySlotAcquired& acquired) {
    try {
        begin_slot(actual_slot, acquired);
        finish_slot(frame, context, acquired);
    } catch (...) {
        acquired.source_failed = true;
        throw;
    }
}
} // namespace

void destroy_native_procedural_factory_slot_00bbc4e0(void* actual_slot,
    const NativeProceduralFactorySlotFrame& frame, const NativeResourceRegistryLookupContext& context,
    NativeProceduralFactorySlotAcquired& acquired) {
    slot(actual_slot, frame, context, acquired);
}
void destroy_native_procedural_factory_slot_00bbc500(void* actual_slot,
    const NativeProceduralFactorySlotFrame& frame, const NativeResourceRegistryLookupContext& context,
    NativeProceduralFactorySlotAcquired& acquired) {
    slot(actual_slot, frame, context, acquired);
}
void destroy_native_procedural_factory_table_00bbc520(void* table,
    const NativeProceduralFactoryTableFrame& frame, const NativeResourceRegistryLookupContext& context,
    NativeProceduralFactoryTableAcquired& acquired) {
    if (acquired.started)
        throw std::logic_error("procedural factory table acquisition is not fresh");
    acquired.started = true;
    acquired.captured_table = table;
    acquired.native_eh_state = -1;
    frame.cleanup_self = bits(table); // native PUSH ECX at entry S-10
    frame.cleanup_self = bits(table); // MOV [ESP+8], captured ESI
    try {
        for (unsigned i = 0; i != 4; ++i) {
            acquired.normal_index = i;
            auto& current = acquired.normal[i];
            begin_slot(add(table, (3u - i) * 4u), current); // current owner BEFORE state
            acquired.native_eh_state = 2 - static_cast<int>(i);
            try {
                finish_slot(frame.normal, context, current);
            } catch (...) {
                current.source_failed = true;
                throw;
            }
        }
        acquired.completed = true;
    } catch (...) {
        acquired.source_failed = true;
        acquired.cleanup_started = true;
        try {
            while (acquired.native_eh_state >= 0) {
                const int action = acquired.native_eh_state;
                acquired.native_eh_state = action - 1; // consume BEFORE current self/action
                acquired.cleanup_action = action;
                void* const current_self = pointer(frame.cleanup_self);
                acquired.cleanup_receiver[action] = current_self;
                void* const current_slot = add(current_self, static_cast<Word>(action) * 4u);
                if (action == 0) {
                    destroy_native_procedural_factory_slot_00bbc4e0(current_slot,
                        frame.cleanup[action], context, acquired.cleanup[action]);
                } else {
                    destroy_native_procedural_factory_slot_00bbc500(current_slot,
                        frame.cleanup[action], context, acquired.cleanup[action]);
                }
            }
            acquired.cleanup_completed = true;
        } catch (...) {
            acquired.cleanup_failed = true; // stop; no replay or invented settled state
            throw;
        }
        throw;
    }
}
void shutdown_native_procedural_factory_table_00bbc5d0(void* volatile& publication,
    const NativeProceduralFactoryTableFrame& frame, const NativeResourceRegistryLookupContext& context,
    NativeProceduralFactoryTableShutdownAcquired& acquired) {
    if (acquired.started)
        throw std::logic_error("procedural factory table shutdown acquisition is not fresh");
    acquired.started = true;
    void* const table = publication; // one CURRENT01090900 read
    acquired.captured_table = table;
    try {
        if (table) {
            acquired.table_entered = true;
            destroy_native_procedural_factory_table_00bbc520(table, frame, context, acquired.table);
            acquired.table_returned = true;
            acquired.free_entered = true;
            singleton_lifetime_free(table); // captured pointer, no publication reload/clear
            acquired.table_freed = true;
        }
        acquired.completed = true;
    } catch (...) {
        acquired.source_failed = true;
        throw;
    }
}
void* delete_native_table_caustics_factory_00bbc890(void* factory, const volatile Word& flags) {
    return delete_native_caustics_factory_00bbc650(factory, flags);
}
void* delete_native_table_shore_wave_factory_00bbc8e0(void* factory, const volatile Word& flags) {
    return delete_native_shore_wave_factory_00bbc7a0(factory, flags);
}
} // namespace bsp
